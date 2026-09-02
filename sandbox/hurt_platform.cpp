#include "hurt_platform.h"

#include "player.h"

void HurtPlatform::Start() {

    mStaticMesh = GetComponent<StaticMeshComponent>();
}

void HurtPlatform::Update(float deltaTime) {
    GameObject::Update(deltaTime);
}

void HurtPlatform::OnCollisionEnter(const Collision& collision) {

    if (collision.Object->GetName() == "PlayerObject") {
        Player* player = Cast<Player>(collision.Object);

        if (player && player->GetHealth() > 0) {
            player->Knockback(glm::vec3(0.0f, 1.0f, 0.8f), 7.0f);
            player->TakeDamage(10);
        }
    }

    if (mStaticMesh) {
        mStaticMesh->GetMaterial()->SetParameterValue("_BaseColor", glm::vec4(1.0));
    }
}

void HurtPlatform::OnCollisionExit(const Collision& collision) {

    if (mStaticMesh) {
        mStaticMesh->GetMaterial()->SetParameterValue("_BaseColor", glm::vec4(1.0, 0.0, 0.0, 1.0));
    }
}
