#include "pilcrow/modules/physics/systems/CollisionDetection.h"

#include <iostream>

void CollisionDetection::Init(World &world) { m_World = &world; }

void CollisionDetection::Update(float dt) {
  for(auto it{Entities.begin()}; it != Entities.end(); ++it) {
    auto            entity{*it};
    Transform &     tf{entity.Get<Transform>()};
    CircleCollider &cc{entity.Get<CircleCollider>()};
    for(auto it2{it + 1}; it2 != Entities.end(); ++it2) {
      auto            entity2{*it2};
      Transform &     tf2{entity2.Get<Transform>()};
      CircleCollider &cc2{entity2.Get<CircleCollider>()};
      if(cc.Group == cc2.Group) continue;
      glm::vec2 pos1{tf.position.x, tf.position.y};
      glm::vec2 pos2{tf2.position.x, tf2.position.y};
      if(float dist{glm::distance(pos1, pos2)};
         dist < (cc.Radius + cc2.Radius)) {
        EntityRef ER1 = entity.GetEntity();
        EntityRef ER2 = entity2.GetEntity();
        m_World->Emit(CircleCollisionEvent{&ER1, &ER2});
      }
    }
  }
}
