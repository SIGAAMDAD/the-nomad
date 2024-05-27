#ifndef __DECOMPRESS_ZIP__
#define __DECOMPRESS_ZIP__

#pragma once

#include "decompress.h"

class CDecompressZip : public CDecompressBase
{
public:
    CDecompressZip( void );
    virtual ~CDecompressZip() override;
};

#endif