/* -*- mode: C; c-basic-offset: 4; intent-tabs-mode: nil -*-
 *
 * Copyright <c> 2012 Sifteo, Inc. All rights reserved.
 */

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "CubeWrapper.h"
#include <sifteo.h>
#include "Config.h"
#include "assets.gen.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace Buddies {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

const Vec2 kPartPositions[NUM_SIDES] =
{
    Vec2(32, -8),
    Vec2(-8, 32),
    Vec2(32, 72),
    Vec2(72, 32),
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

const Sifteo::AssetImage &getBgAsset(int buddyId)
{
    switch (buddyId)
    {
        default:
        case 0: return bg1;
        case 1: return bg2;
        case 2: return bg3;
        case 3: return bg4;
        case 4: return bg5;
        case 5: return bg6;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

const Sifteo::PinnedAssetImage &getPartsAsset(int buddyId)
{
    switch (buddyId)
    {
        default:
        case 0: return parts1;
        case 1: return parts2;
        case 2: return parts3;
        case 3: return parts4;
        case 4: return parts5;
        case 5: return parts6;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

CubeWrapper::CubeWrapper()
    : mCube()
    , mEnabled(false)
    , mBuddyId(0)
    , mPieces()
    , mPiecesSolution()
    , mPieceOffsets()
    , mMode(BUDDY_MODE_NORMAL)
    , mTouching(false)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void CubeWrapper::Reset()
{
    mPiecesSolution[0].mBuddy = mBuddyId;
    mPiecesSolution[0].mPart = 0;
    mPiecesSolution[0].mRotation = 0;
    
    mPiecesSolution[1].mBuddy = mBuddyId;
    mPiecesSolution[1].mPart = 1;
    mPiecesSolution[1].mRotation = 0;
    
    mPiecesSolution[2].mBuddy = mBuddyId;
    mPiecesSolution[2].mPart = 2;
    mPiecesSolution[2].mRotation = 0;
    
    mPiecesSolution[3].mBuddy = mBuddyId;
    mPiecesSolution[3].mPart = 3;
    mPiecesSolution[3].mRotation = 0;
    
    for (unsigned int i = 0; i < arraysize(mPiecesSolution); ++i)
    {
        mPieces[i] = mPiecesSolution[i];
    }
    
    for (unsigned int i = 0; i < arraysize(mPieceOffsets); ++i)
    {
        mPieceOffsets[i] = 0;
    }
    
    mMode = BUDDY_MODE_NORMAL;
    mTouching = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void CubeWrapper::Update()
{
    ASSERT(IsEnabled());
    
    if (mTouching)
    {
        if (!mCube.touching())
        {
            OnButtonRelease();
            mTouching = false;
        }
    }
    else
    {
        if (mCube.touching())
        {
            OnButtonPress();
            mTouching = true;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void CubeWrapper::Draw(ShuffleState shuffleState, float shuffleScoreTime)
{
    ASSERT(IsEnabled());
    
    Video().clear();
    Video().BG0_drawAsset(Vec2(0, 0), getBgAsset(mBuddyId));
    
    if (mMode == BUDDY_MODE_HINT)
    {
        PaintFacePart(mPiecesSolution[SIDE_TOP], SIDE_TOP);
        PaintFacePart(mPiecesSolution[SIDE_LEFT], SIDE_LEFT);
        PaintFacePart(mPiecesSolution[SIDE_BOTTOM], SIDE_BOTTOM);
        PaintFacePart(mPiecesSolution[SIDE_RIGHT], SIDE_RIGHT);
    }
    else
    {
        PaintFacePart(mPieces[SIDE_TOP], SIDE_TOP);
        PaintFacePart(mPieces[SIDE_LEFT], SIDE_LEFT);
        PaintFacePart(mPieces[SIDE_BOTTOM], SIDE_BOTTOM);
        PaintFacePart(mPieces[SIDE_RIGHT], SIDE_RIGHT);
    }
    
    if (kShuffleMode)
    {
        switch (shuffleState)
        {
            case SHUFFLE_STATE_SHAKE_TO_SCRAMBLE:
            {
                PaintBanner(BannerShakeToScramble);
                break;
            }
            case SHUFFLE_STATE_UNSCRAMBLE_THE_FACES:
            {
                PaintBanner(BannerUnscrambleTheFaces);
                break;
            }
            case SHUFFLE_STATE_PLAY:
            {
                if (IsSolved())
                {
                    PaintBanner(BannerFaceComplete);
                }
                break;
            }
            case SHUFFLE_STATE_SCORE:
            {
                if (mCube.id() == 0)
                {
                    int minutes = int(shuffleScoreTime) / 60;
                    int seconds = int(shuffleScoreTime - (minutes * 60.0f));
                    
                    PaintBannerScore(BannerYourTime, minutes, seconds);
                }
                else
                {
                    PaintBanner(BannerShakeToScramble);
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }
    else
    {
        // TODO: Why do I need to do this?
        BG1Helper bg1helper(mCube);
        bg1helper.Flush();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

bool CubeWrapper::IsSolved() const
{
    ASSERT(IsEnabled());
    
    for (unsigned int i = 0; i < arraysize(mPiecesSolution); ++i)
    {
        if (mPieces[i].mBuddy != mPiecesSolution[i].mBuddy ||
            mPieces[i].mPart != mPiecesSolution[i].mPart ||
            mPieces[i].mRotation != mPiecesSolution[i].mRotation)
        {
            return false;
        }
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void CubeWrapper::InitVideoRom()
{
    ASSERT(IsEnabled());
    
    VidMode_BG0_ROM rom(mCube.vbuf);
    rom.init();
    rom.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void CubeWrapper::PaintProgressBar()
{
    ASSERT(IsEnabled());
    
    VidMode_BG0_ROM rom(mCube.vbuf);
    rom.BG0_progressBar(Vec2(0, 7), mCube.assetProgress(GameAssets, VidMode_BG0::LCD_width), 2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

bool CubeWrapper::IsDoneLoading()
{
    ASSERT(IsEnabled());
    
    return mCube.assetDone(GameAssets);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

bool CubeWrapper::IsEnabled() const
{
    return mEnabled;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void CubeWrapper::Enable(Cube::ID cubeId, unsigned int buddyId)
{
    ASSERT(!IsEnabled());
    
    mCube.enable(cubeId);
    mCube.loadAssets(GameAssets);
            
    mEnabled = true;
    mBuddyId = buddyId;
    
    Reset();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void CubeWrapper::Disable()
{
    ASSERT(IsEnabled());
    
    mEnabled = false;
    mCube.disable();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int CubeWrapper::GetBuddyId() const
{
    return mBuddyId;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

const Piece &CubeWrapper::GetPiece(unsigned int side) const
{
    ASSERT(side < arraysize(mPieces));
    
    return mPieces[side];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void CubeWrapper::SetPiece(unsigned int side, const Piece &piece)
{
    ASSERT(side < arraysize(mPieces));
    
    mPieces[side] = piece;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void CubeWrapper::SetPieceOffset(unsigned int side, int offset)
{
    ASSERT(side < arraysize(mPieceOffsets));
    
    mPieceOffsets[side] = offset;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

BuddyMode CubeWrapper::GetMode() const
{
    return mMode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

bool CubeWrapper::IsTouching() const
{
    return mTouching;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void CubeWrapper::PaintBanner(const Sifteo::AssetImage &asset)
{
    BG1Helper bg1helper(mCube);
    bg1helper.DrawAsset(Vec2(0, 0), asset);
    bg1helper.Flush();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void CubeWrapper::PaintBannerScore(const Sifteo::AssetImage &asset, int minutes, int seconds)
{
    int x = 10;
    
    BG1Helper bg1helper(mCube);
    bg1helper.DrawAsset(Vec2(0, 0), asset); // Banner Background
    bg1helper.DrawAsset(Vec2(x++, 1), FontScore, minutes / 10); // Mintues (10s)
    bg1helper.DrawAsset(Vec2(x++, 1), FontScore, minutes % 10); // Minutes ( 1s)
    bg1helper.DrawAsset(Vec2(x++, 1), FontScore, 10); // ":"
    bg1helper.DrawAsset(Vec2(x++, 1), FontScore, seconds / 10); // Seconds (10s)
    bg1helper.DrawAsset(Vec2(x++, 1), FontScore, seconds % 10); // Seconds ( 1s)
    bg1helper.Flush();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

Sifteo::VidMode_BG0_SPR_BG1 CubeWrapper::Video()
{
    return VidMode_BG0_SPR_BG1(mCube.vbuf);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void CubeWrapper::PaintFacePart(const Piece &piece, unsigned int side)
{
    ASSERT(piece.mPart >= 0 && piece.mPart < NUM_SIDES);
    ASSERT(piece.mRotation >= 0 && piece.mRotation < 4);
    
    const Sifteo::PinnedAssetImage &asset = getPartsAsset(piece.mBuddy);
    unsigned int frame = (piece.mRotation * NUM_SIDES) + piece.mPart;
    
    ASSERT(frame < asset.frames);
    Video().setSpriteImage(side, asset, frame);
    
    Vec2 point = kPartPositions[side];
    
    switch(side)
    {
        case SIDE_TOP:
        {
            point.y += mPieceOffsets[side];
            break;
        }
        case SIDE_LEFT:
        {
            point.x += mPieceOffsets[side];
            break;
        }
        case SIDE_BOTTOM:
        {
            point.y -= mPieceOffsets[side];
            break;
        }
        case SIDE_RIGHT:
        {
            point.x -= mPieceOffsets[side];
            break;
        }
    }
    
    Video().moveSprite(side, point);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void CubeWrapper::OnButtonPress()
{
    mMode = BUDDY_MODE_HINT;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void CubeWrapper::OnButtonRelease()
{
    mMode = BUDDY_MODE_NORMAL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

}
