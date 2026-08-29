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
        
        if (player) {
            player->TakeDamage(10);
        }

    }

    if (mStaticMesh) {
        mStaticMesh->GetMaterial()->SetParameter("_BaseColor", glm::vec4(1.0, 0.0, 0.0, 1.0));
    }
}

void HurtPlatform::OnCollisionExit(const Collision& collision) {
    
    if (mStaticMesh) {
        mStaticMesh->GetMaterial()->SetParameter("_BaseColor", glm::vec4(1.0));
    }
}
