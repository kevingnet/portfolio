/////////////////////////////////////////////////////////////////////////////
// CCommunicate.cpp : Implementation of the CCommunicate class.
// Handles all serial i/o routines.
//
// The class is largley a generic read/write comminication class with
// the exception of throwing results back to the parent class from
// the write thread and timeout monitoring in the read thread.
//
// A large amount of this code was derived from the "COM32" examples
// found in the MSDN.

#include "stdafx.h"
#include <string.h>
#include "comm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Default size of the Input Buffer used by this code.
#define INPUTBUFFERSIZE 2048


////////////////////////////////////////////////////
// Used to write comm data to a disk file in debug mode.
// #define _DEBUGLOGFILE 1



////////////////////////////////////////////////////
// Constructor for the communication class. Init.members
// and bring the communications system on-line

CCommunicate::CCommunicate()
{
	m_Initialised		= FALSE;
	m_ReceiveTimeout	= 500;
}

CCommunicate::CCommunicate( CommSettingsType* pComm )
{
	Initialize( pComm );
}


BOOL CCommunicate::Initialize( CommSettingsType* pComm )
{
	HANDLE hNewComm;

	#ifdef _DEBUGLOGFILE
		m_DebugStream  = fopen( "debug.log", "w" );
	#endif

	// Init. members.
	m_ErrCode = IDS_SUCCESS;
	m_BaudRate = pComm->Baud;
	m_ByteSize = (BYTE)pComm->DataBits;  
	m_Parity   = (BYTE)pComm->Parity;    
	m_StopBits = (BYTE)pComm->StopBits; 
	m_FlowCtrl = pComm->FlowCtrl;
	m_XonXoff  = 0;

	// Init. globals
	m_ProcessControl.hCommFile = NULL;
	m_ProcessControl.hCloseEvent = NULL;

   // Open COMM device
   if ( ( hNewComm =
      CreateFile( pComm->Port, GENERIC_READ | GENERIC_WRITE,
                  0,                    // exclusive access
                  NULL,                 // no security attrs
                  OPEN_EXISTING,
                  FILE_ATTRIBUTE_NORMAL | 
                  FILE_FLAG_OVERLAPPED, // overlapped I/O
                  NULL )) == (HANDLE) -1 )
	{
		m_ErrCode = IDS_ERR_COMMOPEN;
		return FALSE;
	}

	// Start up communications.
	if( !StartComm( hNewComm ) ) {
		m_ErrCode = IDS_ERR_COMMCLEAR;
	} else {
		m_Initialised = TRUE;
	}

	return (m_ErrCode == IDS_SUCCESS);
}



////////////////////////////////////////////////
// Destructor for the communications class.
// Take the communications system off-line
CCommunicate::~CCommunicate(  )
{
	if( m_Initialised ) {
		// Shut down Comm stuff
		StopComm();

		#ifdef _DEBUGLOGFILE
			fclose( m_DebugStream );
		#endif
	}
}


//////////////////////////////////////////////////////
//  StartComm()
//  Starts communications over the comm port.
//
//  StartComm makes sure there isn't communication in progress already,
//  the hNewCommFile is valid, and all the threads can be created.  It
//  also configures the hNewCommFile for the appropriate COMM settings.
//
//  If StartComm fails for any reason, it's up to the calling application
//  to close the Comm file handle.
//
//
BOOL CCommunicate::StartComm( HANDLE hNewCom )
{
	// Is this a valid comm handle?
	if (GetFileType( hNewCom ) != FILE_TYPE_CHAR) {
		TRACE("File handle is not a comm handle.\n");
		return FALSE;
	}

	// Are we already doing comm?
	if (m_ProcessControl.hCommFile != NULL) {
		TRACE("Already have a comm file open\n");
		return FALSE;
	}
	m_ProcessControl.hCommFile = hNewCom;

	// Setting and querying the comm port configurations.

	// Configure the comm settings.
	COMMTIMEOUTS commtimeouts;
	DCB dcb;
	COMMPROP commprop;
	DWORD fdwEvtMask;

	// These are here just so you can set a breakpoint
	// and see what the comm settings are.  Most Comm settings
	// are already set through TAPI.
	GetCommState(m_ProcessControl.hCommFile, &dcb);
	GetCommProperties(m_ProcessControl.hCommFile, &commprop);
	GetCommMask(m_ProcessControl.hCommFile, &fdwEvtMask);
	GetCommTimeouts(m_ProcessControl.hCommFile, &commtimeouts);

	// The CommTimeout numbers will very likely change if you are
	// coding to meet some kind of specification where
	// you need to reply within a certain amount of time after
	// recieving the last byte.  However,  If 1/4th of a second
	// goes by between recieving two characters, its a good 
	// indication that the transmitting end has finished, even
	// assuming a 1200 baud modem.
	commtimeouts.ReadIntervalTimeout         = 2;
	commtimeouts.ReadTotalTimeoutMultiplier  = 0;
	commtimeouts.ReadTotalTimeoutConstant    = 0;
	commtimeouts.WriteTotalTimeoutMultiplier = 0;
	commtimeouts.WriteTotalTimeoutConstant   = 0;

	SetCommTimeouts(m_ProcessControl.hCommFile, &commtimeouts);

	// Setup the DCB dependancies
	SetupConnection( &dcb );
	SetCommState(m_ProcessControl.hCommFile, &dcb);

	// Create the event that will signal the threads to close.
	m_ProcessControl.hCloseEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_ProcessControl.hCloseEvent == NULL) {
		TRACE( "Unable to CreateEvent: ");
		m_ProcessControl.hCommFile = NULL;
		return FALSE;
	}

	// Create the read and write threads.
	m_cpReadThread		= AfxBeginThread( (AFX_THREADPROC)StartReadThreadProc, this, THREAD_PRIORITY_HIGHEST );
	m_cpWriteThread	= AfxBeginThread( (AFX_THREADPROC)StartWriteThreadProc, this, THREAD_PRIORITY_ABOVE_NORMAL );

	// Everything was created ok.  Ready to go!
	return TRUE;
}

void CCommunicate::WaitForRead( int Timeout )
{
	// this function is called to activate the read thread with a specified
	// timeout.
}


void CCommunicate::ReceiveData( LPBYTE pDate, int Size )
{
}

//////////////////////////////////////////////////////////
//  CCommunicate::StopComm
//  Stop and end all communication threads.
//
//    Tries to gracefully signal all communication threads to
//    close, but terminates them if it has to.
//
void CCommunicate::StopComm()
{
	// No need to continue if we're not communicating.
	if (m_ProcessControl.hCommFile == NULL)
		return;

	TRACE("Stopping the Comm\n");

	// Close the threads. 
	// Note: Since we are using CWinThread via afxBeginThread,
	//       the class destructor should clean things up for us.
	CloseReadThread();
	CloseWriteThread();

	// Not needed anymore, so close it down.
	CloseHandle(m_ProcessControl.hCloseEvent);

	// Now close the comm port handle.
	CloseHandle(m_ProcessControl.hCommFile);
	m_ProcessControl.hCommFile = NULL;
}


///////////////////////////////////////////////////////
//  CCommunicate::WriteCommString(LPCSTR, DWORD)
//  Send a String to the Write Thread to be written to the Comm.
//
//    This is a wrapper function so that other modules don't care that
//    Comm writing is done via PostMessage to a Write thread.  Note that
//    using PostMessage speeds up response to the UI (very little delay to
//    'write' a string) and provides a natural buffer if the comm is slow
//    (ie:  the messages just pile up in the message queue).
//
//    Note that it is assumed that pszStringToWrite is allocated with
//    LocalAlloc, and that if WriteCommString succeeds, its the job of the
//    Write thread to LocalFree it.  If WriteCommString fails, then its
//    the job of the calling function to free the string.
//
BOOL CCommunicate::WriteCommString(PBYTE lpszStringToWrite, DWORD dwSizeofStringToWrite)
{
   HLOCAL   handle = LocalAlloc( LMEM_MOVEABLE, dwSizeofStringToWrite );
   LPVOID   pData  = LocalLock( handle );

   memcpy( pData, lpszStringToWrite, dwSizeofStringToWrite );
   LocalUnlock( handle );

    if ( m_cpWriteThread->m_hThread ) {

        if (PostThreadMessage( m_cpWriteThread->m_nThreadID, PWM_COMMWRITE, 
                (WPARAM) dwSizeofStringToWrite, (LPARAM)handle )) {
            return TRUE;
        } else {
            TRACE("Failed to Post to Write thread.\n");
        }
    } else {
        TRACE("Write thread not created\n");
    }

    LocalFree( handle );
    return FALSE;
}



//////////////////////////////////////////////////////
// SetupConnection
// This routines sets up the DCB based on settings
void CCommunicate::SetupConnection( DCB *dcb )
{
   BYTE       bSet ;

   dcb->BaudRate = m_BaudRate;
   dcb->ByteSize = m_ByteSize;
   dcb->Parity   = m_Parity;
   dcb->StopBits = m_StopBits;

   // setup hardware flow control
   bSet = (BYTE) ( ( m_FlowCtrl & FC_DTRDSR) != 0);
   dcb->fOutxDsrFlow = bSet ;
   if (bSet)
      dcb->fDtrControl = DTR_CONTROL_HANDSHAKE ;
   else
      dcb->fDtrControl = DTR_CONTROL_ENABLE ;

   bSet = (BYTE) ( ( m_FlowCtrl & FC_RTSCTS) != 0) ;
	dcb->fOutxCtsFlow = bSet ;
   if (bSet)
      dcb->fRtsControl = RTS_CONTROL_HANDSHAKE ;
   else
      dcb->fRtsControl = RTS_CONTROL_ENABLE ;

   // setup software flow control
   bSet = (BYTE) ( (m_FlowCtrl & FC_XONXOFF) != 0) ;

   dcb->fInX = dcb->fOutX = bSet ;
   dcb->XonChar = ASCII_XON ;
   dcb->XoffChar = ASCII_XOFF ;
   dcb->XonLim = 100 ;
   dcb->XoffLim = 100 ;

   // other various settings
   dcb->fBinary = TRUE ;
   dcb->fParity = TRUE ;
   dcb->fAbortOnError = FALSE;
} 


////////////////////////////////////////////////////
//  CCommunicate::CloseReadThread
//  Close the Read Thread.
//
//  Closes the Read thread by signaling the CloseEvent.
//  Purges any outstanding reads on the comm port.
//
//  Note that terminating a thread leaks memory (read the docs).
//  Besides the normal leak incurred, there is an event object
//  that doesn't get closed.  This isn't worth worrying about 
//  since it shouldn't happen anyway.
//
//
void CCommunicate::CloseReadThread()
{

	HANDLE hThread = m_cpReadThread->m_hThread;

	// If it exists...
	if ( hThread ) {
		TRACE("Closing Read Thread\n");

		// Signal the event to close the worker threads.
		SetEvent(m_ProcessControl.hCloseEvent);

		// Purge all outstanding reads
		PurgeComm(m_ProcessControl.hCommFile, PURGE_RXABORT | PURGE_RXCLEAR);

		// EXCEPTION ERROR!

		// Wait 10 seconds for it to exit.  Shouldn't happen.
		if (WaitForSingleObject( hThread, 10000) == WAIT_TIMEOUT) {
			TRACE("Read thread not exiting!\n");
		}
	}
}


//////////////////////////////////////////////////////
//  CCommunicate::CloseWriteThread
//  Closes the Write Thread.
//
//  Closes the write thread by signaling the CloseEvent.
//  Purges any outstanding writes on the comm port.
//
//  Note that terminating a thread leaks memory (read the docs).
//  Besides the normal leak incurred, there is an event object
//  that doesn't get closed.  This isn't worth worrying about 
//  since it shouldn't happen anyway.
//
void CCommunicate::CloseWriteThread()
{
	// If it exists...
	if (m_cpWriteThread->m_hThread) {
		TRACE("Closing Write Thread\n");

		// Signal the event to close the worker threads.
		SetEvent(m_ProcessControl.hCloseEvent);

		// Purge all outstanding writes.
		PurgeComm(m_ProcessControl.hCommFile, PURGE_TXABORT | PURGE_TXCLEAR);

		// Wait 10 seconds for it to exit.  Shouldn't happen.
		if( WaitForSingleObject(m_cpWriteThread->m_hThread, 10000) == WAIT_TIMEOUT) {
			TRACE("Write thread not exiting!\n");
		}
	}
}




////////////////////////////////////////////////////////////////
// READ THREAD STARTS HERE
//

//////////////////////////////////////////////////////
//  StartReadThreadProc
//
//  The Read Thread uses overlapped ReadFile and sends any strings
//  read from the comm port to the UI to be printed.  This is
//  eventually done through a PostMessage so that the Read Thread
//  is never away from the comm port very long.  This also provides
//  natural desynchronization between the Read thread and the UI.
//
//  If the CloseEvent object is signaled, the Read Thread exits.
//
//  Note that there is absolutely *no* interpretation of the data.
//
//
DWORD StartReadThreadProc(LPVOID lpvParam )
{
	char szInputBuffer[INPUTBUFFERSIZE];
	DWORD nNumberOfBytesRead;

	HANDLE HandlesToWaitFor[3];
	DWORD dwHandleSignaled;

	DWORD fdwEvtMask = EV_RXCHAR;

	CCommunicate *pcComm = (CCommunicate *)lpvParam;

	// Needed for overlapped I/O (ReadFile)
	OVERLAPPED overlappedRead  = {0, 0, 0, 0, NULL};

	// Needed for overlapped Comm Event handling.
	OVERLAPPED overlappedCommEvent = {0, 0, 0, 0, NULL};

	// Lets put an event in the Read overlapped structure.
	overlappedRead.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (overlappedRead.hEvent == NULL) {
		TRACE("Unable to CreateEvent: ");
		goto EndReadThread;
	}

	// And an event for the CommEvent overlapped structure.
	overlappedCommEvent.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (overlappedCommEvent.hEvent == NULL) {
		TRACE( "Unable to CreateEvent: ");
		goto EndReadThread;
	}

	// We will be waiting on these objects.
	HandlesToWaitFor[0] = pcComm->m_ProcessControl.hCloseEvent;
	HandlesToWaitFor[1] = overlappedCommEvent.hEvent;
	HandlesToWaitFor[2] = overlappedRead.hEvent;

	// Setup CommEvent handling.

	// Set the comm mask so we receive error signals.
	if (!SetCommMask(pcComm->m_ProcessControl.hCommFile, EV_ERR)) {
		TRACE("Unable to SetCommMask: ");
		goto EndReadThread;
	}

	// Start waiting for CommEvents (Errors)
	if (!SetupCommEvent(&overlappedCommEvent, &fdwEvtMask, pcComm )) {
		goto EndReadThread;
	}

	// Start waiting for Read events.
	if (!SetupReadEvent(&overlappedRead, szInputBuffer, INPUTBUFFERSIZE, &nNumberOfBytesRead, pcComm )) {
		//PostHangupCall();
		goto EndReadThread;
	}

	// Keep looping until we break out.
	while (TRUE) {
		int TOut = pcComm->m_ReceiveTimeout;

		if( TOut == 0 ) {
			TOut = INFINITE;
		}

		// Wait until some event occurs (data to read; error; stopping).
		dwHandleSignaled = WaitForMultipleObjects(3, HandlesToWaitFor, FALSE, TOut );

		// Which event occured?
		switch(dwHandleSignaled) {
			case WAIT_OBJECT_0:     // Signal to end the thread.
			{
				// Time to exit.
				goto EndReadThread;
			}
			case WAIT_OBJECT_0 + 1: // CommEvent signaled.
			{
				// Handle the CommEvent.
				if (!HandleCommEvent(&overlappedCommEvent, &fdwEvtMask, TRUE, pcComm ))  {
					goto EndReadThread;
				}

				// Start waiting for the next CommEvent.
				if (!SetupCommEvent(&overlappedCommEvent, &fdwEvtMask, pcComm )) {
					goto EndReadThread;
				}
				break;
			}
			case WAIT_OBJECT_0 + 2: // Read Event signaled.
			{
				// Get the new data!
				if (!HandleReadEvent(&overlappedRead,
                            szInputBuffer, INPUTBUFFERSIZE,
                            &nNumberOfBytesRead, pcComm )) {
					goto EndReadThread;
				}

				// Wait for more new data.
				if (!SetupReadEvent(&overlappedRead,
                            szInputBuffer, INPUTBUFFERSIZE,
                            &nNumberOfBytesRead, pcComm )) {
					goto EndReadThread;
				}
				break;
			}
			case WAIT_FAILED:       // Wait failed.  Shouldn't happen.
			{
				TRACE("Read WAIT_FAILED: ");
				DWORD errcode = GetLastError();
            goto EndReadThread;
			}
			case WAIT_TIMEOUT:
			{
				// no response. Call application with 0 data.
				HandleReadData(NULL, 0, pcComm );
//				if( !SetupCommEvent(&overlappedCommEvent, &fdwEvtMask, pcComm )) {
//					TRACE( "Timeout setup failed\n\r" );
//					goto EndReadThread;
//				}
				break;
			}
			default:    // This case should never occur.
			{
				TRACE("Unexpected Wait return value" );
				goto EndReadThread;
			}
		} // End of switch(dwHandleSignaled).
	} // End of while(TRUE) loop.


	// Time to clean up Read Thread.
	EndReadThread:

	TRACE("Read thread shutting down\n");
	PurgeComm(pcComm->m_ProcessControl.hCommFile, PURGE_RXABORT | PURGE_RXCLEAR);
	CloseHandle(overlappedRead.hEvent);
	CloseHandle(overlappedCommEvent.hEvent);

	return 0;
}


//////////////////////////////////////////////////////
//  SetupReadEvent(LPOVERLAPPED, LPSTR, DWORD, LPDWORD)
//  Sets up an overlapped ReadFile
//
//  This function is a helper function for the Read Thread.  This
//  function sets up the overlapped ReadFile so that it can later
//  be waited on (or more appropriatly, so the event in the overlapped
//  structure can be waited upon).  If there is data waiting, it is
//  handled and the next ReadFile is initiated.
//  Another possible reason for returning FALSE is if the comm port
//  is closed by the service provider.
//    
BOOL SetupReadEvent(LPOVERLAPPED lpOverlappedRead,
    LPSTR lpszInputBuffer, DWORD dwSizeofBuffer,
    LPDWORD lpnNumberOfBytesRead, CCommunicate *pcComm )
{
	DWORD dwLastError;

StartSetupReadEvent:

	// Make sure the CloseEvent hasn't been signaled yet.
	// Check is needed because this function is potentially recursive.
	if (WAIT_TIMEOUT != WaitForSingleObject(pcComm->m_ProcessControl.hCloseEvent,0))
		return FALSE;
    
	// Start the overlapped ReadFile.
	if (ReadFile(pcComm->m_ProcessControl.hCommFile, 
            lpszInputBuffer, dwSizeofBuffer,
            lpnNumberOfBytesRead, lpOverlappedRead)) {
		// This would only happen if there was data waiting to be read.
		TRACE("Data waiting for ReadFile.\n");
        
		// Handle the data.
		if (!HandleReadData((LPBYTE)lpszInputBuffer, *lpnNumberOfBytesRead, pcComm )) {
			return FALSE;
		}

		// Start waiting for more data.
		goto StartSetupReadEvent;
	}

	// ReadFile failed.  Expected because of overlapped I/O.
	dwLastError = GetLastError();

	// LastError was ERROR_IO_PENDING, as expected.
	if (dwLastError == ERROR_IO_PENDING) {
		return TRUE;
	}

	// Its possible for this error to occur if the 
	// service provider has closed the port.  Time to end.
	if (dwLastError == ERROR_INVALID_HANDLE) {
		TRACE( "Likely that the Service Provider has closed the port.\n");
		return FALSE;
	}

	// Unexpected error. No idea what could cause this to happen.
	TRACE("Unexpected ReadFile error: ");
   
	//PostHangupCall();
	return FALSE;
}
 
 
//////////////////////////////////////////////////////
//  HandleReadEvent(LPOVERLAPPED, LPSTR, DWORD, LPDWORD)
//  Retrieves and handles data when there is data ready.
//
//  This function is another helper function for the Read Thread.  This
//  is the function that is called when there is data available after
//  an overlapped ReadFile has been setup.  It retrieves the data and
//  handles it.
//
BOOL HandleReadEvent(LPOVERLAPPED lpOverlappedRead,
    LPSTR lpszInputBuffer, DWORD dwSizeofBuffer,
    LPDWORD lpnNumberOfBytesRead, CCommunicate *pcComm )
{
    DWORD dwLastError;

    if( GetOverlappedResult(pcComm->m_ProcessControl.hCommFile,
            lpOverlappedRead, lpnNumberOfBytesRead, FALSE) ) {
       return HandleReadData((LPBYTE)lpszInputBuffer, *lpnNumberOfBytesRead, pcComm);
    }

    // Error in GetOverlappedResult; handle it.

    dwLastError = GetLastError();

    // Its possible for this error to occur if the 
    // service provider has closed the port.  Time to end.
    if (dwLastError == ERROR_INVALID_HANDLE) {
        TRACE( "Likely that the Service Provider has closed the port.\n");
        return FALSE;
    }

    TRACE( "Unexpected GetOverlappedResult Read Error: ");

    return FALSE;
}


//////////////////////////////////////////////////////
//  HandleReadData(LPCSTR, DWORD)
//  Deals with data after its been read from the comm file.
//
//  This function is yet another helper function for the Read Thread.
//  It LocalAlloc()s a buffer, copies the new data to this buffer and
//  calls PostWriteToDisplayCtl to let the EditCtls module deal with
//  the data.  Its assumed that PostWriteToDisplayCtl posts the message
//  rather than dealing with it right away so that the Read Thread
//  is free to get right back to waiting for data.  Its also assumed
//  that the EditCtls module is responsible for LocalFree()ing the
//  pointer that is passed on.
//
BOOL HandleReadData(LPBYTE pBuff, DWORD Count, CCommunicate *pcComm )
{
    // If we got data and didn't just time out empty...
    if( Count ) {
		pcComm->ReceiveData( pBuff, Count );
    } else {
		 // inform the application of a response timeout.
		pcComm->ReceiveData( NULL, 0 );	
	 }
	return TRUE;
}


//////////////////////////////////////////////////////
//  SetupCommEvent(LPOVERLAPPED, LPDWORD)
//  Sets up the overlapped WaitCommEvent call.
//
//  This function is a helper function for the Read Thread that sets up
//  the WaitCommEvent so we can deal with comm events (like Comm errors)
//  if they occur.
//
BOOL SetupCommEvent(LPOVERLAPPED lpOverlappedCommEvent,
    LPDWORD lpfdwEvtMask, CCommunicate *pcComm )
{
    DWORD dwLastError;

  StartSetupCommEvent:

    // Make sure the CloseEvent hasn't been signaled yet.
    // Check is needed because this function is potentially recursive.
    if (WAIT_TIMEOUT != WaitForSingleObject(pcComm->m_ProcessControl.hCloseEvent,0))
        return FALSE;

    // Start waiting for Comm Errors.
    if (WaitCommEvent(pcComm->m_ProcessControl.hCommFile, lpfdwEvtMask, lpOverlappedCommEvent))
    {
        // This could happen if there was an error waiting on the
        // comm port.  Lets try and handle it.

        TRACE("Event (Error) waiting before WaitCommEvent.\n");

        if (!HandleCommEvent(NULL, lpfdwEvtMask, FALSE, pcComm ))
            return FALSE;

        // What could cause infinite recursion at this point?
        goto StartSetupCommEvent;
    }

    // We expect ERROR_IO_PENDING returned from WaitCommEvent
    // because we are waiting with an overlapped structure.

    dwLastError = GetLastError();

    // LastError was ERROR_IO_PENDING, as expected.
    if (dwLastError == ERROR_IO_PENDING)
    {
        TRACE("Waiting for a CommEvent (Error) to occur.\n");
        return TRUE;
    }

    // Its possible for this error to occur if the 
    // service provider has closed the port.  Time to end.
    if (dwLastError == ERROR_INVALID_HANDLE)
    {
        TRACE(
            "Likely that the Service Provider has closed the port.\n");
        return FALSE;
    }

    // Unexpected error. No idea what could cause this to happen.
    TRACE("Unexpected WaitCommEvent error: ");
    return FALSE;
}


//////////////////////////////////////////////////////
//  HandleCommEvent(LPOVERLAPPED, LPDWORD, BOOL)
//  Handle an outstanding Comm Event.
//
//  This function is a helper function for the Read Thread that (if
//  fRetrieveEvent == TRUE) retrieves an outstanding CommEvent and
//  deals with it.  The only event that should occur is an EV_ERR event,
//  signalling that there has been an error on the comm port.
//
//  Normally, comm errors would not be put into the normal data stream
//  as this sample is demonstrating.  Putting it in a status bar would
//  be more appropriate for a real application.
//
BOOL HandleCommEvent(LPOVERLAPPED lpOverlappedCommEvent, 
    LPDWORD lpfdwEvtMask, BOOL fRetrieveEvent, CCommunicate *pcComm )
{
    DWORD dwDummy;
    LPSTR lpszOutput;
    char szError[128] = "";
    DWORD dwErrors;
    DWORD nOutput;
    DWORD dwLastError;


    lpszOutput = new char[256];
    if (lpszOutput == NULL)
    {
        TRACE("LocalAlloc: ");
        return FALSE;
    }

    // If this fails, it could be because the file was closed (and I/O is
    // finished) or because the overlapped I/O is still in progress.  In
    // either case (or any others) its a bug and return FALSE.
    if (fRetrieveEvent)
        if (!GetOverlappedResult(pcComm->m_ProcessControl.hCommFile, 
                lpOverlappedCommEvent, &dwDummy, FALSE))
        {
            dwLastError = GetLastError();

            // Its possible for this error to occur if the 
            // service provider has closed the port.  Time to end.
            if (dwLastError == ERROR_INVALID_HANDLE)
            {
                TRACE(
                    "Likely that the Service Provider has closed the port.\n");
				delete lpszOutput;
                return FALSE;
            }

            TRACE(
                "Unexpected GetOverlappedResult for WaitCommEvent: ");
			delete lpszOutput;
            return FALSE;
        }

    // Was the event an error?
    if (*lpfdwEvtMask & EV_ERR)
    {
        // Which error was it?
        if (!ClearCommError(pcComm->m_ProcessControl.hCommFile, &dwErrors, NULL))
        {
            dwLastError = GetLastError();

            // Its possible for this error to occur if the 
            // service provider has closed the port.  Time to end.
            if (dwLastError == ERROR_INVALID_HANDLE)
            {
                TRACE(
                    "Likely that the Service Provider has closed the port.\n");
				delete lpszOutput;
                return FALSE;
            }

            TRACE("ClearCommError: ");
			delete lpszOutput;
            return FALSE;
        }

        // Its possible that multiple errors occured and were handled
        // in the last ClearCommError.  Because all errors were signaled
        // individually, but cleared all at once, pending comm events 
        // can yield EV_ERR while dwErrors equals 0.  Ignore this event.
        if (dwErrors == 0)
        {
            strcat(szError, "NULL Error");
        }
       
        if (dwErrors & CE_FRAME)
        {
            if (szError[0])
                strcat(szError," and ");

            strcat(szError,"CE_FRAME");
        }

        if (dwErrors & CE_OVERRUN)
        {
            if (szError[0])
                strcat(szError," and ");

            strcat(szError,"CE_OVERRUN");
        }

        if (dwErrors & CE_RXPARITY)
        {
            if (szError[0])
                strcat(szError," and ");

            strcat(szError,"CE_RXPARITY");
        }

        if (dwErrors & ~ (CE_FRAME | CE_OVERRUN | CE_RXPARITY))
        {
            if (szError[0])
                strcat(szError," and ");

            strcat(szError,"EV_ERR Unknown EvtMask");
        }


        nOutput = wsprintf(lpszOutput,
            "Comm Event: '%s', EvtMask = '%lx'\n",
            szError, dwErrors);

        //PostWriteToDisplayCtl(lpszOutput, nOutput);
		delete lpszOutput;
        return TRUE;

    }

    // Should not have gotten here.  Only interested in ERR conditions.

    TRACE("Unexpected comm event");
    return FALSE;
}



////////////////////////////////////////////////////////////////
// WRITE THREAD STARTS HERE
//


//////////////////////////////////////////////////////
//  StartWriteThreadProc(LPVOID)
//  The starting point for the Write thread.
//
//  The Write thread uses a PeekMessage loop to wait for a string to write,
//  and when it gets one, it writes it to the Comm port.  If the CloseEvent
//  object is signaled, then it exits.  The use of messages to tell the
//  Write thread what to write provides a natural desynchronization between
//  the UI and the Write thread.
//
DWORD StartWriteThreadProc(LPVOID lpvParam)
{
    MSG msg;
    DWORD dwHandleSignaled;

    // Needed for overlapped I/O.
    OVERLAPPED overlappedWrite = {0, 0, 0, 0, NULL};

	CCommunicate *pcComm = (CCommunicate *)lpvParam;

    overlappedWrite.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (overlappedWrite.hEvent == NULL)
    {
        TRACE( "Unable to CreateEvent: ");
        goto EndWriteThread;
    }

    // This is the main loop.  Loop until we break out.
    while (TRUE)
    {
        if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // If there are no messages pending, wait for a message or 
            // the CloseEvent.
            dwHandleSignaled = 
                MsgWaitForMultipleObjects(1, &pcComm->m_ProcessControl.hCloseEvent, FALSE,
                    INFINITE, QS_ALLINPUT);

            switch(dwHandleSignaled)
            {
                case WAIT_OBJECT_0:     // CloseEvent signaled!
                {
                    // Time to exit.
                    goto EndWriteThread;
                }

                case WAIT_OBJECT_0 + 1: // New message was received.
                {
                    // Get the message that woke us up by looping again.
                    continue;
                }

                case WAIT_FAILED:       // Wait failed.  Shouldn't happen.
                {
                    TRACE("Write WAIT_FAILED: ");
                    goto EndWriteThread;
                }

                default:                // This case should never occur.
                {
                    TRACE("Unexpected Wait return value" );
                    goto EndWriteThread;
                }

            }
        }

        // Make sure the CloseEvent isn't signaled while retrieving messages.
        if (WAIT_TIMEOUT != WaitForSingleObject(pcComm->m_ProcessControl.hCloseEvent,0))
            goto EndWriteThread;

        // Process the message.

        // This could happen if a dialog is created on this thread.
        // This doesn't occur in this sample, but might if modified.
        if (msg.hwnd != NULL)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            continue;
        }

        // Handle the message.
        switch(msg.message)
        {
            case PWM_COMMWRITE:  // New string to write to Comm port.
            {
                // Write the string to the comm port.  HandleWriteData
                // does not return until the whole string has been written,
                // an error occurs or until the CloseEvent is signaled.

               // get the memory from the Passed in handle
               LPCSTR pData = (LPCSTR)LocalLock( (HLOCAL)msg.lParam );
               if (!HandleWriteData(&overlappedWrite,
                        pData, (DWORD) msg.wParam, pcComm ))
                {
                    // If it failed, either we got a signal to end or there
                    // really was a failure.

                     LocalUnlock( (HLOCAL)msg.lParam );
                     LocalFree((HLOCAL) msg.lParam); 
                     goto EndWriteThread;
                }

                // Data was sent in a LocalAlloc()d buffer.  Must free it.
                LocalUnlock( (HLOCAL)msg.lParam );
                LocalFree((HLOCAL) msg.lParam); 
                break;
            }
    

            // What other messages could the thread get?
            default:
            {
                char Output[256];
    
                wsprintf(Output,
                    "Unexpected message posted to Write thread: %ui\n",
                    msg.message );
                    
                TRACE(Output);
                break;
            }
        } // End of switch(message)

    } // End of main loop.

    // Thats the end.  Now clean up.
  EndWriteThread:

    TRACE("Write thread shutting down\n");

    PurgeComm(pcComm->m_ProcessControl.hCommFile, PURGE_TXABORT | PURGE_TXCLEAR);

    CloseHandle(overlappedWrite.hEvent);
    return 0;
}
 

////////////////////////////////////////////////////////
//  HandleWriteData(LPOVERLAPPED, LPCSTR, DWORD)
//  Writes a given string to the comm file handle.
//
//  This function is a helper function for the Write Thread.  It
//  is this call that actually writes a string to the comm file.
//  Note that this call blocks and waits for the Write to complete
//  or for the CloseEvent object to signal that the thread should end.
//  Another possible reason for returning FALSE is if the comm port
//  is closed by the service provider.
//
BOOL HandleWriteData(LPOVERLAPPED lpOverlappedWrite,
    LPCSTR lpszStringToWrite, DWORD dwNumberOfBytesToWrite, CCommunicate * pcComm)
{
    DWORD dwLastError;

    DWORD dwNumberOfBytesWritten = 0;
    DWORD dwWhereToStartWriting = 0; // Start at the beginning.

    DWORD dwHandleSignaled;
    HANDLE HandlesToWaitFor[2];

    HandlesToWaitFor[0] = pcComm->m_ProcessControl.hCloseEvent;
    HandlesToWaitFor[1] = lpOverlappedWrite -> hEvent;

    // Keep looping until all characters have been written.
    do
    {

        if (!WriteFile(pcComm->m_ProcessControl.hCommFile, 
                &lpszStringToWrite[ dwWhereToStartWriting ], 
                dwNumberOfBytesToWrite, &dwNumberOfBytesWritten,
                lpOverlappedWrite))
        {
            // WriteFile failed.  Expected; lets handle it.
            dwLastError = GetLastError();

            // Its possible for this error to occur if the 
            // service provider has closed the port.  Time to end.
            if (dwLastError == ERROR_INVALID_HANDLE)
            {
                TRACE(
                    "Likely that the Service Provider has closed the port.\n");
                return FALSE;
            }

            // Unexpected error.  No idea what.
            if (dwLastError != ERROR_IO_PENDING)
            {
                TRACE(
                    "Error to writing to CommFile");
                
                TRACE("Closing TAPI\n");
                return FALSE;
            }

            // This is the expected ERROR_IO_PENDING case.


            // Wait for either overlapped I/O completion,
            // or for the CloseEvent to get signaled.
            dwHandleSignaled = 
                WaitForMultipleObjects(2, HandlesToWaitFor, 
                    FALSE, INFINITE);

            switch(dwHandleSignaled)
            {
                case WAIT_OBJECT_0:     // CloseEvent signaled!
                {
                    // Time to exit.
                    return FALSE;
                }

                case WAIT_OBJECT_0 + 1: // Wait finished.
                {
                    // Time to get the results of the WriteFile
                    break;
                }

                case WAIT_FAILED: // Wait failed.  Shouldn't happen.
                {
                    TRACE(
                        "Write WAIT_FAILED: ");
                    return FALSE;
                }

                default: // This case should never occur.
                {
                    TRACE("Unexpected Wait return value " );
                    return FALSE;
                }
            }

            if (!GetOverlappedResult(pcComm->m_ProcessControl.hCommFile,
                     lpOverlappedWrite,
                     &dwNumberOfBytesWritten, TRUE))
            {
                dwLastError = GetLastError();

                // Its possible for this error to occur if the 
                // service provider has closed the port.
                if (dwLastError == ERROR_INVALID_HANDLE)
                {
                    TRACE("Likely that the Service Provider has closed the port.\n");
                    return FALSE;
                }

                // No idea what could cause another error.
                TRACE("Error writing to CommFile while waiting");
                return FALSE;
            }
        }

        // Some data was written.  Make sure it all got written.

        dwNumberOfBytesToWrite -= dwNumberOfBytesWritten;
        dwWhereToStartWriting += dwNumberOfBytesWritten;
    }
    while(dwNumberOfBytesToWrite > 0);  // Write the whole thing!

    // Wrote the whole string.
    return TRUE;
}


