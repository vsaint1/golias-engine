#pragma once
#include "text.h"


class ProgressBar {
public:
    
    ProgressBar();
    
    void Draw();

private:
    float progress = 0.0f;
};
