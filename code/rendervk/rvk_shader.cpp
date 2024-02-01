#include "rvk_local.h"
#include "rvk_shader.h"

CShaderProgram::CShaderProgram( void )
{
}

CShaderProgram::~CShaderProgram()
{
    if ( m_hShaderModule ) {
        vkDestroyShaderModule( rg.m_pDevice, m_hShaderModule, rg.m_pAllocator );
    }
}

void CShaderProgram::Load( const char *filename )
{
    union {
        void *v;
        char *b;
    } f;
    uint64_t fileSize;
    VkShaderModuleCreateInfo createInfo;
    VkPipelineShaderStageCreateInfo shaderStageInfo;

    fileSize = FS_LoadFile( filename, &f.v );
    if ( !fileSize || !f.v ) {
        N_Error( ERR_FATAL, "CShaderProgram::Load: failed to load shader file '%s'", filename );
    }

    memset( &creatInfo, 0, sizeof(createInfo) );
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = fileSize;
    createInfo.pCode = (uint32_t *)f.v;

    if ( vkCreateShaderModule( rg.m_pDevice, &createInfo, rg.m_pAllocator, &m_hShaderModule ) != VK_SUCCESS ) {
        N_Error( ERR_FATAL, "CShaderProgram::Load: failed to create shader program module" );
    }

    FS_FreeFile( f.v );

    memset( &shaderStageInfo, 0, sizeof(shaderStageInfo) );
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStageInfo.module = m_hShaderModule;
    shaderStageInfo.pName = "";
}
