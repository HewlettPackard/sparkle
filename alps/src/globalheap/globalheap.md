Global Symmetric Heap {#globalheap-page}
=====================

# Interface

We provide a Global Symmetric Heap that provides users (or programs) with a 
shared heap memory allocator (alps::GlobalHeap).
The allocator lets users allocate variable-size chunks of persistent shared 
memory (a.k.a fabric-attached memory) through the familiar malloc/free interface.

We extend the malloc interface to take an extra memory-attributes argument that
can be used to provide to the allocator a hint about the desirable properties 
of the allocated memory. For example, a user can use this hint to express 
locality requirements by stating the node (or interleave group) to allocate 
memory from.

The allocator interface uses relocatable pointers provided by the underlying 
PEGASUS layer.

The interface also provides limited support for fault tolerance. When opening
a heap, a user can ask for a generation number identifying the current instance
of the heap. The user can use this generation number to recover the specific 
heap instance after a crash.

# Utility Program

For convenience, we provide a utility program that can create a new heap, 
format an existing heap, and report usage statistics for an existing heap.

# Limitations

- No support for remote frees: 

# Heap Layout

We implement the Heap on top of a shared file that can be backed either by 
the L4TM Librarian File System (LFS) or TMPFS. Space for the file can be 
pre-allocated at creation time to avoid runtime overhead associated with 
space allocation. The Heap may also be partitioned into multiple files, which 
allows parallelizing and speeding up heap creation. 


![Heap layout in persistent memory](globalheap-layout.png) 
@image latex globalheap-layout.pdf "Heap layout in persistent memory" width = 10cm

Figure shows the layout of the heap. The heap is partitioned into fixed-size
zones, which can be partitioned into variable-size extents. Zones enable 
concurrent allocation by multiple processes: a process using the heap leases 
one or multiple zones for allocation. Zones also enable dynamically extending
the heap size (not implemented yet).

Extents enable variable-size allocation of contiguous regions of heap space. 
Extents are allocated are multiple of a configurable block size and can be as 
small as one block and nearly as large as a zone (some space is consumed by 
extent metadata).

A single-block extent can be managed as a slap, which further splits the block
into smaller same-size blocks to support allocation of objects smaller than 
the extent block with low fragmentation.

Per-process volatile metadata cache and speed access to the metadata of the 
persistent layout. For example, a volatile extent-tree tracks per-zone free 
extents by start address and size to speed locating free space. 


\example globalheap/root.cc
