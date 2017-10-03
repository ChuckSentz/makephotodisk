/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.

Revision History:


   PORTED FROM MakePhotoDisk2

   Rev 1.0   13 Jun 2007 12:42:02   CSentz(desktop)
Initial revision.


***************************************************************************************/
#include "StdAfx.h"
#include "exifdirectoryitem.h"
#include "EXIFHeader.h"




EXIFDirectoryItem::EXIFDirectoryItem( uint16 tag, uint16 type, uint32 count, bool reverse_bytes, uchar * data, EXIFDirectory * dir )
{
	m_tag = tag ;
	m_type = type ;
	m_count = count ;
	m_reverse_bytes = reverse_bytes ;
	m_data = data ;
	m_dir = dir ;
}




EXIFDirectoryItem::~EXIFDirectoryItem(void)
{
	delete m_dir ;
}




EXIFDirectoryItem::EXIFDirectoryItem( EXIFDirectoryItem & source )
{
	m_tag = source.m_tag ;
	m_type = source.m_type ;
	m_count = source.m_count ;
	m_reverse_bytes = source.m_reverse_bytes ;
	m_data = source.m_data ;
	
	if ( source.m_dir )
		m_dir = new EXIFDirectory( * source.m_dir ) ;
}




EXIFDirectoryItem::EXIFDirectoryItem( )
{
	m_tag = 0 ;
	m_type = 0 ;
	m_count = 0 ;
	m_data = NULL ;
	m_reverse_bytes = false ;
	m_dir = NULL ;
}




EXIFDirectoryItem & EXIFDirectoryItem::operator=( EXIFDirectoryItem & original )
{
	if ( this != &original )
	{
		m_tag = original.m_tag ;
		m_type = original.m_type ;
		m_count = original.m_count ;
		m_data = original.m_data ;
		m_reverse_bytes = original.m_reverse_bytes ;
		
		if ( original.m_dir )
			m_dir = new EXIFDirectory( * original.m_dir ) ;
	}
	
	return *this ;
}


