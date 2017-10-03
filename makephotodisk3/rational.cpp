/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.

Revision History:


PORTED FROM MakePhotoDisk2


   Rev 1.0   13 Jun 2007 12:42:02   CSentz(desktop)
Initial revision.


***************************************************************************************/

#include "StdAfx.h"
#include ".\rational.h"



Rational::Rational(void)
{
	m_num = 0 ;
	m_den = 1 ;
}






Rational::Rational( int32 num, int32 den )
{
	if ( 0 == den )
	{
		den = 1 ;
		num = 0 ;
	}
	else	
	{
		if ( num < 0 && den < 0 )
		{
			num = -num ;
			den = -den ;
		}
	
		Normalize( num, den ) ;
	}
	
	m_num = num ; 
	m_den = den ;
}



int32 Rational::gcd( int32 v1, int32 v2 )
{
	// gcd algorithm requires remainders to be always positive, while computers return negative remainders. So caller
	// must keep track of signage and pass us positive numerator and denominator
	ASSERT( v1 >= 0 && v2 >= 0 ) ;

	if ( v2 < v1 )
		return gcd( v2, v1 ) ;
	
	if ( 0 == v1 )
		return 1 ;
	
	uint32	r ;
	
	if ( 0 == ( r = v2 % v1 ) )
		return v1 ;
	else if ( 1 == r )
		return 1 ;
	else
		return gcd( r, v1 ) ;
}



// exposures are often expressed as 10/600'ths, etc. 
void Rational::Normalize( int32 & num, int32 & den ) 
{
	int32	d ;
	bool	nneg ;
	bool	dneg ;

	if ( nneg = ( num < 0 ) )
		num = -num ;

	if ( dneg = ( den < 0 ) )
		den = -den ;

	if ( 1 < ( d = gcd( num, den ) ) )
	{
		num /= d ;
		den /= d ;
	}
	
	if ( nneg )
		num = -num ;
		
	if ( dneg )
		den = -den ;
}
