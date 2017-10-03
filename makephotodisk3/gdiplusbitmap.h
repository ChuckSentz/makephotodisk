/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.



CGdiPlusBitmap wrapper class for loading PNG's from compiled resources. 

***************************************************************************************/
#pragma once


/*
*
*	Helper classes from solution on codeproject.com which let use load a graphic 
*	(whether bmp, jpeg, or png doesn't matter) that's compiled as a resource. 
*
*	original source: 
*	http://www.codeproject.com/Articles/3537/Loading-JPG-PNG-resources-using-GDI
*
*/

class CGdiPlusBitmap
{
protected:
	Gdiplus::Bitmap* m_bitmap ;

public:
	CGdiPlusBitmap()
	{ 
		m_bitmap = NULL ; 
	} ;

	CGdiPlusBitmap( const TCHAR * filename )			
	{ 
		m_bitmap = NULL; 
		Load( filename ) ; 
	} ;

	virtual ~CGdiPlusBitmap( )					
	{ 
		Empty( ) ; 
	} ;

	void Empty( )
	{ 
		delete m_bitmap ; 
		m_bitmap = NULL ; 
	} ;

	bool Load( const TCHAR * filename )
	{
		Empty( ) ;
		m_bitmap = Gdiplus::Bitmap::FromFile( filename ) ;
		return m_bitmap->GetLastStatus( ) == Gdiplus::Ok ;
	} ;

	operator Gdiplus::Bitmap * ( ) const
	{ 
		return m_bitmap ;
	} ;
} ;




class CGdiPlusBitmapResource : public CGdiPlusBitmap
{
protected:
	HGLOBAL m_hBuffer ;

public:
	CGdiPlusBitmapResource( )
	{ 
		m_hBuffer = NULL ; 
	} ;

	CGdiPlusBitmapResource( const TCHAR * resource_name, const TCHAR * resource_type = RT_RCDATA, HMODULE hInst = NULL )
	{ 
		m_hBuffer = NULL ; 
		Load( resource_name, resource_type, hInst ) ; 
	} ;

	CGdiPlusBitmapResource( UINT resource_id, const TCHAR * resource_type = RT_RCDATA, HMODULE hInst = NULL )
	{ 
		m_hBuffer = NULL ; 
		Load( resource_id, resource_type, hInst ) ; 
	} ;

	CGdiPlusBitmapResource( UINT resource_id, UINT resource_type, HMODULE hInst = NULL )
	{ 
		m_hBuffer = NULL ; 
		Load( resource_id, resource_type, hInst ) ; 
	} ;

	virtual ~CGdiPlusBitmapResource( )			
	{ 
		Empty( ) ; 
	} ;

	void Empty( ) ;

	bool Load( const TCHAR * resource_name, const TCHAR * resource_type = RT_RCDATA, HMODULE hInst = NULL) ;
	bool Load( UINT resource_id, const TCHAR * resource_type = RT_RCDATA, HMODULE hInst = NULL )
	{ 
		return Load( MAKEINTRESOURCE( resource_id ), resource_type, hInst ) ;
	} ;

	bool Load( UINT resource_id, UINT resource_type, HMODULE hInst = NULL )
	{ 
		return Load( MAKEINTRESOURCE( resource_id ), MAKEINTRESOURCE( resource_type ), hInst ) ; 
	} ;
};





inline void CGdiPlusBitmapResource::Empty( )
{
	CGdiPlusBitmap::Empty( ) ;

	if ( m_hBuffer )
	{
		::GlobalUnlock( m_hBuffer ) ;
		::GlobalFree( m_hBuffer ) ;
		m_hBuffer = NULL ;
	} 
}


inline bool CGdiPlusBitmapResource::Load( const TCHAR * resource_name, const TCHAR * resource_type, HMODULE hInst ) 
{
	Empty( ) ;

	HRSRC hResource = ::FindResource( hInst, resource_name, resource_type) ;

	if ( !hResource )
		return false ;
	
	DWORD imageSize = ::SizeofResource( hInst, hResource ) ;

	if ( !imageSize )
		return false ;

	const void* pResourceData = ::LockResource( ::LoadResource( hInst, hResource ) ) ;
	if ( !pResourceData )
		return false ;

	m_hBuffer  = ::GlobalAlloc( GMEM_MOVEABLE, imageSize ) ;

	if ( m_hBuffer )
	{
		void * pBuffer = ::GlobalLock( m_hBuffer ) ;

		if ( pBuffer )
		{
			CopyMemory( pBuffer, pResourceData, imageSize ) ;

			IStream* pStream = NULL ;

			if ( ::CreateStreamOnHGlobal( m_hBuffer, FALSE, &pStream ) == S_OK )
			{
				m_bitmap = Gdiplus::Bitmap::FromStream( pStream ) ;
				pStream->Release( ) ;

				if ( m_bitmap )
				{
					if ( m_bitmap->GetLastStatus( ) == Gdiplus::Ok )
						return true ;

					delete m_bitmap ;
					m_bitmap = NULL ;
				}
			}
			::GlobalUnlock( m_hBuffer ) ;
		}
		::GlobalFree( m_hBuffer ) ;
		m_hBuffer = NULL ;
	}

	return false ;
}
