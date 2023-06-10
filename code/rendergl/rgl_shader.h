#ifndef _RGL_SHADER_
#define _RGL_SHADER_

#pragma once

shader_t* R_CreateShader(const char *filepath);
void R_BindShader(const shader_t *shader);
void R_UnbindShader(void);
void R_InitShaders(void);

#endif