=====================
1. COPYRIGHT
=====================
Copyright (C) 2016-2021
This file is part of the Hierarchical Data Format query language (HDFql)
For more information about HDFql, please visit the website http://www.hdfql.com


=====================
2. DESCRIPTION
=====================
HDFql stands for "Hierarchical Data Format query language" and is the first
tool that enables users to manage HDF5 files through a high-level language.
This language was designed to be simple to use and similar to SQL thus
dramatically reducing the learning effort. HDFql can be seen as an alternative
to the C API (which contains more than 400 low-level functions that are far
from easy to use!) and to existing wrappers for C++, Java, Python, C#, Fortran
and R for manipulating HDF5 files. In addition, and whenever possible, it
automatically employs parallelism to speed-up operations hiding its inherent
complexity from the user.

As an example, imagine that one needs to create an HDF5 file named "my_file.h5"
and, inside it, a group named "my_group" containing a one dimensional (size 3)
dataset named "my_dataset" of data type integer. Additionally, the dataset is
compressed using ZLIB and initialized with values 4, 8 and 6. In HDFql, this
can easily be implemented as follows:

   create and use file my_file.h5
   create dataset my_group/my_dataset as int(3) enable zlib values(4, 8, 6)

In contrast, using the C API on the same example is quite cumbersome:

   hid_t file;
   hid_t group;
   hid_t dataspace;
   hid_t property;
   hid_t dataset;
   hsize_t dimension;
   int value[3];
   file = H5Fcreate("my_file.h5", H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT);
   group = H5Gcreate(file, "my_group", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
   dimension = 3;
   dataspace = H5Screate_simple(1, &dimension, NULL);
   property = H5Pcreate(H5P_DATASET_CREATE);
   H5Pset_chunk(property, 1, &dimension);
   H5Pset_deflate(property, 9);
   dataset = H5Dcreate(group, "my_dataset", H5T_NATIVE_INT, dataspace, H5P_DEFAULT, property, H5P_DEFAULT);
   value[0] = 4;
   value[1] = 8;
   value[2] = 6;
   H5Dwrite(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &value);


=====================
3. ORGANIZATION
=====================
This distribution is organized as follows:

   HDFql-x.y.z
        |
        + example (contains C, C++, Java, Python, C#, Fortran and R examples)
        |
        + include (contains HDFql C and C++ header files)
        |
        + lib (contains HDFql C static and shared libraries)
        |
        + bin (contains HDFql command-line interface and a proper launcher)
        |
        + plugin (contains plugins used by HDFql)
        |
        + wrapper (contains HDFql wrappers)
        |    |
        |    + cpp (contains HDFql C++ wrapper)
        |    |
        |    + java (contains HDFql Java wrapper)
        |    |
        |    + python (contains HDFql Python wrapper)
        |    |
        |    + csharp (contains HDFql C# wrapper)
        |    |
        |    + fortran (contains HDFql Fortran wrapper)
        |    |
        |    + R (contains HDFql R wrapper)
        |
        + doc (contains HDFql reference manual)
        |
        - LICENSE.txt (file that contains information about HDFql license)
        |
        - RELEASE.txt (file that contains information about HDFql releases)
        |
        - README.txt (refers to the present file)


=====================
4. USAGE
=====================
4.1 Programmatically
HDFql can be used in C, C++ and Fortran programs through static and shared
libraries. These libraries are stored in the directory "lib" for C, and
directories "cpp" and "fortran" (which are under the directory "wrapper") for
C++ and Fortran respectively. Additionally, HDFql can be used in Java, Python,
C# and R programs through appropriate wrappers. These are stored in the
directories "java", "python", "csharp" and "R" (which are under the directory
"wrapper"). Examples on how to use HDFql in all these programming languages can
be found in the directory "example". For a complete documentation on how to use
HDFql, please visit the website http://www.hdfql.com/#documentation.

4.2 Command-Line Interface
A command-line interface named "HDFqlCLI" is available and can be used for
manipulating HDF5 files. It is stored in the directory "bin". To launch the
command-line interface, open a terminal ("cmd" if in Windows, "xterm" if in
Linux, or "Terminal" if in macOS), go to the directory "bin", and type
"HDFqlCLI" (if in Windows) or "./HDFqlCLI" (if in Linux/macOS). The list of
parameters accepted by the command-line interface can be seen by launching it
with the parameter "--help".

