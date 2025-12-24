#pragma once
#include "core/engine.h"


class TestObject : public golias::GameObject {
public:
    TestObject();
    ~TestObject() = default;

    void Update(float deltaTime) override;

};
