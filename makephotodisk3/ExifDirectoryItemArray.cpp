/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.

Revision History:


   PORTED FROM MakePhotoDisk2

   Rev 1.0   13 Jun 2007 12:42:02   CSentz(desktop)
Initial revision.


***************************************************************************************/
#include "StdAfx.h"
#include "exifdirectoryitemarray.h"




EXIFDirectoryItemArray::EXIFDirectoryItemArray(void)
{
	m_num_entries = 0 ;
}




EXIFDirectoryItemArray::~EXIFDirectoryItemArray(void)
{
	int	i ;
	
	for ( i = 0 ; i < m_subarray.GetCount( ) ; i++ )
		delete [] m_subarray.GetAt( i ) ;
	m_subarray.RemoveAll( ) ;
}





EXIFDirectoryItemArray::EXIFDirectoryItemArray( EXIFDirectoryItemArray & original )
{
	int i ;
	
	for ( i = 0 ; i < original.GetCount( ) ; i++ )
		AddItem( original.GetAt( i ) ) ;
}




EXIFDirectoryItemArray& EXIFDirectoryItemArray::operator=( EXIFDirectoryItemArray & source )
{
	if ( this != &source )
	{
		int	i ;
		
		for ( i = 0 ; i < m_subarray.GetCount( ) ; i++ )
			delete [] m_subarray.GetAt( i ) ;
		
		for ( i = 0 ; i < source.GetCount( ) ; i++ )
			AddItem( source.GetAt( i ) ) ;		
	}
	return *this ;
}




void EXIFDirectoryItemArray::AddItem( EXIFDirectoryItem & item )
{
	if ( 0 == m_num_entries % SUBARRAY_SIZE )
	{
		m_subarray.Add( new EXIFDirectoryItem[ SUBARRAY_SIZE ] ) ;
	}
	
	m_subarray[ m_num_entries / SUBARRAY_SIZE ][ m_num_entries % SUBARRAY_SIZE ] = item ;
	m_num_entries++ ;
}
