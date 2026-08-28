#include "bullet.h"

#include "player.h"

void Bullet::Update(float deltaTime) {
    GameObject::Update(deltaTime);

    mLifetime -= deltaTime;
    if (mLifetime <= 0.0f) {
        Destroy();
    }
}


void Bullet::OnCollisionEnter(const Collision& collision) {

    if (collision.Object->GetName() == "PlayerObject") {
        Player* player = static_cast<Player*>(collision.Object);
        GOLIAS_LOG_INFO("Entered Object %s |  Position (%.2f,%.2f,%.2f)",
                        "idk",
                        player->GetWorldPosition().x,
                        player->GetWorldPosition().y,
                        player->GetWorldPosition().z);
    }
}

void Bullet::OnCollisionExit(const Collision& collision) {

    // GOLIAS_LOG_INFO("Exited Object %s |  Position (%.2f,%.2f,%.2f)",
    //                 collision.Object->GetName().c_str(),
    //                 collision.Position.x,
    //                 collision.Position.y,
    //                 collision.Position.z);
}
