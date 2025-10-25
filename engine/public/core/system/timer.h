#pragma once

#include "stdafx.h"

/*!
@file timer.h
    @brief Timer class definition.

    This file contains the definition of the Timer class, which is used to measure time intervals and manage frame timing in the engine.
    @ingroup Time
    @version 0.0.1

*/
class Timer {
public:
    double delta;
    double elapsed_time;

    int get_fps() const;

    void start();

    void tick();

private:
    Uint64 now;
    Uint64 last;
};