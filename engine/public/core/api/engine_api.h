#pragma once

#include "core/component/components.h"


/*!

    @brief Requests a scene change to the specified scene name.
    
    This function creates a `SceneChangeRequest` component in the ECS world with the provided scene name. The scene manager system will process this request and handle the actual scene transition.

    @param scene_name The name of the scene to switch to.

    @version 0.0.1

*/
void change_scene(const std::string& scene_name);


// Entity related API
flecs::entity get_entity_by_name(const std::string& name);

bool entity_has_component(flecs::entity& e, const std::string& component_name);

bool entity_add_component(flecs::entity& e, const std::string& component_name);

void entity_remove_component(flecs::entity& e, const std::string& component_name);

bool entity_is_valid(flecs::entity& e);
