#ifndef EVENTDATA_H
#define EVENTDATA_H

#include <sifteo.h>
#include "CubeStateMachine.h"

using namespace Sifteo;

union EventData
{
    EventData() {}
    struct
    {
        char mWord[MAX_LETTERS_PER_WORD + 1];
        unsigned char mCubeOrderingIndexes[NUM_CUBES];
        unsigned char mLetterStartIndexes[NUM_CUBES];
        unsigned char mNumAnagrams;
        unsigned char mLeadingSpaces;
        unsigned char mMaxLettersPerCube;
    } mNewPuzzle;

    struct
    {
        char mWord[MAX_LETTERS_PER_WORD + 1];
        unsigned char mCubeOrderingIndexes[NUM_CUBES];
        unsigned char mLetterStartIndexes[NUM_CUBES];
        unsigned char mLeadingSpaces;
        unsigned char mMaxLettersPerCube;
    } mNewMeta;

    struct
    {
        const char* mWord;
        PCubeID mCubeIDStart;
        bool mBonus;
    } mWordFound; // used for NewWordFound and OldWordFound

    struct
    {
        PCubeID mCubeIDStart;
    } mWordBroken;

    struct
    {
        bool mFirst;
        unsigned mPreviousStateIndex;
    } mEnterState;    

    struct
    {
        unsigned mPreviousStateIndex;
        unsigned mNewStateIndex;
    } mGameStateChanged;

    struct
    {
        PCubeID mCubeID;
    } mInput; // also tilt, touch

    struct
    {
        PCubeID mCubeID;
    } mTouchAndHoldWaitForUntouch;

    struct
    {
        char mHintSolution[NUM_CUBES][MAX_LETTERS_PER_CUBE];
    } mHintSolutionUpdate;

    struct
    {
        float mDT;
    } mUpdate;
};

#endif // EVENTDATA_H
