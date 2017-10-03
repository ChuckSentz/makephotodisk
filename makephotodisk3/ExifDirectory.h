/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.

Revision History:


   Rev 1.0   13 Jun 2007 12:42:08   CSentz(desktop)
Initial revision.


***************************************************************************************/
#pragma once

#include "ExifDirectoryItemArray.h"


class EXIFDirectory
{
protected:
	EXIFDirectoryItemArray								m_entries ;
	EXIFDirectory *										m_next ;

	// these following are mostly here for debugging purposes and should be taken out eventually, unless they turn out to have a real use
	uint16	m_entry_count ;
	uchar *	m_actual_dir ;

public:
	EXIFDirectory( uint16 num_entries, uint32 dir_addr, uchar * header_data ) ;
	EXIFDirectory( EXIFDirectory & original ) ;
	virtual ~EXIFDirectory(void);
	
	inline int GetCount( ) const
		{
			return (int) m_entries.GetCount( ) ;
		} ;
		
	inline const EXIFDirectoryItem & GetItemAt( int i ) const
		{
			ASSERT( i < m_entries.GetCount( ) ) ;
			return m_entries.GetAt( i ) ;
		} ;
	
	inline const EXIFDirectory * GetNext( ) const
		{
			return m_next ;
		} ;
	
	inline void SetNext( EXIFDirectory * next )
		{
			m_next = next ;
		} ;
		
	inline void AddItem( EXIFDirectoryItem & item )
		{
			m_entries.AddItem( item ) ;
		} ;
} ;
