#pragma once

#include "stdafx.h"


class Timer {
public:
    double delta;
    double elapsed_time;

    void start();

    void tick();

private:
    Uint64 now;
    Uint64 last;
};
