#pragma once
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SimpleReflection/Meta.hpp"

sreflDeclareExternalType(glm::mat2);
sreflDeclareExternalType(glm::mat3);
sreflDeclareExternalType(glm::mat4);
sreflDeclareExternalType(glm::mat4x3);

sreflDeclareExternalType(glm::vec2);
sreflDeclareExternalType(glm::vec3);
sreflDeclareExternalType(glm::vec4);

sreflDeclareExternalType(glm::quat);

sreflDeclareExternalType(glm::ivec2);
sreflDeclareExternalType(glm::ivec3);
sreflDeclareExternalType(glm::ivec4);

namespace GlmReflection
{
  void InitializeReflection();
} // namespace GlmReflection
