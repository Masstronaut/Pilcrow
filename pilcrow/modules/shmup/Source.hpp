#pragma once

#include <array>

#include "pilcrow/engine/core/Detectors.hpp"

#include "pilcrow/engine/core/entity/EntitiesWith.hpp"
#include "pilcrow/engine/core/entity/ArchetypeRef.hpp"

#include "pilcrow/engine/core/components/Transform.h"
#include "pilcrow/engine/core/RenderSystem.h"

#include "pilcrow/modules/physics/components/CircleCollider.h"

struct [[Meta::Reflectable("shmup")]] Player
{
  int mController;
};

sreflDeclareExternalType(Player);

struct [[Meta::Reflectable("shmup")]] Bullet
{
  float mLife = 2.5f;
};

sreflDeclareExternalType(Bullet);



struct [[Meta::Reflectable("shmup")]] Enemy
{
  float mLife = 5.5f;
};

sreflDeclareExternalType(Enemy);

class PlayerSystem {
private:
  struct BulletSpecification
  {
    glm::vec3 scale;
    glm::vec3 translation;
  };

  std::array<glm::vec3, 4> mDirection;
  std::array<bool, 4> mFiring;
  std::array<float, 4> mutable mFiringCooldown;
  std::vector<BulletSpecification> mutable mBulletsToSpawn;
  ArchetypeRef mBulletArchetype;
  World& mWorld;

public:
  PlayerSystem(World& aWorld, ArchetypeRef mBulletArchetype);

  void OnKeyEvent(KeyEvent const& aKeyEvent);
  void OnGamepadButtonEvent(GamepadButtonEvent const& aKeyEvent);

  void PreProcess();
  void Update();
  void Process(Player const& aPlayer,Transform& aTransform) const;
  void PostProcess();

  float Dt;
};

class BulletSystem {
private:
  std::vector<EntityRef> mBulletsToRemove;

public:
  BulletSystem();

  void Update();
  void PostProcess();

  EntitiesWith<Bullet, Transform> Entities;
  float Dt;
};

class EnemySystem {
private:
  std::vector<EntityRef> mEnemiesToRemove;
  ArchetypeRef mEnemyArchetype;
  World& mWorld;
  float mSpawnCooldown;


public:
  EnemySystem(ArchetypeRef aEnemyArchetype, World& aWorld);

  void OnCollision(const CircleCollisionEvent &event);

  void PreProcess();
  void Update();
  void PostProcess();

  EntitiesWith<Enemy, Transform> Entities;
  float Dt;
};


