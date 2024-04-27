#ifndef __MODULE_JSON_STRING__
#define __MODULE_JSON_STRING__

#pragma once

#include <string>
#include <foonathan/memory/container.hpp>
#include <foonathan/memory/temporary_allocator.hpp>
#include <foonathan/memory/namespace_alias.hpp>
#include <foonathan/memory/aligned_allocator.hpp>
#include <foonathan/memory/virtual_memory.hpp>

//
// CJsonString: an optimized version of std::string, cuz I don't want to be calling malloc for 64 bytes
//
class CJsonString : public std::basic_string<char, std::char_traits<char>, memory::std_allocator<char, memory::temporary_allocator>>
{
public:
    CJsonString( void ) = default;
};

#endif