# INSTALLATION GUIDE

## Installation of OpenSceneGraph For 3D Simulation

- Download source files of <b>OpenSceneGraph</b> from this link: http://trac.openscenegraph.org/downloads/developer_releases/OpenSceneGraph-3.4.0.zip

### Windows

- Install <b>CMake</b> and <b>MinGW</b> for compile source files of <b>OpenSceneGraph</b>

    <b>CMake</b> download link: https://cmake.org/download/
    
    <b>MinGW</b> download link: https://sourceforge.net/projects/mingw/files/
    
- Open <b>CMake</b> and press **"Browse Source"** button and select **"/OpenSceneGraph-3.4.0/src"**

- Then create a new folder which its name **"mingw-build"** in same directory and from <b>CMake</b> press **"Browse Build"** button and select folder which you have created

- Then press **"Configure"** button and select **"MinGW Makefiles"** from list

- After configuration completed succesfull press **"Generate"** button

- After Makefile generation completed close <b>CMake</b>

- Then open command line and change directory to **"/OpenSceneGraph-3.4.0/mingw-build"** 

- Then enter **"mingw32-make"** command, it will begin compile of <b>OpenSceneGraph</b> source codes

- After compilation completed successfully enter **"mingw32-make install"** command, it will install compiled binary packages to your computer.

- Then add these folders to your computer's environment variables:
  
 - **/OpenSceneGraph/bin/**
 - **/MinGW/bin**
  
- At last, install <b>CLion</b> IDE and open our project

### Ubuntu

- First you need to install <b>OpenGL</b> library in Ubuntu with these command from terminal.

  **sudo apt-get install libgl1-mesa-dev mesa-common-dev**

- Then change directory to main directory of downloaded <b>OpenSceneGraph</b> package

- Then enter **"cmake CMakeLists.txt"** command, it will generates Makefiles to compile <b>OpenSceneGraph</b> source codes
 
- After generation completed, enter **"make"** command, it will begin compile of <b>OpenSceneGraph</b> source codes

- After compilation completed successfully enter **"make install"** command, it will install compiled binary packages to your computer.

- At last, install <b>CLion IDE</b> and open our project
