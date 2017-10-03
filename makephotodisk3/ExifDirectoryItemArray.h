/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.

Revision History:


   Rev 1.0   13 Jun 2007 12:42:08   CSentz(desktop)
Initial revision.


***************************************************************************************/
#pragma once

#include "EXIFDirectoryItem.h"
#include "afxtempl.h"


class EXIFDirectoryItemArray
{
	enum
	{
		SUBARRAY_SIZE	=	100 
	} ;

	CTypedPtrArray< CPtrArray, EXIFDirectoryItem * >	m_subarray ;
	int	m_num_entries ;

public:
	EXIFDirectoryItemArray(void);
	EXIFDirectoryItemArray( EXIFDirectoryItemArray & original ) ;
	~EXIFDirectoryItemArray(void);
	
	EXIFDirectoryItemArray& operator=( EXIFDirectoryItemArray & source ) ;
	
	void AddItem( EXIFDirectoryItem & item ) ;

	EXIFDirectoryItem & GetAt( int i ) const 
	{
		ASSERT( i < m_num_entries ) ;
		
		int	array_no ;
		int	array_elem ;
		
		array_no = i / SUBARRAY_SIZE ;
		array_elem = i % SUBARRAY_SIZE ;
		
		return m_subarray[ array_no ][ array_elem ] ;
	} ;
	
	int GetCount( ) const 
	{
		return m_num_entries ;
	} ;
};
