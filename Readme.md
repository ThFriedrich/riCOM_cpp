# riCOM
[![BUILD](https://github.com/ThFriedrich/riCOM_cpp/actions/workflows/build.yml/badge.svg)](https://github.com/ThFriedrich/riCOM_cpp/blob/master/.github/workflows/build.yml)
[![License: GPL3](https://img.shields.io/badge/License-GPL3-brightgreen.svg)](https://opensource.org/licenses/GPL-3.0) 

This repository contains the C++ implementation of the riCOM (Real Time Centre Of Mass) algorithm for 4D Scanning electron microscopy as described in [this preprint](https://arxiv.org/abs/2112.04442). At this point it is compatible with .mib files (Quantum Detectors Merlin) and .t3p (Timepix camera) data and can be run in live mode with the Merlin camera.

![image](https://user-images.githubusercontent.com/47680554/164481785-d7407e48-1fa0-4673-a03b-ba80b3a5fcae.png)

## Installation
### Binaries
- Just download the precompiled executable for your system from [Releases](https://github.com/ThFriedrich/riCOM_cpp/releases) or for the latest development version the artefacts of the automated compilation run from the Repositories "Actions" Tab, for example [here](https://github.com/ThFriedrich/riCOM_cpp/actions/workflows/build.yml)! 
- The Windows version includes additional libraries, which need to be kept in the same folder as the executable. Linux requires SDL2 and FFTW3 libraries on your system (see below).
- Alternatively build from source as outlined below. Make sure you clone the repository including submodules: ```git clone --recurse-submodules -j2 git@github.com:ThFriedrich/riCOM_cpp.git``` 
- The project uses features of C++ standard 17. You may need appropriate compilers and libraries.

**Generally build instructions can be followed step by step from the [automated build setup](https://github.com/ThFriedrich/riCOM_cpp/blob/master/.github/workflows/build.yml) from command line**
### Build instructions Linux
#### Dependencies 
- [g++/gcc](https://gcc.gnu.org/) compiler version 9
  - ```sudo apt-get install gcc-9 g++-9```
- [make & cmake](https://cmake.org/)
- [SDL2](http://www.libsdl.org) 
  - ```sudo apt-get install libsdl2-dev libsdl2-image-dev```
- [FFTW3](https://fftw.org/)
  - ```sudo apt-get install libfftw3-dev``` 

#### Building from Command Line
```bash
mkdir build
cd build
cmake --config Release ..
make
```

### Build instructions Windows
#### Dependencies 
- [Visual Studio Community](https://visualstudio.microsoft.com/vs/community/) with C++ Desktop Development Kit or msbuild tools
  - including the msvc compiler, cmake and msbuild
- [SDL2](http://www.libsdl.org) 
  - Download 'SDL2-devel-2.0.xx-VC.zip' (Visual C++ 32/64-bit) from http://www.libsdl.org/download-2.0.php#source and [SDL2-image](http://www.libsdl.org/projects/SDL_image/release/SDL2_image-devel-2.0.5-VC.zip)
  - Both zip folder share a common sub-structure. Extract contents of the zips to 'C:/SDL2_VC', merging the contents of both zips into the same folder structure.
- [FFTW](https://fftw.org/)
  - Download and extract [FFTW3 library](https://fftw.org/pub/fftw/fftw-3.3.5-dll64.zip)
  - run ```lib /def:libfftw3-3.def```

#### Building from Command Line (**Use __Developer__ Powershell for VS**)
```bat
mkdir build
cd build
cmake ..
msbuild .\RICOM.vcxproj /p:configuration=Release
```
The executable RICOM.exe will be in the folder 'build\Release'. Make  sure **the file 'SDL2.dll' is in the same location as the executable**, or copy it to your system libraries, e.g. 'C:\Windows\System32' 

## Running the program
### Running in Live mode with Quantum Detector MerlinEM camera
To send instructions to the camera a python scipt is executed internally, which relies on the [Merlin Interface package](https://gitlab.com/tfriedrich/merlin_interface). It is a submodule of this repository, but it can also be manually downloaded. 

### Running example files
A set of compatible example datasets are provided in an open data repository on [Zenodo](https://zenodo.org/record/5572123#.YbHNzNso9hF).

### Command Line Interface
The program can be run from command line without opening a GUI. The input file path and other parameters are being specified by additional parameters as listed [here](https://github.com/ThFriedrich/riCOM_cpp/blob/303d8a9fb5b056810d83f51e5f858dc5cd608d56/src/main.cpp#L880).  
Example usage:
```bash
./RICOM -filename default1.mib -nx 64 -ny 64
```
This functionality was originally intended for dubugging purposes. It is therefore not fully developed yet, but may be useful for some users nevertheless.

## License
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

**You may contact the [University of Antwerp](jo.verbeeck@uantwerpen.be) for alternative licensing options if 
necessary.**
