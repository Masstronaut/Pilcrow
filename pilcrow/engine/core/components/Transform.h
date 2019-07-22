#pragma once
#include "pilcrow/engine/core/ReflectedGlm.hpp"

#include "SimpleReflection/Meta.hpp"

struct [[Meta::Reflectable("engine")]] Transform{
  glm::vec3 position{0.f, 0.f, 0.f};
  
    // Euler Rotation expressed in radians
  glm::vec3 rotation{0.f, 0.f, 0.f};
  glm::vec3 scale{1.f, 1.f, 1.f};
};

sreflDeclareExternalType(Transform);
