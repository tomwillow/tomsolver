#include <iostream>

#include "windows.h"
#define _CRTDBG_MAP_ALLOC // to get more details
#include <stdlib.h>
#include <crtdbg.h> //for malloc and free

#include <cassert>

class MemoryLeakDetection final {
public:
    MemoryLeakDetection() {
        _CrtMemCheckpoint(&sOld); // take a snapshot
    }

    ~MemoryLeakDetection() {
        _CrtMemCheckpoint(&sNew);                    // take a snapshot
        if (_CrtMemDifference(&sDiff, &sOld, &sNew)) // if there is a difference
        {
            // OutputDebugString(TEXT("-----------_CrtMemDumpStatistics ---------"));
            //_CrtMemDumpStatistics(&sDiff);
            // OutputDebugString(TEXT("-----------_CrtMemDumpAllObjectsSince ---------"));
            //_CrtMemDumpAllObjectsSince(&sOld);
            // OutputDebugString(TEXT("-----------_CrtDumpMemoryLeaks ---------"));
            _CrtDumpMemoryLeaks();

            assert(0 && "Memory leak is detected! See debug output for detail.");
        }
    }

    void SetBreakAlloc(long index) const noexcept
    {
        _CrtSetBreakAlloc(index);
    }

private:
    _CrtMemState sOld;
    _CrtMemState sNew;
    _CrtMemState sDiff;
};