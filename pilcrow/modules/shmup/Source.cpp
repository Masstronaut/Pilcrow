#include <glad/include/glad.h>  // MUST be included BEFORE glfw3.h

#include <GLFW/glfw3.h>

#include "pilcrow/modules/shmup/Source.hpp"


void PlayerSystem::PreProcess()
{
  std::cout << "preprocess\n";
}

void PlayerSystem::PostProcess()
{
  std::cout << "postprocess\n";
}

void PlayerSystem::Process(Transform& aTransform, Player const& aPlayer)
{
  std::cout << "process\n";
}
