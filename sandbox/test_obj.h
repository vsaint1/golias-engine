#pragma once

#include "golias.h"

using namespace golias;

class TestObject : public GameObject {
public:
    TestObject();

    void Update(float deltaTime) override;

};
