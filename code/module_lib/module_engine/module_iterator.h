#ifndef __MODULE_ITERATOR__
#define __MODULE_ITERATOR__

#pragma once

#include "../module_public.h"

template<typename T>
class CModuleIterator
{
public:
    CModuleIterator( void ) {
        it = NULL;
    }
    CModuleIterator( T *object ) {
        it = object->begin();
    }
    CModuleIterator( CModuleIterator& other ) {
        it = other.it;
    }
    ~CModuleIterator() {
    }

    CModuleIterator& operator=( CModuleIterator& other ) {
        it = other.it;
        return *this;
    }
    bool operator==( const CModuleIterator& other ) const {
        return it == other.it;
    }
    bool operator!=( const CModuleIterator& other ) const {
        return it != other.it;
    }
    bool operator>=( const CModuleIterator& other ) const {
        return it >= other.it;
    }
    bool operator<=( const CModuleIterator& other ) const {
        return it <= other.it;
    }
    bool operator>( const CModuleIterator& other ) const {
        return it > other.it;
    }
    bool operator<( const CModuleIterator& other ) const {
        return it < other.it;
    }
    bool operator==( const CModuleConstIterator& other ) const {
        return it == other.it;
    }
    bool operator!=( const CModuleConstIterator& other ) const {
        return it != other.it;
    }
    bool operator>=( const CModuleConstIterator& other ) const {
        return it >= other.it;
    }
    bool operator<=( const CModuleConstIterator& other ) const {
        return it <= other.it;
    }
    bool operator>( const CModuleConstIterator& other ) const {
        return it > other.it;
    }
    bool operator<( const CModuleConstIterator& other ) const {
        return it < other.it;
    }

    bool valid( void ) const {
        return it != NULL;
    }

    typename T::reference value( void ) const {
        return *it;
    }
    typename T::const_reference value( void ) const {
        return *it;
    }

    CModuleIterator& operator+=( uint64_t num ) {
        it += num;
        return *this;
    }
    CModuleIterator& operator-=( uint64_t num ) {
        it -= num;
        return *this;
    }
    CModuleIterator& operator++( void ) {
        it++;
        return *this;
    }
    CModuleIterator& operator++( int ) {
        it++;
        return *this;
    }
    CModuleIterator& operator--( void ) {
        it--;
        return *this;
    }
    CModuleIterator& operator--( int ) {
        it--;
        return *this;
    }
private:
    typename T::iterator it;
};


template<typename T>
class CModuleConstIterator
{
public:
    CModuleConstIterator( void ) {
        it = NULL;
    }
    CModuleConstIterator( const T *object ) {
        it = object->cbegin();
    }
    CModuleConstIterator( CModuleConstIterator& other ) {
        it = other.it;
    }
    ~CModuleConstIterator() {
    }

    CModuleConstIterator& operator=( CModuleConstIterator& other ) {
        it = other.it;
        return *this;
    }
    bool operator==( const CModuleIterator& other ) const {
        return it == other.it;
    }
    bool operator!=( const CModuleIterator& other ) const {
        return it != other.it;
    }
    bool operator>=( const CModuleIterator& other ) const {
        return it >= other.it;
    }
    bool operator<=( const CModuleIterator& other ) const {
        return it <= other.it;
    }
    bool operator>( const CModuleIterator& other ) const {
        return it > other.it;
    }
    bool operator<( const CModuleIterator& other ) const {
        return it < other.it;
    }
    bool operator==( const CModuleConstIterator& other ) const {
        return it == other.it;
    }
    bool operator!=( const CModuleConstIterator& other ) const {
        return it != other.it;
    }
    bool operator>=( const CModuleConstIterator& other ) const {
        return it >= other.it;
    }
    bool operator<=( const CModuleConstIterator& other ) const {
        return it <= other.it;
    }
    bool operator>( const CModuleConstIterator& other ) const {
        return it > other.it;
    }
    bool operator<( const CModuleConstIterator& other ) const {
        return it < other.it;
    }
    bool valid( void ) const {
        return it != NULL;
    }

    typename T::const_reference value( void ) const {
        return *it;
    }

    CModuleConstIterator& operator+=( uint64_t num ) {
        it += num;
        return *this;
    }
    CModuleConstIterator& operator-=( uint64_t num ) {
        it -= num;
        return *this;
    }
    CModuleConstIterator& operator++( void ) {
        it++;
        return *this;
    }
    CModuleConstIterator& operator++( int ) {
        it++;
        return *this;
    }
    CModuleConstIterator& operator--( void ) {
        it--;
        return *this;
    }
    CModuleConstIterator& operator--( int ) {
        it--;
        return *this;
    }
private:
    typename T::const_iterator it;
};

using string_iterator = CModuleIterator<string_t>;
using const_string_iterator = CModuleConstIterator<string_t>;
using vector_iterator = CModuleIterator<CScriptArray>;
using const_vector_iterator = CModuleConstIterator<CScriptArray>;

#endif