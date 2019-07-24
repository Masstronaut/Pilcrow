#include <cstdlib>
#include <glad/include/glad.h>  // MUST be included BEFORE glfw3.h

#include <GLFW/glfw3.h>

#include "pilcrow/engine/core/RenderSystem.h"
#include "pilcrow/engine/core/SettingsFileReader.hpp"
#include "pilcrow/engine/core/Simulation.hpp"
#include "pilcrow/engine/core/World.hpp"

#include "pilcrow/modules/physics/components/CircleCollider.h"

#include "pilcrow/modules/shmup/Source.hpp"

/////////////////////////////////////////////////////////////////////
// Players
/////////////////////////////////////////////////////////////////////
PlayerSystem::PlayerSystem(World &aWorld, ArchetypeRef aBulletArchetype)
  : mBulletArchetype{aBulletArchetype}, mWorld{aWorld} {
  aWorld.On([this](KeyEvent const &aKeyEvent) { OnKeyEvent(aKeyEvent); });

  aWorld.On([this](GamepadButtonEvent const &aKeyEvent) {
    OnGamepadButtonEvent(aKeyEvent);
  });

  for(auto &direction : mDirection) {
    direction = {0.0f, 0.0f, 0.0f};
  }

  for(auto &firing : mFiring) {
    firing = false;
  }

  for(auto &cooldown : mFiringCooldown) {
    cooldown = -1.f;
  }
}

void PlayerSystem::OnKeyEvent(KeyEvent const &aKeyEvent) {
  auto &direction = mDirection[0];

  if(aKeyEvent.Key == GLFW_KEY_UP) {
    direction.y += 2.f;
  } else if(aKeyEvent.Key == GLFW_KEY_DOWN) {
    direction.y -= 2.f;
  } else if(aKeyEvent.Key == GLFW_KEY_LEFT) {
    direction.x -= 2.f;
  } else if(aKeyEvent.Key == GLFW_KEY_RIGHT) {
    direction.x += 2.f;
  } else if(aKeyEvent.Key == GLFW_KEY_ENTER) {
    mFiring[0] = true;
  }
}

void PlayerSystem::OnGamepadButtonEvent(
  GamepadButtonEvent const &aButtonEvent) {
  auto &direction = mDirection[0];

  if(aButtonEvent.Button == GLFW_GAMEPAD_BUTTON_DPAD_UP) {
    direction.y += 2.f;
  } else if(aButtonEvent.Button == GLFW_GAMEPAD_BUTTON_DPAD_DOWN) {
    direction.y -= 2.f;
  } else if(aButtonEvent.Button == GLFW_GAMEPAD_BUTTON_DPAD_LEFT) {
    direction.x -= 2.f;
  } else if(aButtonEvent.Button == GLFW_GAMEPAD_BUTTON_DPAD_RIGHT) {
    direction.x += 2.f;
  } else if(aButtonEvent.Button == GLFW_GAMEPAD_BUTTON_A) {
    mFiring[0] = true;
  }
}

void PlayerSystem::PreProcess() {
  // std::cout << "preprocess\n";
}

void PlayerSystem::Update() {
  for(auto &cooldown : mFiringCooldown) {
    if(cooldown > 0.f) { cooldown -= Dt; }
  }
}

void PlayerSystem::Process(Player const &aPlayer, Transform &aTransform) const {
  aTransform.position += Dt * mDirection[aPlayer.mController];

  if(aTransform.position.x < -2.9f) {
    aTransform.position.x = -2.9f;
  } else if(aTransform.position.x > 1.5f) {
    aTransform.position.x = 1.5f;
  }

  if(aTransform.position.y < -1.5f) {
    aTransform.position.y = -1.5f;
  } else if(aTransform.position.y > 1.5f) {
    aTransform.position.y = 1.5f;
  }

  if(mFiring[aPlayer.mController]
     && (mFiringCooldown[aPlayer.mController] <= 0.f)) {
    BulletSpecification bullet;
    bullet.scale = { 1.f, 1.f, 1.f };
    bullet.scale = glm::vec3{ 0.0722623, 0.0722623, 0.0722623 } * 0.25f;

    bullet.translation = aTransform.position;
    bullet.translation.x += 1.f;

    mBulletsToSpawn.emplace_back(bullet);

    mFiringCooldown[aPlayer.mController] = 0.2f;
  }
}

void PlayerSystem::PostProcess() {
  for(auto &direction : mDirection) {
    direction = {0.0f, 0.0f, 0.0f};
  }

  for(auto &firing : mFiring) {
    firing = false;
  }

  for(auto &bulletToCreate : mBulletsToSpawn) {
    auto &bullet             = mWorld.Spawn(mBulletArchetype);
    auto &bulletTransform    = bullet.Get<Transform>();
    auto &collider           = bullet.Get<CircleCollider>();
    bulletTransform.scale    = bulletToCreate.scale;
    bulletTransform.position = bulletToCreate.translation;
    bulletTransform.rotation = {0.f, 0.f, glm::radians(-90.f)};
    collider.Group           = CollisionGroup::bullet;
    collider.Radius         *= 3;
  }

  mBulletsToSpawn.clear();
}

/////////////////////////////////////////////////////////////////////
// Bullets
/////////////////////////////////////////////////////////////////////
BulletSystem::BulletSystem() {}

void BulletSystem::Update() {
  for(auto &e : Entities) {
    auto &transform = e.Get<Transform>();
    auto &bullet    = e.Get<Bullet>();

    transform.position.x += Dt * 3.5f;
    bullet.mLife -= Dt;

    if(bullet.mLife <= 0.f) { mBulletsToRemove.emplace_back(e.GetEntity()); }
  }
}

void BulletSystem::PostProcess() {
  for(auto &bullet : mBulletsToRemove) {
    bullet.Kill();
  }

  mBulletsToRemove.clear();
}

/////////////////////////////////////////////////////////////////////
// Enemies
/////////////////////////////////////////////////////////////////////
float RandomFloatBetweenZeroAndOne() {
  return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

int RandomNumberOfEnemiesToSpawn() { return rand() % 5; }

int PositiveOrNegativeOne() {
  if(RandomFloatBetweenZeroAndOne() > 0.5f) { return 1; }

  return -1;
}

EnemySystem::EnemySystem(ArchetypeRef aEnemyArchetype, World &aWorld)
  : mEnemyArchetype{aEnemyArchetype}, mWorld{aWorld} {
  mWorld.On([this](const CircleCollisionEvent &event) { OnCollision(event); });
}

void EnemySystem::OnCollision(const CircleCollisionEvent &event) {
  if(event.Entity1->Get<CircleCollider>().Group == CollisionGroup::bullet) {
    if(event.Entity2->Get<CircleCollider>().Group == CollisionGroup::enemy) {
      mEnemiesToRemove.push_back(*event.Entity2);
      mEnemiesToRemove.push_back(*event.Entity1);

	}
  } else if(event.Entity2->Get<CircleCollider>().Group
            == CollisionGroup::bullet) {
    if(event.Entity1->Get<CircleCollider>().Group == CollisionGroup::enemy) {
      mEnemiesToRemove.push_back(*event.Entity1);
      mEnemiesToRemove.push_back(*event.Entity2);
    }
  }
}

void EnemySystem::PreProcess() {
  mSpawnCooldown -= Dt;

  if(mSpawnCooldown <= 0.f) {
    mSpawnCooldown = 1.0f;

    auto toSpawn = RandomNumberOfEnemiesToSpawn();

    for(int i = 0; i < toSpawn; ++i) {
      auto &enemy    = mWorld.Spawn(mEnemyArchetype);
      auto &model    = enemy.Get<CModel>();
      auto &collider = enemy.Get<CircleCollider>();
      auto  s        = glm::vec3{ 0.0722623f, 0.0722623f, 0.0722623f } * 0.40f;

      auto &transform      = enemy.Get<Transform>();
      transform.position.x = 4;
      transform.position.y
        = RandomFloatBetweenZeroAndOne() * 1.5f * PositiveOrNegativeOne();
      transform.scale    = s;
      transform.rotation = {0.f, 0.f, glm::radians(90.f)};
      collider.Group     = CollisionGroup::enemy;
      //collider.Radius    = s;
    }
  }
}

void EnemySystem::Update() {
  for(auto &e : Entities) {
    auto &transform = e.Get<Transform>();
    auto &enemy     = e.Get<Enemy>();

    transform.position.x -= Dt * 1.2f;
    enemy.mLife -= Dt;

    if(enemy.mLife <= 0.f) { mEnemiesToRemove.emplace_back(e.GetEntity()); }
  }
}

void EnemySystem::PostProcess() {
  for(auto &enemy : mEnemiesToRemove) {
    enemy.Kill();
  }

  mEnemiesToRemove.clear();
}
