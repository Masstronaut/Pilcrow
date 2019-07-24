// std
#include <iostream>  //cout

// empty line so clang-format alphabetizing doesn't put glad below glfw

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>  //glfw window stuff
#include <GLFW/glfw3native.h>

// ours
#include "GlobalD3DResources.h"
#include "pilcrow/modules/jellyfish_renderer/GL/GLWindow.h"

// TODO(unknown): fix, put in settings or something
const unsigned glmajor = 4;
const unsigned glminor = 3;

namespace Jellyfish {
GLWindow::GLWindow() { g_singleton_window = this; }

GLWindow::~GLWindow() {
  glfwDestroyWindow(m_WindowHandle);
  m_WindowHandle = nullptr;
  glfwTerminate();
}

void GLWindow::CreateGameWindow(unsigned    width,
                                unsigned    height,
                                bool        fullscreen,
                                std::string title) {
  if(glfwInit() != 0) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glmajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glminor);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_FOCUSED, GL_TRUE);
#if defined(__APPLE__)
    // required for MAC OSX support
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create the window and get a pointer to the handle
    if(fullscreen) {
      // Get Monitor Resolution
      GLFWmonitor *      primaryMonitor = glfwGetPrimaryMonitor();
      const GLFWvidmode *primaryMonMode = glfwGetVideoMode(primaryMonitor);
      int                tempWidth      = primaryMonMode->width;
      int                tempHeight     = primaryMonMode->height;

      // Create a borderless fullscreen window at current resolution
      m_Size         = glm::uvec2(tempWidth, tempHeight);
      m_WindowHandle = glfwCreateWindow(tempWidth,
                                        tempHeight,
                                        title.c_str(),
                                        primaryMonitor,
                                        nullptr);
    } else {
      // Create a restored resizable window at specified resolution
      glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
      m_Size = glm::uvec2(width, height);
      m_WindowHandle
        = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    }

    if(m_WindowHandle == nullptr) {
      std::cout << "Failed to create window! Bad Handle." << std::endl;
      system("pause");
      return;
    }

    // Set Context
    glfwMakeContextCurrent(m_WindowHandle);

    // Set Callback Functions
    glfwSetFramebufferSizeCallback(m_WindowHandle, Callback_ResizeWindow);
    glfwSetWindowCloseCallback(m_WindowHandle, Callback_WindowClose);
    glfwSetWindowPosCallback(m_WindowHandle, Callback_WindowMove);
    glfwSetCursorPosCallback(m_WindowHandle, Callback_CursorPosition);
    glfwSetMouseButtonCallback(m_WindowHandle, Callback_MouseButton);
    // glfwSetScrollCallback(window, scroll_callback);

    // saves last button state betwen polling to capture button press/release
    glfwSetInputMode(m_WindowHandle, GLFW_STICKY_MOUSE_BUTTONS, 1);

    // using glfwgetkey instead
    // glfwSetKeyCallback(window, key_callback);

    // Set this to true so GLEW knows to use a modern approach to retrieving
    // function pointers and extensions
    // glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    // if (glewInit() != GLEW_OK)
    //{
    //	std::cout << "Failed to initialize GLEW" << std::endl;
    //	system("pause");
    //	return;
    //}

    GlobalDeviceResources.RegisterDeviceNotify(this);

    GlobalDeviceResources.SetWindow(glfwGetWin32Window(m_WindowHandle),
                                    width,
                                    height);

    GlobalDeviceResources.CreateDeviceResources();

    GlobalDeviceResources.CreateWindowSizeDependentResources();

    GlobalSetupRootParams();

    // Define the viewport dimensions
    GLsizei w = this->GetWindowWidth();
    GLsizei h = this->GetWindowHeight();
    glfwGetFramebufferSize(m_WindowHandle, &w, &h);

    GlobalWidth  = width;
    GlobalHeight = height;

	inRender = false;
  } else {
    std::cout << "Failed to initialize GLFW!" << std::endl;
    system("pause");
    return;
  }
  // ALL OK!

  FrameStart();
}
void GLWindow::UpdateGameWindow() {}
void GLWindow::DisplayGameWindow() {
  inRender = false;

  auto device           = GlobalDeviceResources.GetD3DDevice();
  auto commandList      = GlobalDeviceResources.GetCommandList();
  auto commandAllocator = GlobalDeviceResources.GetCommandAllocator();

  PIXBeginEvent(GlobalDeviceResources.GetCommandQueue(),
                PIX_COLOR_DEFAULT,
                L"Present");
  { GlobalDeviceResources.Present(); }
  PIXEndEvent(GlobalDeviceResources.GetCommandQueue());
}

bool GLWindow::IsOpen() { return m_Open; }
bool GLWindow::IsActive() { return m_Active; }
void GLWindow::setActive(bool /*active*/) {}

void GLWindow::ResizeWindow(unsigned width, unsigned height) {
  if((width != this->m_Size.x) || (height != this->m_Size.y)) {
    // send off event to notify other systems
    EWindowResized event;
    event.newSize = glm::uvec2{width, height};
    event.oldSize = m_Size;
    g_singleton_window->Emit(event);

    m_Size.x = width;
    m_Size.y = height;
    std::cout << "Window was resized to: " << width << " by " << height
              << std::endl;

    glfwMakeContextCurrent(m_WindowHandle);

    GlobalWidth  = width;
    GlobalHeight = height;
  }
}

unsigned GLWindow::GetWindowWidth() { return m_Size.x; }
unsigned GLWindow::GetWindowHeight() { return m_Size.y; }

void GLWindow::FrameStart() {
  auto device           = GlobalDeviceResources.GetD3DDevice();
  auto commandList      = GlobalDeviceResources.GetCommandList();
  auto commandAllocator = GlobalDeviceResources.GetCommandAllocator();

  if (inRender)
  { return;
  }

  inRender = true;

  // Start first frame.
  {
    GlobalDeviceResources.Prepare();

    {
      FLOAT black[] = {0.15f, 0.15f, 0.50f, 1.f};
      commandList
        ->ClearRenderTargetView(GlobalDeviceResources.GetRenderTargetView(),
                                black,
                                0,
                                nullptr);
      commandList
        ->ClearDepthStencilView(GlobalDeviceResources.GetDepthStencilView(),
                                D3D12_CLEAR_FLAG_DEPTH
                                  | D3D12_CLEAR_FLAG_STENCIL,
                                1.f,
                                0,
                                0,
                                nullptr);
      commandList
        ->OMSetRenderTargets(1,
                             &GlobalDeviceResources.GetRenderTargetView(),
                             true,
                             &GlobalDeviceResources.GetDepthStencilView());
    }
  }
}

void GLWindow::FrameEnd() {
  DisplayGameWindow();
  PollEvents();
}

void GLWindow::SetWindowTitle(const std::string &title) {
  glfwSetWindowTitle(m_WindowHandle, title.c_str());
}
void GLWindow::SetWindowState(WindowState state) {
  if(m_State != state) {
    WindowState         oldState{m_State};
    EWindowStateChanged event{state, m_State};
    this->Emit(event);

    // TODO(unknown): stringify events at compile time for friendly output
    switch(state) {
      case WindowState::maximized:
        glfwMaximizeWindow(m_WindowHandle);
        std::cout << "Window state was changed to: "
                  << "maximized" << std::endl;
        break;
      case WindowState::minimized:
        glfwIconifyWindow(m_WindowHandle);
        std::cout << "Window state was changed to: "
                  << "minimized" << std::endl;
        break;
      case WindowState::restored:
        glfwRestoreWindow(m_WindowHandle);
        std::cout << "Window state was changed to: "
                  << "restored" << std::endl;
        break;
      case WindowState::closed:
        glfwSetWindowShouldClose(m_WindowHandle, 1);
        std::cout << "Window state was changed to: "
                  << "closed" << std::endl;
        break;
      default:
        return;
    }
    m_State = state;
  }  // endif
}  // endfunc

void GLWindow::PollEvents() {
  // Makes the glfw callbacks happen
  glfwPollEvents();
}

// Polls Input devices, returns current input in data passed as ref
// HIGHLY Likely to deprecate in favor of a key callback
void GLWindow::PollInput(std::vector<int> &keyarray) {
  // Keyboard Press
  keyarray.clear();  // clear prev input
  // GLFW key codes range from -1 to 350
  for(int key = -1; key <= 350; ++key) {
    if(glfwGetKey(m_WindowHandle, key) == GLFW_PRESS) {
      // could use events but array is probably faster
      // EKeyPressed event {key};
      // this->Emit(event);
      keyarray.push_back(key);
    }  // endif
  }    // endfor

  // Mouse
  // determine if a button is held, TODO: cursorClickedState array, just doing
  // for all buttons right now
  for(int i = 0; i < 10; ++i) {
    if((g_singleton_window->m_Cursor.buttonStates[i] == GLFW_PRESS)
       && (g_singleton_window->m_Cursor.buttonHeld[i] != GLFW_PRESS)) {
      g_singleton_window->m_Cursor.buttonHeld[i] = GLFW_PRESS;
      std::cout << "Button state changed to held for button: " << i
                << std::endl;
    } else if((g_singleton_window->m_Cursor.buttonStates[i] == GLFW_RELEASE)
              && (g_singleton_window->m_Cursor.buttonHeld[i] != GLFW_RELEASE)) {
      g_singleton_window->m_Cursor.buttonHeld[i] = GLFW_RELEASE;
      std::cout << "Button state changed to normal for button: " << i
                << std::endl;
    }
  }

}  // endfunc

void GLWindow::Callback_ResizeWindow(GLFWwindow * /*windowhandle*/,
                                     int width,
                                     int height) {
  g_singleton_window->ResizeWindow(width, height);
}
void GLWindow::Callback_WindowClose(GLFWwindow *windowhandle) {
  glfwSetWindowShouldClose(windowhandle, 1);
  std::cout << "glfw window should close flag was set!" << std::endl;

  g_singleton_window->SetWindowState(WindowState::closed);
}

void GLWindow::Callback_WindowMove(GLFWwindow * /*windowhandle*/, 
                                   int /*xpos*/,
                                   int /*ypos*/) {
}

void GLWindow::Callback_CursorPosition(GLFWwindow * /*windowhandle*/,
                                       double xpos, 
                                       double ypos) {
  // send event
  EMouseMoved event;
  event.newPosition = glm::dvec2{xpos, ypos};
  event.oldPosition = g_singleton_window->m_Cursor.position;
  event.cursorData  = g_singleton_window->m_Cursor;  // shallow copy?
  g_singleton_window->Emit(event);

  // update pos
  g_singleton_window->m_Cursor.position = glm::dvec2{xpos, ypos};
}
void GLWindow::Callback_MouseButton(GLFWwindow * /*window*/,
                                    int button,
                                    int action,
                                    int /*mods*/) {
  // mods is is all possible glfw modifier bit states as defined below
  /*
  #define 	GLFW_MOD_SHIFT   0x0001
    If this bit is set one or more Shift keys were held down. More...

      #define 	GLFW_MOD_CONTROL   0x0002
        If this bit is set one or more Control keys were held down. More...

      #define 	GLFW_MOD_ALT   0x0004
        If this bit is set one or more Alt keys were held down. More...

      #define 	GLFW_MOD_SUPER   0x0008
    If this bit is set one or more Super keys were held down. More..
  */

  /// TODO: mod states

  // action is all possible glfw mouse action states as defined below
  /*
    #define GLFW_RELEASE   0
    The key or mouse button was released.

    #define GLFW_PRESS   1
        The key or mouse button was pressed.

        #define GLFW_REPEAT   2
    The key was held down until it repeated
      */

  // DEBUG:
  // get the proper name from the action
  std::string actionName;
  switch(action) {
    case GLFW_RELEASE: {
      actionName = "Released";
      break;
    }
    case GLFW_PRESS: {
      actionName = "Pressed";
      break;
    }
    case GLFW_REPEAT: {
      actionName = "Repeat";
      break;
    }
  }

  // get the proper name from the button number, assumes 0 and 1 are left and
  // right
  std::string buttonName;
  switch(button)  // glfw defines are set from 1 to 8, int vales 0 to 7, last,
                  // left, right, middle
  {
    case GLFW_MOUSE_BUTTON_LEFT: {
      buttonName = "Left";
      break;
    }
    case GLFW_MOUSE_BUTTON_RIGHT: {
      buttonName = "Right";
      break;
    }
    case GLFW_MOUSE_BUTTON_MIDDLE: {
      buttonName = "Middle";
      break;
    }
    default: {
      buttonName = std::to_string(button);
      break;
    }
  }

  std::cout << "Mouse was Activated.  Button: " << buttonName
            << " Action: " << actionName << std::endl;

  // send event data
  EMouseAction event;
  event.button         = button;
  event.action[button] = action;
  g_singleton_window->Emit(event);

  // set window state data
  g_singleton_window->m_Cursor.buttonStates[button] = action;
}

void GLWindow::CaptureMouse(bool aCapture) {
  if (aCapture)
  {
    glfwSetInputMode(m_WindowHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }
  else
  {
    glfwSetInputMode(m_WindowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
}

void GLWindow::OnDeviceLost() {
  throw std::exception("Function not implemented yet.");
}

void GLWindow::OnDeviceRestored() {
  throw std::exception("Function not implemented yet.");
}
}  // namespace Jellyfish
