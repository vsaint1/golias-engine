#pragma once

#include "core/component/components.h"


/*!

    @brief Requests a scene change to the specified scene name.
    
    This function creates a `SceneChangeRequest` component in the ECS world with the provided scene name. The scene manager system will process this request and handle the actual scene transition.

    @param scene_name The name of the scene to switch to.

    @version 0.0.1

*/
void change_scene(const std::string& scene_name);