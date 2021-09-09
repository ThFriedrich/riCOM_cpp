[![BUILD](https://github.com/ThFriedrich/riCOM_cpp/actions/workflows/build.yml/badge.svg)](https://github.com/ThFriedrich/riCOM_cpp/actions/workflows/build.yml)

# Build instructions Linux
### Dependencies 
- g++/gcc compiler (usually preinstalled on all distros)
- make & cmake (usually preinstalled on all distros)
- [SDL2](http://www.libsdl.org)  (possibly preinstalled): 
  - Ubuntu: ```sudo apt-get install libsdl2-dev``` 
  - CentOS / Fedora: ```yum install SDL2-devel``` 
  
Build script:
```bash
mkdir build
cd build
cmake --config Release ..
make
```

# Build instructions Windows
### Dependencies 
- [Visual Studio Community](https://visualstudio.microsoft.com/vs/community/) with C++ Desktop Development Kit
  - including the msvc compiler, cmake and msbuild
- [SDL2](http://www.libsdl.org) 
  - Download 'SDL2-devel-2.0.xx-VC.zip' (Visual C++ 32/64-bit) from http://www.libsdl.org/download-2.0.php#source
  - Extract contents of the zip to 'C:/SDL2_VC'

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
