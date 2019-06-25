# Installing ALPS on Cache-Coherent NUMA platforms

This guide provides instructions for building and installing ALPS on most
Linux Debian platforms running on a CC-NUMA x86 system.

This guide assumes the ALPS source code is located in directory $ALPS_SRC and
built into directory $ALPS_BUILD.

## Dependencies

* We assume Linux OS. Testing has been done on Ubuntu 16.04 on x86-64.
* We assume modern C/C++ compilers in the build environment that must 
  support C/C++11. Testing has been done with gcc version 4.9.2 or later. 
* We use the CMake family of build tools. Testing has been done with cmake version 3.0.2 or later.
* boost library
* bash 4.3

## Configuring Build Environment

Before building ALPS, please install any necessary packages by running the
accompanied script:

```
$ cd $ALPS_SRC
$ ./install-dep
```

Please note that the above script requires 'sudo' privilege rights.

## Compilation

It is recommended to build ALPS outside the source tree to avoid polluting the
tree with generated binaries.
So first create a build directory $ALPS_BUILD into which you will configure
and build ALPS:

```
$ mkdir $ALPS_BUILD
```

Prepare the ALPS build system:

```
$ cd $ALPS_BUILD
$ cmake $ALPS_SRC -DTARGET_ARCH_MEM=CC-NUMA
```

Build ALPS:

```
$ make
```

## Testing

Once ALPS is built, unit tests can be run using CTest.
To run unit tests against tmpfs (located at: /dev/shm) change into the build
directory and invoke CTest:

```
$ cd $ALPS_BUILD
ctest -R tmpfs
```

## Installation

To install ALPS binaries in default location:

```
$ make install
```

To install ALPS binaries in custom location:

```
$ make DESTDIR=MY_CUSTOM_LOCATION install
```

## Configuring Runtime Environment

ALPS has many operating parameters, which you can configure before starting a
program using ALPS.
ALPS initializes its internals by loading configuration options in the
following order:
* Load options from a system-wide configuration file: /etc/default/alps.[yml|yaml],
* Load options from the file referenced by the value of the environment
variable $ALPS_CONF, and
* Load options through a user-defined configuration file or object (passed
  through the ALPS API)
