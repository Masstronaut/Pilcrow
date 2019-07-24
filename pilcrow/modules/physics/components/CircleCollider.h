#pragma once

#include <SimpleReflection/Meta.hpp>

enum class CollisionGroup : unsigned {
	none = 0,
	bullet = 1,
	enemy = 2,
	player = 3
};

sreflDeclareExternalType(CollisionGroup);

struct [[Meta::Reflectable("shmup")]] CircleCollider {
  float    Radius{1.f};
  CollisionGroup Group{CollisionGroup::none};
};
sreflDeclareExternalType(CircleCollider);

class EntityRef;
struct CircleCollisionEvent {
  const EntityRef *Entity1;
  const EntityRef *Entity2;
};