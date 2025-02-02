#include "rgl_local.h"

extern void R_GLDebug_Callback_ARB( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam );

void R_InitExtensions( void )
{
	// win32 has IGNORE already defined
	enum { EXT_IGNORE, EXT_USING, EXT_NOTFOUND, EXT_FAILED };
	const char *ext;
	const char *result[4] = { "...ignoring %s\n", "...using %s\n", "...%s not found\n", "...%s failed to load\n" };

#define NGL( ret, name, ... ) n ## name = (PFN ## name) ri.GL_GetProcAddress( #name );


	// set default
	nglBufferSubDataARB = nglBufferSubData;
	nglGenBuffersARB = nglGenBuffers;
	nglDeleteBuffersARB = nglDeleteBuffers;
	nglBufferDataARB = nglBufferData;
	nglMapBufferARB = nglMapBuffer;
	nglUnmapBufferARB = nglUnmapBuffer;
	nglEnableVertexArrayAttribARB = nglEnableVertexArrayAttrib;
	nglDisableVertexAttribArrayARB = nglDisableVertexAttribArray;
	nglVertexAttribPointerARB = nglVertexAttribPointer;
	glContext.vboTarget = GL_ARRAY_BUFFER;
	glContext.iboTarget = GL_ELEMENT_ARRAY_BUFFER;

	if ( !r_useExtensions->i ) {
		ri.Printf( PRINT_INFO, "...Ignoring OpenGL extensions" );
	}

	//
	// ARB_buffer_storage
	//
	ext = "GL_ARB_buffer_storage";
	glContext.ARB_buffer_storage = qfalse;
	if ( NGL_VERSION_ATLEAST( 4, 5 ) || R_HasExtension( ext ) ) {
		glContext.ARB_buffer_storage = qtrue;

		NGL_ARB_buffer_storage

		if ( !nglBufferStorage ) {
			ri.Printf( PRINT_INFO, result[EXT_FAILED], ext );
			glContext.ARB_buffer_storage = qfalse;
		}
		else {
			ri.Printf( PRINT_INFO, result[EXT_USING], ext );
		}
	}
	else {
		ri.Printf( PRINT_INFO, result[EXT_NOTFOUND], ext );
	}

	//
	// ARB_transform_feedback
	//
	ext = "GL_ARB_transform_feedback";
	glContext.transformFeedback = qfalse;
	if ( ( NGL_VERSION_ATLEAST( 4, 0 ) ) || R_HasExtension( ext ) ) {
		NGL_ARB_transform_feedback

		glContext.transformFeedback = qtrue;

		if ( !nglBeginTransformFeedback || !nglEndTransformFeedback ) {
			ri.Printf( PRINT_INFO, result[EXT_FAILED], ext );
			glContext.transformFeedback = qfalse;
		} else {
			ri.Printf( PRINT_INFO, result[EXT_USING], ext );
		}
	} else {
		ri.Printf( PRINT_INFO, result[EXT_NOTFOUND], ext );
	}

	//
	// ARB_direct_state_access
	//
	ext = "GL_ARB_direct_state_access";
	glContext.directStateAccess = qfalse;
	if ( ( NGL_VERSION_ATLEAST( 4, 5 ) ) || R_HasExtension( ext ) ) {
		NGL_ARB_direct_state_access

		glContext.directStateAccess = qtrue;
		if ( !r_arb_direct_state_access->i ) {
			ri.Printf( PRINT_INFO, result[ EXT_IGNORE ], ext );
			glContext.directStateAccess = qfalse;
		}
		else if ( !nglNamedBufferData || !nglNamedBufferSubData || !nglNamedBufferData || !nglMapNamedBufferRange || !nglUnmapNamedBuffer
			|| !nglFlushMappedNamedBufferRange )
		{
			ri.Printf( PRINT_INFO, result[ EXT_FAILED ], ext );
			glContext.directStateAccess = qfalse;
		} else {
			ri.Printf( PRINT_INFO, result[ EXT_USING ], ext );
		}
	} else {
		ri.Printf( PRINT_INFO, result[ EXT_NOTFOUND ], ext );
	}

	//
	// ARB_bindless_texture
	//
	ext = "GL_ARB_bindless_texture";
	glContext.bindlessTextures = qfalse;
	if ( ( NGL_VERSION_ATLEAST( 4, 0 ) || R_HasExtension( ext ) ) ) {
		NGL_ARB_bindless_texture

		glContext.bindlessTextures = qtrue;

		if ( !nglMakeTextureHandleNonResidentARB || !nglIsTextureHandleResidentARB || !nglGetTextureHandleARB ) {
			ri.Printf( PRINT_INFO, result[EXT_FAILED], ext );
			glContext.bindlessTextures = qfalse;
		} else {
			ri.Printf( PRINT_INFO, result[EXT_USING], ext );
		}
	} else {
		ri.Printf( PRINT_INFO, result[EXT_NOTFOUND], ext );
	}

	//
	// ARB_shader_storage_buffer_object
	//
	ext = "GL_ARB_shader_storage_buffer_object";
	glContext.ARB_shader_storage_buffer_object = qfalse;
	if ( NGL_VERSION_ATLEAST( 4, 4 ) || R_HasExtension( ext ) ) {
		glContext.ARB_shader_storage_buffer_object = qtrue;

		if ( !r_arb_shader_storage_buffer_object->i ) {
			ri.Printf( PRINT_INFO, result[ EXT_IGNORE ], ext );
		} else {
			ri.Printf( PRINT_INFO, result[ EXT_USING ], ext );
		}
	} else {
		ri.Printf( PRINT_INFO, result[ EXT_NOTFOUND ], ext );
		ri.Cvar_Set( "r_arb_shader_storage_buffer_object", "0" );
	}


	//
	// ARB_map_buffer_range
	//
	ext = "GL_ARB_map_buffer_range";
	glContext.ARB_map_buffer_range = qfalse;
	if ( NGL_VERSION_ATLEAST( 4, 5 ) || R_HasExtension( ext ) ) {
		glContext.ARB_map_buffer_range = qtrue;

		NGL_ARB_map_buffer_range
		
		if ( !nglMapBufferRange || !nglFlushMappedBufferRange || !nglInvalidateBufferData || !nglInvalidateBufferSubData ) {
			ri.Printf( PRINT_INFO, result[EXT_FAILED], ext );
			glContext.ARB_map_buffer_range = qfalse;
		} else {
			ri.Printf( PRINT_INFO, result[EXT_USING], ext );
		}
	} else {
		ri.Printf( PRINT_INFO, result[EXT_NOTFOUND], ext );
	}

	ri.Cvar_Set( "r_arb_map_buffer_range", va( "%i", glContext.ARB_map_buffer_range ) );

	//
	// ARB_vertex_array_object
	//
	ext = "GL_ARB_vertex_array_object";
	glContext.ARB_vertex_array_object = qfalse;
	if ( NGL_VERSION_ATLEAST( 3, 0 ) || R_HasExtension( ext ) ) {
		if ( NGL_VERSION_ATLEAST( 3, 0 ) ) {
			// force vao, core context requires it
			glContext.ARB_vertex_array_object = qtrue;
		}
		else {
			glContext.ARB_vertex_array_object = !!r_arb_vertex_array_object->i;
		}

		NGL_VertexArrayARB_Procs
		if ( !nglVertexAttribPointerARB || !nglEnableVertexArrayAttribARB || !nglDisableVertexArrayAttribARB
			|| !nglEnableVertexAttribArrayARB || !nglDisableVertexAttribArrayARB )
		{
			ri.Printf( PRINT_INFO, result[EXT_FAILED], ext);
			glContext.ARB_vertex_array_object = qfalse;
		}
		else {
			ri.Printf(PRINT_INFO, result[EXT_USING], ext);
		}
	}
	else {
		ri.Printf(PRINT_INFO, result[EXT_NOTFOUND], ext);
	}

	ri.Cvar_Set( "r_arb_vertex_array_object", va( "%i", glContext.ARB_vertex_array_object ) );

	//
	// ARB_gl_sync
	//
	ext = "GL_ARB_gl_sync";
	glContext.ARB_sync = qfalse;
	if ( NGL_VERSION_ATLEAST( 3, 2 ) || R_HasExtension( ext ) ) {
		glContext.ARB_sync = qtrue;

		NGL_ARB_sync

		if ( !nglFenceSync ) {
			ri.Printf( PRINT_INFO, result[ EXT_FAILED ], ext );
			glContext.ARB_sync = qfalse;
		} else {
			ri.Printf( PRINT_INFO, result[ EXT_USING ], ext );
		}
	} else {
		ri.Printf( PRINT_INFO, result[ EXT_NOTFOUND ], ext );
	}

	//
	// ARB_gl_spirv
	//
	ext = "GL_ARB_gl_spirv";
	glContext.ARB_gl_spirv = qfalse;
	if ( NGL_VERSION_ATLEAST( 4, 0 ) || R_HasExtension( ext ) && r_useShaderCache->i ) {
		glContext.ARB_gl_spirv = qtrue;

		NGL_GLSL_SPIRV_Procs

		if ( !nglShaderBinary || !nglGetProgramBinary ) {
			ri.Printf( PRINT_INFO, result[EXT_FAILED], ext );
			glContext.ARB_gl_spirv = qfalse;
		} else {
			ri.Printf( PRINT_INFO, result[EXT_USING], ext );
		}
	}
	else {
		ri.Printf( PRINT_INFO, result[EXT_NOTFOUND], ext );
	}

	//
	// ARB_shader_subroutine
	//
	ext = "GL_ARB_shader_subroutine";
	glContext.shaderSubroutine = qfalse;
	if ( NGL_VERSION_ATLEAST( 4, 0 ) || R_HasExtension( ext ) ) {
		NGL_ARB_shader_subroutine

		glContext.shaderSubroutine = qtrue;
		if ( !nglGetSubroutineUniformLocation || !nglUniformSubroutinesuiv ) {
			ri.Printf( PRINT_INFO, result[ EXT_FAILED ], ext );
			glContext.shaderSubroutine = qfalse;
		} else {
			ri.Printf( PRINT_INFO, result[ EXT_USING ], ext );
		}
	} else {
		ri.Printf( PRINT_INFO, result[ EXT_NOTFOUND ], ext );
	}

	//
	// ARB_vertex_buffer_object
	//
	ext = "GL_ARB_vertex_buffer_object";
	glContext.ARB_vertex_buffer_object = qfalse;
	if ( NGL_VERSION_ATLEAST( 3, 0 ) || R_HasExtension( ext ) ) {
		NGL_BufferARB_Procs
		
		glContext.ARB_vertex_buffer_object = r_arb_vertex_buffer_object->i;

		if (!nglGenBuffersARB || !nglDeleteBuffersARB || !nglBindBufferARB || !nglBufferDataARB || !nglBufferSubDataARB ) {
			ri.Printf( PRINT_INFO, result[EXT_FAILED], ext );
			glContext.vboTarget = GL_ARRAY_BUFFER;
			glContext.iboTarget = GL_ELEMENT_ARRAY_BUFFER;
			glContext.ARB_vertex_buffer_object = qfalse;
		}
		else {
			ri.Printf( PRINT_INFO, result[EXT_USING], ext );
		}
		
	}
	else {
		ri.Printf( PRINT_INFO, result[EXT_NOTFOUND], ext );
	}

	ri.Cvar_Set( "r_arb_vertex_buffer_object", va( "%i", glContext.ARB_vertex_buffer_object ) );

	//
	// ARB_texture_filter_anisotropic
	//
	ext = "GL_ARB_texture_filter_anisotropic";
	glContext.ARB_texture_filter_anisotropic = qfalse;
	if ( R_HasExtension( ext ) ) {
		nglGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &glContext.maxAnisotropy );
		glContext.ARB_texture_filter_anisotropic = r_arb_texture_filter_anisotropic->i;

		if ( glContext.maxAnisotropy <= 0 ) {
			ri.Printf( PRINT_INFO, "... GL_ARB_texture_filter_anisotropic not property supported\n" );
			glContext.ARB_texture_filter_anisotropic = qfalse;
		}
		else {
			ri.Printf( PRINT_INFO, "...using GL_ARB_texture_filter_anisotropic (max: %f)\n", glContext.maxAnisotropy );
			glContext.ARB_texture_filter_anisotropic = qtrue;
		}
	}
	else {
		ri.Printf( PRINT_INFO, result[EXT_NOTFOUND], ext );
	}

	ri.Cvar_Set( "r_arb_texture_filter_anisotropic", va( "%i", glContext.ARB_texture_filter_anisotropic ) );

	//
	// ARB_texture_float
	//
	ext = "GL_ARB_texture_float";
	glContext.ARB_texture_float = qfalse;
	if ( R_HasExtension( ext ) ) {
		glContext.ARB_texture_float = r_arb_texture_float->i;
		ri.Printf(PRINT_INFO, result[glContext.ARB_texture_float], ext);
	}
	else {
		ri.Printf(PRINT_INFO, result[EXT_NOTFOUND], ext);
	}

	//
	// ARB_sample_shading
	//
	ext = "GL_ARB_sample_shading";
	glContext.ARB_sample_shading = qfalse;
	if ( R_HasExtension( ext ) ) {
		glContext.ARB_sample_shading = qtrue;
		if ( !nglMinSampleShadingARB ) {
			ri.Printf( PRINT_INFO, result[ EXT_FAILED ], ext );
			glContext.ARB_sample_shading = qfalse;
		} else {
			ri.Printf( PRINT_INFO, result[ EXT_USING ], ext );
		}
	} else {
		ri.Printf( PRINT_INFO, result[ EXT_NOTFOUND ], ext );
	}

	//
	// gpu memory info diangostics extensions
	//

	glContext.memInfo = MI_NONE;

	//
	// NVX_gpu_memory_info
	//
	ext = "GL_NVX_gpu_memory_info";
	if ( R_HasExtension( ext ) ) {
		glContext.memInfo = MI_NVX;
		ri.Printf(PRINT_INFO, result[EXT_USING], ext);
	}
	else {
		ri.Printf(PRINT_INFO, result[EXT_NOTFOUND], ext);
	}

	//
	// ATI_meminfo
	//
	ext = "GL_ATI_meminfo";
	if ( R_HasExtension( ext ) ) {
		if (glContext.memInfo == MI_NONE) {
			glContext.memInfo = MI_ATI;
			ri.Printf(PRINT_INFO, result[EXT_USING], ext);
		}
		else {
			ri.Printf(PRINT_INFO, result[EXT_IGNORE], ext);
		}
	}
	else {
		ri.Printf(PRINT_INFO, result[EXT_NOTFOUND], ext);
	}

	glContext.textureCompressionRef = TCR_NONE;

	//
	// ARB_texture_compression_rgtc
	//
	ext = "GL_ARB_texture_compression_rgtc";
	if ( R_HasExtension( ext ) ) {
		qboolean useRgtc = r_arb_texture_compression->i >= 2;
		if ( useRgtc ) {
			glContext.textureCompressionRef |= TCR_RGTC;
		}
		
		ri.Printf(PRINT_INFO, result[useRgtc], ext);
	}
	else {
		ri.Printf(PRINT_INFO, result[EXT_NOTFOUND], ext);
	}

	glContext.swizzleNormalmap = r_arb_texture_compression->i && !(glContext.textureCompressionRef & TCR_RGTC);

	//
	// ARB_texture_compression_bptc
	//
	ext = "GL_ARB_texture_compression_bptc";
	if ( R_HasExtension( ext ) ) {
		qboolean useBptc = r_arb_texture_compression->i >= 3;
		if ( useBptc ) {
			glContext.textureCompressionRef |= TCR_BPTC;
		}
		
		ri.Printf( PRINT_INFO, result[useBptc], ext );
	}
	else {
		ri.Printf( PRINT_INFO, result[EXT_NOTFOUND], ext );
	}

	//
	// ARB_framebuffer_object
	//
	ext = "GL_ARB_framebuffer_object";
	glContext.ARB_framebuffer_object = qfalse;
	glContext.ARB_framebuffer_sRGB = qfalse;
	glContext.ARB_framebuffer_multisample = qfalse;
	glContext.ARB_framebuffer_blit = qfalse;
	if ( NGL_VERSION_ATLEAST( 3, 0 ) || R_HasExtension( ext ) ) {
		glContext.ARB_framebuffer_object = r_arb_framebuffer_object->i;
		glContext.ARB_framebuffer_blit = qtrue;
		glContext.ARB_framebuffer_multisample = qtrue;
		glContext.ARB_framebuffer_sRGB = qtrue;

		ri.Cvar_Set( "r_arb_framebuffer_blit", "1" );
		ri.Cvar_Set( "r_arb_framebuffer_object", "1" );
		ri.Cvar_Set( "r_arb_framebuffer_srgb", "1" );
		ri.Cvar_Set( "r_arb_framebuffer_multisample", "1" );

		nglGetIntegerv( GL_MAX_RENDERBUFFER_SIZE, &glContext.maxRenderBufferSize );
		nglGetIntegerv( GL_MAX_COLOR_ATTACHMENTS, &glContext.maxColorAttachments );

		NGL_FBO_Procs

		ri.Printf( PRINT_INFO, result[glContext.ARB_framebuffer_object], ext );
	}
	else {
		ri.Printf( PRINT_INFO, result[EXT_NOTFOUND], ext );

		ri.Cvar_Set( "r_arb_framebuffer_blit", "0" );
		ri.Cvar_Set( "r_arb_framebuffer_object", "0" );
		ri.Cvar_Set( "r_arb_framebuffer_srgb", "0" );
		ri.Cvar_Set( "r_arb_framebuffer_multisample", "0" );
	}
	
	//
	// ARB_debug_output
	//
	ext = "GL_ARB_debug_output";
	if ( R_HasExtension( ext ) ) {
		glContext.debugType = GL_DBG_ARB;

		NGL_Debug_Procs

		if ( !nglDebugMessageControlARB || !nglDebugMessageInsertARB || !nglDebugMessageCallbackARB ) {
			ri.Printf( PRINT_INFO, result[EXT_FAILED], ext );
		}
		else {
			ri.Printf( PRINT_INFO, result[EXT_USING], ext );

			nglEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB );
			nglEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
			nglEnable( GL_DEBUG_OUTPUT );

			nglDebugMessageControlARB( GL_DEBUG_SOURCE_API_ARB, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE );
			nglDebugMessageControlARB( GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE );
			nglDebugMessageControlARB( GL_DEBUG_SOURCE_SHADER_COMPILER_ARB, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE );
			nglDebugMessageControlARB( GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE );
			nglDebugMessageControlARB( GL_DEBUG_SOURCE_OTHER_ARB, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE );
			nglDebugMessageControlARB( GL_DEBUG_SOURCE_THIRD_PARTY_ARB, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE );

			nglDebugMessageCallbackARB(R_GLDebug_Callback_ARB, NULL);
		}
	}
	else {
		ri.Printf(PRINT_INFO, result[EXT_NOTFOUND], ext);
	}

	//
	// ARB_multisample
	//
	ext = "GL_ARB_multisample";
	if ( R_HasExtension( ext ) ) {
		ri.Printf( PRINT_INFO, result[EXT_USING], ext );
		ri.Cvar_Set( "r_arb_multisample", "1" );
	} else {
		ri.Printf( PRINT_INFO, result[EXT_NOTFOUND], ext );
		ri.Cvar_Set( "r_arb_multisample", "0" );
	}

	//
	// ARB_multitexture
	//
	ext = "GL_ARB_multitexture";
	if ( R_HasExtension( ext ) ) {
		ri.Printf( PRINT_INFO, result[EXT_USING], ext );
		ri.Cvar_Set( "r_arb_multitexture", "1" );
	} else {
		ri.Printf( PRINT_INFO, result[EXT_NOTFOUND], ext );
		ri.Cvar_Set( "r_arb_multitexture", "0" );
	}

	//
	// ARB_color_buffer_float
	//
	ext = "GL_ARB_color_buffer_float";
	if ( R_HasExtension( ext ) ) {
		ri.Printf( PRINT_INFO, result[EXT_USING], ext );
		ri.Cvar_Set( "r_arb_color_buffer_float", "1" );
	} else {
		ri.Printf( PRINT_INFO, result[EXT_NOTFOUND], ext );
		ri.Cvar_Set( "r_arb_color_buffer_float", "0" );
	}

	// determine GLSL version
	N_strncpyz(glContext.glsl_version_str, (const char *)nglGetString(GL_SHADING_LANGUAGE_VERSION), sizeof(glContext.glsl_version_str));
	sscanf(glContext.glsl_version_str, "%i.%i", &glContext.glslVersionMajor, &glContext.glslVersionMinor);
	ri.Printf(PRINT_INFO, "...using GLSL version %s\n", glContext.glsl_version_str);

#undef NGL
}