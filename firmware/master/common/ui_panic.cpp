/*
 * Thundercracker Firmware -- Confidential, not for redistribution.
 * Copyright <c> 2012 Sifteo, Inc. All rights reserved.
 */

#include "ui_panic.h"
#include "vram.h"
#include "svmmemory.h"
#include "cubeslots.h"
#include "cube.h"
#include "systime.h"
#include "led.h"
#include "tasks.h"
#include "homebutton.h"


void UIPanic::init(SvmMemory::VirtAddr vbufVA)
{
    // Set up panic LED state very early on, in case some other part of the UIPanic
    // hangs or otherwise fails to get the message across!

    LED::set(LEDPatterns::panic, true);

    // Initialize, with vbuf memory stolen from userspace at the specified address.

    SvmMemory::PhysAddr vbufPA;
    bool success = SvmMemory::mapRAM(vbufVA, sizeof(_SYSAttachedVideoBuffer), vbufPA);
    avb = success ? reinterpret_cast<_SYSAttachedVideoBuffer*>(vbufPA) : 0;
    ASSERT(avb != 0);
    erase();
}

void UIPanic::erase()
{
    memset(avb, 0, sizeof *avb);
    avb->vbuf.vram.num_lines = 128;
    avb->vbuf.vram.mode = _SYS_VM_BG0_ROM;
}

void UIPanic::paint(_SYSCubeID cube)
{
    /*
     * _Synchronously_ paint this cube.
     * We'll connect and enable it if needed.
     *
     * This is really heavy-handed: All of VRAM is redrawn, we
     * use continuous rendering mode (to avoid needing to know the
     * previous toggle state) and we synchonously wait for everything
     * to finish.
     *
     * The video buffer is only attached during this call.
     *
     * We try not to invoke any Task handlers, as would be the case
     * if we used higher-level paint primitives.
     *
     * If the paint can't be completed in a fixed amount of time,
     * we abort. This ensures that paint() returns even if the
     * indicated cube is no longer reachable.
     */

    dumpScreenToUART();

    SysTime::Ticks deadline = SysTime::ticks() + SysTime::sTicks(1);

    CubeSlot &slot = CubeSlots::instances[cube];
    avb->vbuf.vram.flags = _SYS_VF_CONTINUOUS;

    VRAM::init(avb->vbuf);
    VRAM::unlock(avb->vbuf);
    slot.setVideoBuffer(&avb->vbuf);

    // Wait for the radio transmission to finish
    while (avb->vbuf.cm16 != 0 && SysTime::ticks() < deadline)
        Tasks::idle();

    // Wait for the cube to draw, twice. It may have been partway
    // through a frame when the radio transmission finished.
    for (unsigned i = 0; i < 2; i++) {
        uint8_t baseline = slot.getLastFrameACK();
        while (slot.getLastFrameACK() == baseline && SysTime::ticks() < deadline)
            Tasks::idle();
    }

    // Turn off rendering
    VRAM::pokeb(avb->vbuf, _SYS_VA_FLAGS, 0);
    VRAM::unlock(avb->vbuf);

    // Wait for the radio transmission to finish
    while (avb->vbuf.cm16 != 0&& SysTime::ticks() < deadline)
        Tasks::idle();

    // Stop using this VideoBuffer
    slot.setVideoBuffer(NULL);
}

UIPanic &UIPanic::operator<< (char c)
{
    unsigned index = c - ' ';
    avb->vbuf.vram.bg0_tiles[addr++] = _SYS_TILE77(index);
    return *this;
}

UIPanic &UIPanic::operator<< (const char *str)
{
    while (char c = *(str++))
        *this << c;
    return *this;
}

UIPanic &UIPanic::operator<< (uint8_t byte)
{
    const char *digits = "0123456789ABCDEF";
    *this << digits[byte >> 4] << digits[byte & 0xf];
    return *this;
}

UIPanic &UIPanic::operator<< (uint32_t word)
{
    *this << uint8_t(word >> 24) << uint8_t(word >> 16)
          << uint8_t(word >> 8) << uint8_t(word);
    return *this;
}

void UIPanic::dumpScreenToUART()
{
    //      0123456789ABCDEF
    UART(("+---- PANIC! ----+\r\n"));

    unsigned addr = 0;
    for (unsigned y = 0; y != 16; y++) {
        UART(("|"));
        for (unsigned x = 0; x != 16; x++) {

            // Read back from BG0
            unsigned index = avb->vbuf.vram.bg0_tiles[addr++];
            index = _SYS_INVERSE_TILE77(index);

            // Any non-text tiles are drawn as '.'
            if (index <= unsigned('~' - ' '))
                index += ' ';
            else
                index = '.';

            // Interpret 32-bit word as char plus NUL terminator
            UART((reinterpret_cast<char*>(&index)));
        }
        UART(("|\r\n"));
        addr += 2;
    }

    //      0123456789ABCDEF
    UART(("+----------------+\r\n"));
}

void UIPanic::haltForever()
{
    LOG(("PANIC: System halted!\n"));
    UART(("PANIC: System halted!\r\n"));

    while (1)
        Tasks::idle();
}

void UIPanic::haltUntilButton()
{
    LOG(("PANIC: Waiting for button press\n"));
    UART(("PANIC: Waiting for button press\r\n"));

    // Wait for press
    while (!HomeButton::isPressed())
        Tasks::idle();

    // Wait for release
    while (HomeButton::isPressed())
        Tasks::idle();
}

void UIPanic::paintAndWait()
{
    _SYSCubeIDVector cubes = CubeSlots::sysConnected;
    if (cubes) {
        // Paint on first enabled cube
        paint(Intrinsic::CLZ(cubes));

        // Don't wait on simulation (Fail fast, don't hang the unit tests).
        // But on hardware, wait for a button press so there's time to read
        // the message.

        #ifndef SIFTEO_SIMULATOR
        haltUntilButton();
        #endif

    } else {
        // No cubes. Dump to UART only, and continue immediately.
        dumpScreenToUART();
    }
}
