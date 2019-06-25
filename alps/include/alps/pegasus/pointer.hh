/* 
 * (c) Copyright 2016 Hewlett Packard Enterprise Development LP
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _ALPS_PEGASUS_POINTER_HH_
#define _ALPS_PEGASUS_POINTER_HH_

#include <iostream>
#include <boost/type_traits.hpp>

#include "pegasus.hh"
#include "address_space.hh"
#include "addr.hh"

namespace alps {

/**
 * @defgroup SMARTPOINTERS Smart Pointers
 * @brief Smart pointers for referencing locations within persistent regions.
 *
 * Transient pointers are useful for temporarily referencing persistent memory
 * locations.
 * 
 * Persistent pointers are useful for persistently storing references in 
 * persistent regions (region files).
 *
 * A special null_ptr handle refers to the null pointer that points nowhere.
 *
 * INVARIANTS 
 * - Invariant 1: The virtual and linear persistent pointers do not cross regions.
 *   The invariant is met at runtime by ensuring that pointers never get assigned
 *   a memory location that is mapped to a different region.
 */


//#define SELF_RELATIVE_POINTERS
#define BASE_RELATIVE_POINTERS


#ifdef BASE_RELATIVE_POINTERS

static ZAddr null_ptr(-1, -1);

class BaseRelativePointer {
public:

/**
 * @brief Intermediate representation of a relocatable pointer
 * @ingroup SMARTPOINTERS
 * @details
 * Not a full-fledged smart pointer but rather an intermediate representation
 * that simplifies cross-assignment between transient and persistent pointers.
 */
template<typename RegionType, typename PointedType>
class IPtr {
public:

    IPtr(PointedType* from)
    {
        Region*    region;
        LinearAddr offset;
        void*      vaddr = reinterpret_cast<void*>(from);
        Pegasus::address_space()->rtrans(vaddr, &region, &offset);
        region_ = static_cast<RegionType*>(region);
        addr_.region_id_ = region->id();
        addr_.linear_addr_ = offset;
    } 

    IPtr(RegionType* pregion, LinearAddr offset)
    {
        region_ = pregion;
        addr_.region_id_ = pregion->id();
        addr_.linear_addr_ = offset;
    } 

    RegionType* region_;
    ZAddr       addr_;
};


/**
 * @brief Represents a transient pointer.
 * @ingroup SMARTPOINTERS
 * @details
 * Represents a smart transient pointer that points to a persistent logical 
 * address (i.e., the pointer stores a persistent logical address).
 * It may also be used as an intermediate pointer representation
 */
template<typename RegionType, typename PointedType>
class TPtr {
public:
    typedef PointedType*                  pointer;
    typedef typename boost::
        add_reference<PointedType>::type  reference; 

public:
    TPtr()
        : addr_(null_ptr)
    { }

    TPtr(ZAddr zaddr)
        : addr_(zaddr)
    { 
        region_ = static_cast<RegionType*>(Pegasus::address_space()->region(addr_.region_id_));
    }

    TPtr(RegionId region_id, LinearAddr offset)
    {
        region_ = static_cast<RegionType*>(Pegasus::address_space()->region(region_id));
        addr_.region_id_ = region_id;
        addr_.linear_addr_ = offset;
    } 

    TPtr(RegionType* pregion, LinearAddr offset)
    {
        region_ = pregion;
        addr_.region_id_ = pregion->id();
        addr_.linear_addr_ = offset;
    } 

    TPtr(const TPtr<RegionType,void>& other)
    {
        region_ = other.region_;
        addr_ = other.addr_;
    } 

    TPtr(const IPtr<RegionType, PointedType>& other)
    {
        region_ = other.region_;
        addr_ = other.addr_;    
    }

    TPtr(PointedType* from)
    {
        IPtr<RegionType, PointedType> iptr(from);
        region_ = iptr.region_;
        addr_ = iptr.addr_;
    } 

    operator IPtr<RegionType, PointedType>() const
    {
        return IPtr<RegionType, PointedType>(region_, addr_.linear_addr_);
    }

    TPtr& operator=(const IPtr<RegionType, PointedType>& other)
    {
        region_ = other.region_;
        addr_ = other.addr_;    
        return *this;
    }

    TPtr& operator=(const typename RegionType::template TPtr<PointedType> &tptr)
    {
        region_ = tptr.region_;
        addr_ = tptr.addr_;
        return *this;
    } 

    TPtr& operator=(PointedType* from)
    {
        IPtr<RegionType, PointedType> iptr(from);
        region_ = iptr.region_;
        addr_ = iptr.addr_;

        return *this;
    } 

    bool operator==(const ZAddr& other_zaddr) const {
        return addr_ == other_zaddr;
    }

    bool operator!=(const ZAddr& other_zaddr) const {
        return addr_ != other_zaddr;
    }

    operator ZAddr() const {
        return addr_;
    }

    // Obtains raw pointer from offset.
    pointer get() const
    {
        pointer ret = reinterpret_cast<pointer>(region_->trans(addr_.linear_addr_));
        return ret;
    }

    RegionId region_id() const 
    {
        return addr_.region_id_;
    }

    LinearAddr offset() const
    {
        return addr_.linear_addr_;
    }

    RegionType* region() 
    {
        return region_;
    }

    pointer operator->() const
    {
        pointer ret = this->get();
        return ret;
    }

    reference operator*() const
    {
        pointer p = this->get();
        reference r = *p;
        return r;
    }

    bool operator<(const TPtr rhs) const {
        return addr_.region_id_ == rhs.addr_.region_id_ && addr_.linear_addr_ < rhs.addr_.linear_addr_;
    }

    reference operator[](std::ptrdiff_t idx) const
    {
        return this->get()[idx];
    }

    void inc_offset(std::ptrdiff_t bytes) 
    {
        addr_.linear_addr_ += bytes;
    }

    void dec_offset(std::ptrdiff_t bytes) 
    {
        addr_.linear_addr_ -= bytes;
    }

    TPtr operator+(std::ptrdiff_t diff) const
    {
        TPtr tptr(this->region_, this->offset());
        tptr.inc_offset(sizeof(PointedType)*diff);
        return tptr;
    }

    std::ptrdiff_t operator-(const TPtr& other) const
    {
        return get() - other.get();
    }

    TPtr operator-(std::ptrdiff_t diff) const
    {
        TPtr tptr(this->region_, this->offset());
        tptr.dec_offset(sizeof(PointedType)*diff);
        return tptr;
    }

    TPtr& operator+=(std::ptrdiff_t diff)
    {
        this->inc_offset(sizeof(PointedType)*diff);
        return *this;
    }

    TPtr& operator-=(std::ptrdiff_t diff)
    {
        this->dec_offset(sizeof(PointedType)*diff);
        return *this;
    }

    TPtr& operator++()
    {
        this->inc_offset(sizeof(PointedType));
        return *this;
    }

    TPtr operator++(int)
    {
        TPtr tmp(*this);
        this->inc_offset(sizeof(PointedType));
        return tmp;
    }

    TPtr& operator--()
    {
        this->dec_offset(sizeof(PointedType));
        return *this;
    }

    TPtr operator--(int)
    {
        TPtr tmp(*this);
        this->dec_offset(sizeof(PointedType));
        return tmp;
    }

    //operator unspecified_bool_type() const;
    //bool operator!() const;

    void stream_to(std::ostream& os) const
    {
        os << addr_;
    }

public:

    RegionType* region_;
    ZAddr       addr_;
};


/**
 * @brief Represents a linear persistent pointer.
 * @ingroup SMARTPOINTERS
 *
 * @details
 * Represents a smart persistent pointer that points to a persistent logical 
 * linear address (i.e., the pointer stores a persistent logical linear address).
 * The pointer is valid for exchanging and sharing a reference to 
 * memory location among multiple processes mapping a region.
 * It works with both fixed virtual address mappings and relocatable regions. 
 * It does not support inter-region pointers.
 */
template<typename RegionType, typename PointedType>
class PPtr {
public:
    typedef PointedType*                  pointer;
    typedef typename boost::
        add_reference<PointedType>::type  reference; 

public:

    PPtr(LinearAddr offset)
    {
        addr_.linear_addr_ = offset;
    } 

    PPtr(const PPtr<RegionType, void>& other)
    {
        addr_ = other.addr_;
    } 

    PPtr(IPtr<RegionType, PointedType>& other)
    {
        addr_.linear_addr_ = other.addr_.linear_addr_; 
    }

    operator IPtr<RegionType, PointedType>()
    {
        // to find the region the persistent pointer is stored to we create
        // an intermediate pointer that points to this (persistent pointer)
        IPtr<RegionType, PointedType> iptr((PointedType*) this);
        return IPtr<RegionType, PointedType>(iptr.region_, addr_.linear_addr_);
    }

    PPtr& operator=(PointedType* from)
    {
        IPtr<RegionType, PointedType> iptr(from);
        addr_.linear_addr_ = iptr.addr_.linear_addr_;

        return *this;
    } 

    PPtr& operator=(const IPtr<RegionType, PointedType>& other)
    {
        addr_.linear_addr_ = other.addr_.linear_addr_; 
        return *this;
    }

    PPtr& operator=(const TPtr<RegionType, PointedType> &tptr)
    {
        addr_.linear_addr_ = tptr.addr_.linear_addr_;
        return *this;
    } 

    // Obtains raw pointer from offset.
    pointer get() const
    {
        // to find the region the persistent pointer is stored to we create
        // an intermediate pointer that points to this (persistent pointer)
        IPtr<RegionType, PointedType> iptr((PointedType*) this);
        RegionType* region = iptr.region_;

        // translate the persistent logical address 
        return reinterpret_cast<pointer>(region->trans(addr_.linear_addr_));
    }

    pointer operator->() const
    {
        pointer ret = this->get();
        return ret;
    }

    reference operator*() const
    {
        pointer p = this->get();
        reference r = *p;
        return r;
    }

    bool operator==(const IPtr<RegionType, PointedType>& other_iptr) const {
        return addr_.linear_addr_ == other_iptr.addr_.linear_addr_;
    }

    bool operator!=(const IPtr<RegionType, PointedType>& other_iptr) const {
        return !(*this == other_iptr);
    }

    reference operator[](std::ptrdiff_t idx) const
    {
        return this->get()[idx];
    }

    TPtr<RegionType, PointedType> operator+(std::ptrdiff_t offset) const
    {
        TPtr<RegionType, PointedType> tptr(this->get() + offset);
        return tptr;
    }

    // following operators not implemented yet

    //PPtr operator-(std::ptrdiff_t) const;
    //PPtr & operator+=(std::ptrdiff_t) ;
    //PPtr & operator-=(std::ptrdiff_t) ;
    //PPtr & operator++(void) ;
    //PPtr operator++(int) ;
    //PPtr & operator--(void) ;
    //PPtr operator--(int) ;
    //operator unspecified_bool_type() const;
    //bool operator!() const;

    void stream_to(std::ostream& os) const
    {
        os << addr_;
    }

//private:
    PAddr addr_;
};

template<typename RegionType, typename PointedType>
class ZPtr {
public:
    typedef PointedType*                  pointer;
    typedef typename boost::
        add_reference<PointedType>::type  reference; 

public:
    ZPtr()
        : addr_(null_ptr)
    { }

    ZPtr(ZAddr zaddr)
        : addr_(zaddr)
    { }

    ZPtr(RegionId region_id, LinearAddr offset)
    {
        addr_.region_id_ = region_id;
        addr_.linear_addr_ = offset;
    } 

    ZPtr(const ZPtr<RegionType, void>& other)
    {
        addr_ = other.addr_;
    } 

    ZPtr(const typename RegionType::template TPtr<PointedType> &tptr)
    {
        addr_ = tptr.addr_;
    }

    ZPtr& operator=(const typename RegionType::template TPtr<PointedType> &tptr)
    {
        addr_ = tptr.addr_;
        return *this;
    }

    operator TPtr<RegionType, PointedType>()
    {
        return TPtr<RegionType, PointedType>(addr_);
    }

    ZAddr addr_;
};


};


template<typename RegionType, typename PointedType>
inline std::ostream& operator<<(std::ostream& os, const BaseRelativePointer::TPtr<RegionType,PointedType>& tptr)
{
    tptr.stream_to(os);
    return os;
}




template<typename RegionType, typename PointedType>
inline std::ostream& operator<<(std::ostream& os, const BaseRelativePointer::PPtr<RegionType,PointedType>& pptr)
{
    pptr.stream_to(os);
    return os;
}

#endif
 

#ifdef SELF_RELATIVE_POINTERS

/*
 * Adapted from boost/interprocess/offset_ptr.hpp
 */

/*
 * Boost Software License - Version 1.0 - August 17th, 2003

 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */


// define offset 1 as the null pointer, meaning that this class can't point to 
// the byte after its own this pointer:
// don't choose the offset 0 (that is, an offset_ptr pointing to itself) as the 
// null pointer pointer representation but this is not valid for many use cases 
// since many times structures like linked lists or nodes from STL containers 
// point to themselves
static PAddr null_ptr;


template<class RawPointer>
class pointer_size_t_caster
{
   public:
   explicit pointer_size_t_caster(std::size_t sz)
      : m_ptr(reinterpret_cast<RawPointer>(sz))
   {}

   explicit pointer_size_t_caster(RawPointer p)
      : m_ptr(p)
   {}

   std::size_t size() const
   {   return reinterpret_cast<std::size_t>(m_ptr);   }

   RawPointer pointer() const
   {   return m_ptr;   }

   private:
   RawPointer m_ptr;
};


template<int Dummy>
void * offset_ptr_to_raw_pointer(const volatile void *this_ptr, std::size_t offset)
{
  typedef pointer_size_t_caster<void*> caster_t;
  #ifndef BOOST_INTERPROCESS_OFFSET_PTR_BRANCHLESS_TO_PTR
     if(offset == 1){
        return 0;
     }
     else{
        const caster_t caster((void*)this_ptr);
        return caster_t(caster.size() + offset).pointer();
     }
  #else
     const caster_t caster((void*)this_ptr);
     return caster_t((caster.size() + offset) & -std::size_t(offset != 1)).pointer();
  #endif
}

template<int Dummy>
std::size_t offset_ptr_to_offset(const volatile void *ptr, const volatile void *this_ptr)
{
  typedef pointer_size_t_caster<void*> caster_t;
  #ifndef BOOST_INTERPROCESS_OFFSET_PTR_BRANCHLESS_TO_OFF
     //offset == 1 && ptr != 0 is not legal for this pointer
     if(!ptr){
        return 1;
     }
     else{
        const caster_t this_caster((void*)this_ptr);
        const caster_t ptr_caster((void*)ptr);
        std::size_t offset = ptr_caster.size() - this_caster.size();
        BOOST_ASSERT(offset != 1);
        return offset;
     }
  #else
     const caster_t this_caster((void*)this_ptr);
     const caster_t ptr_caster((void*)ptr);
     //std::size_t other = -std::size_t(ptr != 0);
     //std::size_t offset = (ptr_caster.size() - this_caster.size()) & other;
     //return offset + !other;
     //
     std::size_t offset = (ptr_caster.size() - this_caster.size() - 1) & -std::size_t(ptr != 0);
     return ++offset;
  #endif
}

template<int Dummy>
std::size_t offset_ptr_to_offset_from_other
  (const volatile void *this_ptr, const volatile void *other_ptr, std::size_t other_offset)
{
  typedef pointer_size_t_caster<void*> caster_t;
  #ifndef BOOST_INTERPROCESS_OFFSET_PTR_BRANCHLESS_TO_OFF_FROM_OTHER
  if(other_offset == 1){
     return 1;
  }
  else{
     const caster_t this_caster((void*)this_ptr);
     const caster_t other_caster((void*)other_ptr);
     std::size_t offset = other_caster.size() - this_caster.size() + other_offset;
     BOOST_ASSERT(offset != 1);
     return offset;
  }
  #else
  const caster_t this_caster((void*)this_ptr);
  const caster_t other_caster((void*)other_ptr);
  return ((other_caster.size() - this_caster.size()) & -std::size_t(other_offset != 1)) + other_offset;
  #endif
}



template<typename RegionType, typename PointedType>
class SelfRelativePtr {
public:
    typedef PointedType*                  pointer;
    typedef typename boost::
        add_reference<PointedType>::type  reference; 
    typedef LinearAddr                    OffsetType;

public:
    SelfRelativePtr()
    {
        addr_ = null_ptr;
    }

    // Constructor from linear addr
    SelfRelativePtr(PAddr addr)
    {
        addr_.linear_addr_ = addr.linear_addr_;
    } 

    // Constructor from region and linear addr
    // FIXME: SelfRelativePtr does not support pointer between 
    // regions (at least for now) so we ignore region
    SelfRelativePtr(RegionType* region, PAddr addr)
    {
        addr_.linear_addr_ = addr.linear_addr_;
    } 

    // Constructor from raw pointer
    SelfRelativePtr(pointer ptr)
    {
        addr_.linear_addr_ = static_cast<OffsetType>(offset_ptr_to_offset<0>(ptr, this));
    } 

    // Constructor from other pointer
    template<typename T>
    SelfRelativePtr(T* ptr)
    {
        addr_.linear_addr_ = static_cast<OffsetType>(offset_ptr_to_offset<0>(static_cast<PointedType*>(ptr), this));
    } 

    // Constructor from other offset_ptr
    SelfRelativePtr(const SelfRelativePtr& ptr)
    {
        addr_.linear_addr_ = static_cast<OffsetType>(offset_ptr_to_offset_from_other<0>(this, &ptr, ptr.addr_.linear_addr_));
    } 

    // Constructor from other offset_ptr. If pointers of pointee types are
    // convertible, offset_ptrs will be convertibles. 
    template<class T2>
    SelfRelativePtr(const SelfRelativePtr<RegionType,T2>& ptr)
    {
        addr_.linear_addr_ = static_cast<OffsetType>(offset_ptr_to_offset_from_other<0>(this, &ptr, ptr.offset()));
    }

    // Assignment from pointer (saves extra conversion).
    SelfRelativePtr& operator=(pointer from)
    {
        this->addr_.linear_addr = static_cast<OffsetType>(offset_ptr_to_offset<0>(from, this));

        return *this;
    } 

    //!Assignment from other SelfRelativePtr.
    SelfRelativePtr& operator=(const SelfRelativePtr& other)
    {
        this->addr_.linear_addr_ = static_cast<OffsetType>(offset_ptr_to_offset_from_other<0>(this, &other, other.addr_.linear_addr_));

        return *this;
    }

    // Obtains raw pointer from offset.
    pointer get() const
    {
        return (pointer)offset_ptr_to_raw_pointer<0>(this, this->addr_.linear_addr_);
    }

    LinearAddr offset() const
    {
        return addr_.linear_addr_;
    }

    // FIXME: region is not valid for self relative pointers but we support this so that user code compiles...
    RegionType* region() 
    {
        return NULL;
    }

    pointer operator->() const
    {
        pointer ret = this->get();
        return ret;
    }

    reference operator*() const
    {
        pointer p = this->get();
        reference r = *p;
        return r;
    }

    bool operator==(const PAddr& other) const {
        return addr_.linear_addr_ == other.linear_addr_;
    }

    bool operator!=(const PAddr& other) const {
        return !(*this == other);
    }

    bool operator==(const SelfRelativePtr& other) const {
        return addr_.linear_addr_ == other.addr_.linear_addr_;
    }

    bool operator!=(const SelfRelativePtr& other) const {
        return !(*this == other);
    }

    reference operator[](std::ptrdiff_t idx) const
    {
        return this->get()[idx];
    }

    SelfRelativePtr operator+(std::ptrdiff_t offset) const
    {
        SelfRelativePtr ptr(this->get() + offset);
        return ptr;
    }

    std::ptrdiff_t operator-(const SelfRelativePtr &other) const
    {
        return std::uintptr_t(get()) - std::uintptr_t (other.get());
    }

    operator PAddr() 
    {
        return addr_;
    }

    // following operators not implemented yet

    //PPtr operator-(std::ptrdiff_t) const;
    //PPtr & operator+=(std::ptrdiff_t) ;
    //PPtr & operator-=(std::ptrdiff_t) ;
    //PPtr & operator++(void) ;
    //PPtr operator++(int) ;
    //PPtr & operator--(void) ;
    //PPtr operator--(int) ;
    //operator unspecified_bool_type() const;
    //bool operator!() const;

    void stream_to(std::ostream& os) const
    {
        //os << addr_;
        os << get();
    }

//private:
    PAddr addr_;

};

template<typename RegionType, typename PointedType>
inline std::ostream& operator<<(std::ostream& os, const SelfRelativePtr<RegionType,PointedType>& ptr)
{
    ptr.stream_to(os);
    return os;
}

template<typename RegionType, typename PointedType>
using TPtr = SelfRelativePtr<RegionType, PointedType>;

template<typename RegionType, typename PointedType>
using PPtr = SelfRelativePtr<RegionType, PointedType>;


#endif

#if 0
/**
 * @brief Represents a Zeta (128-bit) far/fat linear persistent pointer.
 * @ingroup SMARTPOINTERS
 * @details
 * Represents a smart Zeta (128-bit) linear persistent pointer that can 
 * point to a persistent logical linear address located in a different region.
 * It does support inter-region pointers.
 */
template<typename RegionType, typename PointedType>
class ZPtr {
public:
    typedef PointedType*                  pointer;
    typedef typename boost::
        add_reference<PointedType>::type  reference; 

public:
    ZPtr()
        : addr_(null_ptr)
    { }

    ZPtr(ZAddr zaddr)
        : addr_(zaddr)
    { }

    ZPtr(RegionId region_id, LinearAddr offset)
    {
        addr_.region_id_ = region_id;
        addr_.linear_addr_ = offset;
    } 

    ZPtr(const ZPtr<RegionType, void>& other)
    {
        addr_ = other.addr_;
    } 

    ZPtr(const typename RegionType::template TPtr<PointedType> &tptr)
    {
        addr_ = tptr.addr_;
    }

    ZPtr& operator=(const typename RegionType::template TPtr<PointedType> &tptr)
    {
        addr_ = tptr.addr_;
        return *this;
    }

    operator TPtr<RegionType, PointedType>()
    {
        return TPtr<RegionType, PointedType>(addr_);
    }

    ZAddr addr_;
};

#endif

} // namespace alps

#endif // _ALPS_PEGASUS_POINTER_HH_
