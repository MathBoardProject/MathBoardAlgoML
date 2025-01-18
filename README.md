# MathBoardAlgoML

# Build
Tested on Linux.

* Linux (and other Unix systems like MacOS):
    * Install necessary packages (different commands based on your distribution)
        * Fedora: 
            * `sudo dnf install cmake git ffmpeg vtk opencv opencv-devel tesseract tesseract-devel spdlog spdlog-devel`
            * **Make SymEngine from source**
        * Arch:
            * `sudo pacman -S --needed cmake git ffmpeg vtk opencv tesseract symengine spdlog`
        * Ubuntu:
            * `sudo apt install cmake git ffmpeg opencv tesseract spdlog`
            * **Make SymEngine from source**
    * `git clone https://github.com/MathBoardProject/MathBoardAlgoML`
    * `cd MathBoardAlgoML`
    * `mkdir build`
    * `cd build`
    * `cmake ..`
    * `cmake --build .`
    * `./mathboard`
* Windows (WSL):
    * Setup WSL (Windows Subsystem for Linux)
    * Inside your WSL container do the Linux steps
* Windows (Visual Studio):
    * `winget install Git.Git Kitware.CMake`
    * Setup vcpkg, add `VCPKG_ROOT` environmental variable and edit `CMakeUserPresets.json`: https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell#1---set-up-vcpkg
    * Open Visual Studio and click "Clone a repository"
    * Under "Repository location" type/paste: https://github.com/MathBoardProject/MathBoardAlgoML
    * Click "Clone" and select the folder inside UI
    * Select preset called "windows", wait for the project to setup
    * Press F5, or run the project from UI
* Windows (Terminal):
    * `winget install Git.Git Kitware.CMake`
    * Setup vcpkg, add `VCPKG_ROOT` environmental variable and edit `CMakeUserPresets.json`: https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell#1---set-up-vcpkg
    * `git clone https://github.com/MathBoardProject/MathBoardAlgoML`
    * `cd MathBoardAlgoML`
    * `mkdir build`
    * `cd build`
    * `cmake --preset=windows ..`
    * `cmake --build .`
    * `.\mathboard`

# Testing
* Linux
   * Install socat:
      * Fedora: `sudo dnf install socat`
      * Arch: `sudo pacman -S --needed socat`
      * Ubuntu: `sudo apt install socat`
   * [Build the project](https://github.com/MathBoardProject/MathBoardAlgoML#Build)
   * `ctest`
