PEGASUS: Persistent Global Address Space for Universal Sharing       {#pegasuspage}
==============================================================

A PErsistent Global Address Space for Universal Sharing (PEGASUS) 
object (alps::AddressSpace) provides a shared address space between 
multiple worker processes.
The address space comprises multiple persistent memory regions, which are 
contiguous segments of the address space that are backed by fabric-attached 
memory (FAM). 
Each process has its own PEGASUS object instance that it can use to map 
and access shared persistent memory regions.
For emulation purposes, we also provide an implementation
of persistent memory regions on top of an in-memory file system (TMPFS).
 
Persistent regions (alps::Region) are instantiated by binding and 
mapping region files (alps::RegionFile) into the address space.
As each process may memory map the region file at a different location, 
each region type provides smart pointers (\ref SMARTPOINTERS) for 
referencing locations within the region in a manner that is independent 
of mapping address. 


