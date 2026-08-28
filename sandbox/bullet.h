#pragma once

#include "golias.h"

using namespace golias;

class Bullet : public GameObject {
    GCLASS(Bullet);

public:
    void Update(float deltaTime);

    void OnCollisionEnter(const Collision& collision);
    void OnCollisionExit(const Collision& collision);

private:
    float mLifetime = 5.0f;
};
