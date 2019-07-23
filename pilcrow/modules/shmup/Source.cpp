#include <glad/include/glad.h>  // MUST be included BEFORE glfw3.h

#include <GLFW/glfw3.h>

#include "pilcrow/modules/shmup/Source.hpp"

PlayerSystem::PlayerSystem(World& aWorld)
{
  aWorld.On([this](KeyEvent const& aKeyEvent) 
    {
      OnKeyEvent(aKeyEvent);
    });
}


void PlayerSystem::OnKeyEvent(KeyEvent const& aKeyEvent)
{
  std::cout << aKeyEvent.Key << "\n";
}

void PlayerSystem::PreProcess()
{
  std::cout << "preprocess\n";
}

void PlayerSystem::PostProcess()
{
  //std::cout << "postprocess\n";
}

void PlayerSystem::Process(Player const& aPlayer,Transform& aTransform)
{
  std::cout << "process\n";
}
