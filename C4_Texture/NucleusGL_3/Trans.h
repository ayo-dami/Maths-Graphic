//--------------------------------------------------------------------------------------------
// File:		Trans.h
// Version:		V2.0
// Author:		Dr. W. Steiger / Dr. R.J. Cant / Daniel Rhodes
// Description:	Based on Trans.h by Dr. W. Steiger / Dr. R.J. Cant
// Notes:		
//--------------------------------------------------------------------------------------------

// System header files
// --

// Windows header files
// --

// DirectX header files
// --

// Custom header files
// --

// Only include this file once, may cause problems with some compilers.
#pragma once
// Same effect as:
// #ifndef TRANS_H
// #define TRANS_H
//
// main body of code
//
// #endif // !defined(TRANS_H)

#define	FTYPE float

typedef struct
{
	FTYPE x,y,z;
} VECTOR;

typedef struct
{
	VECTOR cx,cy,cz;
} MATRIX;

typedef struct
{
	VECTOR shift;
	MATRIX rotate;
} TRANSFORM;


// Graphics transformations function prototypes
FTYPE		Dot( VECTOR vVecA, VECTOR vVecB );
VECTOR		Cross( VECTOR vVecA, VECTOR vVecB );
VECTOR		VScale( FTYPE fScale, VECTOR vVec );
VECTOR		Normalise( VECTOR vVec );
MATRIX		Transpose( MATRIX mMat );
VECTOR		MOnV( MATRIX mMat, VECTOR vVec );
MATRIX		Product( MATRIX mMatA,MATRIX mMatB );
MATRIX		XRot( FTYPE fAngle );
MATRIX		YRot( FTYPE fAngle );
MATRIX		ZRot( FTYPE fAngle );
MATRIX		InverseRotationOnly( TRANSFORM obj ); // Geometry utility
VECTOR		VectorSum( VECTOR vVecA, VECTOR vVecB );
VECTOR		VectorDiff( VECTOR vVecA, VECTOR vVecB );
VECTOR		VectorDevide( VECTOR vVecA, double a);
VECTOR		Project( VECTOR vVec, FTYPE  fViewAngle );
VECTOR		Minus( VECTOR vVec );
VECTOR		DoTransform( VECTOR vPoint, TRANSFORM tAction );
VECTOR		DoViewTransform( VECTOR vPoint,TRANSFORM tAction );
TRANSFORM	BuildTrans( FTYPE fXAngle, FTYPE fYAngle, FTYPE fZAngle, VECTOR vDisp );
TRANSFORM	BuildViewTrans( FTYPE fXAngle, FTYPE fYAngle, FTYPE fZAngle, VECTOR vDisp );
TRANSFORM	CombineTrans( TRANSFORM tTranA, TRANSFORM tTranB );
TRANSFORM	CombineViewTrans( TRANSFORM tTranA, TRANSFORM tTranB );
double Max(double a, double b);
double CalLambert(VECTOR light, VECTOR pos, VECTOR normal);
