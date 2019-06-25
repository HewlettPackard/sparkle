# ALPS: Allocator Layers for Persistent Shared Memory

Author:  Haris Volos (haris.volos@hpe.com)

## Description

![ALPS](doc/figures/alps-logo.png)

ALPS provides a collection of low-level abstraction layers that relief 
the user from the details of mapping, addressing, and allocating persistent 
shared memory.
These layers can be used as building blocks for building higher-level 
memory allocators such as persistent heaps.
Shared persistent memory refers to non-volatile memory shared among
multiple compute nodes and can take different forms, such as
disaggregated non-volatile memory accessible over a fabric, also
known as fabric-attached memory (FAM).

The main abstraction provided by ALPS is a Global Symmetric Heap that
lets users allocate variable-size chunks of persistent memory through
the familiar `malloc()/free()` interface that can be shared among
multiple concurrent processes.  For example, we extended Spark with
an optimized in-memory shuffle implementation where workers produce
and consume data directly from the shared heap instead of exchanging
data through network I/O.

## Master Source

The multi-platform version of ALPS providing the global symmetric heap 
for several platforms, including CC-NUMA and FAM systems, is available 
in the `fam` branch of the `alps` repository. 
Please make sure you switch to the `fam` branch.

Multi-platform branch: https://github.com/hvolos/alps/tree/fam

## Maturity

Alpha release

## Dependencies

- Install additional packages

  ```
  $ sudo apt-get install build-essential cmake libboost-all-dev
  ```

- Install libpmem

  ```
  $ sudo apt-get install autoconf pkg-config doxygen graphviz
  $ git clone https://github.com/FabricAttachedMemory/nvml.git
  $ cd nvml
  $ make
  $ sudo make install
  ```

- Install libfam-atomic

  ```
  $ cd ~
  $ sudo apt-get install autoconf autoconf-archive libtool
  $ sudo apt-get --no-install-recommends install asciidoc xsltproc xmlto
  $ git clone https://github.com/FabricAttachedMemory/libfam-atomic.git
  $ cd libfam-atomic
  $ bash autogen.sh
  $ ./configure
  $ make
  $ sudo make install
  ```

- Setup [FAME](https://github.com/HewlettPackard/mdc-toolkit/blob/master/guide-FAME.md) if you want to try ALPS on top of FAM.

## Build & Test

1. Download the source code:

 ```
 $ git clone https://github.com/hvolos/alps.git
 $ git checkout -t origin/fam
 ```

2. Change into the source directory (assuming the code is at directory $NVMM):

 ```
 $ cd $ALPS
 ```

3. Build

 On CC-NUMA systems:

 ```
 $ mkdir build
 $ cd build
 $ cmake .. -DTARGET_ARCH_MEM=CC-NUMA
 $ make
 ```

 On FAME:

 ```
 $ mkdir build
 $ cd build
 $ cmake .. -DTARGET_ARCH_MEM=NV-NCC-FAM
 $ make
 ```

 The default build type is Release. To switch between Release and Debug:
 ```
 $ cmake .. -DCMAKE_BUILD_TYPE=Release
 $ cmake .. -DCMAKE_BUILD_TYPE=Debug
 ```

4. Test

 On CC-NUMA systems:
 
 ```
 $ ctest -R tmpfs
 ```

 On FAME:

 ```
 $ ctest -R lfs
 ```

## Usage

The interface exposes a _shared heap_ abstraction that provides users
with a shared heap memory allocator.  The allocator lets users
allocate variable-size chunks of physically shared memory through the
familiar `malloc()/free()` interface.  We extend the `malloc`
interface to take an extra memory-attributes argument that can be
used to provide to the allocator a hint about the desirable
properties of the allocated memory.  For example, a user can use this
hint to express locality requirements by stating the NUMA node to
allocate memory from:

```
void* malloc(size_t size, int numa_node_hint);
```

The allocator interface uses relocatable base-relative C++ smart
pointers for logically addressing (naming) allocated blocks.  In
contrast to virtual addresses, our pointers are relocatable meaning
that the shared heap does not have to be memory mapped into the same
virtual address range in each process using the heap.  Instead, a
logical address represents an offset relative to the base of the
virtual address region that maps the shared heap so that the heap can
be memory mapped to different virtual address regions in each
process.

The interface also provides support for fault tolerance necessary to
big-data analytic frameworks.  When opening a heap, a user can ask
for a generation number identifying the current instance of the heap.
Blocks associated with a generation number can be released in two
ways: (1) explicit call to free the memory block, or (2) a bulk-free
call that frees all the blocks associated with a given generation.
Deallocating memory through a bulk-free is useful for releasing
memory when recovering from a crash.

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

Deploying and using ALPS on a platform based on the Librarian File System (LFS)
and Fabric-Attached Memory (FAM) requires a properly configured environment.
The environment is set up through the use of ALPS configuration files.
One can either use [Holodeck](http://github.hpe.com/labs/holodeck)
to automatically configure the environment or configure it manually.

To manually configure, set the following configuration options in
a per-node configuration file (preferably the system-wide configuration
file):

```
LfsOptions:
  node: {{ lfs_node }}
  node_count: {{ lfs_node_count }}
  book_size_bytes: {{ lfs_book_size_bytes }}
```

These options **must** match the ones used by the Librarian Server and
Client running on the respective node.

## Example Programs

ALPS comes with several samples in the `examples` directory.

## Style Guide

We follow the Google C++ style guide available here:

https://google.github.io/styleguide/cppguide.html

## Notes:

No support for remote free: i.e., a process cannot free memory
allocated by another concurrently active process.

## See Also:

- [Managed Data Structures (MDS)](https://github.com/HewlettPackard/mdc-toolkit/blob/master/README-MDS.md)
- [Multi-Process Garbage Collector (MPGC)](https://github.com/HewlettPackard/mdc-toolkit/blob/master/README-MPGC.md)
