#pragma once

#include "pilcrow/engine/core/Detectors.hpp"

#include "pilcrow/engine/core/entity/EntitiesWith.hpp"

#include "pilcrow/engine/core/components/Transform.h"


struct [[Meta::Reflectable("shmup")]] Player
{
  int mController;
};

sreflDeclareExternalType(Player);


class PlayerSystem {
public:
  void PreProcess();
  void Process(Transform& aTransform, Player const& aPlayer);
  void PostProcess();
};
