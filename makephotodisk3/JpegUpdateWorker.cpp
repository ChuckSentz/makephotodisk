/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


   Implementation of JpegUpdateWorker class. 

   This class is intended to be embedded as a member variable in a dialog box or 
   window which displays the status of updating the jpeg's for a disk. It creates
   its own worker thread which 

   The consumer must not alter the document object or the queue of photo image objects.

***************************************************************************************/

#include "stdafx.h"

#include "JpegUpdateWorker.h"
#include "CLRPhotoshopWrapper\CLRPhotoshopWrapper.h"


/******************************************************************************
JpegUpdateWorker::JpegWorkerThread

	Private static member function 

	This function is only invoked in the constructor, and is passed 
	passed as the thread routine to ::CreateThread. It just turns around and 
	invokes the non-static RunThread method on the JpegUpdateWorker
	this pointer passed in. 

******************************************************************************/
DWORD JpegUpdateWorker::JpegWorkerThread( 
	JpegUpdateWorker * _this				// I - the 'this' pointer passed as arg to CreateThread 
)
{
	return _this->RunThread( ) ;
}


/******************************************************************************
JpegUpdateWorker::JpegUpdateWorker

	Constructor. After setting up the critical section and initializing member
	variables, we create a thread with thread routine JpegUpdateWorker and 
	thread argument 'this' pointer. 

******************************************************************************/
JpegUpdateWorker::JpegUpdateWorker(	
	CMakePhotoDisk3Doc * doc,				// I - the document object 
	int image_count							// I - the total count of images we're going to work on 
)
{
	::InitializeCriticalSection( &m_critical_section ) ;
	m_doc = doc ;
	m_cur_image = NULL ;
	m_image_count = image_count ;
	m_processed_jpegs = 0 ;
	m_finished = false ;
	m_cancelled = false ;
	m_new_source = false ;

	m_worker_thread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) JpegWorkerThread, this, 0, NULL ) ;
}



/******************************************************************************
JpegUpdateWorker::~JpegUpdateWorker

	Make sure the worker thread is terminated if it's not done, then cleanup
	the handle and the critical section. 

******************************************************************************/
JpegUpdateWorker::~JpegUpdateWorker()
{
	EnterCriticalSection( &m_critical_section ) ;
	{
		if ( m_worker_thread != NULL ) 
		{
			TerminateThread( m_worker_thread, -1 ) ;
			CloseHandle( m_worker_thread ) ;
		}
	}
	LeaveCriticalSection( &m_critical_section ) ;

	DeleteCriticalSection( &m_critical_section ) ;
}




/******************************************************************************
JpegUpdateWorker::RunThread

	Called from static JpegWorkerThread (which is passed a this pointer). This
	amounts to a thread routine which is able to use member variables. 

******************************************************************************/
DWORD JpegUpdateWorker::RunThread( )
{
	POSITION			pos ;
	PhotoshopWrapper	ps_wrapper ;
	PhotoImage *		image ;

	ps_wrapper.SetDiskRoot( m_doc->GetProjectPath( ) ) ;

	for ( image = m_doc->GetFirstPhoto( pos ) ; !m_cancelled && image != NULL ; image = m_doc->GetNextPhoto( pos ) )
	{
		if ( image->JpegRefreshNeeded( m_doc ) )
		{
			CString		dest_filename ;

			dest_filename.Format( _T( "%d.jpg" ), image->GetNumberOnDisk( ) ) ;

			NewSourceFile( image ) ;

			ps_wrapper.LoadAndPrepareSourceFile( image->GetSourceFilePath( ), dest_filename ) ;
			
			if ( image->ThumbnailRefreshNeeded( m_doc ) )
			{
				ps_wrapper.OutputJpeg( Thumbnail ) ;
				NewJpegDone( ) ;
			}

			if ( image->PresentationRefreshNeeded( m_doc ) ) 
			{
				int			width ;
				int			height ;

				ps_wrapper.OutputJpeg( Presentation, &width, &height ) ;

				image->SetPresentationWidth( width ) ;
				image->SetPresentationHeight( height ) ;

				NewJpegDone( ) ;
			}

			if ( image->MiniPresentationRefreshNeeded( m_doc ) ) 
			{
				int			width ;
				int			height ;

				ps_wrapper.OutputJpeg( MiniPresentation, &width, &height ) ;

				image->SetMiniPresentationWidth( width ) ;
				image->SetMiniPresentationHeight( height ) ;

				NewJpegDone( ) ;
			}

			if ( image->FullsizeRefreshNeeded( m_doc ) ) 
			{
				ps_wrapper.OutputJpeg( FullSize ) ;
				NewJpegDone( ) ;
			}

			if ( image->FacebookRefreshNeeded( m_doc ) ) 
			{
				ps_wrapper.OutputJpeg( Facebook ) ;
				NewJpegDone( ) ;
			}

			if ( image->CompressedRefreshNeeded( m_doc ) ) 
			{
				ps_wrapper.OutputJpeg( Compressed ) ;
				NewJpegDone( ) ;
			}

			ps_wrapper.UnloadSourceFile( ) ;
		}
	}

	m_finished = true ;
	ps_wrapper.QuitApp( ) ;

	// normal return 
	return 0 ;
}




/******************************************************************************
JpegUpdateWorker::NewSourceFile

	Private function, only called from the worker thread to update the current 
	source file name. 

******************************************************************************/
void JpegUpdateWorker::NewSourceFile( 
	PhotoImage * image					// I - the new PhotoImage we're working on 
)
{
	EnterCriticalSection( &m_critical_section ) ;
	{
		m_cur_image = image ;
		m_new_source = true ;
	}
	LeaveCriticalSection( &m_critical_section ) ;
}

/******************************************************************************
JpegUpdateWorker::NewJpegDone

	Private function, only called from the worker thread to increment the 
	count of jpegs processed. 

******************************************************************************/
void JpegUpdateWorker::NewJpegDone( )
{
	EnterCriticalSection( &m_critical_section ) ;
	{
		m_processed_jpegs++ ;
	}
	LeaveCriticalSection( &m_critical_section ) ;
}


/******************************************************************************
JpegUpdateWorker::GetStatus

	Public function. Called from consumer class's timer processing to get the
	status 

	returns true iff. the image source has changed since the last time it
	was called 

	Caller needs to account for the possibility, especially after the first
	call, that cur_image may be NULL on return, but will never be NULL after 
	this function has ever returned true

******************************************************************************/
bool JpegUpdateWorker::GetStatus( 
	PhotoImage * & cur_image,			// O - gets full source file path 
	int & processed_count				// O - gets procesed jpeg count 
)
{
	bool	new_source = false ;

	EnterCriticalSection( &m_critical_section ) ;
	{
		new_source = m_new_source ;
		m_new_source = false ;

		cur_image = m_cur_image ;
		processed_count = m_processed_jpegs ;
	}
	LeaveCriticalSection( &m_critical_section ) ;

	return new_source ;
}


/******************************************************************************
JpegUpdateWorker::CancelProcessing

	Public function called from consumer-class iff user has clicked the cancel
	button 

******************************************************************************/
void JpegUpdateWorker::CancelProcessing( ) 
{
	EnterCriticalSection( &m_critical_section ) ;
	{
		m_cancelled = true ;
	}
	LeaveCriticalSection( &m_critical_section ) ;
}



/******************************************************************************
JpegUpdateWorker::IsFinished

	Public function consumer-class can call to ensure the worker thread has
	finished up. 

	It makes sense for the consumer class to call this after either:

		a) the user has clicked a Cancel button and CancelProcessing() has 
		been called 

		or

		b) A call to GetStatus has returned with processed_count == jpeg_count 

******************************************************************************/
bool JpegUpdateWorker::IsFinished( ) 
{
	return m_finished ;
}
