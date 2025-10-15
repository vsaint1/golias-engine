#pragma once

#include "core/api/engine_api.h"
#include "core/component/components.h"
#include "core/project_config.h"



void push_sdl_event_to_lua(lua_State* L, const SDL_Event& event);

// =======================================================
// Generate Lua bindings for an entity
// =======================================================
void generate_bindings(lua_State* L);

void push_entity_to_lua(lua_State* L, flecs::entity e);