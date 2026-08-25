#include "test_obj.h"


TestObject::TestObject() {
}

void TestObject::Start() {
    Ref<Material> material = Engine::GetInstance().GetAssetManager().Load<Material>("materials/brick.gmat");

    Ref<Mesh> mesh = Mesh::CreateCube();

    StaticMeshComponent* meshComponent = new StaticMeshComponent(mesh, material);


    AddComponent(meshComponent);
}

void TestObject::Update(float deltaTime) {
    GameObject::Update(deltaTime);

    RotateLocal(glm::vec3(0.0f, 1.0f, 0.5f), 0.01f);
}
