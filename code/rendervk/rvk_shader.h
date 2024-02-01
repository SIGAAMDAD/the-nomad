#ifndef __RVK_SHADER__
#define __RVK_SHADER__

#pragma once

class CShaderProgram
{
public:
    CShaderProgram( void );
    ~CShaderProgram();

    void Load( const char *filename );
private:
    VkShaderModule m_hShaderModule;
};

#endif