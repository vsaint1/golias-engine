#include "test_obj.h"


TestObject::TestObject() {


    spdlog::info("TestObject created.");
}

void TestObject::Update(float deltaTime) {
    golias::GameObject::Update(deltaTime);
   

}
