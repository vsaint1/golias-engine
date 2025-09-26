#pragma once

#include "core/api/engine_api.h"
#include "core/component/components.h"
#include "core/project_config.h"

// =======================================================
// Generate Lua bindings for an entity
// =======================================================
void generate_bindings(sol::state_view lua);

void push_entity_to_lua(sol::state_view lua, flecs::entity e);