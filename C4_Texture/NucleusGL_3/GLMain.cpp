//--------------------------------------------------------------------------------------------
// File:		GLMain.cpp
// Version:		V2.1
// Author:		Daniel Rhodes
// Date:		16/12/2005
// Description:	OpenGL Framework adapted to run like Dr. Cant's grafprog
// Notes:		Has facilities to output floating point data to file from a pixel shader
//				21/12/2005 - Added preprocessor stuff to switch reading pixels,
//							 full screen quad, and the window border on / off.
//				06/01/2006 - Added facility to toggle readpixels on and off during execution,
//							 WARNING: using this can cause a massive slowdown dependant on
//									  window size
//--------------------------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN
#define VC_LEANMEAN

#define USEMYCODE		TRUE	// Toggle MyCode use or shader use,
								// set FULLSCREENQUAD to 0 when FALSE
// RECOMMENDED USAGE:  when READPIX on: WINDOWBORDER off, FULLSCREENQUAD on
//					   prevents background pixels appearing in output.
//					   when READPIX off: WINDOWBORDER on, FULLSCREENQUAD off
#define READPIX			0	// Enables debug output from pixel shander to file
							// WARNING: will stop display on screen. 1 on, 0 off.
#define WINDOWBORDER	1	// When false (0) turns off window border so render surface can be
							// the exact size of WINDOWWIDTH x WINDOWHEIGHT. 1 for normal window

// Draw a full screen quad instead of the cube; aids debugging
							// with bReadPixels. 1 = on, 0 = off.
#define WINPOSX			100		// Window x coordinate
#define WINPOSY			100		// Window y coordinate


// System header files
#include <fstream>
#include <string>
#include <stdio.h>

// Windows header files
#include <windows.h>

// OpenGL header files
#include <gl/gl.h>
#include <gl/glu.h>
//#include <gl/glaux.h>
#include "glext.h"
#include "wglext.h"

// Custom header files
#include "MyCode.h"
extern	FILE *debugfile;
// Namespaces
// DON'T use 'using std;' it confuses other namespaces we may need
using std::ifstream;
using std::ofstream;
using std::endl;
using std::string;

// GL functions
PFNWGLGETEXTENSIONSSTRINGARBPROC	wglGetExtensionsStringARB		= NULL;
PFNWGLCHOOSEPIXELFORMATARBPROC		wglChoosePixelFormatARB			= NULL;
PFNGLVALIDATEPROGRAMARBPROC			glValidateProgramARB			= NULL;
PFNGLDETACHOBJECTARBPROC			glDetachObjectARB				= NULL;
PFNGLCREATEPROGRAMOBJECTARBPROC		glCreateProgramObjectARB		= NULL;
PFNGLCREATESHADEROBJECTARBPROC		glCreateShaderObjectARB			= NULL;
PFNGLLINKPROGRAMARBPROC				glLinkProgramARB				= NULL;
PFNGLCOMPILESHADERARBPROC			glCompileShaderARB				= NULL;
PFNGLGETINFOLOGARBPROC				glGetInfoLogARB					= NULL;
PFNGLDELETEOBJECTARBPROC			glDeleteObjectARB				= NULL;
PFNGLUSEPROGRAMOBJECTARBPROC		glUseProgramObjectARB			= NULL;
PFNGLSHADERSOURCEARBPROC			glShaderSourceARB				= NULL;
PFNGLATTACHOBJECTARBPROC			glAttachObjectARB				= NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC	glGetObjectParameterivARB		= NULL;
PFNGLGETUNIFORMLOCATIONARBPROC		glGetUniformLocationARB			= NULL;
PFNGLUNIFORM4FARBPROC				glUniform4fARB					= NULL;
PFNGLUNIFORM1FARBPROC				glUniform1fARB					= NULL;
PFNGLUNIFORM1IARBPROC				glUniform1iARB					= NULL;
PFNGLUNIFORM2FARBPROC				glUniform2fARB					= NULL;
PFNGLUNIFORM2IARBPROC				glUniform2iARB					= NULL;
PFNGLACTIVETEXTUREPROC				glActiveTextureARB				= NULL;
PFNGLMULTITEXCOORD2FPROC			glMultiTexCoord2fARB			= NULL;
PFNGLCLAMPCOLORARBPROC				glClampColorARB					= NULL;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC  glCheckFramebufferStatusEXT		= NULL;
PFNGLGENFRAMEBUFFERSEXTPROC			glGenFramebuffersEXT			= NULL;
PFNGLGENRENDERBUFFERSEXTPROC		glGenRenderbuffersEXT			= NULL;
PFNGLBINDFRAMEBUFFEREXTPROC			glBindFramebufferEXT			= NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC	glFramebufferTexture2DEXT		= NULL;
PFNGLBINDRENDERBUFFEREXTPROC		glBindRenderbufferEXT			= NULL;
PFNGLRENDERBUFFERSTORAGEEXTPROC		glRenderbufferStorageEXT		= NULL;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC	glFramebufferRenderbufferEXT	= NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC		glDeleteFramebuffersEXT			= NULL;

// Global device context.
HDC g_HDC;

// Global handle to your code object
Interface *run;

// xRotation and yRotation is used to rotate the entire scene. - shaders only
float xRotation = 0.0f;
float yRotation = 0.0f;
float zRotation = 0.0f;
float fMoveX = 0.0;
float fMoveY = 0.0;
#if FULLSCREENQUAD // only move back 5 for non fullscreen quads
	float fMoveZ = 0.0;
#else
	float fMoveZ = -5.0;	// Doesn't have to be five but can't be zero for non fullscreen
#endif

bool bReadPixOn = false;

// Stores global width and height for our calculations.
int gWidth	= 0;
int gHeight = 0;

// Shader variables we will bind to.
unsigned int glslTexture;

// GLSlang objects.
GLhandleARB glContext;
GLhandleARB glVertexShader;
GLhandleARB glPixelShader;

// Textures we will use
float *fTexture;			// Input texture
float *fTextureOut;			// Output texture
GLuint iTextures[MAXTEX];	// Array of texture ID's (makes it simpler to swap textures)

// Framebuffer ID
GLuint iFrameBuffer;

// Function Prototypes
bool bReadTexture( );
bool bSetupExtensions( );
bool bCheckFramebufferStatus( );
char *sLoadShader( char *sShaderName );
void SetupPixelFormat( HDC hDC );
bool bReadPixels( );
bool bInitialiseGLTextures( );
bool bInitialiseFBO( );
bool bInitialiseGLSL( );
bool bInitialiseGL( );
void RenderScene( );
void Shutdown( );

// My Functions


//-----------------------------------------------------------------------------
// Name:	GLGETERROR
// Parms:	sLoc = Location of error
// Returns:	none
// Desc:	Displays a message box when an error occurs giving the error type
//			and location
//-----------------------------------------------------------------------------
#define GLGETERROR( sLoc )														\
{																				\
	GLenum error = glGetError();												\
	string sError = "";															\
	string sLocation = "OpenGL error: ";										\
	sLocation.append( sLoc );													\
	char buffer[100] = "";														\
	if( error != GL_NO_ERROR )													\
	{																			\
		sprintf( buffer, "[%s line %d] GL Error: %s\n",							\
				__FILE__, __LINE__, gluErrorString( error ) );					\
		sError.append( buffer );												\
		MessageBox( NULL, sError.c_str( ), sLocation.c_str( ), MB_OK );			\
	}																			\
}

//-----------------------------------------------------------------------------
// Name:	bReadTexture
// Parms:	None
// Returns:	bool - true for success, false for failure
// Desc:	Reads texture info from a text file
//-----------------------------------------------------------------------------
bool bReadTexture( )
{
	ifstream fIn;
	ofstream fOut;	
	string buffer;

	fIn.open( "texture.txt" );
	fOut.open( "GLdebug.txt" );
	fOut.precision( sizeof(float) );

	if ( !fIn.is_open( ) || !fOut.is_open( ) )
		return false;

	int index = 0;

	// Fill texture
	for (UINT i = 0; i < TEXHEIGHT; i++)
	{
		for (UINT j = 0; j < TEXWIDTH; j++)
		{
			index = 4 * (  i * TEXWIDTH + j );

			getline( fIn, buffer, '\n' );
			sscanf( buffer.c_str( ), "%f,%f,%f,%f",
					&fTexture[index], &fTexture[index + 1], &fTexture[index + 2], &fTexture[index + 3] );

			// Output texture data to debug.txt to check read in ok
			fOut << fTexture[index] << ", "
				 << fTexture[index + 1] << ", "
				 << fTexture[index + 2] << ", "
				 << fTexture[index + 3] << endl;
		}
	}			

	fIn.close( );
	fOut.close( );

	return true;
}

//-----------------------------------------------------------------------------
// Name:	bSetupExtensions
// Parms:	None
// Returns:	bool - true for success, false for failure
// Desc:	Load all require OpenGL extensions
//-----------------------------------------------------------------------------
bool bSetupExtensions( )
{
	// Load Shader extensions
	glCreateShaderObjectARB		= (PFNGLCREATESHADEROBJECTARBPROC)			wglGetProcAddress( "glCreateShaderObjectARB" );
	glCreateProgramObjectARB	= (PFNGLCREATEPROGRAMOBJECTARBPROC)			wglGetProcAddress( "glCreateProgramObjectARB" );
	glAttachObjectARB			= (PFNGLATTACHOBJECTARBPROC)				wglGetProcAddress( "glAttachObjectARB" );
	glDetachObjectARB			= (PFNGLDETACHOBJECTARBPROC)				wglGetProcAddress( "glDetachObjectARB" );
	glDeleteObjectARB			= (PFNGLDELETEOBJECTARBPROC)				wglGetProcAddress( "glDeleteObjectARB" );
	glShaderSourceARB			= (PFNGLSHADERSOURCEARBPROC)				wglGetProcAddress( "glShaderSourceARB" );
	glCompileShaderARB			= (PFNGLCOMPILESHADERARBPROC)				wglGetProcAddress( "glCompileShaderARB" );
	glLinkProgramARB			= (PFNGLLINKPROGRAMARBPROC)					wglGetProcAddress( "glLinkProgramARB" );
	glValidateProgramARB		= (PFNGLVALIDATEPROGRAMARBPROC)				wglGetProcAddress( "glValidateProgramARB" );
	glUseProgramObjectARB		= (PFNGLUSEPROGRAMOBJECTARBPROC)			wglGetProcAddress( "glUseProgramObjectARB" );
	glGetObjectParameterivARB	= (PFNGLGETOBJECTPARAMETERIVARBPROC)		wglGetProcAddress( "glGetObjectParameterivARB" );
	glGetInfoLogARB				= (PFNGLGETINFOLOGARBPROC)					wglGetProcAddress( "glGetInfoLogARB" );
	glUniform1iARB				= (PFNGLUNIFORM1IARBPROC)					wglGetProcAddress( "glUniform1iARB" );
	glUniform1fARB				= (PFNGLUNIFORM1FARBPROC)					wglGetProcAddress( "glUniform1fARB" );
	glUniform2iARB				= (PFNGLUNIFORM2IARBPROC)					wglGetProcAddress( "glUniform2iARB" );
    glUniform2fARB				= (PFNGLUNIFORM2FARBPROC)					wglGetProcAddress( "glUniform2fARB" );
	glUniform4fARB				= (PFNGLUNIFORM4FARBPROC)					wglGetProcAddress( "glUniform4fARB" );
	glGetUniformLocationARB		= (PFNGLGETUNIFORMLOCATIONARBPROC)			wglGetProcAddress( "glGetUniformLocationARB" );
	
	// Error checking to make sure shader functions loaded
	if( !glCreateProgramObjectARB || !glCreateShaderObjectARB ||  !glCompileShaderARB ||
		!glLinkProgramARB || !glGetInfoLogARB || !glDeleteObjectARB || !glUseProgramObjectARB ||
		!glShaderSourceARB || !glAttachObjectARB || !glGetObjectParameterivARB ||
		!glGetUniformLocationARB || !glUniform4fARB || !glUniform1iARB )
	{
		MessageBox( NULL, "Error loading GLSlang shader functions.", "Error! ( bSetupExtensions )", MB_OK );
		return false;
	}

	// Load the multitexture extensions and test if all went well.
	glActiveTextureARB		= (PFNGLACTIVETEXTUREARBPROC)					wglGetProcAddress( "glActiveTextureARB" );
	glMultiTexCoord2fARB	= (PFNGLMULTITEXCOORD2FARBPROC)					wglGetProcAddress( "glMultiTexCoord2fARB" );

	// Error checking to make sure multitexture functions loaded
	if( !glActiveTextureARB || !glMultiTexCoord2fARB )
	{
		MessageBox( NULL, "Error loading Multitexture functions.", "Error! ( bSetupExtensions )", MB_OK );
		return false;
	}

	// Load FBO (FrameBuffer Object) extensions
	glCheckFramebufferStatusEXT		= (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)	wglGetProcAddress( "glCheckFramebufferStatusEXT" );
	glGenFramebuffersEXT			= (PFNGLGENFRAMEBUFFERSEXTPROC)			wglGetProcAddress( "glGenFramebuffersEXT" );
	glGenRenderbuffersEXT			= (PFNGLGENRENDERBUFFERSEXTPROC)		wglGetProcAddress( "glGenRenderbuffersEXT" );
	glBindFramebufferEXT			= (PFNGLBINDFRAMEBUFFEREXTPROC)			wglGetProcAddress( "glBindFramebufferEXT" );
	glFramebufferTexture2DEXT		= (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)	wglGetProcAddress( "glFramebufferTexture2DEXT" );
	glBindRenderbufferEXT			= (PFNGLBINDRENDERBUFFEREXTPROC)		wglGetProcAddress( "glBindRenderbufferEXT" );
	glRenderbufferStorageEXT		= (PFNGLRENDERBUFFERSTORAGEEXTPROC)		wglGetProcAddress( "glRenderbufferStorageEXT" );
	glFramebufferRenderbufferEXT	= (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)	wglGetProcAddress( "glFramebufferRenderbufferEXT" );
	glClampColorARB					= (PFNGLCLAMPCOLORARBPROC)				wglGetProcAddress( "glClampColorARB" );
	glDeleteFramebuffersEXT			= (PFNGLDELETEFRAMEBUFFERSEXTPROC)		wglGetProcAddress( "glDeleteFramebuffersEXT" );
 
	// Error checking to make sure FBO (FrameBuffer Object) functions loaded
	if ( !glCheckFramebufferStatusEXT || !glGenFramebuffersEXT || !glGenRenderbuffersEXT ||
		 !glBindFramebufferEXT || !glFramebufferTexture2DEXT || !glBindRenderbufferEXT ||
		 !glRenderbufferStorageEXT || !glFramebufferRenderbufferEXT || !glClampColorARB ||
		 !glDeleteFramebuffersEXT )
	{
		MessageBox( NULL, "Error loading FBO functions.", "Error! ( bSetupExtensions )", MB_OK );
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name:	bCheckFramebufferStatus
// Parms:	None
// Returns:	bool - true for success, false for failure
// Desc:	Check framebuffers set up properly
//-----------------------------------------------------------------------------
bool bCheckFramebufferStatus( )
{
	GLenum status;
	status = glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT );

	switch(status) {
		case GL_FRAMEBUFFER_COMPLETE_EXT:
			return true;
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			// Need to choose different formats...
			MessageBox( NULL, "GL_FRAMEBUFFER_UNSUPPORTED_EXT.", "Error! ( bCheckFramebufferStatus )", MB_OK );
			return false;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
			MessageBox( NULL, "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT.", "Error! ( bCheckFramebufferStatus )", MB_OK );
			return false;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			MessageBox( NULL, "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT.", "Error! ( bCheckFramebufferStatus )", MB_OK );
			return false;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
			MessageBox( NULL, "GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT.", "Error! ( bCheckFramebufferStatus )", MB_OK );
			return false;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			MessageBox( NULL, "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT.", "Error! ( bCheckFramebufferStatus )", MB_OK );
			return false;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			MessageBox( NULL, "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT.", "Error! ( bCheckFramebufferStatus )", MB_OK );
			return false;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			MessageBox( NULL, "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT.", "Error! ( bCheckFramebufferStatus )", MB_OK );
			return false;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			MessageBox( NULL, "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT.", "Error! ( bCheckFramebufferStatus )", MB_OK );
			return false;
			break;
		default:
			// programming error; will fail on all hardware
			MessageBox( NULL, "programming error; will fail on all hardware", "Error! ( bCheckFramebufferStatus )", MB_OK );
			return false;
	}
}

//-----------------------------------------------------------------------------
// Name:	loadShader
// Parms:	shaderName = Shader name
// Returns:	Shader code
// Desc:	Render to Screen
//-----------------------------------------------------------------------------
char* sLoadShader( char *sShaderName )
{
	char tempLine[256] = {0};
	int count = 0;

	// input object.
	ifstream tempInput;

	// Open the shader file
	tempInput.open( sShaderName );

	// If there are errors then return false
	if( !tempInput.is_open( ) )
		return NULL;

	// Loop through each line of the file and get the total size
	while( !tempInput.eof( ) )
	{
		tempInput.getline( tempLine, 256, '\n' );
		count++;
	}

	// Close the shader
	tempInput.close( );

	if( count == 0 )
		return NULL;

	// Read in the data for use this time
	ifstream input;

	// Create array to hold shader code
	char *ShaderSource = new char[256 * count];

	// Re-open the shader and read in the whole thing into the array
	input.open( sShaderName );
	input.getline( ShaderSource , 256 * count, '\0' );

	input.close( ); // Close the shader

	return ShaderSource;
}

//-----------------------------------------------------------------------------
// Name:	SetupPixelFormat
// Parms:	hDC = handle to device context
// Returns:	None
// Desc:	Sets the pixel format
//-----------------------------------------------------------------------------
void SetupPixelFormat( HDC hDC )
{
	int nPixelFormat;

	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof( PIXELFORMATDESCRIPTOR ),	// Size of structure
		1,									// Version number
		PFD_DRAW_TO_WINDOW |				// Support window
		PFD_SUPPORT_OPENGL |				// Support OpenGl
		PFD_DOUBLEBUFFER,					// Support double buffering
		PFD_TYPE_RGBA,						// Support RGBA
		128,								// Bit color mode, Want 128 bit color
		32, 0, 32, 0, 32, 0,				// Color bits
		32,									// Alpha buffer
		0,									// Ignore shift bit
		0,									// No accumulation buffer
		0, 0, 0, 0,							// Ignore accumulation bits
		16,									// Number of depth buffer bits
		0,									// Number of stencil buffer bits
		0,									// 0 means no auxiliary buffer
		PFD_MAIN_PLANE,						// The main drawing plane
		0,									// This is reserved
		0, 0, 0								// Layer masks ignored
	};

	// Chooses the best pixel format and returns index
	nPixelFormat = ChoosePixelFormat( hDC, &pfd );

	// Set pixel format to device context
	SetPixelFormat( hDC, nPixelFormat, &pfd );
}

//-----------------------------------------------------------------------------
// Name:	bReadPixels
// Parms:	None
// Returns:	bool - true for success, false for failure
// Desc:	Read pixel values in floating point format from shader output and write to file (pixels.txt)
//-----------------------------------------------------------------------------
bool bReadPixels( )
{
	UINT index = 0;
	UINT indexZ = 0;

	// Make sure values read aren't clamped or scaled
	glClampColorARB( GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE );
	glClampColorARB( GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE );
	glClampColorARB( GL_CLAMP_READ_COLOR_ARB, GL_FALSE );

	// Set FBO to read from
	glReadBuffer( GL_COLOR_ATTACHMENT0_EXT );
	// Read pixel values
	glReadPixels( 0, 0, WINDOWWIDTH, WINDOWHEIGHT, GL_RGBA, GL_FLOAT, fTextureOut );
	
	GLGETERROR( "bReadPixels" );

	// Output pixel values
	ofstream fOut;
	fOut.open( "pixels.txt" );

	bool bLine = false;
	float fR, fG, fB, fA = 0;

	fOut << "R\t" << "G\t" << "B\t" << "A\t" << endl;

	for (UINT i = 0; i < WINDOWHEIGHT; i++)
	{
		for (UINT j = 0; j < WINDOWWIDTH; j++)
		{
			index = 4 * (  i * WINDOWWIDTH + j );

			fR = fTextureOut[index];
			fG = fTextureOut[index + 1];
			fB = fTextureOut[index + 2];
			fA = fTextureOut[index + 3];

			fOut << fR << "\t"
				 << fG << "\t"
				 << fB << "\t"
				 << fA << endl;
		}
	}
	fOut.close();
	return true;
}

//-----------------------------------------------------------------------------
// Name:	bInitialiseGLTextures
// Parms:	None
// Returns:	bool - true for success, false for failure
// Desc:	Initialises OpenGL Textures
//-----------------------------------------------------------------------------
bool bInitialiseGLTextures( )
{
	UINT index = 0;

	// Same as: fTexture = (float*)malloc( sizeof(float) * ( ( ( TEXWIDTH ) * ( TEXHEIGHT ) ) * 4 ) );
	fTexture = new float[ ( ( ( TEXWIDTH ) * ( TEXHEIGHT ) ) * 4 ) ];

	// Clear all values in texture
	ZeroMemory( fTexture, ( ( ( TEXWIDTH ) * ( TEXHEIGHT ) ) * 4 ) * sizeof(float) );
	// Same as :
	/*
	int index=0;
	for (UINT i = 0; i < TEXHEIGHT; i++)
	{
		for (UINT j = 0; j < TEXWIDTH; j++)
		{
			index = 4 * (  i * TEXWIDTH + j );

			fTexture[index]	   = 0.0f;	// Red
			fTexture[index + 1] = 0.0f;	// Green
			fTexture[index + 2] = 0.0f;	// Blue
			fTexture[index + 3] = 0.0f;	// Alpha
		}
	}
	*/

	// Read texture info from file
//	if ( !bReadTexture( ) )
//		MessageBox( NULL, "Failed to read texture", "Error!", MB_OK );

	fTextureOut = new float[ ( ( ( WINDOWWIDTH ) * ( WINDOWHEIGHT ) ) * 4 ) ];

	// Clear all values in texture
	ZeroMemory( fTextureOut, ( ( ( WINDOWWIDTH ) * ( WINDOWHEIGHT ) ) * 4 ) * sizeof(float) );
	// Same as :
	/*
	int index=0;
	for (UINT i = 0; i < WINDOWHEIGHT; i++)
	{
		for (UINT j = 0; j < WINDOWWIDTH; j++)
		{
			index = 4 * (  i * WINDOWWIDTH + j );

			fTextureOut[index]	   = 0.0f;	// Red
			fTextureOut[index + 1] = 0.0f;	// Green
			fTextureOut[index + 2] = 0.0f;	// Blue
			fTextureOut[index + 3] = 0.0f;	// Alpha
		}
	}
	*/
	for (UINT i = 0; i < WINDOWHEIGHT; i++)
	{
		for (UINT j = 0; j < WINDOWWIDTH; j++)
		{
			index = 4 * (  i * WINDOWWIDTH + j );

			fTextureOut[index]	   = 0.0f;	// Red
			fTextureOut[index + 1] = 0.0f;	// Green
			fTextureOut[index + 2] = 0.0f;	// Blue
			fTextureOut[index + 3] = 0.0f;	// Alpha
		}
	}

	UINT k = 0;

	glGenTextures( 2, iTextures );					// Create The Textures

	// Input texture
	glBindTexture( GL_TEXTURE_2D, iTextures[iTexID0] );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
   
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, TEXWIDTH, TEXHEIGHT, 0, GL_RGBA, GL_FLOAT, fTexture );

	// Texture to render on
	glBindTexture( GL_TEXTURE_2D, iTextures[iTexID1] );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, WINDOWWIDTH, WINDOWHEIGHT, 0, GL_RGBA, GL_FLOAT, 0 );

	// With this we will load the list of extensions the hardware supports.
	char *extension = (char*)glGetString( GL_EXTENSIONS );

	// Check for multitexture support.
	if ( strstr( extension, "GL_ARB_multitexture" ) == 0 )
		return false;

	GLGETERROR( "bInitialiseGLTextures" );

	return true;
}

//-----------------------------------------------------------------------------
// Name:	InitialiseFBO
// Parms:	None
// Returns:	bool - true for success, false for failure
// Desc:	Initialises FBO's
//-----------------------------------------------------------------------------
bool bInitialiseFBO( )
{
	glGenFramebuffersEXT( 1, &iFrameBuffer );
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, iFrameBuffer );
	// Attach texture to framebuffer
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, iTextures[iTexID1], 0 );

	if ( !bCheckFramebufferStatus( ) )
	{
		MessageBox( NULL, "FBO invalid.", "Error! ( InitialiseFBO )", MB_OK );
		return false;
	}

	glClampColorARB( GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE );
	glClampColorARB( GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE );
	glClampColorARB( GL_CLAMP_READ_COLOR_ARB, GL_FALSE );

	GLGETERROR( "bInitialiseFBO" );

	return true;
}

//-----------------------------------------------------------------------------
// Name:	InitialiseGLSL
// Parms:	None
// Returns:	bool - true for success, false for failure
// Desc:	Initialises GLSL
//-----------------------------------------------------------------------------
bool bInitialiseGLSL( )
{
	char *extensions = (char*)glGetString( GL_EXTENSIONS );

	// Check for the neccessary extensions
	if( strstr( extensions, "GL_ARB_shading_language_100" ) == NULL )
		return false;

	if( strstr( extensions, "GL_ARB_shader_objects" ) == NULL )
		return false;
		
	// Create the program object
	glContext = glCreateProgramObjectARB( );

	char error[4096];
	int result;

	// Load the vertex shader
	char *ShaderCode = sLoadShader( "glsl.vs" );
	glVertexShader = glCreateShaderObjectARB( GL_VERTEX_SHADER_ARB );
	glShaderSourceARB( glVertexShader, 1, (const char**)&ShaderCode, NULL );
	glCompileShaderARB( glVertexShader );
	glGetObjectParameterivARB( glVertexShader, GL_OBJECT_COMPILE_STATUS_ARB, &result );
	glAttachObjectARB( glContext, glVertexShader );

	delete[] ShaderCode;
	ShaderCode = NULL;

	// Load the pixel shader
	ShaderCode = sLoadShader( "glsl.ps" );
	glPixelShader = glCreateShaderObjectARB( GL_FRAGMENT_SHADER_ARB );
	glShaderSourceARB( glPixelShader, 1, (const char**)&ShaderCode, NULL );
	glCompileShaderARB( glPixelShader );
	glGetObjectParameterivARB( glPixelShader, GL_OBJECT_COMPILE_STATUS_ARB, &result );
	glAttachObjectARB( glContext, glPixelShader );

	delete[] ShaderCode;
	ShaderCode = NULL;

	// Link shaders
	glLinkProgramARB( glContext );
	glGetObjectParameterivARB( glContext, GL_OBJECT_LINK_STATUS_ARB, &result );

	if(!result)
	{
		glGetInfoLogARB( glContext, sizeof(error), NULL, error );

		MessageBox( NULL, error, "Error linking shaders...", MB_OK );
		return false;
	}

	// Bind shader variables
	glslTexture = glGetUniformLocationARB( glContext, "fTexture" );

	GLGETERROR( "bInitialiseGLSL" );

	return true;
}

//-----------------------------------------------------------------------------
// Name:	InitialiseGL
// Parms:	None
// Returns:	bool - true for success, false for failure
// Desc:	Initialises OpenGL and GLSL
//-----------------------------------------------------------------------------
bool bInitialiseGL( )
{
	glClearColor( 0.6f, 0.6f, 0.8f, 0.0f );					// Clear the screen to light blue
	glShadeModel( GL_SMOOTH );								// Smooth shading in our scenes
	glEnable( GL_DEPTH_TEST );								// Enable desth testing for hidden surface removal
	glEnable( GL_TEXTURE_2D );

	if ( !bSetupExtensions( ) )
	{
		MessageBox( NULL, "Error loading Extensions.", "Error! ( InitialiseGL )", MB_OK );
		return false;
	}

	if ( !bInitialiseGLTextures( ) )
	{
		MessageBox( NULL, "Error loading GL texture functions.", "Error! ( InitialiseGL )", MB_OK );
		return false;
	}

	if ( !bInitialiseGLSL( ) )
	{
		MessageBox( NULL, "Error loading GLSlang functions.", "Error! ( InitialiseGL )", MB_OK );
		return false;
	}

	if ( !bInitialiseFBO( ) )
	{
		MessageBox( NULL, "Error loading GL FBO functions.", "Error! ( InitialiseGL )", MB_OK );
		return false;
	}

	GLGETERROR( "bInitialiseGL" );

	glClearDepth( 1.0f );										// Depth Buffer Setup
	glDepthFunc( GL_LEQUAL );								// The Type Of Depth Testing To Do
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );	// Really Nice Perspective Calculations

	return true;
}

//-----------------------------------------------------------------------------
// Name:	RenderScene
// Parms:	None
// Returns:	None
// Desc:	Render Scene: main part of the rendering loop
//-----------------------------------------------------------------------------
void RenderScene( )
{
	// Clear texture every frame ready for drawing
	ZeroMemory( fTexture, ( ( ( TEXWIDTH ) * ( TEXHEIGHT ) ) * 4 ) * sizeof(float) );
	// Same as :
	/*
	int index=0;
	for (UINT i = 0; i < WINDOWHEIGHT; i++)
	{
		for (UINT j = 0; j < WINDOWWIDTH; j++)
		{
			index = 4 * (  i * WINDOWWIDTH + j );

			fTexture[index]	   = 0.0f;	// Red
			fTexture[index + 1] = 0.0f;	// Green
			fTexture[index + 2] = 0.0f;	// Blue
			fTexture[index + 3] = 0.0f;	// Alpha
		}
	}
	*/
	if ( USEMYCODE )DrawImage( );

	// make sure texture's updated every frame
	glBindTexture( GL_TEXTURE_2D, iTextures[iTexID0] );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
   
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, TEXWIDTH, TEXHEIGHT, 0, GL_RGBA, GL_FLOAT, fTexture );

	if ( READPIX || bReadPixOn ) // Output to main framebuffer OR offscreen FBO
		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, iFrameBuffer );
	else
		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );   // Clears the screen

	glLoadIdentity();	
	
	if ( FULLSCREENQUAD )
		gluOrtho2D( 0.0f, 1.0f, 0.0f, 1.0f );

	// Enable all texture units
	glEnable( GL_TEXTURE_2D );
	glActiveTextureARB(	GL_TEXTURE0_ARB	);
	glBindTexture( GL_TEXTURE_2D, iTextures[iTexID0] );

	// Enable shaders
	glUseProgramObjectARB( glContext );

	// Send the texture unit our shader texture will use.
	glUniform1iARB( glslTexture, 0 );

	glTranslatef( fMoveX, fMoveY, fMoveZ );
	glRotatef( -yRotation, 1.0f, 0.0f, 0.0f );
	glRotatef( -xRotation, 0.0f, 1.0f, 0.0f );	

	glClampColorARB( GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE );
	glClampColorARB( GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE );
	glClampColorARB( GL_CLAMP_READ_COLOR_ARB, GL_FALSE );

	glBegin( GL_QUADS );
		if ( FULLSCREENQUAD )	
		{	// Helps with debugging with bReadPixels by removing the background etc
			// works for 128x128 window, may not necessarly work for other sizes
			// Front Face
			glTexCoord2f( 0.0f, 0.0f ); glVertex3f(  0.0f,  0.0f,  2.41f );
			glTexCoord2f( 1.0f, 0.0f ); glVertex3f(  1.0f,  0.0f,  2.41f );
			glTexCoord2f( 1.0f, 1.0f ); glVertex3f(  1.0f,  1.0f,  2.41f );
			glTexCoord2f( 0.0f, 1.0f ); glVertex3f(  0.0f,  1.0f,  2.41f );
		}
		else
		{	// Draw full cube
			// Front Face
			glTexCoord2f( 0.0f, 0.0f ); glVertex3f( -1.0f, -1.0f,  1.0f );
			glTexCoord2f( 1.0f, 0.0f ); glVertex3f(  1.0f, -1.0f,  1.0f );
			glTexCoord2f( 1.0f, 1.0f ); glVertex3f(  1.0f,  1.0f,  1.0f );
			glTexCoord2f( 0.0f, 1.0f ); glVertex3f( -1.0f,  1.0f,  1.0f );
			// Back Face
			glTexCoord2f( 1.0f, 0.0f ); glVertex3f( -1.0f, -1.0f, -1.0f );
			glTexCoord2f( 1.0f, 1.0f ); glVertex3f( -1.0f,  1.0f, -1.0f );
			glTexCoord2f( 0.0f, 1.0f ); glVertex3f(  1.0f,  1.0f, -1.0f );
			glTexCoord2f( 0.0f, 0.0f ); glVertex3f(  1.0f, -1.0f, -1.0f );
			// Top Face
			glTexCoord2f( 0.0f, 1.0f ); glVertex3f( -1.0f,  1.0f, -1.0f );
			glTexCoord2f( 0.0f, 0.0f ); glVertex3f( -1.0f,  1.0f,  1.0f );
			glTexCoord2f( 1.0f, 0.0f ); glVertex3f(  1.0f,  1.0f,  1.0f );
			glTexCoord2f( 1.0f, 1.0f ); glVertex3f(  1.0f,  1.0f, -1.0f );
			// Bottom Face
			glTexCoord2f( 1.0f, 1.0f ); glVertex3f( -1.0f, -1.0f, -1.0f );
			glTexCoord2f( 0.0f, 1.0f ); glVertex3f(  1.0f, -1.0f, -1.0f );
			glTexCoord2f( 0.0f, 0.0f ); glVertex3f(  1.0f, -1.0f,  1.0f );
			glTexCoord2f( 1.0f, 0.0f ); glVertex3f( -1.0f, -1.0f,  1.0f );
			// Right face
			glTexCoord2f( 1.0f, 0.0f ); glVertex3f( 1.0f, -1.0f, -1.0f );
			glTexCoord2f( 1.0f, 1.0f ); glVertex3f( 1.0f,  1.0f, -1.0f );
			glTexCoord2f( 0.0f, 1.0f ); glVertex3f( 1.0f,  1.0f,  1.0f );
			glTexCoord2f( 0.0f, 0.0f ); glVertex3f( 1.0f, -1.0f,  1.0f );
			// Left Face
			glTexCoord2f( 0.0f, 0.0f ); glVertex3f( -1.0f, -1.0f, -1.0f );
			glTexCoord2f( 1.0f, 0.0f ); glVertex3f( -1.0f, -1.0f,  1.0f );
			glTexCoord2f( 1.0f, 1.0f ); glVertex3f( -1.0f,  1.0f,  1.0f );
			glTexCoord2f( 0.0f, 1.0f ); glVertex3f( -1.0f,  1.0f, -1.0f );
		}
	glEnd();

	if ( READPIX || bReadPixOn )
		bReadPixels( );	// Read back rendered result to file

	// Disable all texture units
	glActiveTextureARB(	GL_TEXTURE0_ARB	);
	glBindTexture( GL_TEXTURE_2D, NULL );

	// Disable shaders
	glUseProgramObjectARB( NULL );
	// Render to the window, using the texture
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
	if ( bReadPixOn )
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );   // Clears the screen

	SwapBuffers( g_HDC );

    GLGETERROR( "RenderScene" );
}

//-----------------------------------------------------------------------------
// Name:	WndProc
// Parms:	hwnd		= handle to window context
//			message		= message to process
//			wParam		= message params
//			lParam		= message params
// Returns:	Unhandled messages, 0 if all messages handled
// Desc:	Message handling function
//-----------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	static HGLRC hRC;                   // Rendering context
	static HDC hDC;                     // Device context
	int	width, height;                  // The window width and height
	static POINT oldMousePos;           // Last mouse position
	static POINT currentMousePos;       // Current mouse position
	static bool isMouseActive;          // Is the left mouse button down

	switch( message )
	{
	case WM_CREATE:						// Window creation
		hDC		= GetDC( hwnd );		// Get the device context for our window
		g_HDC	= hDC;					// Assign the global device context to this one

		SetupPixelFormat( hDC );		// Call the pixel format function

		hRC = wglCreateContext( hDC );	// Create the rendering context
		wglMakeCurrent( hDC, hRC );		// Make the rendering context ( hDC = Device context, hRC = OpenGl rendering context )

		RECT  rcWindow;
		POINT pWindowSize;
		// Get window size
		GetWindowRect( hwnd, &rcWindow );

		pWindowSize.x = rcWindow.right  - rcWindow.left;
		pWindowSize.y = rcWindow.bottom - rcWindow.top;
		
		POINT pClientSize;
		// Get client area size
		GetClientRect( hwnd, &rcWindow );
	
		pClientSize.x = rcWindow.right  - rcWindow.left;
		pClientSize.y = rcWindow.bottom - rcWindow.top;

		gHeight = pWindowSize.y + ( pWindowSize.y - pClientSize.y );
		gWidth  = pWindowSize.x + ( pWindowSize.x - pClientSize.x );

		// Set window size
		SetWindowPos( hwnd, (HWND)0, WINPOSX, WINPOSY, gWidth, gHeight, SWP_NOMOVE );

		return 0;
		break;
	case WM_CLOSE:						// Close message
	case WM_DESTROY:
		wglMakeCurrent( hDC, NULL );
		wglDeleteContext( hRC );		// Delete the rendering context
		
		PostQuitMessage( 0 );				// Close the program
		return 0;
		break;
	case WM_SIZE:						// re-size message
		height = HIWORD( lParam );		// Get the height of the window
		width = LOWORD( lParam );		// Get the width of the window

		if( height == 0 )				// Don't want a height of 0. If it is 0 make it 1
			height = 1;

		gHeight = height;
		gWidth  = width;

		glViewport( 0, 0, gWidth, gHeight );	// resets the viewport to new dimensions
		glMatrixMode( GL_PROJECTION );		// Set the projection matrix
		glLoadIdentity( );					// Reset the modelview matrix

		// calculate the aspect ratio of the window.
		gluPerspective( 45.0f, (GLfloat)gWidth / (GLfloat)gHeight, 0.1f, 1000.0f );

		glMatrixMode( GL_MODELVIEW );   // Set the projection matrix
		glLoadIdentity( );               // Reset the modelview matrix

		return 0;
		break;
	case WM_KEYDOWN:
		if ( USEMYCODE )
			KeyboardControl( wParam );
		else
		{
			case VK_ESCAPE:
				PostQuitMessage( 0 );
				break;
			case VK_LEFT:
				fMoveX++;
				break;
			case VK_RIGHT:
				fMoveX--;
				break;
			case VK_UP:
				fMoveY++;
				break;
			case VK_DOWN:
				fMoveY--;
				break;
			case VK_ADD:
				fMoveZ++;
				break;
			case VK_SUBTRACT:
				fMoveZ--;
				break;
			case VK_R:
				// Toggle read pixels. WARNING: depending on window size can DRASTICALLY
				//								slow down the program execution
				// Reading finished when screen clears to background colour!
				bReadPixOn = !bReadPixOn;
		}
		break;
	case WM_LBUTTONDOWN:
		oldMousePos.x = currentMousePos.x = LOWORD ( lParam );
		oldMousePos.y = currentMousePos.y = HIWORD ( lParam );
		isMouseActive = true;
		break;
	case WM_LBUTTONUP:
		isMouseActive = false;
		break;
	case WM_MOUSEMOVE:
		currentMousePos.x = LOWORD ( lParam );
		currentMousePos.y = HIWORD ( lParam );

		if( isMouseActive )
		{
			if ( USEMYCODE )
				MouseControl( currentMousePos, oldMousePos );
			else
			{
				xRotation -= ( currentMousePos.x - oldMousePos.x );
				yRotation -= ( currentMousePos.y - oldMousePos.y );
			}
		}

		oldMousePos.x = currentMousePos.x;
		oldMousePos.y = currentMousePos.y;
		break;
	default:	// Always have a default in case
		break;
	}

	// Pass all of the unhandled messages to DefWindowProc
	return ( DefWindowProc( hwnd, message, wParam, lParam ) );
}

//-----------------------------------------------------------------------------
// Name:	WinMain
// Parms:	hInstance		= application instance
//			hPrevInstance	= previous application instance
//			lpCmdLine		= 
//			nShowCmd		= 
// Returns:	0 for failure or wParam
// Desc:	
//-----------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
	MSG msg;					// Message variable
	WNDCLASSEX windowClass;		// Window class
	HWND hwnd;					// Window handle
	bool isFinished = false;	// Used to check if the program is done or not

	// Window class
	windowClass.cbSize			= sizeof( WNDCLASSEX );					// Size of the WNDCLASSEX structure
	windowClass.style			= CS_HREDRAW | CS_VREDRAW;				// Style of the window
	windowClass.lpfnWndProc		= WndProc;								// Address to the windows procedure
	windowClass.cbClsExtra		= 0;									// Extra class information
	windowClass.cbWndExtra		= 0;									// Extra window information
	windowClass.hInstance		= hInstance;							// Handle of application Instance
	windowClass.hIcon			= LoadIcon( NULL, IDI_APPLICATION );	// Handle of application Icon
	windowClass.hCursor			= LoadCursor( NULL, IDC_ARROW );		// Mouse cursor
	windowClass.hbrBackground	= NULL;									// Background color
	windowClass.lpszMenuName	= NULL;									// Name of the main menu
	windowClass.lpszClassName	= "NucleusGL";							// Window class name
	windowClass.hIconSm			= LoadIcon( NULL, IDI_APPLICATION );	// Icon when minimized

	// Register class with Windows.
	if( !RegisterClassEx( &windowClass ) )
		return 0;

	DWORD dwWindowStyle;

	if ( WINDOWBORDER )
		dwWindowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	else
		dwWindowStyle = WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	// Create the window
	hwnd = CreateWindowEx( NULL,								// The extended window style
						   "NucleusGL",							// Window Class name
						   "NucleusGL",							// Window name
						   dwWindowStyle,						// Window style
						   WINPOSX, WINPOSY,					// Window x, y coordinate
						   WINDOWWIDTH, WINDOWHEIGHT,			// Window width and height
						   NULL,								// Handle to parent window
						   NULL,								// Menu
						   hInstance,							// Handle to app instance
						   NULL );								// Pointer to window creation data

	// If there was an error with creating the window, close the program
	if(!hwnd)
		return 0;

	ShowWindow( hwnd, SW_SHOW ); // Show the window
	UpdateWindow( hwnd );        // forces a paint message

	isFinished = false;        // false = running, true = not running

	// If the initialization of OpenGL fails we can't run the program
	if( !bInitialiseGL( ) )
		isFinished = true;	
	
	if ( USEMYCODE )
	{
		run = new Interface( hwnd, fTexture, gWidth, gHeight );
		run->Init( hInstance );
	}

	// Messsage loop
	while( !isFinished )
	{
		if( PeekMessage( &msg, NULL, NULL, NULL, PM_REMOVE ) )
		{
			// If a quit message is received then stop rendering and quit the app
			if( msg.message == WM_QUIT )
				isFinished = true;

			TranslateMessage( &msg );  // Translate any messages
			DispatchMessage( &msg );   // Dispatch any messages
		}
		else
			RenderScene( );          // Render a frame
	}

	Shutdown( );

	// Unregister the window class with the OS
	UnregisterClass( "NucleusGL", windowClass.hInstance);

	return (int)msg.wParam;
}

//-----------------------------------------------------------------------------
// Name:	Shutdown
// Parms:	None
// Returns:	None
// Desc:	Shutdown OpenGL and GLSL
//-----------------------------------------------------------------------------
void Shutdown()
{
	// Free texture image
	glDeleteTextures( 2, iTextures );
	glDeleteFramebuffersEXT( 1, &iFrameBuffer );
	delete fTexture;

	// Release the GLSL resources we created
	if ( glVertexShader )
		glDeleteObjectARB( glVertexShader );
	if ( glPixelShader )
		glDeleteObjectARB( glPixelShader );
	if ( glContext )
		glDeleteObjectARB( glContext );

	if ( USEMYCODE )
	{
		run->Shutdown();
		delete run;
	}
}