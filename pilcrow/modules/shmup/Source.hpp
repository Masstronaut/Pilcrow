#pragma once

#include "pilcrow/engine/core/Detectors.hpp"

#include "pilcrow/engine/core/entity/EntitiesWith.hpp"

#include "pilcrow/engine/core/components/Transform.h"
#include "pilcrow/engine/core/RenderSystem.h"


struct [[Meta::Reflectable("shmup")]] Player
{
  int mController;
};

sreflDeclareExternalType(Player);

class PlayerSystem {
private:

public:
  PlayerSystem(World& aWorld);

  void OnKeyEvent(KeyEvent const& aKeyEvent);

  void PreProcess();
  void Process(Player const& aPlayer,Transform& aTransform);
  void PostProcess();
};
