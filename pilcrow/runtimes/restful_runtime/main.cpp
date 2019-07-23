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

struct RigidBody {
  glm::vec3 velocity{0.f, 0.f, 0.f}, acceleration{0.f, 0.f, 0.f};
  bool      isStatic{false}, isGhost{false};
};
struct CircleCollider {
  float radius{1.f};
};
template <typename T>
void ComponentPrinter(const T &comp) {
  std::cout << comp << std::endl;
}

void TransformPrinter(const Transform &trans) 
{
  auto type = srefl::TypeId<Transform>();

  auto position = type->GetField("position")->GetGetter()->Invoke(&trans).As<glm::vec3>();
  auto rotation = type->GetField("rotation")->GetGetter()->Invoke(&trans).As<glm::vec3>();
  auto scale = type->GetField("scale")->GetGetter()->Invoke(&trans).As<glm::vec3>();

  std::cout << "{\n  pos : { " << trans.position.x << ", " << trans.position.y << ", " << trans.position.z << " },\n  "
            << "{\n  rot : { " << trans.rotation.x << ", " << trans.rotation.y << ", " << trans.rotation.z << " },\n  "
            << "{\n  sca : { " << trans.scale.x    <<  ", " << trans.scale.y   << ", " << trans.scale.z    << " }\n}\n";
}

class TransformPrinterSystem {
public:
  EntitiesWith<const Transform> Entities;
  void Update() {
    for(const auto &e : Entities) {
      TransformPrinter(e.Get<const Transform>());
    }
  }
};

class Gravity {
public:
  EntitiesWith<RigidBody> Entities;
  void                    Update(float dt) {
    for(auto &e : Entities) {
      auto &rb = e.Get<RigidBody>();
      rb.acceleration.y -= dt * 9.81f;
    }
  }
};
class VelocitySystem {
public:
  EntitiesWith<Transform, const RigidBody> Entities;
  void                                     Update(float dt) {
    for(auto &e : Entities) {
      const auto &rb = e.Get<const RigidBody>();
      auto &      tf = e.Get<Transform>();
      tf.position += rb.velocity * dt;
    }
  }
};
class AccelerationSystem {
public:
  EntitiesWith<RigidBody> Entities;
  void                    Update(float dt) {
    for(auto &e : Entities) {
      auto &rb = e.Get<RigidBody>();
      rb.velocity += rb.acceleration * dt;
    }
  }
};

struct ParallelVelocitySystem {
  void PreProcess() {}
  void Process(Transform &tf, const RigidBody &rb) const {
    tf.position += rb.velocity * Dt;
    if(tf.position.y <= 0.f) { tf.position.y = 0.f; }
  }
  float Dt = 1.f / 60.f;
};

struct ParallelAccelerationSystem {
  void  PreProcess() {}
  void  Process(RigidBody &rb) const { rb.velocity += rb.acceleration * Dt; }
  float Dt = 1.f / 60.f;
};

struct ParallelGravity {
  ParallelGravity() = default;
  explicit ParallelGravity(glm::vec3 gravity) : m_gravity(gravity) {}
  ParallelGravity(const ParallelGravity &) = default;
  ParallelGravity(ParallelGravity &&)      = default;
  void      PreProcess() {}
  void      Process(RigidBody &rb) const { rb.acceleration += m_gravity * Dt; }
  glm::vec3 m_gravity{0.f, -9.81f, 0.f};
  float     Dt{0.f};
};

// ---------------------------
// *  Global Settings INIT  *
// ---------------------------
// window
unsigned int g_InitialWindowWidth = 100;
unsigned int g_InitialWindowHeight = 100;
bool  g_StartFullscreen     = false;
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

#include "pilcrow/modules/physics/systems/CollisionDetection.h"
#include "pilcrow/modules/physics/systems/Integration.h"
#include "pilcrow/modules/physics/systems/Resolution.h"

ArchetypeRef CreatePlayerArchetype(Simulation& Sim)
{
  auto player = Sim.CreateArchetype("Hero");
  auto& playerModel  = player.Add<CModel>("nanosuit.obj");
  player.Add<Player>();

  float s = playerModel.model->GetScale();
  auto& transform = player.Add<Transform>();
  transform.position = { 0.0f, 0.0f, 3.0f };
  transform.rotation = { 0.f, 0.f, 0.f };
  transform.scale = { s, s, s };

  return player;
}


ArchetypeRef CreateEnemyArchetype(Simulation& Sim)
{
  ArchetypeRef enemy{ Sim.CreateArchetype("Nanosuit Character") };
  enemy.Add<RigidBody>();
  auto& enemyModel = enemy.Add<CModel>("nanosuit.obj");

  float s = 0.f;

  s = enemyModel.model->GetScale();
  auto& transform = enemy.Add<Transform>();
  transform.scale = { s, s, s };
  transform.position = { 0.0f, 0.0f, 0.0f };
  transform.rotation = { 0.f, 0.f, 0.f };

  return enemy;
}



ArchetypeRef CreateCameraArchetype(Simulation& Sim)
{
  ArchetypeRef lens{ Sim.CreateArchetype("Camera Lens") };
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
  // TestWorld.AddSystem<Gravity>("Gravity System");
  // TestWorld.AddSystem<ParallelGravity>("Parallelized Gravity System",
  // glm::vec3{ 0.f, -9.81f, 0.f });
  // TestWorld.AddSystem<ParallelVelocitySystem>("Parallelized Velocity
  // System");
  // TestWorld.AddSystem<ParallelAccelerationSystem>("Parallelized Acceleration
  // System");
  TestWorld.AddSystem<Integration>("Physics Integration");
  TestWorld.AddSystem<CollisionDetection>("Physics Collision Detection");
  TestWorld.AddSystem<Resolution>("Physics Resolution");
  TestWorld.AddSystem<RenderSystem>("Rendering System");
  //TestWorld.AddSystem<TransformPrinterSystem>("Printer System");

  TestWorld.AddSystem<PlayerSystem>("PlayerSystem", TestWorld);

  auto playerArchetype = CreatePlayerArchetype(Sim);
  EntityRef player{ TestWorld.Spawn(playerArchetype) };

  // Camera
  auto cameraArchetype = CreateCameraArchetype(Sim);

  EntityRef cam{ TestWorld.Spawn(cameraArchetype) };
  cam.Get<Camera>().position = glm::vec3{ 0.f, 0.6f, -2.f };
  cam.Get<Camera>().pitch = -1.f;
  cam.Get<Camera>().yaw = -89.f;

  // Enemies
  auto enemyArchetype = CreateEnemyArchetype(Sim);

  std::vector<EntityRef> nanos;
  if(g_SpawnNanos) {
    float scalar = 1.f;  // differentiating scale
    float angle  = 0.f;  // pi/6

    for(int i{0}; i < 4; ++i) {
      for(int j{0}; j < 4; ++j) {
        nanos.emplace_back(TestWorld.Spawn(enemyArchetype));

        auto& transform = nanos.back().Get<Transform>();
        auto& model = nanos.back().Get<CModel>();

        // position
        transform.position = glm::vec3{i * 2, 0, j * 2};

        // rotation
        transform.rotation.y = angle;


        // scale
        transform.scale *= scalar;

        // Iteration
        angle += glm::radians(30.f);
        scalar = scalar + 0.1f;
      }
    }
  }

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

#include "GeneratedReflection/physics_ReflectionCode.h"
#include "GeneratedReflection/engine_ReflectionCode.h"

int main() {
  srefl::InitializeReflection();
  GlmReflection::InitializeReflection();
  engine_ReflectionInitialize::InitializeReflection();
  physics_ReflectionInitialize::InitializeReflection();

  ECSDemo();

  return 0;
}
