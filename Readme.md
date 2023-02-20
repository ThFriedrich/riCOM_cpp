# riCOM
[![Compile](https://github.com/ThFriedrich/riCOM_cpp/actions/workflows/build.yml/badge.svg)](https://github.com/ThFriedrich/riCOM_cpp/blob/master/.github/workflows/build.yml)
[![Build Documentation](https://github.com/ThFriedrich/riCOM_cpp/actions/workflows/doxygen-gh-pages.yml/badge.svg)](https://github.com/ThFriedrich/riCOM_cpp/actions/workflows/doxygen-gh-pages.yml)
[![License: GPL3](https://img.shields.io/badge/License-GPL3-brightgreen.svg)](https://opensource.org/licenses/GPL-3.0) 

This repository contains the C++ implementation of the riCOM (Real Time Centre Of Mass) algorithm for 4D Scanning electron microscopy as described in [this publication](https://www.cambridge.org/core/journals/microscopy-and-microanalysis/article/realtime-integration-center-of-mass-ricom-reconstruction-for-4d-stem/40AC3F51175C32763ADF355E58073355). At this point it is compatible with .mib files (Quantum Detectors Merlin) and .t3p (Timepix camera) data and can be run in live mode with the Merlin camera.

![gui](https://user-images.githubusercontent.com/47680554/211806138-87dd32db-e15d-406f-a55e-187495668741.png)

## Installation
### Binaries
- Just download the precompiled executable for your system from [Releases](https://github.com/ThFriedrich/riCOM_cpp/releases) or for the latest development version the artefacts of the automated compilation run from the Repositories "Actions" Tab, for example [here](https://github.com/ThFriedrich/riCOM_cpp/actions/workflows/build.yml)! 
- The Windows version includes additional libraries, which need to be kept in the same folder as the executable. Linux requires SDL2 and FFTW3 libraries on your system (see below).
- Alternatively build from source as outlined below. Make sure you clone the repository including submodules: ```git clone --recurse-submodules -j2 git@github.com:ThFriedrich/riCOM_cpp.git``` 
- The project uses features of C++ standard 17. You may need appropriate compilers and libraries.
- Generally the performance/speed may not be ideal using precompiled binaries. For best results compile on the machine you want to run the software on, using the "native" option for the "ARCH" variable in the CMakeLists.txt file
   
**Generally build instructions can be followed step by step from the [automated build setup](https://github.com/ThFriedrich/riCOM_cpp/blob/master/.github/workflows/build.yml) from command line**
### Build instructions Linux
#### Dependencies 
- [g++/gcc](https://gcc.gnu.org/) compiler version >=9
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
  - run ```lib /machine:x64 /def:libfftw3f-3.def```

#### Building from Command Line (**Use __Developer__ Powershell for VS**)
```bat
mkdir build
cd build
cmake ..
msbuild .\RICOM.vcxproj /p:configuration=Release
```
The executable RICOM.exe will be in the folder 'build\Release'. Make  sure **the files 'SDL2.dll', 'SDL2_image.dll', 'libfftw3f-3.dll' and 'libfftw3f-3.lib' are in the same location as the executable**. When using oneAPI `icx` compiler from Intel, also the libraries: **the files 'svml_dispmd.dll', 'libmmdd.dll' and 'libmmd.dll'** must be in this directory.

## Running the program
### Running in Live mode with Quantum Detector MerlinEM camera
To send instructions to the camera a python scipt is executed internally, which relies on the [Merlin Interface package](https://gitlab.com/tfriedrich/merlin_interface). It is a submodule of this repository, but it can also be manually downloaded. The file [merlin_interface.py](https://gitlab.com/tfriedrich/merlin_interface/-/blob/master/merlin_interface/merlin_interface.py) should be placed in the same directory as the RICOM executable.

### Running example files
A set of compatible example datasets are provided in an open data repository on [Zenodo](https://zenodo.org/record/5572123#.YbHNzNso9hF).

### Command Line Interface
The program can be run from command line without opening a GUI. The input file path and other parameters are being specified by additional parameters as listed [here](https://github.com/ThFriedrich/riCOM_cpp/blob/master/src/RunCLI.cpp).  
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

## Cite
If you use this software in your research please cite:
```bibtex
@article{
  ricom_2022, 
  title={Real-Time Integration Center of Mass (riCOM) Reconstruction for 4D STEM}, 
  DOI={10.1017/S1431927622000617}, 
  journal={Microscopy and Microanalysis}, 
  publisher={Cambridge University Press}, 
  author={Yu, Chu-Ping and Friedrich, Thomas and Jannis, Daen and Van Aert, Sandra and Verbeeck, Johan}, 
  year={2022}, 
  pages={1–12}}
```
