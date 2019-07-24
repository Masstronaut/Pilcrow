#pragma once

#include "pilcrow/engine/core/entity/EntitiesWith.hpp"

#include "pilcrow/engine/core/components/Transform.h"
#include "pilcrow/modules/physics/components/CircleCollider.h"

#include "pilcrow/modules/physics/components/Body.h"

class CollisionDetection {
public:
  EntitiesWith<Transform, CircleCollider> Entities;
  void                                    Init(World &world);
  void Update(float dt);

private:
  World *m_World{nullptr};
};