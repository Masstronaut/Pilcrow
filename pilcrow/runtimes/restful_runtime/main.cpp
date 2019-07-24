#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include <glad/include/glad.h>  // MUST be included BEFORE glfw3.h

#include <GLFW/glfw3.h>

// TODO(unknown): Remove
#include "pilcrow/engine/core/Camera.hpp"

double dt{0.f};
double lastFrame{0.f};

void UnrecoverableError(const char *msg) {
  std::cout << "An unrecoverable error has occurred:\n";
  std::cout << msg << std::endl;
  system("pause");
  exit(-1);
}

std::string ReadFile(const std::string &path) {
  std::ifstream infile(path);
  std::string   result;
  result.reserve(1024);
  std::array<char, 256> line{};
  while(infile.good() && !infile.eof()) {
    infile.getline(line.data(), 256);
    result += line.data();
    result += "\n";
  }
  return result;
}

#include "Utils/ResourceSettings.hpp"
std::string RelativePath() {
  static std::string path{g_ResourcePath};
  return path;
}

#include "pilcrow/engine/core/Detectors.hpp"

#include "pilcrow/engine/core/entity/EntitiesWith.hpp"

#include "pilcrow/engine/core/components/Transform.h"

#include "pilcrow/modules/shmup/Source.hpp"

// ---------------------------
// *  Global Settings INIT  *
// ---------------------------
// window
unsigned int g_InitialWindowWidth  = 100;
unsigned int g_InitialWindowHeight = 100;
bool         g_StartFullscreen     = false;
// resources
const char *g_ResourcePath = "../../../../Resources/";
// game
bool g_SpawnNanos = false;
// ---------------------------

#include "pilcrow/engine/core/RenderSystem.h"
#include "pilcrow/engine/core/SettingsFileReader.hpp"
#include "pilcrow/engine/core/Simulation.hpp"
#include "pilcrow/engine/core/World.hpp"
#include "pilcrow/engine/rest_api/RESTAPI.h"

#include "pilcrow/modules/physics/components/CircleCollider.h"
#include "pilcrow/modules/physics/systems/CollisionDetection.h"
#include "pilcrow/modules/physics/systems/Integration.h"
#include "pilcrow/modules/physics/systems/Resolution.h"

ArchetypeRef CreatePlayerArchetype(Simulation &Sim) {
  auto  player          = Sim.CreateArchetype("Hero");
  auto &playerModel     = player.Add<CModel>("nanosuit.obj");
  auto &playerComponent = player.Add<Player>();
  auto &playerCollider{player.Add<CircleCollider>()};
  playerComponent.mController = 0;

  float s = 0.0722623;
  auto& transform = player.Add<Transform>();
  transform.position = { -2.75f, 0.0f, 0.0f };
  transform.rotation = { 0.f, 0.f, glm::radians(-90.f) };
  transform.scale = { s, s, s };
  playerCollider.Radius = s;
  playerCollider.Group   = CollisionGroup::player;

  return player;
}

ArchetypeRef CreateBulletArchetype(Simulation& aSimulation)
{
  auto bullet = aSimulation.CreateArchetype("Bullet");
  //auto& bulletModel = bullet.Add<CModel>("bunny.ply");
  auto& bulletModel = bullet.Add<CModel>("nanosuit.obj");
  bullet.Add<Bullet>();
  auto& bulletCollider{ bullet.Add<CircleCollider>() };

  float s = 1.f;
  auto &transform       = bullet.Add<Transform>();
  transform.position    = {0.0f, 0.0f, 0.0f};
  transform.rotation    = {0.f, 0.f, glm::radians(-90.f)};
  transform.scale       = {s, s, s};
  bulletCollider.Radius = s;
  bulletCollider.Group   = CollisionGroup::bullet;

  return bullet;
}

ArchetypeRef CreateEnemyArchetype(Simulation &Sim) {
  ArchetypeRef enemy{Sim.CreateArchetype("Enemy")};
  auto &       enemyModel = enemy.Add<CModel>("nanosuit.obj");
  enemy.Add<Enemy>();
  auto& enemyCollider{ enemy.Add<CircleCollider>() };

  float s = 0.0722623;

  auto& transform = enemy.Add<Transform>();
  transform.scale = { s, s, s };
  transform.position = { 0.0f, 0.0f, 0.0f };
  transform.rotation = { 0.f, 0.f, 0.f };
  enemyCollider.Radius = s;
  enemyCollider.Group  = CollisionGroup::enemy;
  return enemy;
}

ArchetypeRef CreateCameraArchetype(Simulation &Sim) {
  ArchetypeRef lens{Sim.CreateArchetype("Camera Lens")};
  lens.Add<Camera>();
  return lens;
}

void ECSDemo() {
  Simulation Sim;
  World &    TestWorld{Sim.CreateWorld("Test World")};

  // Load Global Game Settings - Always do this before adding systems!
  SettingsFile settings{"Settings.ini"};
  settings.Load();

  TestWorld.AddSystem<WindowManager>("Window Management System");
  TestWorld.AddSystem<RenderSystem>("Rendering System");
  TestWorld.AddSystem<PlayerSystem>("PlayerSystem",
                                    TestWorld,
                                    CreateBulletArchetype(Sim));
  TestWorld.AddSystem<BulletSystem>("BulletSystem");
  TestWorld.AddSystem<EnemySystem>("EnemySystem",
                                   CreateEnemyArchetype(Sim),
                                   TestWorld);
  TestWorld.AddSystem<CollisionDetection>("Collision Detection System");
  TestWorld.On([](const CircleCollisionEvent &event) {
    std::cout << "Collision event:\n"
              << event.Entity1->Name() << " collided with "
              << event.Entity2->Name() << std::endl;
  });
  EntityRef player{TestWorld.Spawn(CreatePlayerArchetype(Sim))};

  // Camera
  auto cameraArchetype = CreateCameraArchetype(Sim);

  EntityRef cam{TestWorld.Spawn(cameraArchetype)};
  cam.Get<Camera>().position = glm::vec3{0.f, 0.0f, 2.f};
  cam.Get<Camera>().pitch    = 0.f;
  cam.Get<Camera>().yaw      = -90.f;

  // Makes the Game exit on window close
  bool WindowOpen = true;
  TestWorld.On([&](const Jellyfish::GLWindow::EWindowStateChanged &event) {
    if(event.newState == Jellyfish::WindowState::closed) { WindowOpen = false; }
  });

  try {
    REST_VM h(Sim, utility::string_t(U("http://*:1357/api/")));
    auto    server = h.Open();
    while(WindowOpen) {
      double currentFrame = glfwGetTime();
      dt                  = currentFrame - lastFrame;
      unsigned FPS{static_cast<unsigned>(1. / dt)};
      lastFrame = currentFrame;
      Sim.Run(dt, TestWorld.Name());
    }
    server.wait();
  } catch(...) {}  // if run without admin perms the server throws on exit since
                   // it can't open a port.
}

#include "pilcrow/modules/physics/Mathematics.h"

#include "GeneratedReflection/engine_ReflectionCode.h"
#include "GeneratedReflection/physics_ReflectionCode.h"

int main() {
  srefl::InitializeReflection();
  GlmReflection::InitializeReflection();
  engine_ReflectionInitialize::InitializeReflection();
  physics_ReflectionInitialize::InitializeReflection();

  ECSDemo();

  return 0;
}
