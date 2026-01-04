#include "bullet.h"

Bullet::Bullet() {
}

void Bullet::Start() {
}

void Bullet::Update(float deltaTime) {
    golias::GameObject::Update(deltaTime);

    lifeTime -= deltaTime;

    if (lifeTime <= 0.0f) {
        Destroy();
    }
}



