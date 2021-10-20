[![BUILD](https://github.com/ThFriedrich/riCOM_cpp/actions/workflows/build.yml/badge.svg)](https://github.com/ThFriedrich/riCOM_cpp/blob/master/.github/workflows/build.yml)

# Installation
- Just download the precompiled executable for your system from the artefacts of the automated compilation run from the Repositories "Actions" Tab, for example [here](https://github.com/ThFriedrich/riCOM_cpp/actions/runs/1365246924)!
- Alternatively build from source as outlined below. Make sure you clone the repository including submodules: ```git clone --recurse-submodules -j2 git@github.com:ThFriedrich/riCOM_cpp.git``` 

**Generally build instructions can be followed step by step from the [automated build setup](https://github.com/ThFriedrich/riCOM_cpp/.github/workflows/build.yml) from command line**
## Build instructions Linux
### Dependencies 
- g++/gcc compiler version 11
  - ```sudo apt-get install gcc-11 g++-11```
- make & cmake (usually preinstalled on all distros)
- [SDL2](http://www.libsdl.org) : 
  - ```sudo apt-get install libsdl2-dev libsdl2-image-dev```
- FFTW3
  - ```sudo apt-get install libfftw3-dev``` 
  
Build script:
```bash
mkdir build
cd build
cmake --config Release ..
make
```

## Build instructions Windows
### Dependencies 
- [Visual Studio Community](https://visualstudio.microsoft.com/vs/community/) with C++ Desktop Development Kit or msbuild tools
  - including the msvc compiler, cmake and msbuild
- [SDL2](http://www.libsdl.org) 
  - Download 'SDL2-devel-2.0.xx-VC.zip' (Visual C++ 32/64-bit) from http://www.libsdl.org/download-2.0.php#source and [SDL2-image](http://www.libsdl.org/projects/SDL_image/release/SDL2_image-devel-2.0.5-VC.zip)
  - Both zip folder share a common sub-structure. Extract contents of the zips to 'C:/SDL2_VC', merging the contents of both zips into the same folder structure.
- FFTW
  - Download and extract [FFTW3 library](https://fftw.org/pub/fftw/fftw-3.3.5-dll64.zip)
  - run ```lib /def:libfftw3-3.def```

### Building from Command Line (**Use __Developer__ Powershell for VS**)
```bat
mkdir build
cd build
cmake ..
msbuild .\RICOM.vcxproj /p:configuration=Release
```
The executable RICOM.exe will be in the folder 'build\Release'. Make  sure **the file 'SDL2.dll' is in the same location as the executable**, or copy it to your system libraries, e.g. 'C:\Windows\System32' 


### Building from Command Line with **Intel** Compiler(**Use __Developer__ Powershell for VS**)
```bat
mkdir build
cd build
cmake -T "Intel C++ Compiler 2021" -DCMAKE_CXX_COMPILER="icx" ..
msbuild .\RICOM.vcxproj /p:configuration=Release
```
The executable RICOM.exe will be in the folder 'build\Release'. Make  sure **the files 'SDL2.dll' and 'libmmd.dll' are in the same location as the executable**, or copy them to your system libraries, e.g. 'C:\Windows\System32' 
