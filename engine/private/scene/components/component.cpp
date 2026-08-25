#include "scene/components/component.h"

#include "scene/game_object.h"

namespace golias {

    size_t Component::sNextComponentID = 0;

    void Component::Start() {
    }

    bool Component::LoadProperties(const Json& properties) {
        return true;
    }


    GameObject* Component::GetOwner() const {
        return mOwner;
    }

    ComponentRegistry& ComponentRegistry::GetInstance() {
        static ComponentRegistry instance;
        return instance;
    }


    Component* ComponentRegistry::CreateComponent(const std::string_view pName) const {
        auto it = creators.find(pName.data());
        if (it != creators.end()) {
            return it->second->Create();
        }

        return nullptr;
    }

    bool ComponentRegistry::HasParent(size_t objTypeId, size_t parentTypeId) const {

        auto record = parents.find(objTypeId);
        if (record == parents.end()) {

            return false;
        }

        const auto& parentList = record->second;

        if (std::find(parentList.begin(), parentList.end(), parentTypeId) != parentList.end()) {
            return true;
        }

        for (const auto& parent : parentList) {
            if (parent == objTypeId) {
                continue;
            }

            if (HasParent(parent, parentTypeId)) {
                return true;
            }
        }

        return false;
    }
} // namespace golias
