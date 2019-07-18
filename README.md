# Engine
## Prerequisites for Windows 10:
* CMake installed(https://cmake.org/download/) 3.14 or newer
* Visual studio 2017 or 2019 installed (https://visualstudio.microsoft.com/vs/)
* Submodules should be checked out/initialized, the gitsubmodulecheckout.bat command should do this for you if you're unfamiliar with it.
## Setup Guide:
 1. Navigate to %repoLocation%\build_scripts\windows
 2. Run the build script for your visual studio version
## Starting Engine:
 1. Open the engine solution in Visual Studio (this is done for you by the build script)
 2. Select your preferred configuration (Debug, RelWithDebInfo, Release, etc).
 3. In the projects pane, right click "restful_runtime" and click "build". This must also be your "StartUp Project", the CMake should make this default.
 4. When the build is completed, press F5 to launch
 5. A console should appear, followed by a window displaying a character model.
