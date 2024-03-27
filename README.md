<h1 align="center">USOS Discord Rich Presence</h1>

A simple C++ executable which shows up as an activity in [Discord](https://discord.com/) when run.
The activity contains information about current classes and/or lectures when linked to your [USOS](https://www.usos.edu.pl/) timetable.

## Building and running

Dependencies to install before building:
* [CMake](https://cmake.org/) ≥ 3.27
* Preferred build system, e.g. [Ninja](https://ninja-build.org/) or [GNU Make](https://www.gnu.org/software/make/)

To prepare the build, run the following commands:
```sh
$ mkdir build
$ cd build
$ cmake ..
```

Then, run your build system, still in the `build` directory, for example:
```sh
$ ninja
```

Finally, run the executable that was generated in the `bin` directory.
