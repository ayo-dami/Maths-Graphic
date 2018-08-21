//--------------------------------------------------------------------------------------------
// File:		MyCode.cpp
// Version:		V2.0
// Author:		Daniel Rhodes
// Description:	Your code goes here
// Notes:		For use with OpenGl 2.0 / DirectX9.0c or higher
//--------------------------------------------------------------------------------------------

// System header files
#include <math.h>

// Windows header files
// --

// Custom header files
#include "MyCode.h"
#include "resource.h"
extern HINSTANCE g_hInstance;
// For dynamic keyboard control
	int m_iMoveX;
	int m_iMoveY;
	int m_iMoveZ;	
	float m_dThetaX;
	float m_dThetaY;
	float m_dThetaZ;
	double m_dViewAngle;
	//debugging control variables
	int iFaceSelection;			// FaceSelection > 0: enables one face (with index selecton - 1)
								// to be drawn on its own to aid debugging.
	bool bDebug;				// Switches on debug output to file when true.
	// number of polygons read in
	int m_iNumOfPolys;
	//Frame Count
	int m_iOurFrameCount;
	// Input filename
	char m_sFilename[30];
	// Viewpoint data, use for parts B/C
	VECTOR m_vDisp, m_vLight;	// Displacement of object coordinates relative
								// to viewing coordinates and light direction
	//Debug File Handle
	FILE *debugfile;



	// Drawing area dimensions: width and height
	int m_iWidth;
	int m_iHeight;

	//Drawing Surface Handle
	float *m_fDrawingSurface;

	//Shading type
	SHADING_TYPE m_Shading;

	//Texture enable
	bool m_bTextureEnable;

// Database data, for the polygons
POLYGON polylist[2000];  // Array to store a list of polygons.

COLOUR Default_Colour={0.5,0.5,0.5};//colours are floats these days

double zbuf[WINDOWWIDTH][WINDOWHEIGHT];
//-----------------------------------------------------------------------------
// Name: KeyboardControl
// Desc: Enable Keyboard Control
//-----------------------------------------------------------------------------
void KeyboardControl( WPARAM wParam )
{
	switch( wParam ) 
	{
	case VK_ESCAPE:	// Exit application
		PostQuitMessage( 0 );
		break;
	case VK_LEFT:
		m_iMoveX--;
		break;
	case VK_RIGHT:
		m_iMoveX++;
		break;
	case VK_UP:
		m_iMoveY--;
		break;
	case VK_DOWN:
		m_iMoveY++;
		break;
	case VK_ADD:
		m_iMoveZ++;
		break;
	case VK_SUBTRACT:
		m_iMoveZ--;
		break;
	case VK_X:
		m_dThetaX += 0.01;
		break;
	case VK_Y:
		m_dThetaY += 0.01;
		break;
	case VK_Z:
		m_dThetaZ += 0.01;
		break;
	case VK_D:
		bDebug = !bDebug; // Toggle Debug output
		break;
	case VK_Q: 	DialogBoxParam( g_hInstance, MAKEINTRESOURCE(IDD_INIT),run->m_hWindow, run->DialogMessageHandlerStatic, (LPARAM)(run));
		break;

	}
}

//-----------------------------------------------------------------------------
// Name: KeyboardControl
// Desc: Enable Keyboard Control
//-----------------------------------------------------------------------------
void MouseControl( POINT currentMousePos, POINT oldMousePos )
{
	m_dThetaX -= ( currentMousePos.x - oldMousePos.x )*0.01;
	m_dThetaY -= ( currentMousePos.y - oldMousePos.y )*0.01;
}


//-----------------------------------------------------------------------------
// Name: displayFinal
// Desc: Routine to display useful info after program exit
//-----------------------------------------------------------------------------
void displayFinal( )
{
	char sDispString[50];
	sprintf( sDispString, "Total Framecount %d", m_iOurFrameCount );
	run->Alert( "Finished", sDispString );
}

//-----------------------------------------------------------------------------
// Name: displayReadInfo
// Desc: Routine to display useful info after file read, shows light vector
//		 as an example, modify and use for debugging as required
//-----------------------------------------------------------------------------
void displayReadInfo( )
{
	char sDispString[50];
	sprintf( sDispString, "%d polygons read", m_iNumOfPolys );
	run->Alert( m_sFilename, sDispString );
	sprintf( sDispString , "Light Vector %f  %f  %f", m_vLight.x, m_vLight.y, m_vLight.z );
	run->Alert( "Start Values:", sDispString );
}


//-----------------------------------------------------------------------------
// Name: LoadPolys
// Desc: Read polygon info from file
//-----------------------------------------------------------------------------
int LoadPolys( FILE *infile )
{
	char cInString[1000];
	int iNumPolys = 0;
	float fLength;
	float fR, fG, fB;		// red, green, blue values
	int i, tex_n, max_tex_n = 0;
	FILE *texfile;
	TEXMAP *tex_locations[100];

	if (m_bTextureEnable)
	{
		fgets(cInString, 1000, infile);
		while (strncmp(cInString, "end of texture", 14) != 0)
		{
			for (i = 0; i < 1000; i++)if (cInString[i] == '\n') cInString[i] = 0;

			texfile = fopen(cInString, "rb");
			tex_locations[max_tex_n] = (TEXMAP *)malloc(sizeof(TEXMAP));

			fgets(cInString, 1000, infile);//get sizes

			sscanf(cInString, "%d%d", &(tex_locations[max_tex_n]->sx), &(tex_locations[max_tex_n]->sy));

			tex_locations[max_tex_n]->texMap = (unsigned char *)malloc(sizeof(char) * 3 * tex_locations[max_tex_n]->sx*tex_locations[max_tex_n]->sy);

			//read file - leaving in packed format for now
			fread(tex_locations[max_tex_n]->texMap, sizeof(char), 3 * tex_locations[max_tex_n]->sx*tex_locations[max_tex_n]->sy, texfile);

			max_tex_n++;
			fclose(texfile);
			fgets(cInString, 1000, infile);//next texture
		}
	}

	do
	{
		fgets( cInString, 1000, infile);						 // Read first/next line of file
		sscanf( cInString, "%d", &polylist[iNumPolys].nv);   // Get number of vertices
		fprintf( debugfile, "number of vertices: %d\n", polylist[iNumPolys].nv);   // print number of vertices to debug file
		if  (polylist[iNumPolys].nv == 0)
			break;	// Get out if terminating zero found

		// Only allocate the memory we need - ALWAYS remember to delete on shutdown

		for (int i = 0; i < polylist[iNumPolys].nv; i++)
		{
			// Read next line of file
			fgets(cInString, 1000, infile);
			//Get Coordinates
			sscanf(cInString, "%f%f%f%d", &( polylist[iNumPolys].vert[i].x ),
										&( polylist[iNumPolys].vert[i].y ),
										&( polylist[iNumPolys].vert[i].z ),
										&( polylist[iNumPolys].vertVecNo[i] ));
		}
	
		polylist[iNumPolys].normal = Cross( VectorDiff( polylist[iNumPolys].vert[0], polylist[iNumPolys].vert[1] ),
									 VectorDiff( polylist[iNumPolys].vert[0],polylist[iNumPolys].vert[2] ) );
		fLength = (float)sqrt( Dot( polylist[iNumPolys].normal, polylist[iNumPolys].normal ) ); // Calculate length of vector

	    polylist[iNumPolys].normal.x /= fLength;	// Normalise
	    polylist[iNumPolys].normal.y /= fLength;	// each
	    polylist[iNumPolys].normal.z /= fLength;	// component
	    fgets(cInString, 1000, infile);		// Read  next line of file
	    sscanf( cInString, "%f%f%f", &fR, &fG, &fB );	// Get Colour, texture
		if (fR>1.0 ||fG>1.0||fB>1.0)//cope with either Open Gll 0-1.0 colours or old style 0-255 colours
		{
			polylist[iNumPolys].colour.r = fR/255.0;
			polylist[iNumPolys].colour.g = fG/255.0;
			polylist[iNumPolys].colour.b = fB/255.0;	
		}
		else
		{
			polylist[iNumPolys].colour.r = fR;
			polylist[iNumPolys].colour.g = fG;
			polylist[iNumPolys].colour.b = fB;	
		}

		if (tex_n < max_tex_n)   polylist[iNumPolys].texMap = tex_locations[tex_n]; //copy texture pointer
		else  polylist[iNumPolys].texMap = tex_locations[0];//safety measure - go for first texture

		if (polylist[iNumPolys].texMap)
		{
			polylist[iNumPolys].tex_vert[0] = { 0.0, 0.0 };
			polylist[iNumPolys].tex_vert[1] = { (float)polylist[iNumPolys].texMap->sx, 0.0 };
			polylist[iNumPolys].tex_vert[2] = { (float)polylist[iNumPolys].texMap->sx, (float)polylist[iNumPolys].texMap->sy };
			polylist[iNumPolys].tex_vert[3] = { 0.0, (float)polylist[iNumPolys].texMap->sy };
		}

		iNumPolys++;
	} while( 1 );

	//calculating vertice normals
	int ii = 0;
	int vert_count = 0;

	for (int iCurrentPoly = 0; iCurrentPoly < iNumPolys; iCurrentPoly++)
	{
		for (int iCurrentVertex = 0; iCurrentVertex < polylist[iCurrentPoly].nv; iCurrentVertex++)
		{
			vert_count = 0;
			polylist[iCurrentPoly].vvect[iCurrentVertex] = { 0,0,0 };

			for (int iCurrentComparedPoly = 0; iCurrentComparedPoly < iNumPolys; iCurrentComparedPoly++)
			{
				for (int iCurrentComparedVertex = 0; iCurrentComparedVertex < polylist[iCurrentComparedPoly].nv; iCurrentComparedVertex++)
				{
					if (polylist[iCurrentPoly].vertVecNo[iCurrentVertex] == polylist[iCurrentComparedPoly].vertVecNo[iCurrentComparedVertex])
					{
						polylist[iCurrentPoly].vvect[iCurrentVertex] = VectorSum(polylist[iCurrentPoly].vvect[iCurrentVertex],
							polylist[iCurrentComparedPoly].normal);

						vert_count++;
					}
				}
			}
			
			//calculate the average vector
			polylist[iCurrentPoly].vvect[iCurrentVertex].x /= vert_count;
			polylist[iCurrentPoly].vvect[iCurrentVertex].y /= vert_count;
			polylist[iCurrentPoly].vvect[iCurrentVertex].z /= vert_count;

			polylist[iCurrentPoly].vvect[iCurrentVertex] = Normalise(polylist[iCurrentPoly].vvect[iCurrentVertex]);
		}
	}

	return iNumPolys;  //Return number of polygons read
}


//-----------------------------------------------------------------------------
// Name: ReadFile
// Desc: Read polygon info from file
//-----------------------------------------------------------------------------
void ReadFile()
{
	FILE *flInFile;
	flInFile = fopen( m_sFilename, "r" );
	m_iNumOfPolys = LoadPolys( flInFile );
	displayReadInfo();
	fclose(flInFile);	
}


//-----------------------------------------------------------------------------
// Name: Plotpix
// Desc: Draw a pixel - Calls nRGBAImage::SetColour(...),
//		 m_kImage MUST be initialised before use!
//		 Example usage: Plotpix( x, y, 255, 0, 0 );
//-----------------------------------------------------------------------------
inline void Plotpix( DWORD dwX, DWORD dwY, float fR, float fG, float fB )
{
	DWORD dwYtemp;

	// If using OpenGL we need to mirror the Y coordinates,
	// as OpenGL uses the opposite coordinate system to us and DirectX
#ifdef 	DIRECTX 
	dwYtemp = dwY;
#else  //OPENGL should be defined
	dwYtemp = ( m_iHeight - 1 ) - dwY;
#endif

	int index = 4 * ( dwYtemp * m_iWidth + dwX );
	m_fDrawingSurface[index]	 = fR;	// Red Channel
	m_fDrawingSurface[index + 1] = fG;	// Green Channel
	m_fDrawingSurface[index + 2] = fB;	// Blue Channel
	m_fDrawingSurface[index + 3] = 0.0; // Alpha Channel
}

//-----------------------------------------------------------------------------
// Name: DrawImage
// Desc: Draws the image
//-----------------------------------------------------------------------------
void DrawImage( )
{
	POLYGON polyTempP, polyTempQ, polyTempQT;	// Temporary polygons for copying transformed, projected / clipped
												// versions of scene polys before drawing them. 
	int iCurrentPoly;							// Current polygon in process
	
	TRANSFORM object_transformation = BuildTrans(m_dThetaX, m_dThetaY, m_dThetaZ, m_vDisp);

	MATRIX invert_rotation_matrix = InverseRotationOnly(object_transformation);

	VECTOR view_in_object_coordinates = MOnV(invert_rotation_matrix, m_vDisp);

	VECTOR light = MOnV(invert_rotation_matrix, m_vLight);

	for (int i = 0; i < WINDOWWIDTH; i++)
		for (int j = 0; j < WINDOWHEIGHT; j++)
			zbuf[i][j] = 0;

	if (m_iNumOfPolys <= 0)
	{
		//DrawTrapezium(100, 200, 100, 160, -0.5f, 1.5f, Default_Colour);
		//DrawTrapezium(100, 200, 200, 220, 0.5, 3, Default_Colour);
		//DrawTrapezium(100, 200, 300, 340, +2, -0.5, Default_Colour);
		//DrawTrapezium(300, 300, 400, 430, -5, 0.1, Default_Colour);
	}
	else
		m_iOurFrameCount++;	// Increment frame counter if we have a polygon to draw

	for ( iCurrentPoly = 0; iCurrentPoly < m_iNumOfPolys; iCurrentPoly++ )	// for each polygon
	{
		if ( iFaceSelection > m_iNumOfPolys )
			iFaceSelection = m_iNumOfPolys;    //Keep debug selector in range
		
		
		if ( iFaceSelection && ( iCurrentPoly + 1 ) != iFaceSelection)
			continue; // Reject unselected polygons if debug selection active.

		polyTempP = polylist[iCurrentPoly];             //copy static data into temp poly structure

		if (Dot(VectorSum(view_in_object_coordinates, polyTempP.vert[0]), polyTempP.normal) < 0)
		{
			continue;
		}
		
		// Copy each vertex in polygon, add displacement to allow shift
		for (int i = 0; i < polyTempP.nv; i++)
		{
			VECTOR tmp1;
			VECTOR tmp2;

			tmp1 = DoTransform(polylist[iCurrentPoly].vert[i], object_transformation);
			tmp2 = Project(tmp1, m_dViewAngle);

			polyTempP.vert[i].x = (tmp2.x + 1)*WINDOWWIDTH / 2 + m_iMoveX;
			polyTempP.vert[i].y = (tmp2.y + 1)*WINDOWHEIGHT / 2 + m_iMoveY;
			polyTempP.vert[i].z = tmp2.z;
		}

				
		if ( bDebug )
			fprintf( debugfile, " number of vertices: %d\n", polyTempP.nv);   // print number of vertices

		fflush( debugfile );
		
		if ( bDebug )	// Print out current poly specs if debug active
		{
			for (int i = 0; i < polyTempP.nv; i++)
			{
				fprintf( debugfile, "before clipping Polygon %d, Vertex %d values: %7.2f, %7.2f, %11.6f\n",
						 iCurrentPoly, i, polyTempP.vert[i].x, polyTempP.vert[i].y, polyTempP.vert[i].z ); 
			}
			fflush( debugfile );
			
		}
		// The section below calls clipping and polygon draw routines, commented out to allow the 
		// program to work without them. You may re-instate once you have appropriate routines,
		// or replace with your own code.

		ClipPolyXHigh( &polyTempP, &polyTempQT, WINDOWWIDTH );	// Clip against upper x boundary
		ClipPolyYHigh( &polyTempQT, &polyTempQ, WINDOWHEIGHT );	// Clip against upper y boundary (bottom of screen)
		ClipPolyXLow( &polyTempQ, &polyTempQT, 0);				// Clip against lower x boundary
		ClipPolyYLow( &polyTempQT, &polyTempQ, 0);				// Clip against lower y boundary (bottom of screen)
		
		//if ( bDebug )	// Print out current poly specs if debug active
		//{
		//	for ( int i = 0; i < polyTempQ.nv; i++ )
		//		fprintf( debugfile, "after clipping Polygon %d Vertex %d values:y %7.2f  %7.2f %11.6f\n",
		//				 iCurrentPoly, i, polyTempQ.vert[i].x, polyTempQ.vert[i].y, polyTempQ.vert[i].z ); 

		//	fflush(debugfile);
		//}
		 
		switch (m_Shading)
		{
		case NONE_SHADING:
			if (m_bTextureEnable)
			{
				DrawPolygonTex(&polyTempQ);
			}
			else
			{
				DrawPolygon(&polyTempQ);
			}
			break;

		case GOURAUD_SHADING:
			DrawPolygonGouraud(&polyTempQ, light);
			break;

		case PHONG_SHADING:
			DrawPolygonPhong(&polyTempQ, light);

		default:
			break;
		}
	}
	if ( m_iNumOfPolys > 0 )
		bDebug = false;	// Switch debug off after first run - switch on again via keyboard control if needed

	
}

//-----------------------------------------------------------------------------
// Name: DrawSquare
// Desc: Draw a sqaure
//-----------------------------------------------------------------------------
void DrawSquare(COLOUR c )
{
	//Note no protection to keep in screen bounds...
	for ( int i = m_iMoveX; i < 50 + m_iMoveX; i++)
	{
		for( int j = m_iMoveY; j < 50 + m_iMoveY; j++)
		{
			Plotpix( i, j, c.r, c.g, c.b );
		}
	}	
}


//void DrawTrapezium(COOR_DATA& coor_data, COLOUR c)
//{
//	double current_z;
//	float z_line_inc;
//
//	for (int j = coor_data.y_top; j < coor_data.y_bottom; j++)
//	{
//		z_line_inc = (coor_data.z_end - coor_data.z_start) / ((float)(coor_data.x_end - coor_data.x_start));
//		current_z = coor_data.z_start;
//
//		for (int i = coor_data.x_start; i < coor_data.x_end; i++)
//		{
//			if (current_z > zbuf[i][j])
//			{
//				Plotpix(i, j, c.r, c.g, c.b);
//				zbuf[i][j] = current_z;
//			}
//
//			current_z += z_line_inc;
//		}
//
//		if (j < coor_data.y_bottom - 1) // add slope value unless at final loop
//		{
//			coor_data.z_end += coor_data.z_slope_right;
//			coor_data.z_start += coor_data.z_slope_left;
//
//			coor_data.x_end += coor_data.x_slope_right;
//			coor_data.x_start += coor_data.x_slope_left;
//		}
//	}
//}


void DrawTrapezium(COOR_DATA& coor_data, COLOUR c)
{
	double current_z;
	float z_line_inc;

	for (float j = coor_data.y_top; j < coor_data.y_bottom; j++)
	{
		z_line_inc = (coor_data.z_end - coor_data.z_start) / ((float)(coor_data.x_end - coor_data.x_start));
		current_z = coor_data.z_start;

		for (float i = coor_data.x_start; i < coor_data.x_end; i++)
		{
			if (current_z > zbuf[(int)i][(int)j])
			{
				Plotpix(i, j, c.r, c.g, c.b);
				zbuf[(int)i][(int)j] = current_z;
			}

			current_z += z_line_inc;
		}

		if (j < coor_data.y_bottom - 1) // add slope value unless at final loop
		{
			coor_data.z_end += coor_data.z_slope_right;
			coor_data.z_start += coor_data.z_slope_left;

			coor_data.x_end += coor_data.x_slope_right;
			coor_data.x_start += coor_data.x_slope_left;
		}
	}
}

void DrawTrapezium(COOR_DATA& coor_data, TEX_DATA& tex_data)
{
	double current_z;
	float z_line_inc;

	TEX_VERT current_pix;
	TEX_VERT tex_line_inc;

	COLOUR t;

	for (float j = coor_data.y_top; j < coor_data.y_bottom; j++)
	{
		if (coor_data.x_end != coor_data.x_start)
		{
			z_line_inc = (coor_data.z_end - coor_data.z_start) / ((float)(coor_data.x_end - coor_data.x_start));
			current_z = coor_data.z_start;

			tex_line_inc = (tex_data.tex_end - tex_data.tex_start) / ((float)(coor_data.x_end - coor_data.x_start));
			current_pix = tex_data.tex_start;

			for (float i = coor_data.x_start; i < coor_data.x_end; i++)
			{
				if (current_z > zbuf[(int)i][(int)j])
				{
					t.g = (float)tex_data.tex_map->texMap[(int)current_pix.y * 256 * 3 + (int)current_pix.x * 3 + 0] / 255.0;
					t.b = (float)tex_data.tex_map->texMap[(int)current_pix.y * 256 * 3 + (int)current_pix.x * 3 + 1] / 255.0;
					t.r = (float)tex_data.tex_map->texMap[(int)current_pix.y * 256 * 3 + (int)current_pix.x * 3 + 2] / 255.0;

					Plotpix(i, j, t.r, t.g, t.b);
					zbuf[(int)i][(int)j] = current_z;
				}

				current_z += z_line_inc;
				current_pix += tex_line_inc;
			}
		}

		if (j < coor_data.y_bottom - 1) // add slope value unless at final loop
		{
			coor_data.z_end += coor_data.z_slope_right;
			coor_data.z_start += coor_data.z_slope_left;

			coor_data.x_end += coor_data.x_slope_right;
			coor_data.x_start += coor_data.x_slope_left;

			tex_data.tex_end += tex_data.tex_step_right;
			tex_data.tex_start += tex_data.tex_step_left;
		}
	}
}

void DrawTrapeziumGouraud(COOR_DATA& coor_data, COLOUR_DATA& colour)
{
	double current_z;
	float z_line_inc;

	COLOUR colour_line_inc;
	COLOUR current_colour;

	for (float j = coor_data.y_top; j < coor_data.y_bottom; j++)
	{
		z_line_inc = (coor_data.z_end - coor_data.z_start) / ((float)(coor_data.x_end - coor_data.x_start));
		current_z = coor_data.z_start;

		colour_line_inc = (colour.colour_end - colour.colour_start) / ((float)(coor_data.x_end - coor_data.x_start));

		current_colour = colour.colour_start;

		for (float i = coor_data.x_start; i < coor_data.x_end; i++)
		{
			if (current_z > zbuf[(int)i][(int)j])
			{
				Plotpix(i, j, current_colour.r, current_colour.g, current_colour.b);
				zbuf[(int)i][(int)j] = current_z;
			}

			current_z += z_line_inc;

			current_colour += colour_line_inc;
		}

		if (j < coor_data.y_bottom - 1) // add slope value unless at final loop
		{
			coor_data.z_end += coor_data.z_slope_right;
			coor_data.z_start += coor_data.z_slope_left;

			colour.colour_end += colour.colour_step_right;
			colour.colour_start += colour.colour_step_left;

			coor_data.x_end += coor_data.x_slope_right;
			coor_data.x_start += coor_data.x_slope_left;
		}
	}
}

void DrawTrapeziumPhong(COOR_DATA& coor_data, NORMAL_VERT_DATA& normal_data, VECTOR light, COLOUR c)
{
	double current_z;
	float z_line_inc;

	VECTOR normal_line_inc;
	VECTOR current_normal;

	COLOUR current_colour;
	VECTOR pos;

	for (float j = coor_data.y_top; j < coor_data.y_bottom; j++)
	{
		z_line_inc = (coor_data.z_end - coor_data.z_start) / ((float)(coor_data.x_end - coor_data.x_start));
		current_z = coor_data.z_start;

		normal_line_inc = VectorDevide(VectorDiff(normal_data.normal_end, normal_data.normal_start), coor_data.x_end - coor_data.x_start);

		current_normal = normal_data.normal_start;

		for (float i = coor_data.x_start; i < coor_data.x_end; i++)
		{
			if (current_z > zbuf[(int)i][(int)j])
			{
				pos.x = i;
				pos.y = j;
				pos.z = (int)current_z;

				current_colour = c * CalLambert(light, pos, Normalise(current_normal));
				Plotpix(i, j, current_colour.r, current_colour.g, current_colour.b);
				zbuf[(int)i][(int)j] = current_z;
			}

			current_z += z_line_inc;

			current_normal = VectorSum(current_normal, normal_line_inc);
		}

		if (j < coor_data.y_bottom - 1) // add slope value unless at final loop
		{
			coor_data.z_end += coor_data.z_slope_right;
			coor_data.z_start += coor_data.z_slope_left;

			normal_data.normal_end = VectorSum(normal_data.normal_end, normal_data.normal_step_right);
			normal_data.normal_start = VectorSum(normal_data.normal_start, normal_data.normal_step_left);

			coor_data.x_end += coor_data.x_slope_right;
			coor_data.x_start += coor_data.x_slope_left;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: DrawPolygon
// Desc: Draw a polygon
//-----------------------------------------------------------------------------
void DrawPolygon(POLYGON *p)
{
	if (p->nv == 0)
		return;

	int top_vertex = 0;

	//finding top vertex (the vertex with y min) // point 6
	float y_ver = p->vert[0].y;

	for (int i = 1; i < p->nv; i++)
	{
		if (y_ver > p->vert[i].y)
		{
			y_ver = p->vert[i].y;
			top_vertex = i;
		}
	}

	COOR_DATA coor;

	int current_left;
	int current_right;

	int next_left = top_vertex;
	int next_right = top_vertex;

	//int ytop;
	coor.y_bottom = p->vert[top_vertex].y;

	coor.x_start = p->vert[top_vertex].x;
	coor.x_end = p->vert[top_vertex].x;

	coor.z_start = p->vert[top_vertex].z;
	coor.z_end = p->vert[top_vertex].z;

	do
	{
		if (p->vert[next_right].y < p->vert[next_left].y) // last trapezium terminates on the right vertex
		{
			current_right = next_right;

			next_right = (current_right == p->nv - 1) ? 0 : current_right + 1;

			if (p->vert[next_right].y == p->vert[current_right].y)
			{
				continue;
			}
			else
			{
				coor.x_slope_right = ((float)(p->vert[next_right].x - p->vert[current_right].x)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));

				coor.z_slope_right = ((float)(p->vert[next_right].z - p->vert[current_right].z)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));

				coor.x_end = p->vert[current_right].x;

				coor.z_end = p->vert[current_right].z;

				coor.y_top = coor.y_bottom;
				coor.y_bottom = (p->vert[next_left].y < p->vert[next_right].y) ? p->vert[next_left].y : p->vert[next_right].y;

				DrawTrapezium(coor, p->colour);
			}
		}
		else if (p->vert[next_right].y > p->vert[next_left].y) // last trapezium terminates on the left vertex
		{
			current_left = next_left;

			next_left = (current_left == 0) ? p->nv - 1 : current_left - 1;

			if (p->vert[next_left].y == p->vert[current_left].y)
			{
				continue;
			}
			else
			{
				coor.x_slope_left = ((float)(p->vert[next_left].x - p->vert[current_left].x)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));

				coor.z_slope_left = ((float)(p->vert[next_left].z - p->vert[current_left].z)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));

				coor.y_top = coor.y_bottom;
				coor.y_bottom = (p->vert[next_left].y < p->vert[next_right].y) ? p->vert[next_left].y : p->vert[next_right].y;

				coor.x_start = p->vert[current_left].x;

				coor.z_start = p->vert[current_left].z;

				DrawTrapezium(coor, p->colour);
			}
		}
		else if (p->vert[next_right].y == p->vert[next_left].y) // last trapezium terminates on both side (vertex x left = vertex x right)
		{
			if ((next_left == next_right + 1) || ((next_left == next_right - p->nv + 1) && (next_left == 0)))  // next_right == next_left + 1 in this case means that the polygon has a bottom that is parallel with X axes, so we need to finish drawing here
			{
				next_left = next_right;
			}
			else
			{
				current_left = next_left;
				current_right = next_right;

				next_left = (current_left == 0) ? p->nv - 1 : current_left - 1;
				next_right = (current_right == p->nv - 1) ? 0 : current_right + 1;

				if (p->vert[next_right].y == p->vert[current_right].y)
				{
					coor.x_slope_left = ((float)(p->vert[next_left].x - p->vert[current_left].x)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
					coor.z_slope_left = ((float)(p->vert[next_left].z - p->vert[current_left].z)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));

					continue;
				}
				else if (p->vert[next_left].y == p->vert[current_left].y)
				{
					coor.x_slope_right = ((float)(p->vert[next_right].x - p->vert[current_right].x)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));
					coor.z_slope_right = ((float)(p->vert[next_right].z - p->vert[current_right].z)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));

					continue;
				}
				else
				{

					coor.x_slope_left = ((float)(p->vert[next_left].x - p->vert[current_left].x)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
					coor.x_slope_right = ((float)(p->vert[next_right].x - p->vert[current_right].x)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));

					coor.z_slope_left = ((float)(p->vert[next_left].z - p->vert[current_left].z)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
					coor.z_slope_right = ((float)(p->vert[next_right].z - p->vert[current_right].z)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));

					coor.y_top = coor.y_bottom;
					coor.y_bottom = (p->vert[next_left].y < p->vert[next_right].y) ? p->vert[next_left].y : p->vert[next_right].y;

					coor.x_start = p->vert[current_left].x;
					coor.x_end = p->vert[current_right].x;

					coor.z_start = p->vert[current_left].z;
					coor.z_end = p->vert[current_right].z;

					DrawTrapezium(coor, p->colour);
				}
			}
		}
	} while (next_left != next_right);
}


void DrawPolygonTex(POLYGON *p)
{
	if (p->nv == 0)
		return;

	int top_vertex = 0;

	//finding top vertex (the vertex with y min) // point 6
	float y_ver = p->vert[0].y;

	for (int i = 1; i < p->nv; i++)
	{
		if (y_ver > p->vert[i].y)
		{
			y_ver = p->vert[i].y;
			top_vertex = i;
		}
	}

	COOR_DATA coor;
	TEX_DATA tex;

	int current_left;
	int current_right;

	int next_left = top_vertex;
	int next_right = top_vertex;

	//int ytop;
	coor.y_bottom = p->vert[top_vertex].y;

	coor.x_start = p->vert[top_vertex].x;
	coor.x_end = p->vert[top_vertex].x;

	coor.z_start = p->vert[top_vertex].z;
	coor.z_end = p->vert[top_vertex].z;

	tex.tex_start = p->tex_vert[top_vertex];
	tex.tex_end = p->tex_vert[top_vertex];

	tex.tex_map = p->texMap;

	do
	{
		if (p->vert[next_right].y < p->vert[next_left].y) // last trapezium terminates on the right vertex
		{
			current_right = next_right;

			next_right = (current_right == p->nv - 1) ? 0 : current_right + 1;

			if (p->vert[next_right].y == p->vert[current_right].y)
			{
				continue;
			}
			else
			{
				coor.x_slope_right = ((float)(p->vert[next_right].x - p->vert[current_right].x)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));
				coor.z_slope_right = ((float)(p->vert[next_right].z - p->vert[current_right].z)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));
				tex.tex_step_right = (p->tex_vert[next_right] - p->tex_vert[current_right]) / ((float)(p->vert[next_right].y - p->vert[current_right].y));

				coor.x_end = p->vert[current_right].x;
				coor.z_end = p->vert[current_right].z;
				tex.tex_end = p->tex_vert[current_right];

				coor.y_top = coor.y_bottom;
				coor.y_bottom = (p->vert[next_left].y < p->vert[next_right].y) ? p->vert[next_left].y : p->vert[next_right].y;

				//DrawTrapezium(coor, p->colour);
				DrawTrapezium(coor, tex);
			}
		}
		else if (p->vert[next_right].y > p->vert[next_left].y) // last trapezium terminates on the left vertex
		{
			current_left = next_left;

			next_left = (current_left == 0) ? p->nv - 1 : current_left - 1;

			if (p->vert[next_left].y == p->vert[current_left].y)
			{
				continue;
			}
			else
			{
				coor.x_slope_left = ((float)(p->vert[next_left].x - p->vert[current_left].x)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
				coor.z_slope_left = ((float)(p->vert[next_left].z - p->vert[current_left].z)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
				tex.tex_step_left = (p->tex_vert[next_left] - p->tex_vert[current_left]) / ((float)(p->vert[next_left].y - p->vert[current_left].y));

				coor.x_start = p->vert[current_left].x;
				coor.z_start = p->vert[current_left].z;
				tex.tex_start = p->tex_vert[current_left];

				coor.y_top = coor.y_bottom;
				coor.y_bottom = (p->vert[next_left].y < p->vert[next_right].y) ? p->vert[next_left].y : p->vert[next_right].y;

				//DrawTrapezium(coor, p->colour);
				DrawTrapezium(coor, tex);
			}
		}
		else if (p->vert[next_right].y == p->vert[next_left].y) // last trapezium terminates on both side (vertex x left = vertex x right)
		{
			if ((next_left == next_right + 1) || ((next_left == next_right - p->nv + 1) && (next_left == 0)))  // next_right == next_left + 1 in this case means that the polygon has a bottom that is parallel with X axes, so we need to finish drawing here
			{
				next_left = next_right;
			}
			else
			{
				current_left = next_left;
				current_right = next_right;

				next_left = (current_left == 0) ? p->nv - 1 : current_left - 1;
				next_right = (current_right == p->nv - 1) ? 0 : current_right + 1;

				if (p->vert[next_right].y == p->vert[current_right].y)
				{
					coor.x_slope_left = ((float)(p->vert[next_left].x - p->vert[current_left].x)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
					coor.z_slope_left = ((float)(p->vert[next_left].z - p->vert[current_left].z)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
					tex.tex_step_left = (p->tex_vert[next_left] - p->tex_vert[current_left]) / ((float)(p->vert[next_left].y - p->vert[current_left].y));

					continue;
				}
				else if (p->vert[next_left].y == p->vert[current_left].y)
				{
					coor.x_slope_right = ((float)(p->vert[next_right].x - p->vert[current_right].x)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));
					coor.z_slope_right = ((float)(p->vert[next_right].z - p->vert[current_right].z)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));
					tex.tex_step_right = (p->tex_vert[next_right] - p->tex_vert[current_right]) / ((float)(p->vert[next_right].y - p->vert[current_right].y));

					continue;
				}
				else
				{

					coor.x_slope_left = ((float)(p->vert[next_left].x - p->vert[current_left].x)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
					coor.x_slope_right = ((float)(p->vert[next_right].x - p->vert[current_right].x)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));

					coor.z_slope_left = ((float)(p->vert[next_left].z - p->vert[current_left].z)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
					coor.z_slope_right = ((float)(p->vert[next_right].z - p->vert[current_right].z)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));

					tex.tex_step_left = (p->tex_vert[next_left] - p->tex_vert[current_left]) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
					tex.tex_step_right = (p->tex_vert[next_right] - p->tex_vert[current_right]) / ((float)(p->vert[next_right].y - p->vert[current_right].y));

					coor.y_top = coor.y_bottom;
					coor.y_bottom = (p->vert[next_left].y < p->vert[next_right].y) ? p->vert[next_left].y : p->vert[next_right].y;

					coor.x_start = p->vert[current_left].x;
					coor.x_end = p->vert[current_right].x;

					coor.z_start = p->vert[current_left].z;
					coor.z_end = p->vert[current_right].z;

					tex.tex_start = p->tex_vert[current_left];
					tex.tex_end = p->tex_vert[current_right];

					//DrawTrapezium(coor, p->colour);
					DrawTrapezium(coor, tex);
				}
			}
		}
	} while (next_left != next_right);
} 


void DrawPolygonGouraud(POLYGON *p, VECTOR light)
{
	if (p->nv == 0)
		return;

	int top_vertex = 0;

	//finding top vertex (the vertex with y min) // point 6
	float y_ver = p->vert[0].y;

	for (int i = 1; i < p->nv; i++)
	{
		if (y_ver > p->vert[i].y)
		{
			y_ver = p->vert[i].y;
			top_vertex = i;
		}
	}


	COLOUR colour[20];

	for (int i = 0; i < p->nv; i++)
	{
		double lambert_factor = max(Dot(p->vvect[i], Normalise(VectorSum(light, p->vert[i]))), 0);

		colour[i] = p->colour*lambert_factor;
	}

	COOR_DATA coor;
	COLOUR_DATA colourd;

	int current_left;
	int current_right;

	int next_left = top_vertex;
	int next_right = top_vertex;

	coor.y_bottom = p->vert[top_vertex].y;

	coor.x_start = p->vert[top_vertex].x;
	coor.x_end = p->vert[top_vertex].x;

	coor.z_start = p->vert[top_vertex].z;
	coor.z_end = p->vert[top_vertex].z;


	colourd.colour_start = colour[top_vertex];	
	colourd.colour_end = colour[top_vertex];

	do
	{
		if (p->vert[next_right].y < p->vert[next_left].y) // last trapezium terminates on the right vertex
		{
			current_right = next_right;

			next_right = (current_right == p->nv - 1) ? 0 : current_right + 1;

			if (p->vert[next_right].y == p->vert[current_right].y)
			{
				continue;
			}
			else
			{
				coor.x_slope_right = ((float)(p->vert[next_right].x - p->vert[current_right].x)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));
				coor.z_slope_right = ((float)(p->vert[next_right].z - p->vert[current_right].z)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));
				colourd.colour_step_right = (colour[next_right] - colour[current_right]) / ((float)(p->vert[next_right].y - p->vert[current_right].y));


				coor.x_end = p->vert[current_right].x;
				coor.z_end = p->vert[current_right].z;
				colourd.colour_end = colour[current_right];

				coor.y_top = coor.y_bottom;
				coor.y_bottom = (p->vert[next_left].y < p->vert[next_right].y) ? p->vert[next_left].y : p->vert[next_right].y;

				DrawTrapeziumGouraud(coor, colourd);
			}
		}
		else if (p->vert[next_right].y > p->vert[next_left].y) // last trapezium terminates on the left vertex
		{
			current_left = next_left;

			next_left = (current_left == 0) ? p->nv - 1 : current_left - 1;

			if (p->vert[next_left].y == p->vert[current_left].y)
			{
				continue;
			}
			else
			{
				coor.x_slope_left = ((float)(p->vert[next_left].x - p->vert[current_left].x)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
				coor.z_slope_left = ((float)(p->vert[next_left].z - p->vert[current_left].z)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
				colourd.colour_step_left = (colour[next_left] - colour[current_left]) / ((float)(p->vert[next_left].y - p->vert[current_left].y));

				coor.y_top = coor.y_bottom;
				coor.y_bottom = (p->vert[next_left].y < p->vert[next_right].y) ? p->vert[next_left].y : p->vert[next_right].y;

				coor.x_start = p->vert[current_left].x;
				coor.z_start = p->vert[current_left].z;
				colourd.colour_start = colour[current_left];

				DrawTrapeziumGouraud(coor, colourd);
			}
		}
		else if (p->vert[next_right].y == p->vert[next_left].y) // last trapezium terminates on both side (vertex x left = vertex x right)
		{
			if ((next_left == next_right + 1) || ((next_left == next_right - p->nv + 1) && (next_left == 0)))  // next_right == next_left + 1 in this case means that the polygon has a bottom that is parallel with X axes, so we need to finish drawing here
			{
				next_left = next_right;
			}
			else
			{
				current_left = next_left;
				current_right = next_right;

				next_left = (current_left == 0) ? p->nv - 1 : current_left - 1;
				next_right = (current_right == p->nv - 1) ? 0 : current_right + 1;

				if (p->vert[next_right].y == p->vert[current_right].y)
				{
					coor.x_slope_left = ((float)(p->vert[next_left].x - p->vert[current_left].x)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
					coor.z_slope_left = ((float)(p->vert[next_left].z - p->vert[current_left].z)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
					colourd.colour_step_left = (colour[next_left] - colour[current_left]) / ((float)(p->vert[next_left].y - p->vert[current_left].y));

					continue;
				}
				else if (p->vert[next_left].y == p->vert[current_left].y)
				{
					coor.x_slope_right = ((float)(p->vert[next_right].x - p->vert[current_right].x)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));
					coor.z_slope_right = ((float)(p->vert[next_right].z - p->vert[current_right].z)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));
					colourd.colour_step_right = (colour[next_right] - colour[current_right]) / ((float)(p->vert[next_right].y - p->vert[current_right].y));

					continue;
				}
				else
				{

					coor.x_slope_left = ((float)(p->vert[next_left].x - p->vert[current_left].x)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
					coor.x_slope_right = ((float)(p->vert[next_right].x - p->vert[current_right].x)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));

					coor.z_slope_left = ((float)(p->vert[next_left].z - p->vert[current_left].z)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
					coor.z_slope_right = ((float)(p->vert[next_right].z - p->vert[current_right].z)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));

					colourd.colour_step_left = (colour[next_left] - colour[current_left]) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
					colourd.colour_step_right = (colour[next_right] - colour[current_right]) / ((float)(p->vert[next_right].y - p->vert[current_right].y));

					coor.y_top = coor.y_bottom;
					coor.y_bottom = (p->vert[next_left].y < p->vert[next_right].y) ? p->vert[next_left].y : p->vert[next_right].y;

					coor.x_start = p->vert[current_left].x;
					coor.x_end = p->vert[current_right].x;

					coor.z_start = p->vert[current_left].z;
					coor.z_end = p->vert[current_right].z;
					colourd.colour_start = colour[current_left];
					colourd.colour_end = colour[current_right];

					DrawTrapeziumGouraud(coor, colourd);
				}
			}
		}
	} while (next_left != next_right);
}


void DrawPolygonPhong(POLYGON *p, VECTOR light)
{
	if (p->nv == 0)
		return;

	int top_vertex = 0;

	//finding top vertex (the vertex with y min) // point 6
	float y_ver = p->vert[0].y;

	for (int i = 1; i < p->nv; i++)
	{
		if (y_ver > p->vert[i].y)
		{
			y_ver = p->vert[i].y;
			top_vertex = i;
		}
	}

	COOR_DATA coor;
	NORMAL_VERT_DATA normal;

	int current_left;
	int current_right;

	int next_left = top_vertex;
	int next_right = top_vertex;

	coor.y_bottom = p->vert[top_vertex].y;

	coor.x_start = p->vert[top_vertex].x;
	coor.x_end = p->vert[top_vertex].x;

	coor.z_start = p->vert[top_vertex].z;
	coor.z_end = p->vert[top_vertex].z;

	normal.normal_start = p->vvect[top_vertex];
	normal.normal_end = p->vvect[top_vertex];

	do
	{
		if (p->vert[next_right].y < p->vert[next_left].y) // last trapezium terminates on the right vertex
		{
			current_right = next_right;

			next_right = (current_right == p->nv - 1) ? 0 : current_right + 1;

			if (p->vert[next_right].y == p->vert[current_right].y)
			{
				continue;
			}
			else
			{
				coor.x_slope_right = ((float)(p->vert[next_right].x - p->vert[current_right].x)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));
				coor.z_slope_right = ((float)(p->vert[next_right].z - p->vert[current_right].z)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));
				normal.normal_step_right = VectorDevide(VectorDiff(p->vvect[next_right], p->vvect[current_right]), p->vert[next_right].y - p->vert[current_right].y);


				coor.x_end = p->vert[current_right].x;
				coor.z_end = p->vert[current_right].z;
				normal.normal_end = p->vvect[current_right];

				coor.y_top = coor.y_bottom;
				coor.y_bottom = (p->vert[next_left].y < p->vert[next_right].y) ? p->vert[next_left].y : p->vert[next_right].y;

				DrawTrapeziumPhong(coor, normal, light, p->colour);
			}
		}
		else if (p->vert[next_right].y > p->vert[next_left].y) // last trapezium terminates on the left vertex
		{
			current_left = next_left;

			next_left = (current_left == 0) ? p->nv - 1 : current_left - 1;

			if (p->vert[next_left].y == p->vert[current_left].y)
			{
				continue;
			}
			else
			{
				coor.x_slope_left = ((float)(p->vert[next_left].x - p->vert[current_left].x)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
				coor.z_slope_left = ((float)(p->vert[next_left].z - p->vert[current_left].z)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
				normal.normal_step_left = VectorDevide(VectorDiff(p->vvect[next_left], p->vvect[current_left]), p->vert[next_left].y - p->vert[current_left].y);

				coor.y_top = coor.y_bottom;
				coor.y_bottom = (p->vert[next_left].y < p->vert[next_right].y) ? p->vert[next_left].y : p->vert[next_right].y;

				coor.x_start = p->vert[current_left].x;
				coor.z_start = p->vert[current_left].z;
				normal.normal_start = p->vvect[current_left];

				DrawTrapeziumPhong(coor, normal, light, p->colour);
			}
		}
		else if (p->vert[next_right].y == p->vert[next_left].y) // last trapezium terminates on both side (vertex x left = vertex x right)
		{
			if ((next_left == next_right + 1) || ((next_left == next_right - p->nv + 1) && (next_left == 0)))  // next_right == next_left + 1 in this case means that the polygon has a bottom that is parallel with X axes, so we need to finish drawing here
			{
				next_left = next_right;
			}
			else
			{
				current_left = next_left;
				current_right = next_right;

				next_left = (current_left == 0) ? p->nv - 1 : current_left - 1;
				next_right = (current_right == p->nv - 1) ? 0 : current_right + 1;

				if (p->vert[next_right].y == p->vert[current_right].y)
				{
					coor.x_slope_left = ((float)(p->vert[next_left].x - p->vert[current_left].x)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
					coor.z_slope_left = ((float)(p->vert[next_left].z - p->vert[current_left].z)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
					normal.normal_step_left = VectorDevide(VectorDiff(p->vvect[next_left], p->vvect[current_left]), p->vert[next_left].y - p->vert[current_left].y);

					continue;
				}
				else if (p->vert[next_left].y == p->vert[current_left].y)
				{
					coor.x_slope_right = ((float)(p->vert[next_right].x - p->vert[current_right].x)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));
					coor.z_slope_right = ((float)(p->vert[next_right].z - p->vert[current_right].z)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));
					normal.normal_step_right = VectorDevide(VectorDiff(p->vvect[next_right], p->vvect[current_right]), p->vert[next_right].y - p->vert[current_right].y);

					continue;
				}
				else
				{

					coor.x_slope_left = ((float)(p->vert[next_left].x - p->vert[current_left].x)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
					coor.x_slope_right = ((float)(p->vert[next_right].x - p->vert[current_right].x)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));

					coor.z_slope_left = ((float)(p->vert[next_left].z - p->vert[current_left].z)) / ((float)(p->vert[next_left].y - p->vert[current_left].y));
					coor.z_slope_right = ((float)(p->vert[next_right].z - p->vert[current_right].z)) / ((float)(p->vert[next_right].y - p->vert[current_right].y));

					normal.normal_step_left = VectorDevide(VectorDiff(p->vvect[next_left], p->vvect[current_left]), p->vert[next_left].y - p->vert[current_left].y);
					normal.normal_step_right = VectorDevide(VectorDiff(p->vvect[next_right], p->vvect[current_right]), p->vert[next_right].y - p->vert[current_right].y);

					coor.y_top = coor.y_bottom;
					coor.y_bottom = (p->vert[next_left].y < p->vert[next_right].y) ? p->vert[next_left].y : p->vert[next_right].y;

					coor.x_start = p->vert[current_left].x;
					coor.x_end = p->vert[current_right].x;

					coor.z_start = p->vert[current_left].z;
					coor.z_end = p->vert[current_right].z;
					normal.normal_start = p->vvect[current_left];
					normal.normal_end = p->vvect[current_right];

					DrawTrapeziumPhong(coor, normal, light, p->colour);
				}
			}
		}
	} while (next_left != next_right);
}


//-----------------------------------------------------------------------------
// Name: ClipPolyXLow
// Desc: Clipping Routine for lower x boundary
//-----------------------------------------------------------------------------
int ClipPolyXLow(POLYGON *pIinput, POLYGON *pOutput, int iXBound)
{
	int current_vertex, previous_vertex;

	//copy non-vertex-related part
	pOutput->colour = pIinput->colour;
	pOutput->normal = pIinput->normal;
	memcpy(pOutput->vertVecNo, pIinput->vertVecNo, sizeof(pIinput->vertVecNo));
	pOutput->texMap = pIinput->texMap;
	memcpy(pOutput->tex_vert, pIinput->tex_vert, sizeof(pOutput->tex_vert));

	pOutput->nv = 0;

	for (current_vertex = 0; current_vertex < pIinput->nv; current_vertex++)
	{
		previous_vertex = (current_vertex == 0) ? pIinput->nv - 1 : current_vertex - 1;
		if (pIinput->vert[previous_vertex].x >= iXBound)
		{
			if (pIinput->vert[current_vertex].x >= iXBound)
			{
				pOutput->vert[pOutput->nv] = pIinput->vert[current_vertex];
				pOutput->vvect[pOutput->nv] = pIinput->vvect[current_vertex];
				pOutput->tex_vert[pOutput->nv] = pIinput->tex_vert[current_vertex];

				pOutput->nv++;
			}
			else
			{
				pOutput->vert[pOutput->nv].x = iXBound;

				pOutput->vert[pOutput->nv].y = (pIinput->vert[current_vertex].x*pIinput->vert[previous_vertex].y - pIinput->vert[previous_vertex].x*pIinput->vert[current_vertex].y
					+ iXBound*pIinput->vert[current_vertex].y - iXBound*pIinput->vert[previous_vertex].y) /
					(pIinput->vert[current_vertex].x - pIinput->vert[previous_vertex].x);

				pOutput->vert[pOutput->nv].z = (pIinput->vert[current_vertex].x*pIinput->vert[previous_vertex].z - pIinput->vert[previous_vertex].x*pIinput->vert[current_vertex].z
					+ iXBound*pIinput->vert[current_vertex].z - iXBound*pIinput->vert[previous_vertex].z) /
					(pIinput->vert[current_vertex].x - pIinput->vert[previous_vertex].x);

				pOutput->vvect[pOutput->nv] = pIinput->vvect[current_vertex];

				pOutput->nv++;
			}
		}
		else
		{
			if (pIinput->vert[current_vertex].x >= iXBound)
			{
				pOutput->vert[pOutput->nv].x = iXBound;
				pOutput->vert[pOutput->nv].y = (pIinput->vert[current_vertex].x*pIinput->vert[previous_vertex].y - pIinput->vert[previous_vertex].x*pIinput->vert[current_vertex].y
					+ iXBound*pIinput->vert[current_vertex].y - iXBound*pIinput->vert[previous_vertex].y) /
					(pIinput->vert[current_vertex].x - pIinput->vert[previous_vertex].x);

				pOutput->vert[pOutput->nv].z = (pIinput->vert[current_vertex].x*pIinput->vert[previous_vertex].z - pIinput->vert[previous_vertex].x*pIinput->vert[current_vertex].z
					+ iXBound*pIinput->vert[current_vertex].z - iXBound*pIinput->vert[previous_vertex].z) /
					(pIinput->vert[current_vertex].x - pIinput->vert[previous_vertex].x);

				pOutput->vvect[pOutput->nv] = pIinput->vvect[previous_vertex];

				pOutput->nv++;

				pOutput->vert[pOutput->nv] = pIinput->vert[current_vertex];
				pOutput->vvect[pOutput->nv] = pIinput->vvect[current_vertex];

				pOutput->nv++;
			}
			else
			{
				//do nothing
			}
		}
	}

	return pOutput->nv;
}

//-----------------------------------------------------------------------------
// Name: ClipPolyYLow
// Desc: Clipping Routine for lower y boundary
//-----------------------------------------------------------------------------
int ClipPolyYLow(POLYGON *pIinput, POLYGON *pOutput, int iYBound)
{
	// Tell calling routine how many vertices in pOutput array
	int current_vertex, previous_vertex;

	//copy non-vertex-related part
	pOutput->colour = pIinput->colour;
	pOutput->normal = pIinput->normal;
	memcpy(pOutput->vertVecNo, pIinput->vertVecNo, sizeof(pIinput->vertVecNo));
	pOutput->texMap = pIinput->texMap;
	memcpy(pOutput->tex_vert, pIinput->tex_vert, sizeof(pOutput->tex_vert));

	pOutput->nv = 0;

	for (current_vertex = 0; current_vertex < pIinput->nv; current_vertex++)
	{
		previous_vertex = (current_vertex == 0) ? pIinput->nv - 1 : current_vertex - 1;
		if (pIinput->vert[previous_vertex].y >= iYBound)
		{
			if (pIinput->vert[current_vertex].y >= iYBound)
			{
				pOutput->vert[pOutput->nv] = pIinput->vert[current_vertex];
				pOutput->vvect[pOutput->nv] = pIinput->vvect[current_vertex];
				pOutput->nv++;
			}
			else
			{
				pOutput->vert[pOutput->nv].y = iYBound;

				pOutput->vert[pOutput->nv].x = (pIinput->vert[previous_vertex].x*pIinput->vert[current_vertex].y - pIinput->vert[current_vertex].x*pIinput->vert[previous_vertex].y
					+ iYBound*pIinput->vert[current_vertex].x - iYBound*pIinput->vert[previous_vertex].x) /
					(pIinput->vert[current_vertex].y - pIinput->vert[previous_vertex].y);

				pOutput->vert[pOutput->nv].z = (pIinput->vert[previous_vertex].z*pIinput->vert[current_vertex].y - pIinput->vert[current_vertex].z*pIinput->vert[previous_vertex].y
					+ iYBound*pIinput->vert[current_vertex].z - iYBound*pIinput->vert[previous_vertex].z) /
					(pIinput->vert[current_vertex].y - pIinput->vert[previous_vertex].y);

				pOutput->vvect[pOutput->nv] = pIinput->vvect[current_vertex];

				pOutput->nv++;
			}
		}
		else
		{
			if (pIinput->vert[current_vertex].y >= iYBound)
			{
				pOutput->vert[pOutput->nv].y = iYBound;

				pOutput->vert[pOutput->nv].x = (pIinput->vert[previous_vertex].x*pIinput->vert[current_vertex].y - pIinput->vert[current_vertex].x*pIinput->vert[previous_vertex].y
					+ iYBound*pIinput->vert[current_vertex].x - iYBound*pIinput->vert[previous_vertex].x) /
					(pIinput->vert[current_vertex].y - pIinput->vert[previous_vertex].y);

				pOutput->vert[pOutput->nv].z = (pIinput->vert[previous_vertex].z*pIinput->vert[current_vertex].y - pIinput->vert[current_vertex].z*pIinput->vert[previous_vertex].y
					+ iYBound*pIinput->vert[current_vertex].z - iYBound*pIinput->vert[previous_vertex].z) /
					(pIinput->vert[current_vertex].y - pIinput->vert[previous_vertex].y);

				pOutput->vvect[pOutput->nv] = pIinput->vvect[previous_vertex];

				pOutput->nv++;

				pOutput->vert[pOutput->nv] = pIinput->vert[current_vertex];
				pOutput->vvect[pOutput->nv] = pIinput->vvect[current_vertex];

				pOutput->nv++;
			}
			else
			{
				//do nothing
			}
		}
	}

	return pOutput->nv;
}

//-----------------------------------------------------------------------------
// Name: ClipPolyXHi
// Desc: Clipping Routine for upper x boundary
//-----------------------------------------------------------------------------
int ClipPolyXHigh(POLYGON *pIinput, POLYGON *pOutput, int iXBound)
{
	// Tell calling routine how many vertices in pOutput array
	int current_vertex, previous_vertex;
	
	//copy non-vertex-related part
	pOutput->colour = pIinput->colour;
	pOutput->normal = pIinput->normal;				 
	memcpy(pOutput->vertVecNo, pIinput->vertVecNo, sizeof(pIinput->vertVecNo));
	pOutput->texMap = pIinput->texMap;
	memcpy(pOutput->tex_vert, pIinput->tex_vert, sizeof(pOutput->tex_vert));

	pOutput->nv = 0;

	for (current_vertex = 0; current_vertex < pIinput->nv; current_vertex++)
	{
		previous_vertex = (current_vertex == 0) ? pIinput->nv - 1 : current_vertex - 1;
		if (pIinput->vert[previous_vertex].x <= iXBound)
		{
			if (pIinput->vert[current_vertex].x <= iXBound)
			{
				pOutput->vert[pOutput->nv] = pIinput->vert[current_vertex];
				pOutput->vvect[pOutput->nv] = pIinput->vvect[current_vertex];
				pOutput->nv++;
			}
			else
			{
				pOutput->vert[pOutput->nv].x = iXBound;

				pOutput->vert[pOutput->nv].y = (pIinput->vert[current_vertex].x*pIinput->vert[previous_vertex].y - pIinput->vert[previous_vertex].x*pIinput->vert[current_vertex].y
					+ iXBound*pIinput->vert[current_vertex].y - iXBound*pIinput->vert[previous_vertex].y) /
					(pIinput->vert[current_vertex].x - pIinput->vert[previous_vertex].x);

				pOutput->vert[pOutput->nv].z = (pIinput->vert[current_vertex].x*pIinput->vert[previous_vertex].z - pIinput->vert[previous_vertex].x*pIinput->vert[current_vertex].z
					+ iXBound*pIinput->vert[current_vertex].z - iXBound*pIinput->vert[previous_vertex].z) /
					(pIinput->vert[current_vertex].x - pIinput->vert[previous_vertex].x);

				pOutput->vvect[pOutput->nv] = pIinput->vvect[current_vertex];

				pOutput->nv++;
			}
		}
		else
		{
			if (pIinput->vert[current_vertex].x <= iXBound)
			{
				pOutput->vert[pOutput->nv].x = iXBound;
				pOutput->vert[pOutput->nv].y = (pIinput->vert[current_vertex].x*pIinput->vert[previous_vertex].y - pIinput->vert[previous_vertex].x*pIinput->vert[current_vertex].y
					+ iXBound*pIinput->vert[current_vertex].y - iXBound*pIinput->vert[previous_vertex].y) /
					(pIinput->vert[current_vertex].x - pIinput->vert[previous_vertex].x);

				pOutput->vert[pOutput->nv].z = (pIinput->vert[current_vertex].x*pIinput->vert[previous_vertex].z - pIinput->vert[previous_vertex].x*pIinput->vert[current_vertex].z
					+ iXBound*pIinput->vert[current_vertex].z - iXBound*pIinput->vert[previous_vertex].z) /
					(pIinput->vert[current_vertex].x - pIinput->vert[previous_vertex].x);

				pOutput->vvect[pOutput->nv] = pIinput->vvect[previous_vertex];

				pOutput->nv++;

				pOutput->vert[pOutput->nv] = pIinput->vert[current_vertex];
				pOutput->vvect[pOutput->nv] = pIinput->vvect[current_vertex];

				pOutput->nv++;
			}
			else
			{
				//do nothing
			}
		}
	}

	return pOutput->nv;
}

//-----------------------------------------------------------------------------
// Name: ClipPolyYHi
// Desc: Clipping Routine for upper y boundary
//-----------------------------------------------------------------------------
int ClipPolyYHigh(POLYGON *pIinput, POLYGON *pOutput, int iYBound)
{
	// Tell calling routine how many vertices in pOutput array
	int current_vertex, previous_vertex;

	//copy non-vertex-related part
	pOutput->colour = pIinput->colour;
	pOutput->normal = pIinput->normal;
	memcpy(pOutput->vertVecNo, pIinput->vertVecNo, sizeof(pIinput->vertVecNo));
	pOutput->texMap = pIinput->texMap;
	memcpy(pOutput->tex_vert, pIinput->tex_vert, sizeof(pOutput->tex_vert));

	pOutput->nv = 0;

	for (current_vertex = 0; current_vertex < pIinput->nv; current_vertex++)
	{
		previous_vertex = (current_vertex == 0) ? pIinput->nv - 1 : current_vertex - 1;
		if (pIinput->vert[previous_vertex].y <= iYBound)
		{
			if (pIinput->vert[current_vertex].y <= iYBound)
			{
				pOutput->vert[pOutput->nv] = pIinput->vert[current_vertex];
				pOutput->vvect[pOutput->nv] = pIinput->vvect[current_vertex];
				pOutput->nv++;
			}
			else
			{
				pOutput->vert[pOutput->nv].y = iYBound;

				pOutput->vert[pOutput->nv].x = (pIinput->vert[previous_vertex].x*pIinput->vert[current_vertex].y - pIinput->vert[current_vertex].x*pIinput->vert[previous_vertex].y
					+ iYBound*pIinput->vert[current_vertex].x - iYBound*pIinput->vert[previous_vertex].x) /
					(pIinput->vert[current_vertex].y - pIinput->vert[previous_vertex].y);

				pOutput->vert[pOutput->nv].z = (pIinput->vert[previous_vertex].z*pIinput->vert[current_vertex].y - pIinput->vert[current_vertex].z*pIinput->vert[previous_vertex].y
					+ iYBound*pIinput->vert[current_vertex].z - iYBound*pIinput->vert[previous_vertex].z) /
					(pIinput->vert[current_vertex].y - pIinput->vert[previous_vertex].y);

				pOutput->vvect[pOutput->nv] = pIinput->vvect[current_vertex];

				pOutput->nv++;
			}
		}
		else
		{
			if (pIinput->vert[current_vertex].y <= iYBound)
			{
				pOutput->vert[pOutput->nv].y = iYBound;

				pOutput->vert[pOutput->nv].x = (pIinput->vert[previous_vertex].x*pIinput->vert[current_vertex].y - pIinput->vert[current_vertex].x*pIinput->vert[previous_vertex].y
					+ iYBound*pIinput->vert[current_vertex].x - iYBound*pIinput->vert[previous_vertex].x) /
					(pIinput->vert[current_vertex].y - pIinput->vert[previous_vertex].y);

				pOutput->vert[pOutput->nv].z = (pIinput->vert[previous_vertex].z*pIinput->vert[current_vertex].y - pIinput->vert[current_vertex].z*pIinput->vert[previous_vertex].y
					+ iYBound*pIinput->vert[current_vertex].z - iYBound*pIinput->vert[previous_vertex].z) /
					(pIinput->vert[current_vertex].y - pIinput->vert[previous_vertex].y);

				pOutput->vvect[pOutput->nv] = pIinput->vvect[previous_vertex];

				pOutput->nv++;

				pOutput->vert[pOutput->nv] = pIinput->vert[current_vertex];
				pOutput->vvect[pOutput->nv] = pIinput->vvect[current_vertex];

				pOutput->nv++;
			}
			else
			{
				//do nothing
			}
		}
	}

	return pOutput->nv;
}

//-----------------------------------------------------------------------------
// Name: Init
// Desc: Initialises Direct3D etc.
//		 This is called before switch to graphics mode,
//		 example of z buffer initialisation shown in comments,
//		 ignore for parts a/b.
//-----------------------------------------------------------------------------
