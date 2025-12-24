#pragma once
#include "core/engine.h"


class TestObject : public golias::GameObject {
public:
    TestObject();
    ~TestObject() = default;

    void Update(float deltaTime) override;

private:
    golias::Material material;
    std::shared_ptr<golias::Mesh> mesh;
};
