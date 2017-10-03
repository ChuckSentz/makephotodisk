/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.

Revision History:


   Rev 1.0   13 Jun 2007 12:42:08   CSentz(desktop)
Initial revision.


***************************************************************************************/
#pragma once

class EXIFDirectory ;

class EXIFDirectoryItem
{
	uint16	m_tag ;
	uint16	m_type ;
	uint32	m_count ;
	bool	m_reverse_bytes ;
	uchar *	m_data ;
	EXIFDirectory *	m_dir ;
	

public:
	EXIFDirectoryItem( ) ;
	EXIFDirectoryItem( EXIFDirectoryItem & source ) ;
	EXIFDirectoryItem( uint16 tag, uint16 type, uint32 count, bool reverse_bytes, uchar * data, EXIFDirectory * dir ) ;
	~EXIFDirectoryItem( void ) ;
	
	EXIFDirectoryItem & operator=( EXIFDirectoryItem & original ) ;
	
	inline void SetDir( EXIFDirectory * dir ) 
		{
			m_dir = dir ;
		} ;
	
	inline uint16 GetTag( ) const 
		{
			return m_tag ;
		} ;
		
	inline uint16 GetType( ) const
		{
			return m_type ;
		} ;
		
	inline uint32 GetCount( ) const
		{
			return m_count ;
		} ;
		
	inline const uchar * GetData( ) const
		{
			return m_data ;
		} ;

	inline bool GetReverseBytes( ) const
		{
			return m_reverse_bytes ;
		} ;
		
	inline const EXIFDirectory * GetDir( ) const
		{
			return m_dir ;
		} ;
};


