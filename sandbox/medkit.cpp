#include "medkit.h"

#include "player.h"

void Medkit::Start() {

    if (AudioSourceComponent* audio = GetComponent<AudioSourceComponent>()) {
        mAudioSource = audio;
    }

    mPhysicsComponent = GetComponent<PhysicsComponent>();

    if (GameObject* visual = FindChildByName("Medkit")) {
        mMeshComponent = visual->GetComponent<StaticMeshComponent>();
    }

    ToggleVisibility(true);
}

void Medkit::Update(float deltaTime) {
    GameObject::Update(deltaTime);

    if (mRespawnTimer > 0.0f) {
        mRespawnTimer -= deltaTime;

        if (mRespawnTimer <= 0.0f) {
            mRespawnTimer = 0.0f;

            mIsAvailable = true;

            ToggleVisibility(true);
        }
    }
}

void Medkit::ToggleVisibility(bool visible) {
    if (mMeshComponent) {
        mMeshComponent->SetVisible(visible);
    }

    if (mPhysicsComponent) {

        if (RigidBody* body = mPhysicsComponent->GetRigidBody()) {
            body->SetEnabled(visible);
        }
    }
}

void Medkit::OnCollisionEnter(const Collision& collision) {

    if (collision.Object->GetName() == "PlayerObject") {
        Player* player = Cast<Player>(collision.Object);

        if (!mIsAvailable) {
            return;
        }

        if (player) {
            player->Heal(mHealAmount);
        }

        if (mAudioSource) {
            mAudioSource->Play("medkit_pickup");
        }

        mIsAvailable  = false;
        mRespawnTimer = mRespawnTime;

        ToggleVisibility(false);
    }
}

void Medkit::OnCollisionExit(const Collision& collision) {
}
