#pragma once

#include "golias.h"

using namespace golias;

class TestObject : public GameObject {
    GCLASS(TestObject)
public:
    TestObject();

    void Start() override;

    void Update(float deltaTime) override;

private:
    float mRotationSpeed = 1.0f;
};
