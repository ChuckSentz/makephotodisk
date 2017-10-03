/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.

Revision History:


   PORTED FROM MakePhotoDisk2

   Rev 1.0   13 Jun 2007 12:42:02   CSentz(desktop)
Initial revision.


***************************************************************************************/
#include "StdAfx.h"


#include "jpegsection.h"




JPegSection::JPegSection(void)
{
	m_type = 0 ;
	m_length = 0 ;
	m_padding = 0 ;
	m_data = NULL ;
}




JPegSection::~JPegSection(void)
{
	delete [] m_data ;
}



JPegSection & JPegSection::operator=( JPegSection & source )
{
	if ( this != &source ) 
	{
		m_type = source.m_type ;
		m_length = source.m_length ;
		m_padding = source.m_padding ;
		m_data = new uchar[ m_length ] ;
		memcpy( m_data, source.m_data, m_length ) ;
	}
		
	return *this ;
}



JPegSection::JPegSection( JPegSection & source )
{
	*this = source ;
}










/*

From EXIF 2.2 - 

4.7 JPEG Marker Segments Used in Exif
	In addition to the compressed data (Interoperability coded data), a compressed file contains one each of
	the marker segments APP1, DQT, DHT, SOF and SOS. The compressed data begins with an SOI and
	ends with an EOI marker. A restart marker (DRI, RSTm) may be inserted optionally. Another option is to
	have two or more APP2 marker segments. APPn other than APP1 and APP2 or COM segments are not
	used by Exif. However Exif readers should be designed skip over unknown APPn and COM.
	APP1 shall be recorded immediately after SOI, and if there is an APP2 it follows after APP1. DQT, DHT,
	DRI and SOF come after APP2 and before SOS, in any order.

The marker segments used in Exif are listed in Table 19.

	Table 19 Marker Segments

		Marker Name								Marker Code		Description
		SOI Start of Image						FFD8.H			Start of compressed data
		APP1 Application Segment 1				FFE1.H			Exif attribute information
		APP2 Application Segment 2				FFE2.H			Exif extended data
		DQT Define Quantization Table			FFDB.H			Quantization table definition
		DHT Define Huffman Table				FFC4.H			Huffman table definition
		DRI Define Restart Interoperability		FFDD.H			Restart Interoperability definition
		SOF Start of Frame						FFC0.H			Parameter data relating to frame
		SOS Start of Scan						FFDA.H			Parameters relating to components
		EOI End of Image						FFD9.H			End of compressed data

The data structures of markers defined in JPEG Baseline DCT and used in Exif compressed files, as well
as the APP1 and APP2 data structures defined specifically for this standard, are explained below.

*/
bool JPegSection::Read( CFile & src_file ) 
{
	// Accept 0-8 'FF's before the section start. Hitting EOF or too much padding means it's an invalid section.
	do
	{
		if ( !src_file.Read( &m_type, sizeof( m_type ) ) )
			return false ;
	}
	while ( m_type == 0xFF && m_padding++ < 8 ) ;

	// need at least one FF, but not more than 8
	if ( 0 == m_padding || 8 < m_padding )
		return false ;
	
	// all the section types we deal with here have a two byte length field following the section tag. 
	// (SOI and EOI don't have length fields, but we don't handle them in this function).
	src_file.Read( &m_length, sizeof( m_length ) ) ;
    _swab( (char *) &m_length, (char *) &m_length, sizeof( int16 ) ) ;

	// Just anohter quick sanity check - the length includes the 2 bytes needed to store the length, so
	// 2 is a bare minimum
    if ( m_length < 2 )
		return false ;

	m_data = new uchar[ m_length ] ;
	memcpy( m_data, &m_length, sizeof( uint16 ) ) ;
	_swab( (char *) m_data, (char *) m_data, sizeof( int16 ) ) ;		// swap the length bytes at the start of m_data back the way they are on disk 

	if ( m_length - 2 != src_file.Read( m_data + 2, m_length - 2 ) )
		return false ;


    return true ;
}



bool JPegSection::Write( CFile & dest_file )
{
	int		i ;
	uchar	pad_char = 0xFF ;
	bool	isOK ;
	
	try
	{
		for ( i = 0 ; i < m_padding ; i++ )	
			dest_file.Write( &pad_char, 1 ) ;
		dest_file.Write( &m_type, sizeof( m_type ) ) ;
		dest_file.Write( m_data, m_length ) ;
		
		isOK = true ;
	}
	catch ( ... )
	{
		isOK = false ;
	}

	return isOK ;
}

