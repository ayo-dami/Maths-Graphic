//--------------------------------------------------------------------------------------------
// File:		glsl.vs
// Version:		V2.0
// Author:		Daniel Rhodes
// Date:		13/07/2005
// Description:	Texture	Mapping	Vertex Shader in GLSL
// Notes:		GLSL documentation can be found	at:
//				http://developer.3dlabs.com/openGL2/index.htm
// Input:		gl_Vertex						= vertex object	space position
//				gl_ModelViewProjectionMatrix	= current model	view projection	matrix
//				gl_MultiTexCoord0				= texture coordinates for unit 0
// Output:		gl_Position						= vertex object	space position
//				gl_TexCoord						= vertex texture coordinates
//--------------------------------------------------------------------------------------------

void main()
{
	// Set the output vertex position in clip space
	gl_Position = ftransform( );
   
	// Pass the tex coords to the pixel shader
	gl_TexCoord[0] = gl_MultiTexCoord0;
}