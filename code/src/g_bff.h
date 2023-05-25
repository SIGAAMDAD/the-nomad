#ifndef _G_BFF_
#define _G_BFF_

#pragma once

bff_chunk_t* G_GetChunk(const char *chunkname);
void G_LoadBFF(const std::string& bffname);

#endif