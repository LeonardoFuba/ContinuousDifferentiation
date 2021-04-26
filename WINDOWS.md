# Basic Plugin for Evoked potential detection

## Pre requisits

-  [Cmake](https://cmake.org/download/)

-  [Visual Studio](https://visualstudio.microsoft.com/pt-br/downloads/)

---

## How to used on Windows

First of all, install Open [Ephys binary](https://open-ephys.org/gui) distribution or generate `open-ephys-GUI` build files through the source code


  ### Building files through the source code
  1. Clone [plugin-GUI](https://github.com/open-ephys/plugin-GUI)
  2. Open terminal on Build directory
  3. Generate the Visual Studio 2019 project files by typing the following from the command prompt:

  ```bash
    cmake -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE=Release -A x64 ..
  ```
  > NOTE: 
  >  - `DCMAKE_BUILD_TYPE` argument can be `Release` or `Debug`
  >
  >  - `-A` argument can be `x64` or `x86`
  > 
  >  -  For earlier versions of Visual Studio, substitute the last command with:
  >     ```bash
  >     -G "Visual Studio 12 2013"
  >     -G "Visual Studio 14 2015"
  >     -G "Visual Studio 15 2017"
  >     ```

  4. Open `open-ephys-GUI.sln` with Visual Studio and build the `ALL_BUILD` after choose `Release` or `Debug` in top bar


  ### Generate OEPlugin build files
  4. Clone this repository
  5. Place it follow one of these folder structures: 

  ```
  code_directory/                                 code_directory/
    plugin-GUI/                                     plugin-GUI/
      Build/                                          Build/
        Debug/                                          Release/
          open-ephys.lib                                  open-ephys.lib
    OEPlugins/                                      OEPlugins/
      BasicLFPMonitor/                                BasicLFPMonitor/
        Build/                                          Build/
        libs/                                           libs/
        Source/                                         Source/
        CMAKE_README.txt                                CMAKE_README.txt
        CMakeLists.txt                                  CMakeLists.txt
        README.md                                       README.md
  ```

  > NOTE: 
  > ##### Changing the location of the GUI repository
  > In case the plugin-GUI repository is not located alongside the plugin repository, there are two ways to specify its location
  > 
  > 1. Creating an environment variable called `GUI_BASE_DIR`
  > ```bash
  > export GUI_BASE_DIR=path/to/GUI
  > cmake -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE=Release -A x64 ..
  > ```
  > or
  > ```bash
  > GUI_BASE_DIR=/path/to/GUI cmake -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE=Release -A x64 ..
  > ```
  > 
  > 2. By using a cmake variable with the `-D` argument
  > ```bash
  > cmake -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE=Release -DGUI_BASE_DIR=/path/to/GUI -A x64 ..
  > ```


  ### Compile OEPlugin build files

  6. Repeat step 2 and 3
  7. Open `OE_PLUGIN_<plugin-name>.sln` file with Visual Studio
  8. Select the appropriate configuration (Debug/Release) in the top bar
  9. Either build the solution or build the `ALL_BUILD` project. That will run the build process on all projects except `INSTALL`, thus building the plugin.
  10. Right click on `INSTALL` solution an compile ( Selecting the INSTALL project and manually building it will trigger the install procedure, copying the plugin and any required files, if any, to the GUI’s appropriate directories. )
