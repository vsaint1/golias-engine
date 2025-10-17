#include "core/component/logic/system_helper.h"


const aiNodeAnim* find_node_anim(const aiAnimation* animation, const std::string& nodeName) {
    for (unsigned int i = 0; i < animation->mNumChannels; i++) {
        const aiNodeAnim* nodeAnim = animation->mChannels[i];
        if (std::string(nodeAnim->mNodeName.C_Str()) == nodeName) {
            return nodeAnim;
        }
    }
    return nullptr;
}

glm::vec3 interpolate_pos(float animTime, const aiNodeAnim* nodeAnim) {
    // Single keyframe - no interpolation needed
    if (nodeAnim->mNumPositionKeys == 1) {
        const aiVector3D& v = nodeAnim->mPositionKeys[0].mValue;
        return glm::vec3(v.x, v.y, v.z);
    }

    unsigned int positionIndex = 0;
    for (unsigned int i = 0; i < nodeAnim->mNumPositionKeys - 1; i++) {
        if (animTime < (float) nodeAnim->mPositionKeys[i + 1].mTime) {
            positionIndex = i;
            break;
        }
    }

    unsigned int nextIndex = positionIndex + 1;
    float deltaTime        = (float) (nodeAnim->mPositionKeys[nextIndex].mTime - nodeAnim->mPositionKeys[positionIndex].mTime);
    
    if (deltaTime < 0.0001f) {
        const aiVector3D& v = nodeAnim->mPositionKeys[positionIndex].mValue;
        return glm::vec3(v.x, v.y, v.z);
    }
    
    float factor = (animTime - (float) nodeAnim->mPositionKeys[positionIndex].mTime) / deltaTime;

    const aiVector3D& start = nodeAnim->mPositionKeys[positionIndex].mValue;
    const aiVector3D& end   = nodeAnim->mPositionKeys[nextIndex].mValue;

    return glm::mix(glm::vec3(start.x, start.y, start.z), 
                    glm::vec3(end.x, end.y, end.z), factor);
}

glm::quat interpolate_rot(float animTime, const aiNodeAnim* nodeAnim) {
    // Single keyframe - no interpolation needed
    if (nodeAnim->mNumRotationKeys == 1) {
        const aiQuaternion& q = nodeAnim->mRotationKeys[0].mValue;
        return glm::quat(q.w, q.x, q.y, q.z);
    }

    unsigned int rotationIndex = 0;
    for (unsigned int i = 0; i < nodeAnim->mNumRotationKeys - 1; i++) {
        if (animTime < (float) nodeAnim->mRotationKeys[i + 1].mTime) {
            rotationIndex = i;
            break;
        }
    }

    unsigned int nextIndex = rotationIndex + 1;
    float deltaTime        = (float) (nodeAnim->mRotationKeys[nextIndex].mTime - nodeAnim->mRotationKeys[rotationIndex].mTime);
    
  
    if (deltaTime < 0.0001f) {
        const aiQuaternion& q = nodeAnim->mRotationKeys[rotationIndex].mValue;
        return glm::quat(q.w, q.x, q.y, q.z);
    }
    
    float factor = (animTime - (float) nodeAnim->mRotationKeys[rotationIndex].mTime) / deltaTime;

    const aiQuaternion& startQuat = nodeAnim->mRotationKeys[rotationIndex].mValue;
    const aiQuaternion& endQuat   = nodeAnim->mRotationKeys[nextIndex].mValue;

    return glm::slerp(glm::quat(startQuat.w, startQuat.x, startQuat.y, startQuat.z),
                      glm::quat(endQuat.w, endQuat.x, endQuat.y, endQuat.z), factor);
}

glm::vec3 interpolate_scale(float animTime, const aiNodeAnim* nodeAnim) {
    // Single keyframe - no interpolation needed
    if (nodeAnim->mNumScalingKeys == 1) {
        const aiVector3D& v = nodeAnim->mScalingKeys[0].mValue;
        return glm::vec3(v.x, v.y, v.z);
    }

    unsigned int scaleIndex = 0;
    for (unsigned int i = 0; i < nodeAnim->mNumScalingKeys - 1; i++) {
        if (animTime < (float) nodeAnim->mScalingKeys[i + 1].mTime) {
            scaleIndex = i;
            break;
        }
    }

    unsigned int nextIndex = scaleIndex + 1;
    float deltaTime        = (float) (nodeAnim->mScalingKeys[nextIndex].mTime - nodeAnim->mScalingKeys[scaleIndex].mTime);
    
    if (deltaTime < 0.0001f) {
        const aiVector3D& v = nodeAnim->mScalingKeys[scaleIndex].mValue;
        return glm::vec3(v.x, v.y, v.z);
    }
    
    float factor = (animTime - (float) nodeAnim->mScalingKeys[scaleIndex].mTime) / deltaTime;

    const aiVector3D& start = nodeAnim->mScalingKeys[scaleIndex].mValue;
    const aiVector3D& end   = nodeAnim->mScalingKeys[nextIndex].mValue;

    return glm::mix(glm::vec3(start.x, start.y, start.z), 
                    glm::vec3(end.x, end.y, end.z), factor);
}

void read_node_hierarchy(float animTime, const aiNode* node, const glm::mat4& parentTransform, const aiAnimation* animation,
                         const Model& model, std::unordered_map<std::string, glm::mat4>& bone_map) {

    std::string nodeName(node->mName.C_Str());
    const aiMatrix4x4& nodeTransformAI = node->mTransformation;
    
    glm::mat4 nodeTransform;
    
    const aiNodeAnim* nodeAnim = find_node_anim(animation, nodeName);

    if (nodeAnim) {
        glm::vec3 position = interpolate_pos(animTime, nodeAnim);
        glm::quat rotation = interpolate_rot(animTime, nodeAnim);
        glm::vec3 scale    = interpolate_scale(animTime, nodeAnim);

        glm::mat4 rotMat = glm::mat4_cast(rotation);
        
        nodeTransform[0] = rotMat[0] * scale.x;
        nodeTransform[1] = rotMat[1] * scale.y;
        nodeTransform[2] = rotMat[2] * scale.z;
        nodeTransform[3] = glm::vec4(position, 1.0f);
    } else {
       
        nodeTransform = glm::transpose(glm::make_mat4(&nodeTransformAI.a1));
    }

    glm::mat4 globalTransform = parentTransform * nodeTransform;
    bone_map[nodeName] = globalTransform;

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        read_node_hierarchy(animTime, node->mChildren[i], globalTransform, animation, model, bone_map);
    }
}


int sort_by_z_index(flecs::entity_t e1, const Transform2D* t1, flecs::entity_t e2, const Transform2D* t2) {
    (void) e1;
    (void) e2;
    return t1->z_index - t2->z_index;
}


