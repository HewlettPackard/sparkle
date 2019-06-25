ALPS: Allocation Layers for Persistent Shared Memory   {#mainpage}
====================================================

ALPS provides low-level abstraction layers that relief the user 
from the details of mapping, addressing, and allocating persistent shared 
memory (also known as fabric-attached memory).
These layers can be used as a building block for building higher-level 
allocators such as persistent heaps.

![ALPS](alps-layers.png) 
@image latex alps-layers.pdf "ALPS layers" width = 10cm

Currently, we provide two layers (or API classes). 
First, a [PErsistent Global Address Space for Universal Sharing (PEGASUS)](@ref pegasuspage)
layer provides a shared address space between multiple worker processes.
Second, a [Global Symmetric Heap](@ref globalheap-page) layer for allocating
variable-size chunks of shared persistent memory (a.k.a. fabric-attached memory).

The APIs strive to be as generic as possible. Thus, we do not hardcode policy 
but instead seek providing generic mechanisms that can be used to support 
higher level policies. 

## Example Programs

ALPS comes with several samples in the `examples` directory.

## This Document
 
This document is written in Doxygen and maintained in the ALPS git instance at:
 
[git://git-pa1.labs.hpecorp.net/ssftm/alps](git://git-pa1.labs.hpecorp.net/ssftm/alps)

## Generating the documentation 

We include the ALPS documentation as part of the source (as opposed to using 
a hosted wiki, such as the github wiki, as the definitive documentation) to 
enable the documentation to evolve along with the source code and be captured 
by revision control (currently git). This way the code automatically includes 
the version of the documentation that is relevant regardless of which version 
or release you have checked out or downloaded.
 
 @verbatim
 $ cd $ALPS/doc
 $ doxygen
 @endverbatim

## Stable download locations

The most recent version is published at: N/A

## Contact information
 
@author Haris Volos <haris.volos@hpe.com>
