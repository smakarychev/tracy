#ifndef __TRACYSLAB_HPP__
#define __TRACYSLAB_HPP__

#include <assert.h>
#include <vector>

#include "TracyMemory.hpp"

namespace tracy
{

template<size_t BlockSize>
class Slab
{
public:
    Slab()
        : m_ptr( new char[BlockSize] )
        , m_offset( 0 )
        , m_buffer( { m_ptr } )
    {
        memUsage.fetch_add( BlockSize, std::memory_order_relaxed );
    }

    ~Slab()
    {
        memUsage.fetch_sub( BlockSize * m_buffer.size(), std::memory_order_relaxed );
        for( auto& v : m_buffer )
        {
            delete[] v;
        }
    }

    void* AllocRaw( size_t size )
    {
        assert( size <= BlockSize );
        if( m_offset + size > BlockSize )
        {
            m_ptr = new char[BlockSize];
            m_offset = 0;
            m_buffer.emplace_back( m_ptr );
            memUsage.fetch_add( BlockSize, std::memory_order_relaxed );
        }
        void* ret = m_ptr + m_offset;
        m_offset += size;
        return ret;
    }

    template<typename T>
    T* AllocInit()
    {
        return new( AllocRaw( sizeof( T ) ) ) T;
    }

    template<typename T>
    T* Alloc()
    {
        return (T*)AllocRaw( sizeof( T ) );
    }

    template<typename T>
    T* Alloc( size_t size )
    {
        return (T*)AllocRaw( sizeof( T ) * size );
    }

    void Unalloc( size_t size )
    {
        assert( size <= m_offset );
        m_offset -= size;
    }

    void Reset()
    {
        if( m_buffer.size() > 1 )
        {
            memUsage.fetch_sub( BlockSize * ( m_buffer.size() - 1 ), std::memory_order_relaxed );
            for( int i=1; i<m_buffer.size(); i++ )
            {
                delete[] m_buffer[i];
            }
            m_ptr = m_buffer[0];
            m_buffer.clear();
            m_buffer.emplace_back( m_ptr );
        }
        m_offset = 0;
    }

    Slab( const Slab& ) = delete;
    Slab( Slab&& ) = delete;

    Slab& operator=( const Slab& ) = delete;
    Slab& operator=( Slab&& ) = delete;

private:
    char* m_ptr;
    uint32_t m_offset;
    std::vector<char*> m_buffer;
};

}

#endif
