/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_init.c -- functions that are not called every frame

#include "tr_local.h"

glconfig_t	glConfig;
qboolean	nonPowerOfTwoTextures;
qboolean	textureFilterAnisotropic;
int			maxAnisotropy;
int			gl_version;
int			gl_clamp_mode;	// GL_CLAMP or GL_CLAMP_TO_EGGE

glstate_t	glState;

glstatic_t	gls;

static void GfxInfo( void );
static void VarInfo( void );
static void GL_SetDefaultState( void );

cvar_t	*r_flareSize;
cvar_t	*r_flareFade;
cvar_t	*r_flareCoeff;

cvar_t	*r_railWidth;
cvar_t	*r_railCoreWidth;
cvar_t	*r_railSegmentLength;

cvar_t	*r_detailTextures;

cvar_t	*r_znear;
cvar_t	*r_zproj;
cvar_t	*r_stereoSeparation;

cvar_t	*r_skipBackEnd;

cvar_t	*r_anaglyphMode;

cvar_t	*r_greyscale;

static cvar_t *r_ignorehwgamma;

cvar_t  *r_teleporterFlash;

cvar_t	*r_fastsky;
cvar_t	*r_neatsky;
cvar_t	*r_drawSun;
cvar_t	*r_dynamiclight;
cvar_t  *r_mergeLightmaps;
#ifdef USE_PMLIGHT
cvar_t	*r_dlightMode;
cvar_t	*r_dlightSpecPower;
cvar_t	*r_dlightSpecColor;
cvar_t	*r_dlightScale;
cvar_t	*r_dlightIntensity;
#endif // USE_PMLIGHT

cvar_t	*r_dlightSaturation;

#ifdef USE_VBO
cvar_t	*r_vbo;
#endif

#ifdef USE_FBO
cvar_t	*r_fbo;
cvar_t	*r_hdr;
cvar_t	*r_bloom;
cvar_t	*r_bloom_threshold;
cvar_t	*r_bloom_threshold_mode;
cvar_t	*r_bloom_modulate;
cvar_t	*r_bloom_passes;
cvar_t	*r_bloom_blend_base;
cvar_t	*r_bloom_intensity;
cvar_t	*r_bloom_filter_size;
cvar_t	*r_bloom_reflection;

cvar_t	*r_renderWidth;
cvar_t	*r_renderHeight;
cvar_t	*r_renderScale;
#endif // USE_FBO

cvar_t	*r_dlightBacks;

cvar_t	*r_lodbias;
cvar_t	*r_lodscale;

cvar_t	*r_norefresh;
cvar_t	*r_drawentities;
cvar_t	*r_drawworld;
cvar_t	*r_speeds;
cvar_t	*r_fullbright;
cvar_t	*r_novis;
cvar_t	*r_nocull;
cvar_t	*r_facePlaneCull;
cvar_t	*r_showcluster;
cvar_t	*r_nocurves;

cvar_t	*r_allowExtensions;

cvar_t	*r_ext_compressed_textures;
cvar_t	*r_ext_multitexture;
cvar_t	*r_ext_compiled_vertex_array;
cvar_t	*r_ext_texture_env_add;
cvar_t	*r_ext_texture_filter_anisotropic;
cvar_t	*r_ext_max_anisotropy;

cvar_t	*r_ignoreGLErrors;

//cvar_t	*r_stencilbits;
cvar_t	*r_texturebits;

#ifdef USE_FBO
cvar_t	*r_ext_multisample;
cvar_t	*r_ext_supersample;
#endif // USE_FBO

cvar_t	*r_drawBuffer;
cvar_t	*r_lightmap;
cvar_t	*r_vertexLight;
cvar_t	*r_shadows;
cvar_t	*r_flares;
cvar_t	*r_nobind;
cvar_t	*r_singleShader;
cvar_t	*r_roundImagesDown;
cvar_t	*r_colorMipLevels;
cvar_t	*r_picmip;
cvar_t	*r_nomip;
cvar_t	*r_showtris;
cvar_t	*r_showsky;
cvar_t	*r_shownormals;
cvar_t	*r_finish;
cvar_t	*r_clear;
cvar_t	*r_textureMode;
cvar_t	*r_offsetFactor;
cvar_t	*r_offsetUnits;
cvar_t	*r_gamma;
cvar_t	*r_intensity;
cvar_t	*r_lockpvs;
cvar_t	*r_noportals;
cvar_t	*r_portalOnly;

cvar_t	*r_subdivisions;
cvar_t	*r_lodCurveError;

cvar_t	*r_overBrightBits;
cvar_t	*r_mapOverBrightBits;
cvar_t	*r_mapGreyScale;

cvar_t	*r_debugSurface;
cvar_t	*r_simpleMipMaps;

cvar_t	*r_showImages;
cvar_t	*r_defaultImage;

cvar_t	*r_ambientScale;
cvar_t	*r_directedScale;
cvar_t	*r_debugLight;
cvar_t	*r_debugSort;
cvar_t	*r_printShaders;
cvar_t	*r_saveFontData;

cvar_t	*r_marksOnTriangleMeshes;

cvar_t	*r_aviMotionJpegQuality;
cvar_t	*r_screenshotJpegQuality;

static cvar_t *r_maxpolys;
static cvar_t* r_maxpolyverts;
int		max_polys;
int		max_polyverts;

static char gl_extensions[ 32768 ];

#define GLE( ret, name, ... ) ret ( APIENTRY * q##name )( __VA_ARGS__ );
	QGL_Core_PROCS;
	QGL_Ext_PROCS;
	QGL_ARB_PROGRAM_PROCS;
	QGL_VBO_PROCS;
	QGL_FBO_PROCS;
	QGL_FBO_OPT_PROCS;
#undef GLE

typedef struct {
	void **symbol;
	const char *name;
} sym_t;

#define GLE( ret, name, ... ) { (void**)&q##name, XSTRING(name) },
static sym_t core_procs[] = { QGL_Core_PROCS };
static sym_t ext_procs[] = { QGL_Ext_PROCS };
static sym_t arb_procs[] = { QGL_ARB_PROGRAM_PROCS };
static sym_t vbo_procs[] = { QGL_VBO_PROCS };
static sym_t fbo_procs[] = { QGL_FBO_PROCS };
static sym_t fbo_opt_procs[] = { QGL_FBO_OPT_PROCS };
#undef GLE


/*
==================
R_ResolveSymbols

returns NULL on success or last failed symbol name otherwise
==================
*/
static const char *R_ResolveSymbols( sym_t *syms, int count )
{
	int i;
	for ( i = 0; i < count; i++ )
	{
		*syms[ i ].symbol = ri.GL_GetProcAddress( syms[ i ].name );
		if ( *syms[ i ].symbol == NULL )
		{
			return syms[ i ].name;
		}
	}
	return NULL;
}


static void R_ClearSymbols( sym_t *syms, int count )
{
	int i;
	for ( i = 0; i < count; i++ )
	{
		*syms[ i ].symbol = NULL;
	}
}


static void R_ClearSymTables( void )
{
	R_ClearSymbols( core_procs, ARRAY_LEN( core_procs ) );
	R_ClearSymbols( ext_procs, ARRAY_LEN( ext_procs ) );
	R_ClearSymbols( arb_procs, ARRAY_LEN( arb_procs ) );
	R_ClearSymbols( vbo_procs, ARRAY_LEN( vbo_procs ) );
	R_ClearSymbols( fbo_procs, ARRAY_LEN( fbo_procs ) );
	R_ClearSymbols( fbo_opt_procs, ARRAY_LEN( fbo_opt_procs ) );
}


// for modular renderer
#ifdef USE_RENDERER_DLOPEN
void QDECL Com_Error( errorParm_t code, const char *fmt, ... )
{
	char buf[ 4096 ];
	va_list	argptr;
	va_start( argptr, fmt );
	Q_vsnprintf( buf, sizeof( buf ), fmt, argptr );
	va_end( argptr );
	ri.Error( code, "%s", buf );
}

void QDECL Com_Printf( const char *fmt, ... )
{
	char buf[ MAXPRINTMSG ];
	va_list	argptr;
	va_start( argptr, fmt );
	Q_vsnprintf( buf, sizeof( buf ), fmt, argptr );
	va_end( argptr );

	ri.Printf( PRINT_ALL, "%s", buf );
}
#endif


/*
** R_HaveExtension
*/
static qboolean R_HaveExtension( const char *ext )
{
	const char *ptr = Q_stristr( gl_extensions, ext );
	if (ptr == NULL)
		return qfalse;
	ptr += strlen(ext);
	return ((*ptr == ' ') || (*ptr == '\0'));  // verify its complete string.
}


/*
** R_InitExtensions
*/
static void R_InitExtensions( void )
{
	GLint max_texture_size = 0;
	float version;
	size_t len;
	const char *err;

	if ( !qglGetString( GL_EXTENSIONS ) )
	{
		ri.Error( ERR_FATAL, "OpenGL installation is broken. Please fix video drivers and/or restart your system" );
	}

	// get our config strings
	Q_strncpyz( glConfig.vendor_string, (char *)qglGetString (GL_VENDOR), sizeof( glConfig.vendor_string ) );
	Q_strncpyz( glConfig.renderer_string, (char *)qglGetString (GL_RENDERER), sizeof( glConfig.renderer_string ) );
	len = strlen( glConfig.renderer_string );
	if ( len && glConfig.renderer_string[ len - 1 ] == '\n' )
		glConfig.renderer_string[ len - 1 ] = '\0';
	Q_strncpyz( glConfig.version_string, (char *)qglGetString( GL_VERSION ), sizeof( glConfig.version_string ) );

	Q_strncpyz( gl_extensions, (char *)qglGetString( GL_EXTENSIONS ), sizeof( gl_extensions ) );
	Q_strncpyz( glConfig.extensions_string, gl_extensions, sizeof( glConfig.extensions_string ) );

	version = Q_atof( (const char *)qglGetString( GL_VERSION ) );
	gl_version = (int)(version * 10.001);

	glConfig.textureCompression = TC_NONE;

	glConfig.textureEnvAddAvailable = qfalse;

	textureFilterAnisotropic = qfalse;
	maxAnisotropy = 0;

	nonPowerOfTwoTextures = qfalse;

	qglLockArraysEXT = NULL;
	qglUnlockArraysEXT = NULL;

	glConfig.numTextureUnits = 1;
	qglMultiTexCoord2fARB = NULL;
	qglActiveTextureARB = NULL;
	qglClientActiveTextureARB = NULL;

	gl_clamp_mode = GL_CLAMP; // by default

	// OpenGL driver constants
	qglGetIntegerv( GL_MAX_TEXTURE_SIZE, &max_texture_size );
	glConfig.maxTextureSize = max_texture_size;

	// stubbed or broken drivers may have reported 0...
	if ( glConfig.maxTextureSize <= 0 )
		glConfig.maxTextureSize = 0;
	else if ( glConfig.maxTextureSize > 2048 )
		glConfig.maxTextureSize = 2048; // ResampleTexture() relies on that maximum

	if ( !r_allowExtensions->integer )
	{
		ri.Printf( PRINT_ALL, "*** IGNORING OPENGL EXTENSIONS ***\n" );
		return;
	}

	ri.Printf( PRINT_ALL, "Initializing OpenGL extensions\n" );

	if ( R_HaveExtension( "GL_EXT_texture_edge_clamp" ) || R_HaveExtension( "GL_SGIS_texture_edge_clamp" ) ) {
		gl_clamp_mode = GL_CLAMP_TO_EDGE;
		ri.Printf( PRINT_ALL, "...using GL_EXT_texture_edge_clamp\n" );
	} else {
		ri.Printf( PRINT_ALL, "...GL_EXT_texture_edge_clamp not found\n" );
		ri.Printf( PRINT_ALL, S_COLOR_YELLOW "...Degraded texture support likely!\n" );
	}

	// GL_EXT_texture_compression_s3tc
	if ( R_HaveExtension( "GL_ARB_texture_compression" ) &&
		 R_HaveExtension( "GL_EXT_texture_compression_s3tc" ) )
	{
		if ( r_ext_compressed_textures->integer ){
			glConfig.textureCompression = TC_S3TC_ARB;
			ri.Printf( PRINT_ALL, "...using GL_EXT_texture_compression_s3tc\n" );
		} else {
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_texture_compression_s3tc\n" );
		}
	} else {
		ri.Printf( PRINT_ALL, "...GL_EXT_texture_compression_s3tc not found\n" );
	}

	// GL_S3_s3tc
	if ( glConfig.textureCompression == TC_NONE && r_ext_compressed_textures->integer ) {
		if ( R_HaveExtension( "GL_S3_s3tc" ) ) {
			if ( r_ext_compressed_textures->integer ) {
				glConfig.textureCompression = TC_S3TC;
				ri.Printf( PRINT_ALL, "...using GL_S3_s3tc\n" );
			} else {
				glConfig.textureCompression = TC_NONE;
				ri.Printf( PRINT_ALL, "...ignoring GL_S3_s3tc\n" );
			}
		} else {
			ri.Printf( PRINT_ALL, "...GL_S3_s3tc not found\n" );
		}
	}

	// GL_EXT_texture_env_add
	if ( R_HaveExtension( "EXT_texture_env_add" ) ) {
		if ( r_ext_texture_env_add->integer ) {
			glConfig.textureEnvAddAvailable = qtrue;
			ri.Printf( PRINT_ALL, "...using GL_EXT_texture_env_add\n" );
		} else {
			glConfig.textureEnvAddAvailable = qfalse;
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_texture_env_add\n" );
		}
	} else {
		ri.Printf( PRINT_ALL, "...GL_EXT_texture_env_add not found\n" );
	}

	// GL_ARB_multitexture
	if ( R_HaveExtension( "GL_ARB_multitexture" ) )
	{
		if ( r_ext_multitexture->integer )
		{
			qglMultiTexCoord2fARB = ri.GL_GetProcAddress( "glMultiTexCoord2fARB" );
			qglActiveTextureARB = ri.GL_GetProcAddress( "glActiveTextureARB" );
			qglClientActiveTextureARB = ri.GL_GetProcAddress( "glClientActiveTextureARB" );

			if ( qglActiveTextureARB && qglClientActiveTextureARB )
			{
				GLint textureUnits = 0;

				qglGetIntegerv( GL_MAX_ACTIVE_TEXTURES_ARB, &textureUnits );

				if ( textureUnits > 1 )
				{
					GLint max_shader_units = 0;
					GLint max_bind_units = 0;

					qglGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS, &max_shader_units );
					qglGetIntegerv( GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_bind_units );

					if ( max_bind_units > max_shader_units )
						max_bind_units = max_shader_units;
					if ( max_bind_units > MAX_TEXTURE_UNITS )
						max_bind_units = MAX_TEXTURE_UNITS;

					glConfig.numTextureUnits = MAX( textureUnits, max_bind_units );
					ri.Printf( PRINT_ALL, "...using GL_ARB_multitexture\n" );
				}
				else
				{
					qglMultiTexCoord2fARB = NULL;
					qglActiveTextureARB = NULL;
					qglClientActiveTextureARB = NULL;
					ri.Printf( PRINT_ALL, "...not using GL_ARB_multitexture, < 2 texture units\n" );
				}
			}
		}
		else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_ARB_multitexture\n" );
		}
	}
	else
	{
		ri.Printf( PRINT_ALL, "...GL_ARB_multitexture not found\n" );
	}

	// GL_EXT_compiled_vertex_array
	if ( R_HaveExtension( "GL_EXT_compiled_vertex_array" ) )
	{
		if ( r_ext_compiled_vertex_array->integer )
		{
			ri.Printf( PRINT_ALL, "...using GL_EXT_compiled_vertex_array\n" );
			qglLockArraysEXT = ri.GL_GetProcAddress( "glLockArraysEXT" );
			qglUnlockArraysEXT = ri.GL_GetProcAddress( "glUnlockArraysEXT" );
			if ( !qglLockArraysEXT || !qglUnlockArraysEXT ) {
				ri.Error( ERR_FATAL, "bad getprocaddress" );
			}
		}
		else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_compiled_vertex_array\n" );
		}
	}
	else
	{
		ri.Printf( PRINT_ALL, "...GL_EXT_compiled_vertex_array not found\n" );
	}

	if ( R_HaveExtension( "GL_EXT_texture_filter_anisotropic" ) )
	{
		if ( r_ext_texture_filter_anisotropic->integer ) {
			qglGetIntegerv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy );
			if ( maxAnisotropy <= 0 ) {
				ri.Printf( PRINT_ALL, "...GL_EXT_texture_filter_anisotropic not properly supported!\n" );
				maxAnisotropy = 0;
			}
			else
			{
				ri.Printf( PRINT_ALL, "...using GL_EXT_texture_filter_anisotropic (max: %i)\n", maxAnisotropy );
				textureFilterAnisotropic = qtrue;
				maxAnisotropy = MIN( r_ext_max_anisotropy->integer, maxAnisotropy );
			}
		}
		else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_texture_filter_anisotropic\n" );
		}
	}
	else
	{
		ri.Printf( PRINT_ALL, "...GL_EXT_texture_filter_anisotropic not found\n" );
	}

	if ( R_HaveExtension( "GL_ARB_vertex_program" ) && R_HaveExtension( "GL_ARB_fragment_program" ) )
	{
		err = R_ResolveSymbols( arb_procs, ARRAY_LEN( arb_procs ) );
		if ( err )
		{
			ri.Printf( PRINT_WARNING, "Error resolving ARB program function '%s'\n", err );
			qglGenProgramsARB = NULL; // indicates presence of ARB shaders functionality
		}
		else
		{
			ri.Printf( PRINT_ALL, "...using ARB vertex/fragment programs\n" );
		}
	}

#ifdef USE_VBO
	if ( R_HaveExtension( "ARB_vertex_buffer_object" ) && qglActiveTextureARB )
	{
		err = R_ResolveSymbols( vbo_procs, ARRAY_LEN( vbo_procs ) );
		if ( err )
		{
			ri.Printf( PRINT_WARNING, "Error resolving VBO function '%s'\n", err );
			qglBindBufferARB = NULL; // indicates presence of VBO functionality
		}
		else
		{
			ri.Printf( PRINT_ALL, "...using ARB vertex buffer objects\n" );
		}
	}
#endif // USE_VBO

#ifdef USE_FBO
	if ( R_HaveExtension( "GL_EXT_framebuffer_object" ) && R_HaveExtension( "GL_EXT_framebuffer_blit" ) )
	{
		err = R_ResolveSymbols( fbo_procs, ARRAY_LEN( fbo_procs ) );
		if ( err )
		{
			ri.Printf( PRINT_WARNING, "Error resolving FBO function '%s'\n", err );
			qglGenFramebuffers = NULL; // indicates presence of FBO functionality
		}
		else
		{
			// resolve optional fbo functions, without any warnings
			R_ResolveSymbols( fbo_opt_procs, ARRAY_LEN( fbo_opt_procs ) );
		}
	}
#endif // USE_FBO
}


/*
** InitOpenGL
**
** This function is responsible for initializing a valid OpenGL subsystem.  This
** is done by calling GLimp_Init (which gives us a working OGL subsystem) then
** setting variables, checking GL constants, and reporting the gfx system config
** to the user.
*/
static void InitOpenGL( void )
{
	//
	// initialize OS specific portions of the renderer
	//
	// GLimp_Init directly or indirectly references the following cvars:
	//		- r_fullscreen
	//		- r_mode
	//		- r_(color|depth|stencil)bits
	//		- r_ignorehwgamma
	//		- r_gamma
	//

	if ( glConfig.vidWidth == 0 )
	{
		const char *err;

		if ( !ri.GLimp_Init )
		{
			ri.Error( ERR_FATAL, "OpenGL interface is not initialized" );
		}

		ri.GLimp_Init( &glConfig );

		R_ClearSymTables();

		err = R_ResolveSymbols( core_procs, ARRAY_LEN( core_procs ) );
		if ( err )
			ri.Error( ERR_FATAL, "Error resolving core OpenGL function '%s'", err );

		R_InitExtensions();

		gls.windowWidth = glConfig.vidWidth;
		gls.windowHeight = glConfig.vidHeight;

		gls.captureWidth = glConfig.vidWidth;
		gls.captureHeight = glConfig.vidHeight;

		ri.CL_SetScaling( 1.0, gls.captureWidth, gls.captureHeight );

#ifdef USE_FBO
		if ( r_fbo->integer && qglGenProgramsARB && qglGenFramebuffers )
		{
			if ( r_renderScale->integer )
			{
				glConfig.vidWidth = r_renderWidth->integer;
				glConfig.vidHeight = r_renderHeight->integer;
			}

			gls.captureWidth = glConfig.vidWidth;
			gls.captureHeight = glConfig.vidHeight;

			ri.CL_SetScaling( 1.0, gls.captureWidth, gls.captureHeight );

			if ( r_ext_supersample->integer )
			{
				glConfig.vidWidth *= 2;
				glConfig.vidHeight *= 2;
				ri.CL_SetScaling( 2.0, gls.captureWidth, gls.captureHeight );
			}
		}
#endif

		QGL_InitARB();

		glConfig.deviceSupportsGamma = qfalse;

		ri.GLimp_InitGamma( &glConfig );

		gls.deviceSupportsGamma = glConfig.deviceSupportsGamma;

		if ( r_ignorehwgamma->integer )
			glConfig.deviceSupportsGamma = qfalse;

		// print info
		GfxInfo();

		gls.initTime = ri.Milliseconds();
	}
	else
	{
#ifdef USE_FBO
		QGL_SetRenderScale( qfalse );
#endif
	}

	if ( !qglViewport ) // might happen after REF_KEEP_WINDOW
	{
		const char *err = R_ResolveSymbols( core_procs, ARRAY_LEN( core_procs ) );
		if ( err )
			ri.Error( ERR_FATAL, "Error resolving core OpenGL function '%s'", err );

		R_InitExtensions();

		QGL_InitARB();

		// print info
		GfxInfo();

		gls.initTime = ri.Milliseconds();
	}

	// set default state
	GL_SetDefaultState();

	tr.inited = qtrue;
}


/*
==================
GL_CheckErrors
==================
*/
void GL_CheckErrors( void ) {
    int		err;
    const char *s;
    char buf[32];

    err = qglGetError();
    if ( err == GL_NO_ERROR ) {
        return;
    }
    if ( r_ignoreGLErrors->integer ) {
        return;
    }
    switch( err ) {
        case GL_INVALID_ENUM:
            s = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            s = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            s = "GL_INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            s = "GL_STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            s = "GL_STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            s = "GL_OUT_OF_MEMORY";
            break;
        default:
            Com_sprintf( buf, sizeof(buf), "%i", err);
            s = buf;
            break;
    }

    ri.Error( ERR_FATAL, "GL_CheckErrors: %s", s );
}


/*
==============================================================================

						SCREEN SHOTS

NOTE TTimo
some thoughts about the screenshots system:
screenshots get written in fs_homepath + fs_gamedir
vanilla q3 .. baseq3/screenshots/ *.tga
team arena .. missionpack/screenshots/ *.tga

two commands: "screenshot" and "screenshotJPEG"
we use statics to store a count and start writing the first screenshot/screenshot????.tga (.jpg) available
(with FS_FileExists / FS_FOpenFileWrite calls)
FIXME: the statics don't get a reinit between fs_game changes

==============================================================================
*/

/*
==================
RB_ReadPixels

Reads an image but takes care of alignment issues for reading RGB images.

Reads a minimum offset for where the RGB data starts in the image from
integer stored at pointer offset. When the function has returned the actual
offset was written back to address offset. This address will always have an
alignment of packAlign to ensure efficient copying.

Stores the length of padding after a line of pixels to address padlen

Return value must be freed with ri.Hunk_FreeTempMemory()
==================
*/
static byte *RB_ReadPixels(int x, int y, int width, int height, size_t *offset, int *padlen, int lineAlign )
{
	byte *buffer, *bufstart;
	int padwidth, linelen;
	int	bufAlign;
	GLint packAlign;

	qglGetIntegerv(GL_PACK_ALIGNMENT, &packAlign);

	linelen = width * 3;

	if ( packAlign < lineAlign )
		padwidth = PAD(linelen, lineAlign);
	else
		padwidth = PAD(linelen, packAlign);

	bufAlign = MAX( packAlign, 16 ); // for SIMD

	// Allocate a few more bytes so that we can choose an alignment we like
	buffer = ri.Hunk_AllocateTempMemory(padwidth * height + *offset + bufAlign - 1);
	bufstart = PADP((intptr_t) buffer + *offset, bufAlign);

	qglReadPixels( x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, bufstart );

	*offset = bufstart - buffer;
	*padlen = PAD(linelen, packAlign) - linelen;

	return buffer;
}


/*
==================
RB_TakeScreenshot
==================
*/
void RB_TakeScreenshot( int x, int y, int width, int height, const char *fileName )
{
	const int header_size = 18;
	byte *allbuf, *buffer;
	byte *srcptr, *destptr;
	byte *endline, *endmem;
	byte temp;
	int linelen, padlen;
	size_t offset, memcount;

	offset = header_size;
	allbuf = RB_ReadPixels( x, y, width, height, &offset, &padlen, 0 );
	buffer = allbuf + offset - header_size;

	Com_Memset( buffer, 0, header_size );
	buffer[2] = 2;		// uncompressed type
	buffer[12] = width & 255;
	buffer[13] = width >> 8;
	buffer[14] = height & 255;
	buffer[15] = height >> 8;
	buffer[16] = 24;	// pixel size

	// swap rgb to bgr and remove padding from line endings
	linelen = width * 3;

	srcptr = destptr = allbuf + offset;
	endmem = srcptr + (linelen + padlen) * height;

	while(srcptr < endmem)
	{
		endline = srcptr + linelen;

		while(srcptr < endline)
		{
			temp = srcptr[0];
			*destptr++ = srcptr[2];
			*destptr++ = srcptr[1];
			*destptr++ = temp;

			srcptr += 3;
		}

		// Skip the pad
		srcptr += padlen;
	}

	memcount = linelen * height;

	// gamma correction
	R_GammaCorrect( allbuf + offset, memcount );

	ri.FS_WriteFile( fileName, buffer, memcount + header_size );

	ri.Hunk_FreeTempMemory( allbuf );
}


/*
==================
RB_TakeScreenshotJPEG
==================
*/
void RB_TakeScreenshotJPEG( int x, int y, int width, int height, const char *fileName )
{
	byte *buffer;
	size_t offset = 0, memcount;
	int padlen;

	buffer = RB_ReadPixels(x, y, width, height, &offset, &padlen, 0);
	memcount = (width * 3 + padlen) * height;

	// gamma correction
	R_GammaCorrect( buffer + offset, memcount );

	ri.CL_SaveJPG( fileName, r_screenshotJpegQuality->integer, width, height, buffer + offset, padlen );
	ri.Hunk_FreeTempMemory( buffer );
}


static void FillBMPHeader( byte *buffer, int width, int height, int memcount, int header_size )
{
	int filesize;
	Com_Memset( buffer, 0, header_size );

	// bitmap file header
	buffer[0] = 'B';
	buffer[1] = 'M';
	filesize = memcount + header_size;
	buffer[2] = (filesize >> 0) & 255;
	buffer[3] = (filesize >> 8) & 255;
	buffer[4] = (filesize >> 16) & 255;
	buffer[5] = (filesize >> 24) & 255;
	buffer[10] = header_size; // data offset

	// bitmap info header
	buffer[14] = 40; // size of this header
	buffer[18] = (width >> 0) & 255;
	buffer[19] = (width >> 8) & 255;
	buffer[20] = (width >> 16) & 255;
	buffer[21] = (width >> 24) & 255;

	buffer[22] = (height >> 0) & 255;
	buffer[23] = (height >> 8) & 255;
	buffer[24] = (height >> 16) & 255;
	buffer[25] = (height >> 24) & 255;
	buffer[26] = 1; // number of color planes
	buffer[28] = 24; // bpp

	buffer[34] = (memcount >> 0) & 255;
	buffer[35] = (memcount >> 8) & 255;
	buffer[36] = (memcount >> 16) & 255;
	buffer[37] = (memcount >> 24) & 255;
	buffer[38] = 0xC4; // horizontal dpi
	buffer[39] = 0x0E; // horizontal dpi
	buffer[42] = 0xC4; // vertical dpi
	buffer[43] = 0x0E; // vertical dpi
}


/*
==================
RB_TakeScreenshotBMP
==================
*/
void RB_TakeScreenshotBMP( int x, int y, int width, int height, const char *fileName, int clipboardOnly )
{
	byte *allbuf;
	byte *buffer; // destination buffer
	byte *srcptr, *srcline;
	byte *destptr, *dstline;
	byte *endmem;
	byte temp[4];
	size_t memcount, offset;
	const int header_size = 54; // bitmapfileheader(14) + bitmapinfoheader(40)
	int scanlen, padlen;
	int scanpad, len;

	offset = header_size;

	allbuf = RB_ReadPixels( x, y, width, height, &offset, &padlen, 4 );
	buffer = allbuf + offset;

	// scanline length
	scanlen = PAD( width*3, 4 );
	scanpad = scanlen - width*3;
	memcount = scanlen * height;

	// swap rgb to bgr and add line padding
	if ( scanpad == 0 && padlen == 0 ) {
		// fastest case
		srcptr = destptr = allbuf + offset;
		endmem = srcptr + scanlen * height;
		while ( srcptr < endmem ) {
			temp[0] = srcptr[0];
			destptr[0] = srcptr[2];
			destptr[2] = temp[0];
			destptr += 3;
			srcptr += 3;
		}
	} else {
		// move destination buffer forward if source padding is greater than for BMP
		if ( padlen > scanpad )
			buffer += (width * 3 + padlen - scanlen ) * height;
		// point on last line
		srcptr = allbuf + offset + (height-1) * (width * 3 + padlen);
		destptr = buffer + (height-1) * scanlen;
		len = (width * 3 - 3);
		while ( destptr >= buffer ) {
			srcline = srcptr + len;
			dstline = destptr + len;
			while ( srcline >= srcptr ) {
				temp[2] = srcline[0];
				temp[1] = srcline[1];
				temp[0] = srcline[2];
				dstline[0] = temp[0];
				dstline[1] = temp[1];
				dstline[2] = temp[2];
				dstline-=3;
				srcline-=3;
			}
			srcptr -= (width * 3 + padlen);
			destptr -= scanlen;
		}
	}

	// fill this last to avoid data overwrite in case when we're moving destination buffer forward
	FillBMPHeader( buffer - header_size, width, height, memcount, header_size );

	// gamma correction
	R_GammaCorrect( buffer, memcount );

	if ( clipboardOnly ) {
		// copy starting from bitmapinfoheader
		ri.Sys_SetClipboardBitmap( buffer - 40, memcount + 40 );
	} else {
		ri.FS_WriteFile( fileName, buffer - header_size, memcount + header_size );
	}

	ri.Hunk_FreeTempMemory( allbuf );
}


/*
==================
R_ScreenshotFilename
==================
*/
static void R_ScreenshotFilename( char *fileName, const char *fileExt ) {
	qtime_t t;
	int count;

	count = 0;
	ri.Com_RealTime( &t );

	Com_sprintf( fileName, MAX_OSPATH, "screenshots/shot-%04d%02d%02d-%02d%02d%02d.%s",
			1900 + t.tm_year, 1 + t.tm_mon,	t.tm_mday,
			t.tm_hour, t.tm_min, t.tm_sec, fileExt );

	while (	ri.FS_FileExists( fileName ) && ++count < 1000 ) {
		Com_sprintf( fileName, MAX_OSPATH, "screenshots/shot-%04d%02d%02d-%02d%02d%02d-%d.%s",
				1900 + t.tm_year, 1 + t.tm_mon,	t.tm_mday,
				t.tm_hour, t.tm_min, t.tm_sec, count, fileExt );
	}
}


/*
====================
R_LevelShot

levelshots are specialized 128*128 thumbnails for
the menu system, sampled down from full screen distorted images
====================
*/
static void R_LevelShot( void ) {
	char		checkname[MAX_OSPATH];
	byte		*buffer;
	byte		*source, *allsource;
	byte		*src, *dst;
	size_t		offset = 0;
	int			padlen;
	int			x, y;
	int			r, g, b;
	float		xScale, yScale;
	int			xx, yy;

	Com_sprintf(checkname, sizeof(checkname), "levelshots/%s.tga", tr.world->baseName);

	allsource = RB_ReadPixels(0, 0, gls.captureWidth, gls.captureHeight, &offset, &padlen, 0 );
	source = allsource + offset;

	buffer = ri.Hunk_AllocateTempMemory(128 * 128*3 + 18);
	Com_Memset (buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = 128;
	buffer[14] = 128;
	buffer[16] = 24;	// pixel size

	// resample from source
	xScale = glConfig.vidWidth / 512.0f;
	yScale = glConfig.vidHeight / 384.0f;
	for ( y = 0 ; y < 128 ; y++ ) {
		for ( x = 0 ; x < 128 ; x++ ) {
			r = g = b = 0;
			for ( yy = 0 ; yy < 3 ; yy++ ) {
				for ( xx = 0 ; xx < 4 ; xx++ ) {
					src = source + (3 * glConfig.vidWidth + padlen) * (int)((y*3 + yy) * yScale) +
						3 * (int) ((x*4 + xx) * xScale);
					r += src[0];
					g += src[1];
					b += src[2];
				}
			}
			dst = buffer + 18 + 3 * ( y * 128 + x );
			dst[0] = b / 12;
			dst[1] = g / 12;
			dst[2] = r / 12;
		}
	}

	// gamma correction
	R_GammaCorrect( buffer + 18, 128 * 128 * 3 );

	ri.FS_WriteFile( checkname, buffer, 128 * 128*3 + 18 );

	ri.Hunk_FreeTempMemory(buffer);
	ri.Hunk_FreeTempMemory(allsource);

	ri.Printf( PRINT_ALL, "Wrote %s\n", checkname );
}


/*
==================
R_ScreenShot_f

screenshot
screenshot [silent]
screenshot [levelshot]
screenshot [filename]

Doesn't print the pacifier message if there is a second arg
==================
*/
static void R_ScreenShot_f( void ) {
	char		checkname[MAX_OSPATH];
	qboolean	silent;
	int			typeMask;
	const char	*ext;

	if ( ri.CL_IsMinimized() && !RE_CanMinimize() ) {
		ri.Printf( PRINT_WARNING, "WARNING: unable to take screenshot when minimized because FBO is not available/enabled.\n" );
		return;
	}

	if ( !strcmp( ri.Cmd_Argv(1), "levelshot" ) ) {
		R_LevelShot();
		return;
	}

	if ( Q_stricmp( ri.Cmd_Argv(0), "screenshotJPEG" ) == 0 ) {
		typeMask = SCREENSHOT_JPG;
		ext = "jpg";
	} else if ( Q_stricmp( ri.Cmd_Argv(0), "screenshotBMP" ) == 0 ) {
		typeMask = SCREENSHOT_BMP;
		ext = "bmp";
	} else {
		typeMask = SCREENSHOT_TGA;
		ext = "tga";
	}

	// check if already scheduled
	if ( backEnd.screenshotMask & typeMask )
		return;

	if ( !strcmp( ri.Cmd_Argv(1), "silent" ) ) {
		silent = qtrue;
	} else if ( typeMask == SCREENSHOT_BMP && !strcmp( ri.Cmd_Argv(1), "clipboard" ) ) {
		backEnd.screenshotMask |= SCREENSHOT_BMP_CLIPBOARD;
		silent = qtrue;
	} else {
		silent = qfalse;
	}

	if ( ri.Cmd_Argc() == 2 && !silent ) {
		// explicit filename
		Com_sprintf( checkname, MAX_OSPATH, "screenshots/%s.%s", ri.Cmd_Argv( 1 ), ext );
	} else {
		if ( backEnd.screenshotMask & SCREENSHOT_BMP_CLIPBOARD ) {
			// no need for filename, copy to system buffer
			checkname[0] = '\0';
		} else {
			// scan for a free filename
			R_ScreenshotFilename( checkname, ext );
		}
	}

	// we will make screenshot right at the end of RE_EndFrame()
	backEnd.screenshotMask |= typeMask;
	if ( typeMask == SCREENSHOT_JPG ) {
		backEnd.screenShotJPGsilent = silent;
		Q_strncpyz( backEnd.screenshotJPG, checkname, sizeof( backEnd.screenshotJPG ) );
	} else if ( typeMask == SCREENSHOT_BMP ) {
		backEnd.screenShotBMPsilent = silent;
		Q_strncpyz( backEnd.screenshotBMP, checkname, sizeof( backEnd.screenshotBMP ) );
	} else {
		backEnd.screenShotTGAsilent = silent;
		Q_strncpyz( backEnd.screenshotTGA, checkname, sizeof( backEnd.screenshotTGA ) );
	}
}


//============================================================================

/*
==================
RB_TakeVideoFrameCmd
==================
*/
const void *RB_TakeVideoFrameCmd( const void *data )
{
	const videoFrameCommand_t *cmd;
	byte		*cBuf;
	size_t		memcount, linelen;
	int			padwidth, avipadwidth, padlen, avipadlen;
	int			packAlign;

	cmd = (const videoFrameCommand_t *)data;

	qglGetIntegerv(GL_PACK_ALIGNMENT, &packAlign);

	linelen = cmd->width * 3;

	// Alignment stuff for glReadPixels
	padwidth = PAD(linelen, packAlign);
	padlen = padwidth - linelen;
	// AVI line padding
	avipadwidth = PAD(linelen, AVI_LINE_PADDING);
	avipadlen = avipadwidth - linelen;

	cBuf = PADP(cmd->captureBuffer, packAlign);

	qglReadPixels(0, 0, cmd->width, cmd->height, GL_RGB,
		GL_UNSIGNED_BYTE, cBuf);

	memcount = padwidth * cmd->height;

	// gamma correction
	R_GammaCorrect( cBuf, memcount );

	if ( cmd->motionJpeg )
	{
		memcount = ri.CL_SaveJPGToBuffer( cmd->encodeBuffer, linelen * cmd->height,
			r_aviMotionJpegQuality->integer,
			cmd->width, cmd->height, cBuf, padlen );
		ri.CL_WriteAVIVideoFrame(cmd->encodeBuffer, memcount);
	}
	else
	{
		byte *lineend, *memend;
		byte *srcptr, *destptr;

		srcptr = cBuf;
		destptr = cmd->encodeBuffer;
		memend = srcptr + memcount;

		// swap R and B and remove line paddings
		while(srcptr < memend)
		{
			lineend = srcptr + linelen;
			while(srcptr < lineend)
			{
				*destptr++ = srcptr[2];
				*destptr++ = srcptr[1];
				*destptr++ = srcptr[0];
				srcptr += 3;
			}

			Com_Memset(destptr, '\0', avipadlen);
			destptr += avipadlen;

			srcptr += padlen;
		}

		ri.CL_WriteAVIVideoFrame(cmd->encodeBuffer, avipadwidth * cmd->height);
	}

	return (const void *)(cmd + 1);
}


//============================================================================

/*
** GL_SetDefaultState
*/
static void GL_SetDefaultState( void )
{
	int i;

	glState.currenttmu = 0;
	glState.currentArray = 0;

	for ( i = 0; i < MAX_TEXTURE_UNITS; i++ )
	{
		glState.currenttextures[ i ] = 0;
		glState.glClientStateBits[ i ] = 0;
	}

	qglClearDepth( 1.0f );

	qglCullFace( GL_FRONT );
	glState.faceCulling = -1;

	qglColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

	// initialize downstream texture unit if we're running
	// in a multitexture environment
	if ( qglActiveTextureARB )
	{
		qglActiveTextureARB( GL_TEXTURE1_ARB );
		GL_TextureMode( r_textureMode->string );
		GL_TexEnv( GL_MODULATE );
		qglDisable( GL_TEXTURE_2D );
		qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
		qglActiveTextureARB( GL_TEXTURE0_ARB );
	}

	qglEnable( GL_TEXTURE_2D );
	GL_TextureMode( r_textureMode->string );
	GL_TexEnv( GL_MODULATE );

	qglShadeModel( GL_SMOOTH );
	qglDepthFunc( GL_LEQUAL );

	// the vertex array is always enabled, but the color and texture
	// arrays are enabled and disabled around the compiled vertex array call
	qglEnableClientState( GL_VERTEX_ARRAY );

	qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
	qglDisableClientState( GL_COLOR_ARRAY );
	qglDisableClientState( GL_NORMAL_ARRAY );

	//
	// make sure our GL state vector is set correctly
	//
	glState.glStateBits = GLS_DEPTHTEST_DISABLE | GLS_DEPTHMASK_TRUE;

	qglPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	qglDepthMask( GL_TRUE );
	qglDisable( GL_DEPTH_TEST );
	qglEnable( GL_SCISSOR_TEST );
	qglDisable( GL_CULL_FACE );
	qglDisable( GL_BLEND );
}


/*
===============
RE_SyncRender
===============
*/
static void RE_SyncRender( void )
{
	if ( qglFinish && backEnd.doneSurfaces )
	{
		qglFinish();
	}
}


/*
================
R_PrintLongString

Workaround for ri.Printf's 1024 characters buffer limit.
================
*/
static void R_PrintLongString(const char *string) {
	char buffer[1024];
	const char *p;
	int size = strlen(string);

	p = string;
	while(size > 0)
	{
		Q_strncpyz(buffer, p, sizeof (buffer) );
		ri.Printf( PRINT_DEVELOPER, "%s", buffer );
		p += 1023;
		size -= 1023;
	}
}


/*
================
GfxInfo

Prints persistent rendering configuration
================
*/
static void GfxInfo( void )
{
	const char *enablestrings[] = { "disabled", "enabled" };
	const char *fsstrings[] = { "windowed", "fullscreen" };
	const char *fs;
	int mode;

	ri.Printf( PRINT_ALL, "\nGL_VENDOR: %s\n", glConfig.vendor_string );
	ri.Printf( PRINT_ALL, "GL_RENDERER: %s\n", glConfig.renderer_string );
	ri.Printf( PRINT_ALL, "GL_VERSION: %s\n", glConfig.version_string );
	ri.Printf( PRINT_DEVELOPER, "GL_EXTENSIONS: " );
	R_PrintLongString( glConfig.extensions_string );
	ri.Printf( PRINT_ALL, "\n" );
	ri.Printf( PRINT_ALL, "GL_MAX_TEXTURE_SIZE: %d\n", glConfig.maxTextureSize );
	ri.Printf( PRINT_ALL, "GL_MAX_TEXTURE_UNITS_ARB: %d\n", glConfig.numTextureUnits );
	ri.Printf( PRINT_ALL, "\nPIXELFORMAT: color(%d-bits) Z(%d-bit) stencil(%d-bits)\n", glConfig.colorBits, glConfig.depthBits, glConfig.stencilBits );

	if ( glConfig.isFullscreen )
	{
		const char *modefs = ri.Cvar_VariableString( "r_modeFullscreen" );
		if ( *modefs )
			mode = atoi( modefs );
		else
			mode = ri.Cvar_VariableIntegerValue( "r_mode" );
		fs = fsstrings[1];
	}
	else
	{
		mode = ri.Cvar_VariableIntegerValue( "r_mode" );
		fs = fsstrings[0];
	}

	if ( glConfig.vidWidth != gls.windowWidth || glConfig.vidHeight != gls.windowHeight )
	{
		ri.Printf( PRINT_ALL, "RENDER: %d x %d, MODE: %d, %d x %d %s hz:", glConfig.vidWidth, glConfig.vidHeight, mode, gls.windowWidth, gls.windowHeight, fs );
	}
	else
	{
		ri.Printf( PRINT_ALL, "MODE: %d, %d x %d %s hz:", mode, gls.windowWidth, gls.windowHeight, fs );
	}

	if ( glConfig.displayFrequency )
	{
		ri.Printf( PRINT_ALL, "%d\n", glConfig.displayFrequency );
	}
	else
	{
		ri.Printf( PRINT_ALL, "N/A\n" );
	}

	ri.Printf( PRINT_ALL, "multitexture: %s\n", enablestrings[qglActiveTextureARB != 0] );
	ri.Printf( PRINT_ALL, "compiled vertex arrays: %s\n", enablestrings[qglLockArraysEXT != 0 ] );
	ri.Printf( PRINT_ALL, "texenv add: %s\n", enablestrings[glConfig.textureEnvAddAvailable != 0] );
	ri.Printf( PRINT_ALL, "compressed textures: %s\n", enablestrings[glConfig.textureCompression!=TC_NONE] );
}


/*
================
VarInfo

Prints info that may change every R_Init() call
================
*/
static void VarInfo( void )
{
	if ( glConfig.deviceSupportsGamma ) {
		ri.Printf( PRINT_ALL, "GAMMA: hardware w/ %d overbright bits\n", tr.overbrightBits );
	} else {
		ri.Printf( PRINT_ALL, "GAMMA: software w/ %d overbright bits\n", tr.overbrightBits );
	}

	ri.Printf( PRINT_ALL, "texturemode: %s\n", r_textureMode->string );
	ri.Printf( PRINT_ALL, "texture bits: %d\n", r_texturebits->integer ? r_texturebits->integer : 32 );
	ri.Printf( PRINT_ALL, "picmip: %d%s\n", r_picmip->integer, r_nomip->integer ? ", worldspawn only" : "" );

	if ( r_vertexLight->integer || glConfig.hardwareType == GLHW_PERMEDIA2 ) {
		ri.Printf( PRINT_ALL, "HACK: using vertex lightmap approximation\n" );
	} else if ( glConfig.hardwareType == GLHW_RAGEPRO ) {
		ri.Printf( PRINT_ALL, "HACK: ragePro approximations\n" );
	} else if ( glConfig.hardwareType == GLHW_RIVA128 ) {
		ri.Printf( PRINT_ALL, "HACK: riva128 approximations\n" );
	}

	if ( r_finish->integer ) {
		ri.Printf( PRINT_ALL, "Forcing glFinish\n" );
	}
}


/*
===============
GfxInfo_f
===============
*/
static void GfxInfo_f( void )
{
	GfxInfo();
	VarInfo();
}


/*
===============
R_Register
===============
*/
static void R_Register( void )
{
	// make sure all the commands added here are also removed in R_Shutdown
	ri.Cmd_AddCommand( "imagelist", R_ImageList_f );
	ri.Cmd_AddCommand( "shaderlist", R_ShaderList_f );
	ri.Cmd_AddCommand( "skinlist", R_SkinList_f );
	ri.Cmd_AddCommand( "modellist", R_Modellist_f );
	ri.Cmd_AddCommand( "screenshot", R_ScreenShot_f );
	ri.Cmd_AddCommand( "screenshotJPEG", R_ScreenShot_f );
	ri.Cmd_AddCommand( "screenshotBMP", R_ScreenShot_f );
	ri.Cmd_AddCommand( "gfxinfo", GfxInfo_f );

	//
	// temporary latched variables that can only change over a restart
	//
	r_fullbright = ri.Cvar_Get( "r_fullbright", "0", CVAR_LATCH );
	ri.Cvar_SetDescription( r_fullbright, "Debugging tool to render the entire level without lighting." );
	r_overBrightBits = ri.Cvar_Get( "r_overBrightBits", "1", CVAR_ARCHIVE_ND | CVAR_LATCH );
	ri.Cvar_SetDescription( r_overBrightBits, "Sets the intensity of overall brightness of texture pixels." );
	r_mapOverBrightBits = ri.Cvar_Get( "r_mapOverBrightBits", "2", CVAR_ARCHIVE_ND | CVAR_LATCH );
	ri.Cvar_SetDescription( r_mapOverBrightBits, "Sets the number of overbright bits baked into all lightmaps and map data." );
	r_intensity = ri.Cvar_Get( "r_intensity", "1", CVAR_ARCHIVE_ND | CVAR_LATCH );
	ri.Cvar_CheckRange( r_intensity, "1", "255", CV_FLOAT );
	ri.Cvar_SetDescription( r_intensity, "Global texture lighting scale." );
	r_singleShader = ri.Cvar_Get( "r_singleShader", "0", CVAR_CHEAT | CVAR_LATCH );
	ri.Cvar_SetDescription( r_singleShader, "Debugging tool that only uses the default shader for all rendering." );
	r_defaultImage = ri.Cvar_Get( "r_defaultImage", "", CVAR_ARCHIVE_ND | CVAR_LATCH );
	ri.Cvar_SetDescription( r_defaultImage, "Replace default (missing) image texture by either exact file or solid #rgb|#rrggbb background color." );

	r_simpleMipMaps = ri.Cvar_Get( "r_simpleMipMaps", "1", CVAR_ARCHIVE_ND | CVAR_LATCH );
	ri.Cvar_SetDescription( r_simpleMipMaps, "Whether or not to use a simple mipmapping algorithm or a more correct one:\n 0: off (proper linear filter)\n 1: on (for slower machines)" );
	r_vertexLight = ri.Cvar_Get( "r_vertexLight", "0", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_vertexLight, "Set to 1 to use vertex light instead of lightmaps, collapse all multi-stage shaders into single-stage ones, might cause rendering artifacts." );

	r_picmip = ri.Cvar_Get( "r_picmip", "0", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_CheckRange( r_picmip, "0", "16", CV_INTEGER );
	ri.Cvar_SetDescription( r_picmip, "Set texture quality, lower is better." );

	r_nomip = ri.Cvar_Get( "r_nomip", "0", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_CheckRange( r_nomip, "0", "1", CV_INTEGER );
	ri.Cvar_SetDescription( r_nomip, "Apply picmip only on worldspawn textures." );

	r_neatsky = ri.Cvar_Get( "r_neatsky", "0", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_neatsky, "Disables texture mipping for skies." );
	r_roundImagesDown = ri.Cvar_Get ("r_roundImagesDown", "1", CVAR_ARCHIVE_ND | CVAR_LATCH );
	ri.Cvar_SetDescription( r_roundImagesDown, "When images are scaled, round images down instead of up." );
	r_colorMipLevels = ri.Cvar_Get ("r_colorMipLevels", "0", CVAR_LATCH );
	ri.Cvar_SetDescription( r_colorMipLevels, "Debugging tool to artificially color different mipmap levels so that they are more apparent." );
	r_detailTextures = ri.Cvar_Get( "r_detailtextures", "1", CVAR_ARCHIVE_ND | CVAR_LATCH );
	ri.Cvar_SetDescription( r_detailTextures, "Enables usage of shader stages flagged as detail." );
	r_texturebits = ri.Cvar_Get( "r_texturebits", "0", CVAR_ARCHIVE_ND | CVAR_LATCH );
	ri.Cvar_SetDescription( r_texturebits, "Number of texture bits per texture." );

	r_mergeLightmaps = ri.Cvar_Get( "r_mergeLightmaps", "1", CVAR_ARCHIVE_ND | CVAR_LATCH );
	ri.Cvar_SetDescription( r_mergeLightmaps, "Merge built-in small lightmaps into bigger lightmaps (atlases)." );

#ifdef USE_VBO
	r_vbo = ri.Cvar_Get( "r_vbo", "0", CVAR_ARCHIVE_ND | CVAR_LATCH );
	ri.Cvar_SetDescription( r_vbo, "Use Vertex Buffer Objects to cache static map geometry, may improve FPS on modern GPUs, increases hunk memory usage by 15-30MB (map-dependent)." );
#endif

	r_mapGreyScale = ri.Cvar_Get( "r_mapGreyScale", "0", CVAR_ARCHIVE_ND | CVAR_LATCH );
	ri.Cvar_CheckRange( r_mapGreyScale, "-1", "1", CV_FLOAT );
	ri.Cvar_SetDescription(r_mapGreyScale, "Desaturate world map textures only, works independently from \\r_greyscale, negative values only desaturate lightmaps.");

	r_subdivisions = ri.Cvar_Get( "r_subdivisions", "1", CVAR_ARCHIVE_ND | CVAR_LATCH );
	ri.Cvar_SetDescription(r_subdivisions, "Distance to subdivide bezier curved surfaces. Higher values mean less subdivision and less geometric complexity.");

	r_maxpolys = ri.Cvar_Get( "r_maxpolys", XSTRING( MAX_POLYS ), CVAR_LATCH );
	ri.Cvar_SetDescription( r_maxpolys, "Maximum number of polygons to draw in a scene." );
	r_maxpolyverts = ri.Cvar_Get( "r_maxpolyverts", XSTRING( MAX_POLYVERTS ), CVAR_LATCH );
	ri.Cvar_SetDescription( r_maxpolyverts, "Maximum number of polygon vertices to draw in a scene." );

	//
	// archived variables that can change at any time
	//
	r_lodCurveError = ri.Cvar_Get( "r_lodCurveError", "250", CVAR_ARCHIVE_ND );
	ri.Cvar_CheckRange( r_lodCurveError, "-1", "8192", CV_FLOAT );
	ri.Cvar_SetDescription( r_lodCurveError, "Level of detail error on curved surface grids. Higher values result in better quality at a distance." );
	r_lodbias = ri.Cvar_Get( "r_lodbias", "-2", CVAR_ARCHIVE_ND );
	ri.Cvar_SetDescription( r_lodbias, "Sets the level of detail of in-game models:\n -2: Ultra (further delays LOD transition in the distance)\n -1: Very High (delays LOD transition in the distance)\n 0: High\n 1: Medium\n 2: Low" );
	r_znear = ri.Cvar_Get( "r_znear", "4", CVAR_CHEAT );
	ri.Cvar_CheckRange( r_znear, "0.001", "200", CV_FLOAT );
	ri.Cvar_SetDescription( r_znear, "Viewport distance from view origin (how close objects can be to the player before they're clipped out of the scene)." );
	r_zproj = ri.Cvar_Get( "r_zproj", "64", CVAR_ARCHIVE_ND );
	ri.Cvar_SetDescription( r_zproj, "Projected viewport frustum." );
	r_stereoSeparation = ri.Cvar_Get( "r_stereoSeparation", "64", CVAR_ARCHIVE_ND );
	ri.Cvar_SetDescription( r_stereoSeparation, "Control eye separation. Resulting separation is \\r_zproj divided by this value in standard units." );
	r_ignoreGLErrors = ri.Cvar_Get( "r_ignoreGLErrors", "1", CVAR_ARCHIVE_ND );
	ri.Cvar_SetDescription( r_ignoreGLErrors, "Ignore OpenGL errors." );
	r_teleporterFlash = ri.Cvar_Get( "r_teleporterFlash", "1", CVAR_ARCHIVE );
	ri.Cvar_SetDescription( r_teleporterFlash, "Show a white screen instead of a black screen when being teleported in hyperspace." );
	r_fastsky = ri.Cvar_Get( "r_fastsky", "0", CVAR_ARCHIVE_ND );
	ri.Cvar_SetDescription( r_fastsky, "Draw flat colored skies." );
	r_drawSun = ri.Cvar_Get( "r_drawSun", "0", CVAR_ARCHIVE_ND );
	ri.Cvar_SetDescription( r_drawSun, "Draw sun shader in skies." );
	r_dynamiclight = ri.Cvar_Get( "r_dynamiclight", "1", CVAR_ARCHIVE );
	ri.Cvar_SetDescription( r_dynamiclight, "Enables dynamic lighting." );
#ifdef USE_PMLIGHT
#if arm32 || arm64 // RPi4 GL driver have very poor ARB shaders performance...
	r_dlightMode = ri.Cvar_Get( "r_dlightMode", "0", CVAR_ARCHIVE );
#else
	r_dlightMode = ri.Cvar_Get( "r_dlightMode", "1", CVAR_ARCHIVE );
#endif
	ri.Cvar_CheckRange( r_dlightMode, "0", "2", CV_INTEGER );
	ri.Cvar_SetDescription( r_dlightMode, "Dynamic light mode:\n 0: VQ3 'fake' dynamic lights\n 1: High-quality per-pixel dynamic lights, slightly faster than VQ3's on modern hardware\n 2: Same as 1 but applies to all MD3 models too" );
	r_dlightScale = ri.Cvar_Get( "r_dlightScale", "0.5", CVAR_ARCHIVE_ND );
	ri.Cvar_CheckRange( r_dlightScale, "0.1", "1", CV_FLOAT );
	ri.Cvar_SetDescription( r_dlightScale, "Scales dynamic light radius." );
	r_dlightSpecPower = ri.Cvar_Get( "r_dlightSpecPower", "8", CVAR_ARCHIVE_ND );
	ri.Cvar_CheckRange( r_dlightSpecPower, "1", "32", CV_FLOAT );
	ri.Cvar_SetDescription( r_dlightSpecPower, "Factors specularity effect from dynamic lights on surfaces." );
	ri.Cvar_SetGroup( r_dlightSpecPower, CVG_RENDERER );
	r_dlightSpecColor = ri.Cvar_Get( "r_dlightSpecColor", "-0.25", CVAR_ARCHIVE_ND );
	ri.Cvar_CheckRange( r_dlightSpecColor, "-1", "1", CV_FLOAT );
	ri.Cvar_SetDescription( r_dlightSpecColor, "Color base for specular component:\n <= 0: use current texture and modulate by abs(r_dlightSpecColor)\n > 0: use constant color with RGB components set to \\r_dlightSpecColor" );
	ri.Cvar_SetGroup( r_dlightSpecColor, CVG_RENDERER );
	r_dlightIntensity = ri.Cvar_Get( "r_dlightIntensity", "1.0", CVAR_ARCHIVE_ND );
	ri.Cvar_CheckRange( r_dlightIntensity, "0.1", "1", CV_FLOAT );
	ri.Cvar_SetDescription( r_dlightIntensity, "Adjusts dynamic light intensity but not radius." );
#endif // USE_PMLIGHT
	r_dlightSaturation = ri.Cvar_Get( "r_dlightSaturation", "1", CVAR_ARCHIVE_ND );
	ri.Cvar_CheckRange( r_dlightSaturation, "0", "1", CV_FLOAT );

#ifdef USE_FBO
	r_ext_multisample = ri.Cvar_Get( "r_ext_multisample", "0", CVAR_ARCHIVE_ND );
	ri.Cvar_CheckRange( r_ext_multisample, "0", "8", CV_INTEGER );
	ri.Cvar_SetDescription( r_ext_multisample, "For anti-aliasing geometry edges, valid values: 0|2|4|6|8. Requires \\r_fbo 1." );
	ri.Cvar_SetGroup( r_ext_multisample, CVG_RENDERER );
	r_hdr = ri.Cvar_Get( "r_hdr", "0", CVAR_ARCHIVE_ND );
	ri.Cvar_SetDescription(r_hdr, "Enables high dynamic range frame buffer texture format. Requires \\r_fbo 1.\n -1: 4-bit, for testing purposes, heavy color banding, might not work on all systems\n  0: 8 bit, default, moderate color banding with multi-stage shaders\n  1: 16 bit, enhanced blending precision, no color banding, might decrease performance on AMD / Intel GPUs\n" );
	ri.Cvar_SetGroup( r_hdr, CVG_RENDERER );
	// bloom
	r_bloom = ri.Cvar_Get( "r_bloom", "0", CVAR_ARCHIVE_ND );
	r_bloom->flags &= ~CVAR_LATCH; // If we were running renderervk before, we need to remove latch
	ri.Cvar_SetDescription(r_bloom, "Enables bloom post-processing effect. Requires \\r_fbo 1.");
	r_bloom_threshold = ri.Cvar_Get( "r_bloom_threshold", "0.6", CVAR_ARCHIVE_ND );
	ri.Cvar_SetDescription(r_bloom_threshold, "Color level to extract to bloom texture, default is 0.6.");
	ri.Cvar_SetGroup( r_bloom_threshold, CVG_RENDERER );
	r_bloom_threshold_mode = ri.Cvar_Get( "r_bloom_threshold_mode", "0", CVAR_ARCHIVE_ND );
	ri.Cvar_SetDescription( r_bloom_threshold_mode, "Color extraction mode:\n 0: (r|g|b) >= threshold\n 1: (r + g + b ) / 3 >= threshold\n 2: luma(r, g, b) >= threshold" );
	ri.Cvar_SetGroup( r_bloom_threshold_mode, CVG_RENDERER );
	r_bloom_intensity = ri.Cvar_Get( "r_bloom_intensity", "0.5", CVAR_ARCHIVE_ND );
	ri.Cvar_SetDescription( r_bloom_intensity, "Final bloom blend factor, default is 0.5." );
	r_bloom_passes = ri.Cvar_Get( "r_bloom_passes", "5", CVAR_ARCHIVE_ND | CVAR_LATCH );
	ri.Cvar_CheckRange( r_bloom_passes, "3", XSTRING( MAX_BLUR_PASSES ), CV_INTEGER );
	ri.Cvar_SetDescription( r_bloom_passes, "Count of downsampled passes (framebuffers) to blend on final bloom image, default is 5." );
	r_bloom_blend_base = ri.Cvar_Get( "r_bloom_blend_base", "1", CVAR_ARCHIVE_ND );
	ri.Cvar_CheckRange( r_bloom_blend_base, "0", va("%i", r_bloom_passes->integer-1), CV_INTEGER );
	ri.Cvar_SetDescription( r_bloom_blend_base, "0-based, topmost downsampled framebuffer to use for final image, high values can be used for stronger haze effect, results in overall weaker intensity." );
	ri.Cvar_SetGroup( r_bloom_blend_base, CVG_RENDERER );
	r_bloom_modulate = ri.Cvar_Get( "r_bloom_modulate", "0", CVAR_ARCHIVE_ND );
	ri.Cvar_SetDescription( r_bloom_modulate, "Modulate extracted color:\n 0: off (color = color, i.e. no changes)\n 1: by itself (color = color * color)\n 2: by intensity (color = color * luma(color))" );
	ri.Cvar_SetGroup( r_bloom_modulate, CVG_RENDERER );
	r_bloom_filter_size = ri.Cvar_Get( "r_bloom_filter_size", "6", CVAR_ARCHIVE_ND );
	ri.Cvar_CheckRange( r_bloom_filter_size, XSTRING( MIN_FILTER_SIZE ), XSTRING( MAX_FILTER_SIZE ), CV_INTEGER );
	ri.Cvar_SetDescription( r_bloom_filter_size, "Filter size of Gaussian Blur effect for each pass, bigger filter size means stronger and wider blur, lower values are faster, default is 6." );
	ri.Cvar_SetGroup( r_bloom_filter_size, CVG_RENDERER );

	r_bloom_reflection = ri.Cvar_Get( "r_bloom_reflection", "0", CVAR_ARCHIVE_ND );
	ri.Cvar_CheckRange( r_bloom_reflection, "-4", "4", CV_FLOAT );
	ri.Cvar_SetDescription( r_bloom_reflection, "Bloom lens reflection effect, value is an intensity factor of the effect, negative value means blend only reflection and skip main bloom texture." );
#endif // USE_FBO

	r_dlightBacks = ri.Cvar_Get( "r_dlightBacks", "1", CVAR_ARCHIVE_ND );
	ri.Cvar_SetDescription( r_dlightBacks, "Whether or not dynamic lights should light up back-face culled geometry, affects only VQ3 dynamic lights." );
	r_finish = ri.Cvar_Get( "r_finish", "0", CVAR_ARCHIVE_ND );
	ri.Cvar_SetDescription( r_finish, "Force a glFinish call after rendering a scene." );
	r_textureMode = ri.Cvar_Get( "r_textureMode", "GL_LINEAR_MIPMAP_NEAREST", CVAR_ARCHIVE );
	ri.Cvar_SetDescription( r_textureMode, "Texture interpolation mode:\n GL_NEAREST: Nearest neighbor interpolation and will therefore appear similar to Quake II except with the added colored lighting\n GL_LINEAR: Linear interpolation and will appear to blend in objects that are closer than the resolution that the textures are set as\n GL_NEAREST_MIPMAP_NEAREST: Nearest neighbor interpolation with mipmapping for bilinear hardware, mipmapping will blend objects that are farther away than the resolution that they are set as\n GL_LINEAR_MIPMAP_NEAREST: Linear interpolation with mipmapping for bilinear hardware\n GL_NEAREST_MIPMAP_LINEAR: Nearest neighbor interpolation with mipmapping for trilinear hardware\n GL_LINEAR_MIPMAP_LINEAR: Linear interpolation with mipmapping for trilinear hardware" );
	ri.Cvar_SetGroup( r_textureMode, CVG_RENDERER );
	r_gamma = ri.Cvar_Get( "r_gamma", "1", CVAR_ARCHIVE_ND );
	ri.Cvar_CheckRange( r_gamma, "0.5", "3", CV_FLOAT );
	ri.Cvar_SetDescription( r_gamma, "Gamma correction factor." );
	ri.Cvar_SetGroup( r_gamma, CVG_RENDERER );
	r_facePlaneCull = ri.Cvar_Get ("r_facePlaneCull", "1", CVAR_ARCHIVE_ND );
	ri.Cvar_SetDescription( r_facePlaneCull, "Enables culling of planar surfaces with back side test." );

	r_railWidth = ri.Cvar_Get( "r_railWidth", "16", CVAR_ARCHIVE_ND );
	ri.Cvar_SetDescription( r_railWidth, "Radius of railgun trails." );
	r_railCoreWidth = ri.Cvar_Get( "r_railCoreWidth", "6", CVAR_ARCHIVE_ND );
	ri.Cvar_SetDescription( r_railCoreWidth, "Size of railgun trail rings when enabled in game code (normally \\cg_oldRail 0)." );
	r_railSegmentLength = ri.Cvar_Get( "r_railSegmentLength", "32", CVAR_ARCHIVE_ND );
	ri.Cvar_SetDescription( r_railSegmentLength, "Length of segments in railgun trails." );

	r_ambientScale = ri.Cvar_Get( "r_ambientScale", "0.6", CVAR_CHEAT );
	ri.Cvar_SetDescription( r_ambientScale, "Light grid ambient light scaling on entity models." );
	r_directedScale = ri.Cvar_Get( "r_directedScale", "1", CVAR_CHEAT );
	ri.Cvar_SetDescription( r_directedScale, "Light grid direct light scaling on entity models." );

	r_anaglyphMode = ri.Cvar_Get( "r_anaglyphMode", "0", CVAR_ARCHIVE_ND );
	ri.Cvar_SetDescription( r_anaglyphMode, "Enable rendering of anaglyph images. Valid options for 3D glasses types:\n 0: Disabled\n 1: Red-cyan\n 2: Red-blue\n 3: Red-green\n 4: Green-magenta" );

	r_greyscale = ri.Cvar_Get( "r_greyscale", "0", CVAR_ARCHIVE_ND );
	ri.Cvar_CheckRange( r_greyscale, "-1", "1", CV_FLOAT );
	ri.Cvar_SetDescription( r_greyscale, "Desaturate rendered frame, requires \\r_fbo 1." );
	ri.Cvar_SetGroup( r_greyscale, CVG_RENDERER );

	//
	// temporary variables that can change at any time
	//
	r_showImages = ri.Cvar_Get( "r_showImages", "0", CVAR_TEMP );
	ri.Cvar_SetDescription( r_showImages, "Draw all images currently loaded into memory:\n 0: Disabled\n 1: Show images set to uniform size\n 2: Show images with scaled relative to largest image" );

	r_debugLight = ri.Cvar_Get( "r_debuglight", "0", CVAR_TEMP );
	ri.Cvar_SetDescription( r_debugLight, "Debugging tool to print ambient and directed lighting information." );
	r_debugSort = ri.Cvar_Get( "r_debugSort", "0", CVAR_CHEAT );
	ri.Cvar_SetDescription( r_debugSort, "Debugging tool to filter out shaders with depth sorting order values higher than the set value." );
	r_printShaders = ri.Cvar_Get( "r_printShaders", "0", 0 );
	ri.Cvar_SetDescription( r_printShaders, "Debugging tool to print on console of the number of shaders used." );
	r_saveFontData = ri.Cvar_Get( "r_saveFontData", "0", 0 );

	r_nocurves = ri.Cvar_Get ("r_nocurves", "0", CVAR_CHEAT );
	ri.Cvar_SetDescription( r_nocurves, "Set to 1 to disable drawing world bezier curves. Set to 0 to enable." );
	r_drawworld = ri.Cvar_Get ("r_drawworld", "1", CVAR_CHEAT );
	ri.Cvar_SetDescription( r_drawworld, "Set to 0 to disable drawing the world. Set to 1 to enable." );
	r_lightmap = ri.Cvar_Get ("r_lightmap", "0", 0 );
	ri.Cvar_SetDescription( r_lightmap, "Show only lightmaps on all world surfaces." );
	r_portalOnly = ri.Cvar_Get ("r_portalOnly", "0", CVAR_CHEAT );
	ri.Cvar_SetDescription( r_portalOnly, "Set to 1 to render only first mirror/portal view if it is present on the scene." );

	r_flareSize = ri.Cvar_Get( "r_flareSize", "40", CVAR_CHEAT );
	ri.Cvar_SetDescription( r_flareSize, "Radius of light flares. Requires \\r_flares 1." );
	r_flareFade = ri.Cvar_Get( "r_flareFade", "10", CVAR_CHEAT );
	ri.Cvar_SetDescription( r_flareFade, "Distance to fade out light flares. Requires \\r_flares 1." );
	r_flareCoeff = ri.Cvar_Get( "r_flareCoeff", "150", CVAR_CHEAT );
	ri.Cvar_CheckRange( r_flareCoeff, "0.1", NULL, CV_FLOAT );
	ri.Cvar_SetDescription( r_flareCoeff, "Coefficient for the light flare intensity falloff function. Requires \\r_flares 1." );

	r_skipBackEnd = ri.Cvar_Get ("r_skipBackEnd", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription( r_skipBackEnd, "Skips loading rendering backend." );

	r_lodscale = ri.Cvar_Get( "r_lodscale", "5", CVAR_CHEAT );
	ri.Cvar_SetDescription( r_lodscale, "Set scale for level of detail adjustment." );
	r_norefresh = ri.Cvar_Get ("r_norefresh", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription( r_norefresh, "Bypasses refreshing of the rendered scene." );
	r_drawentities = ri.Cvar_Get ("r_drawentities", "1", CVAR_CHEAT );
	ri.Cvar_SetDescription( r_drawentities, "Draw all world entities." );
	r_nocull = ri.Cvar_Get ("r_nocull", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription( r_nocull, "Draw all culled objects." );
	r_novis = ri.Cvar_Get ("r_novis", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription( r_novis, "Disables usage of PVS." );
	r_showcluster = ri.Cvar_Get ("r_showcluster", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription( r_showcluster, "Shows current cluster index." );
	r_speeds = ri.Cvar_Get ("r_speeds", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription( r_speeds, "Prints out various debugging stats from PVS:\n 0: Disabled\n 1: Backend BSP\n 2: Frontend grid culling\n 3: Current view cluster index\n 4: Dynamic lighting\n 5: zFar clipping\n 6: Flares" );
	r_debugSurface = ri.Cvar_Get ("r_debugSurface", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription( r_debugSurface, "Backend visual debugging tool for bezier mesh surfaces." );
	r_nobind = ri.Cvar_Get ("r_nobind", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription( r_nobind, "Backend debugging tool: Disables texture binding." );
	r_showtris = ri.Cvar_Get ("r_showtris", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription( r_showtris, "Debugging tool: Wireframe rendering of polygon triangles in the world." );
	r_showsky = ri.Cvar_Get( "r_showsky", "0", 0 );
	ri.Cvar_SetDescription( r_showsky, "Forces sky in front of all surfaces." );
	r_shownormals = ri.Cvar_Get ("r_shownormals", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription( r_shownormals, "Debugging tool: Show wireframe surface normals." );
	r_clear = ri.Cvar_Get( "r_clear", "0", 0 );
	ri.Cvar_SetDescription( r_clear, "Forces screen buffer clearing every frame, removing any hall of mirrors effect in void.\n Use \\r_clearColor to set color." );
	r_offsetFactor = ri.Cvar_Get( "r_offsetFactor", "-1", CVAR_CHEAT );
	ri.Cvar_SetDescription( r_offsetFactor, "Offset factor for shaders with polygonOffset stages." );
	r_offsetUnits = ri.Cvar_Get( "r_offsetunits", "-2", CVAR_CHEAT );
	ri.Cvar_SetDescription( r_offsetUnits, "Offset units for shaders with polygonOffset stages." );
	r_drawBuffer = ri.Cvar_Get( "r_drawBuffer", "GL_BACK", CVAR_CHEAT );
	ri.Cvar_SetDescription( r_drawBuffer, "Sets which frame buffer to draw into." );
	r_lockpvs = ri.Cvar_Get ("r_lockpvs", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription( r_lockpvs, "Debugging tool: Locks to current potentially visible set. Useful for testing vis-culling in maps." );
	r_noportals = ri.Cvar_Get( "r_noportals", "0", 0 );
	ri.Cvar_SetDescription(r_noportals, "Disables in-game portals, valid values: 0: Portals enabled\n 1: Portals disabled\n 2: Portals and mirrors disabled" );
	r_shadows = ri.Cvar_Get( "cg_shadows", "1", 0 );

	r_marksOnTriangleMeshes = ri.Cvar_Get("r_marksOnTriangleMeshes", "0", CVAR_ARCHIVE_ND );
	ri.Cvar_SetDescription( r_marksOnTriangleMeshes, "Enables impact marks on triangle mesh surfaces (ie: MD3 models.) Requires impact marks to be enabled in the game code." );

	r_aviMotionJpegQuality = ri.Cvar_Get( "r_aviMotionJpegQuality", "90", CVAR_ARCHIVE_ND );
	ri.Cvar_SetDescription( r_aviMotionJpegQuality, "Controls quality of Jpeg video capture when \\cl_aviMotionJpeg 1." );
	r_screenshotJpegQuality = ri.Cvar_Get( "r_screenshotJpegQuality", "90", CVAR_ARCHIVE_ND );
	ri.Cvar_SetDescription( r_screenshotJpegQuality, "Controls quality of Jpeg screenshots when using screenshotJpeg." );

	if ( glConfig.vidWidth )
		return;

	//
	// latched and archived variables that can only change over a vid_restart
	//
	r_allowExtensions = ri.Cvar_Get( "r_allowExtensions", "1", CVAR_ARCHIVE_ND | CVAR_LATCH | CVAR_DEVELOPER );
	ri.Cvar_SetDescription( r_allowExtensions, "Use all of the OpenGL extensions your card is capable of." );
	r_ext_compressed_textures = ri.Cvar_Get( "r_ext_compressed_textures", "0", CVAR_ARCHIVE_ND | CVAR_LATCH | CVAR_DEVELOPER );
	ri.Cvar_SetDescription( r_ext_compressed_textures, "Enables texture compression." );
	r_ext_multitexture = ri.Cvar_Get( "r_ext_multitexture", "1", CVAR_ARCHIVE_ND | CVAR_LATCH | CVAR_DEVELOPER );
	ri.Cvar_SetDescription( r_ext_multitexture, "Enables hardware multi-texturing (0: off, 1: on)." );
	r_ext_compiled_vertex_array = ri.Cvar_Get( "r_ext_compiled_vertex_array", "1", CVAR_ARCHIVE_ND | CVAR_LATCH | CVAR_DEVELOPER );
	ri.Cvar_SetDescription( r_ext_compiled_vertex_array, "Enables hardware-compiled vertex array rendering method." );
	r_ext_texture_env_add = ri.Cvar_Get( "r_ext_texture_env_add", "1", CVAR_ARCHIVE_ND | CVAR_LATCH | CVAR_DEVELOPER );
	ri.Cvar_SetDescription( r_ext_texture_env_add, "Enables additive blending in multitexturing. Requires \\r_ext_multitexture 1." );

	r_ext_texture_filter_anisotropic = ri.Cvar_Get( "r_ext_texture_filter_anisotropic",	"1", CVAR_ARCHIVE_ND | CVAR_LATCH );
	ri.Cvar_CheckRange( r_ext_texture_filter_anisotropic, "0", "1", CV_INTEGER );
	ri.Cvar_SetDescription( r_ext_texture_filter_anisotropic, "Allow anisotropic filtering." );

	r_ext_max_anisotropy = ri.Cvar_Get( "r_ext_max_anisotropy", "8", CVAR_ARCHIVE_ND | CVAR_LATCH );
	ri.Cvar_CheckRange( r_ext_max_anisotropy, "1", NULL, CV_INTEGER );
	ri.Cvar_SetDescription( r_ext_max_anisotropy, "Sets maximum anisotropic level for your graphics driver. Requires \\r_ext_texture_filter_anisotropic." );

	//r_stencilbits = ri.Cvar_Get( "r_stencilbits", "8", CVAR_ARCHIVE_ND | CVAR_LATCH );
	r_ignorehwgamma = ri.Cvar_Get( "r_ignorehwgamma", "0", CVAR_ARCHIVE_ND | CVAR_LATCH );
	ri.Cvar_CheckRange( r_ignorehwgamma, "0", "1", CV_INTEGER );
	ri.Cvar_SetDescription( r_ignorehwgamma, "Overrides hardware gamma capabilities." );

	r_flares = ri.Cvar_Get( "r_flares", "0", CVAR_ARCHIVE_ND | CVAR_LATCH );
	ri.Cvar_SetDescription( r_flares, "Enables corona effects on light sources." );

#ifdef USE_FBO
	r_fbo = ri.Cvar_Get( "r_fbo", "0", CVAR_ARCHIVE_ND | CVAR_LATCH );
	ri.Cvar_SetDescription( r_fbo, "Use framebuffer objects, enables gamma correction in windowed mode and allows arbitrary video size and screenshot/video capture.\n Required for bloom, HDR rendering, anti-aliasing and greyscale effects.\n OpenGL 3.0+ required." );

	r_ext_supersample = ri.Cvar_Get( "r_ext_supersample", "0", CVAR_ARCHIVE_ND | CVAR_LATCH );
	ri.Cvar_CheckRange( r_ext_supersample, "0", "1", CV_INTEGER );
	ri.Cvar_SetDescription( r_ext_supersample, "Super-sample anti-aliasing, requires \\r_fbo 1." );

	r_renderWidth = ri.Cvar_Get( "r_renderWidth", "800", CVAR_ARCHIVE_ND | CVAR_LATCH );
	ri.Cvar_CheckRange( r_renderWidth, "96", NULL, CV_INTEGER );
	ri.Cvar_SetDescription( r_renderWidth, "Video width to render to when \\r_renderScale > 0." );
	r_renderHeight = ri.Cvar_Get( "r_renderHeight", "600", CVAR_ARCHIVE_ND | CVAR_LATCH );
	ri.Cvar_CheckRange( r_renderHeight, "72", NULL, CV_INTEGER );
	ri.Cvar_SetDescription( r_renderHeight, "Video height to render to when \\r_renderScale > 0." );

	r_renderScale = ri.Cvar_Get( "r_renderScale", "0", CVAR_ARCHIVE_ND | CVAR_LATCH );
	ri.Cvar_CheckRange( r_renderScale, "0", "4", CV_INTEGER );
	ri.Cvar_SetDescription( r_renderScale, "Scaling mode to be used with custom render resolution:\n"
		" 0 - disabled\n"
		" 1 - nearest filtering, stretch to full size\n"
		" 2 - nearest filtering, preserve aspect ratio (black bars on sides)\n"
		" 3 - linear filtering, stretch to full size\n"
		" 4 - linear filtering, preserve aspect ratio (black bars on sides)\n" );
#endif // USE_FBO
}

#define EPSILON 1e-6f

/*
===============
R_Init
===============
*/
void R_Init( void ) {
	int	err;
	int i;
	byte *ptr;

	ri.Printf( PRINT_ALL, "----- R_Init -----\n" );

	// clear all our internal state
	Com_Memset( &tr, 0, sizeof( tr ) );
	Com_Memset( &backEnd, 0, sizeof( backEnd ) );
	Com_Memset( &tess, 0, sizeof( tess ) );
	Com_Memset( &glState, 0, sizeof( glState ) );

	if ( sizeof( glconfig_t ) != 11332 )
		ri.Error( ERR_FATAL, "Mod ABI incompatible: sizeof(glconfig_t) == %u != 11332", (unsigned int) sizeof( glconfig_t ) );

	if ( (intptr_t)tess.xyz & 15 ) {
		ri.Printf( PRINT_WARNING, "tess.xyz not 16 byte aligned\n" );
	}
	Com_Memset( tess.constantColor255, 255, sizeof( tess.constantColor255 ) );

	//
	// init function tables
	//
	for ( i = 0; i < FUNCTABLE_SIZE; i++ ) {
		tr.sinTable[i] = sin( DEG2RAD( i * 360.0f / FUNCTABLE_SIZE ) + 0.0001f );
		tr.squareTable[i] = (i < FUNCTABLE_SIZE / 2) ? 1.0f : -1.0f;
		if ( i == 0 ) {
			tr.sawToothTable[i] = EPSILON;
		} else {
			tr.sawToothTable[i] = (float)i / FUNCTABLE_SIZE;
		}
		tr.inverseSawToothTable[i] = 1.0f - tr.sawToothTable[i];
		if ( i < FUNCTABLE_SIZE / 2 ) {
			if ( i < FUNCTABLE_SIZE / 4 ) {
				if ( i == 0 ) {
					tr.triangleTable[i] = EPSILON;
				} else {
					tr.triangleTable[i] = (float)i / (FUNCTABLE_SIZE / 4);
				}
			} else {
				tr.triangleTable[i] = 1.0f - tr.triangleTable[i - FUNCTABLE_SIZE / 4];
			}
		} else {
			tr.triangleTable[i] = -tr.triangleTable[i - FUNCTABLE_SIZE / 2];
		}
	}

	R_InitFogTable();

	R_NoiseInit();

	R_Register();

	max_polys = r_maxpolys->integer;
	max_polyverts = r_maxpolyverts->integer;

	ptr = ri.Hunk_Alloc( sizeof( *backEndData ) + sizeof(srfPoly_t) * max_polys + sizeof(polyVert_t) * max_polyverts, h_low);
	backEndData = (backEndData_t *) ptr;
	backEndData->polys = (srfPoly_t *) ((char *) ptr + sizeof( *backEndData ));
	backEndData->polyVerts = (polyVert_t *) ((char *) ptr + sizeof( *backEndData ) + sizeof(srfPoly_t) * max_polys);

	R_InitNextFrame();

	InitOpenGL();

	R_InitImages();

	VarInfo();

	R_InitShaders();

	R_InitSkins();

	R_ModelInit();

	R_InitFreeType();

	err = qglGetError();
	if ( err != GL_NO_ERROR )
		ri.Printf( PRINT_WARNING, "glGetError() = 0x%x\n", err );

	ri.Printf( PRINT_ALL, "----- finished R_Init -----\n" );
}


/*
===============
RE_Shutdown
===============
*/
static void RE_Shutdown( refShutdownCode_t code ) {

	ri.Printf( PRINT_ALL, "RE_Shutdown( %i )\n", code );

	ri.Cmd_RemoveCommand( "modellist" );
	ri.Cmd_RemoveCommand( "screenshotBMP" );
	ri.Cmd_RemoveCommand( "screenshotJPEG" );
	ri.Cmd_RemoveCommand( "screenshot" );
	ri.Cmd_RemoveCommand( "imagelist" );
	ri.Cmd_RemoveCommand( "shaderlist" );
	ri.Cmd_RemoveCommand( "skinlist" );
	ri.Cmd_RemoveCommand( "gfxinfo" );
	ri.Cmd_RemoveCommand( "shaderstate" );

	//if ( tr.registered ) {
		//R_IssuePendingRenderCommands();
		R_DeleteTextures();
	//}

	R_DoneFreeType();

	// shut down platform specific OpenGL stuff
	if ( code != REF_KEEP_CONTEXT ) {

		QGL_DoneARB();

#ifdef USE_VBO
		VBO_Cleanup();
#endif

		R_ClearSymTables();

		Com_Memset( &glState, 0, sizeof( glState ) );

		if ( code != REF_KEEP_WINDOW ) {
			ri.GLimp_Shutdown( code == REF_UNLOAD_DLL ? qtrue : qfalse );
			Com_Memset( &glConfig, 0, sizeof( glConfig ) );
		}
	}

	ri.FreeAll();

	tr.registered = qfalse;
	tr.inited = qfalse;
}


/*
=============
RE_EndRegistration

Touch all images to make sure they are resident (probably obsolete on modern systems)
=============
*/
static void RE_EndRegistration( void ) {
	//FBO_BindMain(); // otherwise we may draw images to the back buffer
	//R_IssuePendingRenderCommands();
	//if ( !ri.Sys_LowPhysicalMemory() ) {
	//	RB_ShowImages();
	//}
}


/*
@@@@@@@@@@@@@@@@@@@@@
GetRefAPI
@@@@@@@@@@@@@@@@@@@@@
*/
#ifdef USE_RENDERER_DLOPEN
Q_EXPORT refexport_t* QDECL GetRefAPI ( int apiVersion, refimport_t *rimp ) {
#else
refexport_t *GetRefAPI ( int apiVersion, refimport_t *rimp ) {
#endif

	static refexport_t	re;

	ri = *rimp;

	Com_Memset( &re, 0, sizeof( re ) );

	if ( apiVersion != REF_API_VERSION ) {
		ri.Printf(PRINT_ALL, "Mismatched REF_API_VERSION: expected %i, got %i\n",
			REF_API_VERSION, apiVersion );
		return NULL;
	}

	// the RE_ functions are Renderer Entry points

	re.Shutdown = RE_Shutdown;

	re.BeginRegistration = RE_BeginRegistration;
	re.RegisterModel = RE_RegisterModel;
	re.RegisterSkin = RE_RegisterSkin;
	re.RegisterShader = RE_RegisterShader;
	re.RegisterShaderNoMip = RE_RegisterShaderNoMip;
	re.LoadWorld = RE_LoadWorldMap;
	re.SetWorldVisData = RE_SetWorldVisData;
	re.EndRegistration = RE_EndRegistration;

	re.BeginFrame = RE_BeginFrame;
	re.EndFrame = RE_EndFrame;

	re.MarkFragments = R_MarkFragments;
	re.LerpTag = R_LerpTag;
	re.ModelBounds = R_ModelBounds;

	re.ClearScene = RE_ClearScene;
	re.AddRefEntityToScene = RE_AddRefEntityToScene;
	re.AddPolyToScene = RE_AddPolyToScene;
	re.LightForPoint = R_LightForPoint;
	re.AddLightToScene = RE_AddLightToScene;
	re.AddAdditiveLightToScene = RE_AddAdditiveLightToScene;
	re.AddLinearLightToScene = RE_AddLinearLightToScene;

	re.RenderScene = RE_RenderScene;

	re.SetColor = RE_SetColor;
	re.DrawStretchPic = RE_StretchPic;
	re.DrawStretchRaw = RE_StretchRaw;
	re.UploadCinematic = RE_UploadCinematic;

	re.RegisterFont = RE_RegisterFont;
	re.RemapShader = RE_RemapShader;
	re.GetEntityToken = RE_GetEntityToken;
	re.inPVS = R_inPVS;

	re.TakeVideoFrame = RE_TakeVideoFrame;
	re.SetColorMappings = R_SetColorMappings;

	re.ThrottleBackend = RE_ThrottleBackend;
	re.FinishBloom = RE_FinishBloom;
	re.CanMinimize = RE_CanMinimize;
	re.GetConfig = RE_GetConfig;
	re.VertexLighting = RE_VertexLighting;
	re.SyncRender = RE_SyncRender;

	return &re;
}
