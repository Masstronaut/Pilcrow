#pragma once
#include <pilcrow/modules/jellyfish_renderer/Jellyfish.h>

#include "pilcrow/engine/core/components/Transform.h"
#include "pilcrow/engine/core/entity/EntitiesWith.hpp"

#include "pilcrow/engine/core/Camera.hpp"
#include "pilcrow/engine/core/RenderComponents.h"

#include "pilcrow/engine/core/SettingsFileReader.hpp"

struct WindowManager {
  WindowManager();

  void Init(World &world);

  EntitiesWith<Camera> Entities;
  void                 FrameStart();
  void                 FrameEnd();
  float                Dt{0.f};

  std::vector<int> GetKeys();

private:
  // render context
  Jellyfish::iWindow *pWindow{nullptr};

  void      ProcessInput(Camera &cam);
  glm::vec2 m_windowSizeSetting{g_InitialWindowWidth, g_InitialWindowHeight};
  bool      m_windowFullscreenSetting{g_StartFullscreen};

  std::vector<int> m_keyArray;
  World* m_world;
};

struct KeyEvent
{
  int Key;
};

struct GamepadButtonEvent
{
  int Gamepad;
  int Button;
};

struct RenderSystem {
  void Init(World &world) {
    // Register for Type Lists
    world.RegisterEntitiesWith(camEntities);
    world.RegisterEntitiesWith(textEntities);

    world.On([&](const Jellyfish::GLWindow::EWindowResized &event) {
      m_windowSize = event.newSize;
    });

    program.Load();

    // RENDERER LIB TEST
    Jellyfish::Derp test;
    test.DoAThing();  // prints a thing
  }

  void PreProcess() {
    unsigned int width = m_windowSize.x;
    unsigned int height = m_windowSize.y;

    if (0 == width)
    {
      width = 100;
    }

    if (0 == height)
    {
      height = 100;
    }

    // set up projetion matrices
    m_ortho_projection = glm::ortho(0.f, static_cast<float>(width), 0.f, static_cast<float>(height));
    if(camEntities.cbegin() != camEntities.cend()) {
      camera = &camEntities[0].Get<const Camera>();
      m_persp_projection
        = glm::perspective(glm::radians(camera->fov),
                           static_cast<float>(width) / static_cast<float>(height), 
                           camera->nearplane,
                           camera->farplane);
      program.SetUniform("projection", m_persp_projection);
      program.SetUniform("view", camera->View());
    }
  }

  void Process(const CModel &model, const Transform &tf) const {
    if(!camera) return;
    glm::mat4 modelMatrix;
    modelMatrix = glm::translate(modelMatrix, tf.position);

    modelMatrix
      = glm::rotate(modelMatrix, tf.rotation.x, glm::vec3(1.f, 0.f, 0.f));
    modelMatrix
      = glm::rotate(modelMatrix, tf.rotation.y, glm::vec3(0.f, 1.f, 0.f));
    modelMatrix
      = glm::rotate(modelMatrix, tf.rotation.z, glm::vec3(0.f, 0.f, 1.f));

    modelMatrix = glm::scale(modelMatrix, tf.scale);

    program.SetUniform("model", modelMatrix);

    model.model->AssignShaderToAllMeshes(program);
    model.model->Draw();  // program no longer neede as arg textures TODO
  }

  float NextTextPos(float prevPos) {
    int offset = DEBUG_TEXT_SIZE;
    return prevPos - offset;
  }

  void PostProcess() {
    // Render Text
    for(const auto &entity : textEntities) {
      const RenderText &renderable{entity.Get<const RenderText>()};
      gltr.Render(renderable.Text, renderable.Position, m_ortho_projection,
                  renderable.Color, renderable.Size);
    }

    float position;
    gltr.Render("FPS: " + std::to_string(1.f / Dt),
                {0.f, position = NextTextPos(static_cast<float>(m_windowSize.y))},
                m_ortho_projection, {.5f, .8f, .2f});
    gltr.Render("Camera Pos X: " + std::to_string(camera->position.x),
                {0.f, position = NextTextPos(position)}, m_ortho_projection,
                {0.f, 0.f, 1.f});
    gltr.Render("Camera Pos Y: " + std::to_string(camera->position.y),
                {0.f, position = NextTextPos(position)}, m_ortho_projection,
                {0.f, 0.f, 1.f});
    gltr.Render("Camera Pos Z: " + std::to_string(camera->position.z),
                {0.f, position = NextTextPos(position)}, m_ortho_projection,
                {0.f, 0.f, 1.f});
  }

  void Draw() {
    // TODO
  }

  void FrameEnd() {
    // send to Renderer
    Draw();
  }

  // Type Lists
  EntitiesWith<const Camera>     camEntities;
  EntitiesWith<const RenderText> textEntities;

  // GL Impl
  Jellyfish::GLText            gltr{"Text.sprog"};
  mutable Jellyfish::GLProgram program{"Model.sprog"};

  float         Dt{0.f};
  glm::mat4     m_persp_projection;
  glm::mat4     m_ortho_projection;
  const Camera *camera{nullptr};

  // TODO: Use events instead
  glm::uvec2 m_windowSize{g_InitialWindowWidth, g_InitialWindowHeight};
};
