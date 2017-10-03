/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.

Revision History:


   PORTED FROM MakePhotoDisk2

   Rev 1.0   13 Jun 2007 12:42:02   CSentz(desktop)
Initial revision.


***************************************************************************************/
#include "StdAfx.h"
#include ".\exifdirectory.h"

EXIFDirectory::EXIFDirectory( uint16 num_entries, uint32 dir_addr, uchar * header_data )
{
	m_entry_count = num_entries ;
	m_actual_dir = header_data + dir_addr ;
	m_next = NULL ;
}

EXIFDirectory::~EXIFDirectory(void)
{
	delete m_next ;
}


EXIFDirectory::EXIFDirectory( EXIFDirectory & original )
{
	m_entry_count = original.m_entry_count ;
	m_actual_dir = original.m_actual_dir ;
	m_entries = original.m_entries ;
	
	if ( original.m_next )
		m_next = new EXIFDirectory( *m_next ) ;
	else
		m_next = NULL ;
}
