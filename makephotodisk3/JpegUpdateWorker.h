/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


   declaration of JpegUpdateWorker class. 

   This class is intended to be embedded as a member variable in a dialog box or 
   window which displays the status of updating the jpeg's for a disk. It creates
   its own worker thread which automatically begins processing the jpegs 
   for the document. 

   The consumer must not alter the document object or the queue of photo image objects
   while this class is working. IsFinished returns true when the worker thread has 
   finished. If you need to terminate processing before then, call CancelProcessing 
   first, then check IsFinished( ) to ensure it's done. 

   Destructing the JpegUpdateWorker object before then will call KillThread on the 
   worker thread, and clean up, more or less, but it is not a clean way of wrapping
   things up.... 

***************************************************************************************/
#pragma once

#include "MakePhotoDisk3Doc.h"

class JpegUpdateWorker
{
	CRITICAL_SECTION		m_critical_section ;		// coordinate access between UI thread & worker thread 
	PhotoImage *			m_cur_image ;				// current PhotoImage we're working on 
	int						m_processed_jpegs ;			// count of jpegs output so far 
	int						m_image_count ;				// total count of jpegs to output 
	CMakePhotoDisk3Doc *	m_doc ;						// the document, of course 
	bool					m_finished ;				// finished processing 
	bool					m_cancelled ;				// tells worker thread that user wants to cancel 
	HANDLE					m_worker_thread ;			// thread handle for the worker thread 
	bool					m_new_source ;				// tells UI we have a new source file 

	static DWORD JpegWorkerThread( JpegUpdateWorker * _this ) ;
	DWORD RunThread( ) ;

	void NewSourceFile( PhotoImage * cur_image ) ;
	void NewJpegDone( ) ;


public:
	JpegUpdateWorker( CMakePhotoDisk3Doc * doc, int image_count );
	~JpegUpdateWorker();

	bool GetStatus( PhotoImage * & cur_image, int & processed_count ) ;			// returned true ==> new source file 
	void CancelProcessing( ) ;
	bool IsFinished( ) ;
};

