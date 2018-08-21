//--------------------------------------------------------------------------------------------
// File:		Windows_interface.cpp
// Version:		V1.0
// Author:		RJC
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


static long g_this;

extern	int m_iMoveX;
extern	int m_iMoveY;
extern	int m_iMoveZ;	
extern float m_dThetaX;
extern 	float m_dThetaY;
extern 	float m_dThetaZ;
extern 	double m_dViewAngle;
	//debugging control variables
extern 	int iFaceSelection;			// FaceSelection > 0: enables one face (with index selecton - 1)
								// to be drawn on its own to aid debugging.
extern 	bool bDebug;				// Switches on debug output to file when true.
	// number of polygons read in
extern 	int m_iNumOfPolys;
	//Frame Count
extern 	int m_iOurFrameCount;
	// Input filename
extern 	char m_sFilename[30];
	// Viewpoint data, use for parts B/C
extern 	VECTOR m_vDisp, m_vLight;	// Displacement of object coordinates relative
								// to viewing coordinates and light direction
extern 	int m_iWidth;
extern 	int m_iHeight;

	//Drawing Surface Handle
extern 	float *m_fDrawingSurface;

extern SHADING_TYPE m_Shading;

extern bool m_bTextureEnable;

HINSTANCE g_hInstance;
void Interface::Init( HINSTANCE hInstance )
{
	//For Section B you may wish to re-instate this call
//	DialogBoxParam( hInstance, MAKEINTRESOURCE(IDD_INIT), m_hWindow, DialogMessageHandlerStatic, (LPARAM)this );
/*
	for (int i = 0; i < m_iWidth; i++)
		for (int j = 0; j < m_iHeight; j++)
			zBuffer[i][j] = 0.0;
*/	
}

//-----------------------------------------------------------------------------
// Name: Shutdown
// Desc: Used to clean up any lose ends
//-----------------------------------------------------------------------------
void Interface::Shutdown( )
{
	displayFinal( );

}
//-----------------------------------------------------------------------------
// Name: Interface
// Desc: Default constructor
//-----------------------------------------------------------------------------
Interface::Interface(	HWND p_hWindow, float *p_fDrawingSurface, int p_iWidth, int p_iHeight )
{// Initialise variables
	// Pointer to our dialog's message handler	
	g_this = (long)this;

	iFaceSelection = 0;			// FaceSelection > 0: enables one face (with index selecton - 1)
								// to be drawn on its own to aid debugging.
	bDebug = true;				// Switches on debug output to file when true.

	m_iWidth	= p_iWidth;
	m_iHeight	= p_iHeight;

	m_hWindow = p_hWindow;
	m_fDrawingSurface = p_fDrawingSurface;
	
	m_iX = 300;
	m_iY = 300;
	m_iZ = 300;



	m_iNumOfPolys = 0;

	// Default values, can change in dialog
	m_vDisp.x	= 0;
	m_vDisp.y	= 0;
	m_vDisp.z	= 1000;
	m_vLight.x	= 0;
	m_vLight.y	= 0;
	m_vLight.z	= 1000;

	m_dThetaX		= 0.0;
	m_dThetaY		= 0.0;
	m_dThetaZ		= 0.0; 
	m_dViewAngle	= 0.8;

	m_iMoveX		= 0;
	m_iMoveY		= 0;
	m_iMoveZ		= 1000;
	m_iXShift		= 0;
	m_iYShift		= 0;
	m_iZShiftObj	= 0;

	m_Shading = NONE_SHADING;

	m_bTextureEnable = false;

	// Keep track of the fram counts
	m_iGLFrameCount		= 0;
	m_iOurFrameCount	= 0;

	// Default filename
	strcpy( m_sFilename, "texbox.dat");

	debugfile = fopen("debug.txt","w");
}

//-----------------------------------------------------------------------------
// Name: Alert
// Desc: Display a pop up alert
//-----------------------------------------------------------------------------
void Interface::Alert(char *Head, char *Inner)
{
	MessageBox(m_hWindow, Inner, Head, MB_OK);
}

//-----------------------------------------------------------------------------
// Name: DialogMessageHandlerStatic
// Desc: Static Message Handler for the dialog
//-----------------------------------------------------------------------------
INT_PTR CALLBACK Interface::DialogMessageHandlerStatic( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	// Cheat to avoid using MFC: really bad programming practice, DO NOT do this :-)where
	Interface *pThis = (Interface *)g_this;
	return ( pThis->DialogMessageHandler( hDlg, msg, wParam, lParam ) );
}

//-----------------------------------------------------------------------------
// Name: DialogMessageHandler
// Desc: Message Handler for the dialog
//-----------------------------------------------------------------------------
INT_PTR CALLBACK Interface::DialogMessageHandler( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{	
	BOOL pbSuccess;

	char temp[30] = "";
	int iLength		= 30;

	DWORD err = 0;
	UINT returnval = 0;

	switch (msg)
	{
		case WM_INITDIALOG:
			// Set vaules in dialog
			SetDlgItemText( hDlg, IDC_FILENAME, m_sFilename );
			sprintf( temp, "%f",m_vDisp.x );
			SetDlgItemText( hDlg, IDC_DISTX, temp );
			sprintf( temp, "%f", m_vDisp.y );
			SetDlgItemText( hDlg, IDC_DISTY, temp );
			sprintf( temp, "%f",m_vDisp.z );
			SetDlgItemText( hDlg, IDC_DISTZ, temp );
			sprintf( temp, "%f", m_dThetaX );
			SetDlgItemText( hDlg, IDC_ANGLEX, temp );
			sprintf( temp, "%f", m_dThetaY );
			SetDlgItemText( hDlg, IDC_ANGLEY, temp );
			sprintf( temp, "%f", m_dThetaZ );
			SetDlgItemText( hDlg, IDC_ANGLEZ, temp );
			sprintf( temp, "%f", m_vLight.x );
			SetDlgItemText( hDlg, IDC_LIGHTX, temp );
			sprintf( temp, "%f", m_vLight.y );
			SetDlgItemText( hDlg, IDC_LIGHTY, temp );
			sprintf( temp, "%f", m_vLight.z );
			SetDlgItemText( hDlg, IDC_LIGHTZ, temp );
			CheckDlgButton( hDlg, IDC_NONE, m_Shading == NONE_SHADING );
			CheckDlgButton( hDlg, IDC_GOURAUD, m_Shading == GOURAUD_SHADING );
			CheckDlgButton( hDlg, IDC_PHONG, m_Shading == PHONG_SHADING );
			CheckDlgButton( hDlg, IDC_TEX, m_bTextureEnable == true );
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) 
			{
				case IDOK:	
					m_bTextureEnable = IsDlgButtonChecked(hDlg, IDC_TEX);
					// Read new values from dialog
					GetDlgItemText( hDlg, IDC_FILENAME, temp, iLength );
					strcpy( m_sFilename, temp );
					//m_iMoveX	= GetDlgItemInt( hDlg, IDC_DISTX, &pbSuccess, TRUE );
					//m_iMoveY	= GetDlgItemInt( hDlg, IDC_DISTY, &pbSuccess, TRUE );
					//m_iMoveZ	= GetDlgItemInt( hDlg, IDC_DISTZ, &pbSuccess, TRUE );
					GetDlgItemText( hDlg, IDC_DISTX, temp, iLength );
					m_vDisp.x = (float)atof(temp);
					GetDlgItemText( hDlg, IDC_DISTY, temp, iLength );
					m_vDisp.y = (float)atof(temp);
					GetDlgItemText( hDlg, IDC_DISTZ, temp, iLength );
					m_vDisp.z = (float)atof(temp);
					GetDlgItemText( hDlg, IDC_ANGLEX, temp, iLength );
					m_dThetaX = (float)atof(temp);
					GetDlgItemText( hDlg, IDC_ANGLEY, temp, iLength );
					m_dThetaY = (float)atof(temp);
					GetDlgItemText( hDlg, IDC_ANGLEZ, temp, iLength );
					m_dThetaZ = (float)atof(temp);
					GetDlgItemText( hDlg, IDC_LIGHTX, temp, iLength );
					m_vLight.x = atof(temp);
					GetDlgItemText( hDlg, IDC_LIGHTY, temp, iLength );
					m_vLight.y = atof(temp);
					GetDlgItemText( hDlg, IDC_LIGHTZ, temp, iLength );
					m_vLight.z = atof(temp);
					m_Shading = (IsDlgButtonChecked(hDlg, IDC_NONE) == true) ? NONE_SHADING : (IsDlgButtonChecked(hDlg, IDC_GOURAUD) == true) ? 
						GOURAUD_SHADING : (IsDlgButtonChecked(hDlg, IDC_PHONG) == true) ? PHONG_SHADING : NONE_SHADING;
					EndDialog(hDlg, TRUE);
					ReadFile();
				break;
				case IDCANCEL:
					// Leave default values
					EndDialog(hDlg, TRUE);
					break;
			break;
			}
	break;		
	}

	return FALSE;
}
