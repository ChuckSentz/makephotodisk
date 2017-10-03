/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.

Revision History:


   Rev 1.0   13 Jun 2007 12:42:08   CSentz(desktop)
Initial revision.


***************************************************************************************/
#pragma once

#include "rational.h"
#include "afxtempl.h"

class EXIFHeader ;


class EXIFDigest
{
protected:
	CString		m_filename ;
	CString		m_title ;
	CString		m_description ;
	CString		m_camera ;
	SYSTEMTIME	m_create_time ;
	
	Rational	m_aperture ;
	Rational	m_focal_length ;
	Rational	m_exposure ;
	
	CString		m_lens ;			// only filled in if shooting RAW
	int			m_iso ;				// only filled in if shooting RAW
	bool		m_flash_used ;
	int			m_width ;
	int			m_height ;

	void ParseEXIF( ) ;	

	void GetCameraName( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections ) ;
	void GetCreateDateTime( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections ) ;
	void GetTitle( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections ) ;
	void GetDescription( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections ) ;
	void GetAperture( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections ) ;
	void GetFocalLength( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections ) ;
	void GetExposure( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections ) ;
	void GetLens( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections ) ;
	void GetISO( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections ) ;
	void GetFlashUsed( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections ) ;
	void GetWidth( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections ) ;
	void GetHeight( CTypedPtrArray< CPtrArray, EXIFHeader *> & sections ) ;

public:
	EXIFDigest( const TCHAR * jpeg_filename );
	~EXIFDigest(void);
	
	inline const TCHAR * GetFilename( ) const
		{
			return m_filename ;
		} 
		
	inline const TCHAR * GetTitle( ) const
		{
			if ( 0 < m_title.GetLength( ) )
				return m_title ;
			else
				return NULL ;
		} 
		
	inline const TCHAR * GetDescription( ) const
		{
			if ( 0 < m_description.GetLength( ) )
				return m_description ;
			else
				return NULL ;
		} 

	inline const TCHAR * GetLens( ) const
		{
			if ( 0 < m_lens.GetLength( ) )
				return m_lens ;
			else
				return NULL ;
		}
		
	inline const TCHAR * GetCamera( ) const
		{
			if ( 0 < m_camera.GetLength( ) )
				return m_camera ;
			else
				return NULL ;
		} 
		
	inline SYSTEMTIME GetCreateTime( ) const
		{
			return m_create_time ;
		} 

	inline bool GetValidCreateTime( ) const
		{
			return m_create_time.wYear != 0xcccc && 
					m_create_time.wMonth != 0xcccc && 
					m_create_time.wDay != 0xcccc && 
					m_create_time.wMilliseconds != 0xcccc && 
					m_create_time.wHour != 0xcccc && 
					m_create_time.wMinute != 0xcccc && 
					m_create_time.wSecond != 0xcccc &&
					m_create_time.wDayOfWeek != 0xcccc ;
		} ;

		
	inline Rational GetExposureTime( ) const
		{
			return m_exposure ;
		}

	inline Rational GetFocalLength( ) const
		{
			return m_focal_length ;
		}

	inline Rational GetAperture( ) const
		{
			return m_aperture ;
		}

	inline bool GetFlash( ) const
		{
			return m_flash_used ; 
		}
		
	inline int GetWidth( ) const
		{
			return m_width ;
		}
		
	int GetHeight( ) const
		{
			return m_height ;
		}
		
	int	GetISO( ) const
		{
			return m_iso ;
		}
};
