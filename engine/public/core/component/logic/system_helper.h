#pragma once
#include "core/component/components.h"


// =======================================================
// SOME FUNCTIONS HELPER FOR MANY SYSTEMS                |
// =======================================================

const aiNodeAnim* find_node_anim(const aiAnimation* animation, const std::string& nodeName);

glm::vec3 interpolate_pos(float animTime, const aiNodeAnim* nodeAnim);

glm::quat interpolate_rot(float animTime, const aiNodeAnim* nodeAnim);

glm::vec3 interpolate_scale(float animTime, const aiNodeAnim* nodeAnim);

void read_node_hierarchy(float animTime, const aiNode* node, const glm::mat4& parentTransform, const aiAnimation* animation,
                         const Model& model, std::unordered_map<std::string, glm::mat4>& bone_map);

int sort_by_z_index(flecs::entity_t e1, const Transform2D* t1, flecs::entity_t e2, const Transform2D* t2);