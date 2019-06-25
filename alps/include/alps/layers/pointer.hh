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

#ifndef _ALPS_LAYER_POINTER_HH_
#define _ALPS_LAYER_POINTER_HH_

#include <stdint.h>
#include <iostream>


namespace alps {

namespace detail {

//
// We can't filter out rvalue_references at the same level as
// references or we get ambiguities from msvc:
//

template <typename T>
struct add_reference_impl
{
    typedef T& type;
};

template <typename T>
struct add_reference_impl<T&&>
{
    typedef T&& type;
};

} // namespace detail

template <class T> struct add_reference
{
   typedef typename alps::detail::add_reference_impl<T>::type type;
};
template <class T> struct add_reference<T&>
{
   typedef T& type;
};

// these full specialisations are always required:
template <> struct add_reference<void> { typedef void type; };
template <> struct add_reference<const void> { typedef void type; };
template <> struct add_reference<const volatile void> { typedef void type; };
template <> struct add_reference<volatile void> { typedef void type; };




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

/**
 * @brief Represents a transient pointer.
 * @ingroup SMARTPOINTERS
 * @details
 * Represents a smart transient pointer that points to a persistent logical 
 * address (i.e., the pointer stores a persistent logical address).
 * It may also be used as an intermediate pointer representation
 */
template<typename PointedType>
class TPtr {
public:
    typedef PointedType*           pointer;
    typedef typename alps::
        add_reference<PointedType>::type  reference; 

public:
    // Constructor from raw pointer
    TPtr(pointer ptr = 0)
    { 
        addr_ = reinterpret_cast<uintptr_t>(ptr);
    }

    // Constructor from other pointer
    template<typename T>
    TPtr(T* from)
    {
        addr_ = reinterpret_cast<uintptr_t>(from);
    } 

    // Constructor from other TPtr
    TPtr(const TPtr<PointedType>& other)
    {
        addr_ = other.addr_;    
    }

    // Constructor from other TPtr. If pointers of pointee types are 
    // convertible, TPtrs will be convertibles.
    template<typename T>
    TPtr(const TPtr<T>& other)
    {
        addr_ = other.addr_;    
    }

    TPtr& operator=(const TPtr<PointedType> &other)
    {
        addr_ = other.addr_;
        return *this;
    } 

    TPtr& operator=(PointedType* from)
    {
        addr_ = reinterpret_cast<uintptr_t>(from);
        return *this;
    } 

    // Obtains raw pointer
    pointer get() const
    {
        return reinterpret_cast<pointer>(addr_);
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

    reference operator[](std::ptrdiff_t idx) const
    {
        return this->get()[idx];
    }

    void inc_offset(std::ptrdiff_t bytes) 
    {
        addr_ += bytes;
    }

    void dec_offset(std::ptrdiff_t bytes) 
    {
        addr_ -= bytes;
    }

    TPtr operator+(std::ptrdiff_t diff) const
    {
        TPtr tptr(*this);
        tptr.inc_offset(sizeof(PointedType)*diff);
        return tptr;
    }

    std::ptrdiff_t operator-(const TPtr& other) const
    {
        return get() - other.get();
    }

    TPtr operator-(std::ptrdiff_t diff) const
    {
        TPtr tptr(*this);
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
    bool operator!() const {
        return this->get() == 0;
    }

    void stream_to(std::ostream& os) const
    {
        os << addr_;
    }

public:
    uintptr_t addr_;
};

// TPtr<T1> == TPtr<T2>.
template<class T1, class T2>
inline bool operator== (const TPtr<T1> &pt1, 
                        const TPtr<T2> &pt2)
{  
    return pt1.get() == pt2.get();  
}

// TPtr<T1> != TPtr<T2>.
template<class T1, class T2>
inline bool operator!= (const TPtr<T1> &pt1, 
                        const TPtr<T2> &pt2)
{  
    return pt1.get() != pt2.get();  
}

/// TPtr<T1> < TPtr<T2>.
template<class T1, class T2>
inline bool operator< (const TPtr<T1> &pt1, 
                       const TPtr<T2> &pt2)
{  
    return pt1.get() < pt2.get();  
}

/// TPtr<T1> <= TPtr<T2>.
template<class T1, class T2>
inline bool operator<= (const TPtr<T1> &pt1, 
                        const TPtr<T2> &pt2)
{  
    return pt1.get() <= pt2.get();  
}

/// TPtr<T1> > TPtr<T2>.
template<class T1, class T2>
inline bool operator> (const TPtr<T1> &pt1, 
                       const TPtr<T2> &pt2)
{  
    return pt1.get() > pt2.get();  
}

/// TPtr<T1> >= TPtr<T2>.
template<class T1, class T2>
inline bool operator>= (const TPtr<T1> &pt1, 
                        const TPtr<T2> &pt2)
{  
    return pt1.get() >= pt2.get();  
}

//! operator<<
template<class E, class T, class Y> 
inline std::basic_ostream<E, T> & operator<< 
   (std::basic_ostream<E, T> & os, TPtr<Y> const & p)
{  
    return os << p.get();   
}

const TPtr<void> null_ptr = 0;


template<typename PointedType>
using PPtr = TPtr<PointedType>;

} // namespace alps

#endif // _ALPS_LAYER_POINTER_HH_
