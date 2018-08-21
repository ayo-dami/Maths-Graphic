//--------------------------------------------------------------------------------------------
// File:		Trans.cpp
// Version:		V2.0
// Author:		Dr. R.J. Cant / Daniel Rhodes
// Description:	Based on Trans.cpp by Dr. R.J. Cant 7/1/94 				
//				Routines for performing graphic geometry operations
// Notes:		These routines assume that the vertices which you wish to transform are 
//				stored in the form of VECTOR structures as defined at the start of the file. 
//
//				The routines assume a right handed coordinate system and are best used with 
//				the line of sight in the positive direction along the z axis and the y axis 
//				pointing down.
//
//				You do not need to have both a viewpoint and an object transformation but both 
//				are supported by these routines.
//
//				You should define rotation angles about the x,y,and z axes for both object to
//				world and world to viewpoint systems. Similarly position vectors defining the 
//				position of the object and the position of the viewpoint within the world
//				should be set up.
//
//
//				Next call BuildTrans and BuildViewTrans with these parameters.
//
//
//				Now you can transform any point from object to world coords by calling 
//				do_trans using your output from build trans as parameter and from world 
//				coords to viewing coords by calling do_view_trans using your output from 
//				BuildViewTrans as parameter. Alternatively use CombineViewTrans to bundle 
//				the two transformations together into one which will allow a single call to 
//				do_trans to be used for the complete transformation. For nested layers of 
//				object transformation use CombineTrans to put the transformations together. 
//				Finally you can use Project to create a two dimensional version of the object 
//				in a screen coordinate system which has (0,0) at the centre (-1,-1) at the top 
//				left corner and (+1,+1) at the bottom right corner.
//
//
//				Typical use:
//
//				Given an object positioned at  a point defined by position vector p and
//				rotated through angles thetax,thetay and thetaz.
//
//				TRANSFORM obj;
//				obj=BuildTrans(thetax,thetay,thetaz,p);
//
//				for ("All position vectors "x" which define the object":)
//				{
//				xt=do_trans(x,obj);
//				xs=Project(xt,view_angle);
//				xpix=((xs.x+1)*SCREENWIDTH)/2;
//				ypix=((xs.y+1)*SCREENHEIGHT)/2;
//				}
//				The variables xpix and ypix give the screen position in pixels of each point
//				in the object.
//--------------------------------------------------------------------------------------------

// System header files
#include <math.h>

// Windows header files
// --

// DirectX header files
// --

// Custom header files
#include "Trans.h"

//-----------------------------------------------------------------------------
// Name: Dot
// Desc: Performs a Dot Product operation
//-----------------------------------------------------------------------------
FTYPE Dot( VECTOR vVecA, VECTOR vVecB )
{
	return ( vVecA.x * vVecB.x ) + ( vVecA.y * vVecB.y ) + ( vVecA.z * vVecB.z );
}

//-----------------------------------------------------------------------------
// Name: Cross
// Desc: Performs a cross Product operation
//		 This routine is useful for constructing surface normal vectors,
//		 since the cross Product of two vectors is guaranteed to be at right
//		 angles to both of them.
//-----------------------------------------------------------------------------
VECTOR Cross( VECTOR vVecA, VECTOR vVecB )
{
	VECTOR vResultantVec;

	vResultantVec.x = ( vVecA.y * vVecB.z ) - ( vVecA.z * vVecB.y );
	vResultantVec.y = ( vVecA.z * vVecB.x ) - ( vVecA.x * vVecB.z );
	vResultantVec.z = ( vVecA.x * vVecB.y ) - ( vVecA.y * vVecB.x );

	return vResultantVec;
}

//-----------------------------------------------------------------------------
// Name: VScale
// Desc: Scale the Vector vVec by the number s
//-----------------------------------------------------------------------------
VECTOR VScale( FTYPE fScale, VECTOR vVec )
{
	VECTOR vResultantVec;

	vResultantVec.x = vVec.x * fScale;
	vResultantVec.y = vVec.y * fScale;
	vResultantVec.z = vVec.z * fScale;

	return vResultantVec;
}

//-----------------------------------------------------------------------------
// Name: Normalise
// Desc: Returns Normalised version of vVec
//-----------------------------------------------------------------------------
VECTOR Normalise( VECTOR vVec )
{
	if ( Dot( vVec, vVec ) > 0 )
		return VScale( ( (FTYPE)1.0 / sqrt( Dot( vVec, vVec ) ) ), vVec );
	else return vVec;
}

//-----------------------------------------------------------------------------
// Name: Transpose
// Desc: Transpose any arbitrary matrix and will invert any rotation matrix
//
//			cx:		cy:		cz:
//			x	|	x	|	x
//			y	|	y	|	y
//			z	|	z	|	z
//
//					to
//
//			cx:	x	|	y	|	z
//			cy:	x	|	y	|	z
//			cz:	x	|	y	|	z
//
//-----------------------------------------------------------------------------
MATRIX Transpose( MATRIX mMat )
{
	MATRIX mResultantMat;

	// Vector cx
	mResultantMat.cx.x = mMat.cx.x;
	mResultantMat.cx.y = mMat.cy.x;
	mResultantMat.cx.z = mMat.cz.x;
	// Vector cy
	mResultantMat.cy.x = mMat.cx.y;
	mResultantMat.cy.y = mMat.cy.y;
	mResultantMat.cy.z = mMat.cz.y;
	// Vector cz
	mResultantMat.cz.x = mMat.cx.z;
	mResultantMat.cz.y = mMat.cy.z;
	mResultantMat.cz.z = mMat.cz.z;

	return mResultantMat;
}

//-----------------------------------------------------------------------------
// Name: MOnV
// Desc: Transpose any arbitrary matrix and will invert any rotation matrix
//-----------------------------------------------------------------------------
VECTOR MOnV( MATRIX mMat, VECTOR vVec )
{
	VECTOR vResultantVec;
	MATRIX mTransoposedMat;

	mTransoposedMat = Transpose(mMat);
	vResultantVec.x = Dot( mTransoposedMat.cx, vVec );
	vResultantVec.y = Dot( mTransoposedMat.cy, vVec );
	vResultantVec.z = Dot( mTransoposedMat.cz, vVec );

	return vResultantVec;
}

//-----------------------------------------------------------------------------
// Name: Product
// Desc: Prodict of two vectors
//-----------------------------------------------------------------------------
MATRIX Product( MATRIX mMatA, MATRIX mMatB )
{
	MATRIX mResultantMat;

	mResultantMat.cx = MOnV( mMatA, mMatB.cx );
	mResultantMat.cy = MOnV( mMatA, mMatB.cy );
	mResultantMat.cz = MOnV( mMatA, mMatB.cz );

	return mResultantMat;
}

//-----------------------------------------------------------------------------
// Name: XRot
// Desc: Rotation about X
//
//			1	|	0			|	0
//			0	|	cos(theta)	|	sin(theta)
//			0	|	-sin(theta)	|	cos(theta)
//
//-----------------------------------------------------------------------------
MATRIX XRot( FTYPE fAngle )
{
	MATRIX mResultantMat;
	FTYPE fCos = cos( fAngle );
	FTYPE fSin = sin( fAngle );

	// X components
	mResultantMat.cx.x = 1.0;
	mResultantMat.cy.x = 0;
	mResultantMat.cz.x = 0;
	// Y components
	mResultantMat.cx.y = 0;
	mResultantMat.cy.y = fCos;
	mResultantMat.cz.y = fSin;
	// Z components
	mResultantMat.cx.z = 0;
	mResultantMat.cy.z = -fSin;
	mResultantMat.cz.z = fCos;

	return mResultantMat;
}

//-----------------------------------------------------------------------------
// Name: YRot
// Desc: Rotation about Y
//
//			cos(theta)	|	0	|	-sin(theta)
//			0			|	1	|	0
//			sin(theta)	|	0	|	cos(theta)
//
//-----------------------------------------------------------------------------
MATRIX YRot( FTYPE fAngle )
{
	MATRIX mResultantMat;
	FTYPE fCos = cos( fAngle );
	FTYPE fSin = sin( fAngle );	

	// X components
	mResultantMat.cx.x = fCos;
	mResultantMat.cy.x = 0;
	mResultantMat.cz.x = -fSin;
	// Y components
	mResultantMat.cx.y = 0.0;
	mResultantMat.cy.y = 1.0;
	mResultantMat.cz.y = 0.0;
	// Z components
	mResultantMat.cx.z = fSin;
	mResultantMat.cy.z = 0.0;
	mResultantMat.cz.z = fCos;

	return mResultantMat;
}

//-----------------------------------------------------------------------------
// Name: ZRot
// Desc: Rotation about Z
//
//			cos(theta)	|	sin(theta)	|	0
//			-sin(theta)	|	cos(theta)	|	0
//			0			|	0			|	1
//
//-----------------------------------------------------------------------------
MATRIX ZRot( FTYPE fAngle )
{
	MATRIX mResultantMat;
	FTYPE fCos = cos( fAngle );
	FTYPE fSin = sin( fAngle );

	// X components
	mResultantMat.cx.x = fCos;
	mResultantMat.cy.x = fSin;
	mResultantMat.cz.x = 0.0;
	// Y components
	mResultantMat.cx.y = -fSin;
	mResultantMat.cy.y = fCos;
	mResultantMat.cz.y = 0.0;
	// Z components
	mResultantMat.cx.z = 0.0;
	mResultantMat.cy.z = 0.0;
	mResultantMat.cz.z = 1.0;

	return mResultantMat;
}

//-----------------------------------------------------------------------------
// Name: InverseRotationOnly
// Desc: Quick little routine to enable the viewing vector to be transformed into
//		 object space for back facing polygon rejection.
//-----------------------------------------------------------------------------
MATRIX InverseRotationOnly( TRANSFORM obj )
{
  return ( Transpose( obj.rotate ) );
}

//-----------------------------------------------------------------------------
// Name: VectorSum
// Desc: Sum of two vectors
//-----------------------------------------------------------------------------
VECTOR VectorSum( VECTOR vVecA, VECTOR vVecB )
{
	VECTOR vResultantVec;

	vResultantVec.x = vVecA.x + vVecB.x;
	vResultantVec.y = vVecA.y + vVecB.y;
	vResultantVec.z = vVecA.z + vVecB.z;

	return vResultantVec;
}

//-----------------------------------------------------------------------------
// Name: VectorDiff
// Desc: Difference between two vectors
//-----------------------------------------------------------------------------
VECTOR VectorDiff( VECTOR vVecA, VECTOR vVecB )
{
	VECTOR vResultantVec;

	vResultantVec.x = vVecA.x - vVecB.x;
	vResultantVec.y = vVecA.y - vVecB.y;
	vResultantVec.z = vVecA.z - vVecB.z;

	return vResultantVec;
}

VECTOR	VectorDevide(VECTOR vVecA, double a)
{
	VECTOR vResultantVec;

	vResultantVec.x = vVecA.x / a;
	vResultantVec.y = vVecA.y / a;
	vResultantVec.z = vVecA.z / a;

	return vResultantVec;
}

//-----------------------------------------------------------------------------
// Name: Project
// Desc: Project a coordinate. Angle of view is assumed to be fViewAngle in both
//		 x and y (square screen). The z coord is transformed to 1/z in readiness
//		 for any possible z buffer usage. (Otherwise you can ignore it)
//-----------------------------------------------------------------------------
VECTOR Project( VECTOR vVec, FTYPE  fViewAngle )
{
	FTYPE fCoTAngle = (FTYPE)( 1.0 / tan( fViewAngle / 2.0 ) );
	VECTOR vResultantVec;

	vResultantVec.z = fCoTAngle / vVec.z;
	vResultantVec.x = vResultantVec.z * vVec.x;
	vResultantVec.y = vResultantVec.z * vVec.y;

	return vResultantVec;
}

//-----------------------------------------------------------------------------
// Name: Minus
// Desc: 
//-----------------------------------------------------------------------------
VECTOR Minus(VECTOR vVec)
{
	VECTOR vResultantVec;

	vResultantVec.x = -vVec.x;
	vResultantVec.y = -vVec.y;
	vResultantVec.z = -vVec.z;

	return vResultantVec;
}

//-----------------------------------------------------------------------------
// Name: DoTransform
// Desc: This routine implements a transformation consisting of a rotation and a displacement
//		 on the vector "vPoint". The rotation is performed before the displacement so that an
//		 object can have its orientation and position set independently of each other.
//		 If the displacement were done first it would be changed by the rotation.
//		 Data types and functions used in conjunction with this function:
//
//		 BuildTrans() function required to build up TRANSFORM data type
//
// Similar Functions: DoViewTransform
//-----------------------------------------------------------------------------
VECTOR DoTransform( VECTOR vPoint, TRANSFORM tAction )
{
	return ( VectorSum( MOnV( tAction.rotate, vPoint ), tAction.shift ) );
}

//-----------------------------------------------------------------------------
// Name: DoViewTransform
// Desc: As for the preceding routine but with the viewpoint's preferred ordering
//		 and with the sense of the displacement reversed so that we can work with the 
//		 position of the viewpoint within the world rather than the position of the 
//		 world within the view coords
//		 This routine implements a transformation consisting of a rotation and a displacement
//		 on the vector "vPoint". The rotation is performed after the displacement to simulate
//		 the effect of moving the observer rather than the object. 
//
//		 BuildViewTrans() function required to build up TRANSFORM data type
//
// Similar Functions: DoTransform
//-----------------------------------------------------------------------------
VECTOR DoViewTransform( VECTOR vPoint,TRANSFORM tAction )
{
	return ( MOnV( tAction.rotate, VectorSum( vPoint, Minus( tAction.shift ) ) ) );
}

//-----------------------------------------------------------------------------
// Name: DoViewTransform
// Desc: Constructs a TRANSFORM data set from a displacement (vDisp) and three
//		 angles of rotation. Note that the ordering of the angles is optimum
//		 for objects originally defined with their long axis in the x direction.
//
//		 Used in conjunction with DoTransform() function.
//
// Similar Functions: BuildViewTrans
//-----------------------------------------------------------------------------
TRANSFORM BuildTrans( FTYPE fXAngle, FTYPE fYAngle, FTYPE fZAngle, VECTOR vDisp )
{
	MATRIX		mXRot = XRot(fXAngle);
	MATRIX		mYRot = YRot(fYAngle);
	MATRIX		mZRot = ZRot(fZAngle);
	TRANSFORM	tResultantTrans;

	tResultantTrans.shift	= vDisp;
	tResultantTrans.rotate	= Product( mYRot, Product( mZRot, mXRot ) );

	return tResultantTrans;
}

//-----------------------------------------------------------------------------
// Name: DoViewTransform
// Desc: As for the preceding routine but with the viewpoint's preferred ordering of rotations
//
//		 Used in conjunction with DoViewTransform() function.
//
// Similar Functions: BuildViewTrans
//-----------------------------------------------------------------------------
TRANSFORM BuildViewTrans( FTYPE fXAngle, FTYPE fYAngle, FTYPE fZAngle, VECTOR vDisp )
{
	MATRIX		mXRot = XRot(fXAngle);
	MATRIX		mYRot = YRot(fYAngle);
	MATRIX		mZRot = ZRot(fZAngle);
	TRANSFORM	tResultantTrans;

	tResultantTrans.shift	= vDisp;
	tResultantTrans.rotate	= Product( mZRot, Product( mXRot, mYRot ) );

	return tResultantTrans;
}

//-----------------------------------------------------------------------------
// Name: CombineTrans
// Desc: Concatenates two transformations
//		 The effective transform applies tTranB to the object first - followed by tTranA
//-----------------------------------------------------------------------------
TRANSFORM CombineTrans( TRANSFORM tTranA, TRANSFORM tTranB )
{
	TRANSFORM tResultantTrans;

	tResultantTrans.shift	= VectorSum( tTranA.shift, MOnV( tTranA.rotate, tTranB.shift ) );
	tResultantTrans.rotate	= Product( tTranA.rotate, tTranB.rotate );

	return tResultantTrans; 
}

//-----------------------------------------------------------------------------
// Name: CombineViewTrans
// Desc: The effective transform applies tTranB to the object first - followed by tTranA.
//		 In this case tTranA is a viewpoint transformation and hence has its shift 
//		 and rotate in a different order and has the sense o the viewing shift 
//		 reversed - see DoViewTrans (above)
//-----------------------------------------------------------------------------
TRANSFORM CombineViewTrans( TRANSFORM tTranA, TRANSFORM tTranB )
{

	TRANSFORM tResultantTrans;

	tResultantTrans.shift	= MOnV( tTranA.rotate, VectorSum( Minus( tTranA.shift ), tTranB.shift ) );
	tResultantTrans.rotate	= Product( tTranA.rotate, tTranB.rotate );

	return tResultantTrans; 
}

//-----------------------------------------------------------------------------
// Name: Max
// Desc: 
//
//
//-----------------------------------------------------------------------------
double Max(double a, double b)
{
	return (a > b) ? a : b;
}


double CalLambert(VECTOR light, VECTOR pos, VECTOR normal)
{
	return Max(Dot(normal, Normalise(VectorSum(light, pos))), 0);
}