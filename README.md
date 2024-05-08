<h1 align="center">USOS Discord Rich Presence</h1>

A simple C++ console executable which shows up as an activity in [Discord](https://discord.com/) when run.
The activity contains information about current classes and/or lectures when linked to your [USOS](https://www.usos.edu.pl/) timetable.

## Pre-built binaries

### Full releases

Go to the [latest GitHub release](https://github.com/Kacper0510/usos-rpc/releases/latest) and there you will find executable files
ready to be run, both for Windows and Linux.

To get help with the program's functionality, type the following command into your terminal:
```sh
$ usos-rpc --help
```

### Development builds

Alternatively, if you want to try out the latest features, which might or might not even work, check out
[GitHub Actions](https://github.com/Kacper0510/usos-rpc/actions). They are run for every new commit and contain
artifacts with appropriate executables.

## Building the project yourself

Dependencies to install before building:
* [CMake](https://cmake.org/) ≥ 3.22
* Preferred build system, e.g. [Ninja](https://ninja-build.org/) or [GNU Make](https://www.gnu.org/software/make/)
* Preferred C/C++ compiler, e.g. [GCC](https://gcc.gnu.org/) ≥ 13 or [MSVC](https://learn.microsoft.com/en-us/cpp/) ≥ 17.2

First, clone the repository to your hard drive:
```sh
$ git clone https://github.com/Kacper0510/usos-rpc
$ cd usos-rpc
```

To prepare the build, run the following commands:
```sh
usos-rpc$ mkdir build
usos-rpc$ cd build
usos-rpc/build$ cmake ..
```

Then, run your build system, for example:
```sh
usos-rpc/build$ ninja usos-rpc
# ------------------- OR -------------------
usos-rpc/build$ cmake --build . -t usos-rpc
```

Finally, run the executable that was generated in the `bin` directory.
