/* -*- mode: C; c-basic-offset: 4; intent-tabs-mode: nil -*-
 *
 * Sifteo Thundercracker simulator
 * M. Elizabeth Scott <beth@sifteo.com>
 *
 * Copyright <c> 2012 Sifteo, Inc. All rights reserved.
 */

/*
 * Simulated I2C factory test device
 */

#ifndef _CUBE_TESTJIG_H
#define _CUBE_TESTJIG_H

#include <vector>
#include <list>
#include "tinythread.h"

namespace Cube {


class I2CTestJig {
 public:
    I2CTestJig() : enabled(false) {}

    void setEnabled(bool e) {
        enabled = e;
    }

    void getACK(std::vector<uint8_t> &buffer) {
        // Return the last completed ack, not the one currently in progress.
        tthread::lock_guard<tthread::mutex> guard(mutex);
        buffer = ackPrevious;
    }

    void writeVRAM(uint16_t addr, uint8_t value) {
        tthread::lock_guard<tthread::mutex> guard(mutex);
        packetBuffer.push_back(addr >> 8);
        packetBuffer.push_back(addr);
        packetBuffer.push_back(value);
    }

    void i2cStart() {
        state = enabled ? S_I2C_ADDRESS : S_IDLE;
    }
    
    void i2cStop() {
        if (state != S_IDLE) {
            tthread::lock_guard<tthread::mutex> guard(mutex);

            state = S_IDLE;

            // Double-buffer the last full ACK packet we received.
            // Only applicable if this was an ACK write packet at all.
            if (!ackBuffer.empty())
                ackPrevious = ackBuffer;
        }
    }

    uint8_t i2cWrite(uint8_t byte) {
        switch (state) {

        case S_I2C_ADDRESS: {
            tthread::lock_guard<tthread::mutex> guard(mutex);

            if ((byte & 0xFE) == deviceAddress) {
                // Begin a test packet
                state = (byte & 1) ? S_READ_PACKET : S_WRITE_ACK;
                ackBuffer.clear();
            } else {
                // Not us
                state = S_IDLE;
            }
            break;
        }
            
        case S_WRITE_ACK: {
            tthread::lock_guard<tthread::mutex> guard(mutex);
            ackBuffer.push_back(byte);
            break;
        }

        default:
            break;
        }

        return state != S_IDLE;
    }

    uint8_t i2cRead(uint8_t ack) {
        uint8_t result = 0xff;

        switch (state) {

        case S_READ_PACKET: {
            tthread::lock_guard<tthread::mutex> guard(mutex);

            // NB: If empty, return a sentinel packet of [ff].
            if (!packetBuffer.empty()) {
                result = packetBuffer.front();
                packetBuffer.pop_front();
            }
            break;
        }

        default:
            break;
        }

        return result;
    }

 private:
    static const uint8_t deviceAddress = 0xAA;

    bool enabled;
    std::vector<uint8_t> ackBuffer;
    std::vector<uint8_t> ackPrevious;
    std::list<uint8_t> packetBuffer;
    tthread::mutex mutex;

    enum {
        S_IDLE,
        S_I2C_ADDRESS,
        S_WRITE_ACK,
        S_READ_PACKET,
    } state;
};


};  // namespace Cube

#endif