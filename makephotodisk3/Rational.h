/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.

Revision History:


   Rev 1.0   13 Jun 2007 12:42:08   CSentz(desktop)
Initial revision.


***************************************************************************************/
#pragma once

class Rational
{
	int32 Rational::gcd( int32 v1, int32 v2 ) ;
	void Normalize( int32 & num, int32 & den ) ;

	int32	m_num ;
	int32	m_den ;

public:
	Rational( int32 num, int32 den ) ;
	Rational(void);
	
	inline int32 GetNumerator( ) const 
		{
			return m_num ;
		} ;
	
	inline int32 GetDenominator( ) const
		{
			return m_den ;
		} ;
};
