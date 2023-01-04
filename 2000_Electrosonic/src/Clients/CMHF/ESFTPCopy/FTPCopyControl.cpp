// FTPCopyControl.cpp : Implementation of CFTPCopyControl

#include "stdafx.h"
#include "ESFTPCopy.h"
#include "FTPCopyControl.h"
#include <process.h>

/////////////////////////////////////////////////////////////////////////////
// CFTPCopyControl

CFTPCopyControl::CFTPCopyControl()
{
	m_bWindowOnly			= TRUE;
	m_WorkerThread			= NULL;
	m_hInternet				= NULL;
	m_hConnection			= NULL;
	m_OperationType			= OperationType_Invalid;
	m_Flags					= CopyOperationTypeFlag_BINARY;
	m_CopyMode				= CopyOperationMode_Overwrite;
}

CFTPCopyControl::~CFTPCopyControl()
{
	TerminateThread();
}

STDMETHODIMP CFTPCopyControl::get_strServer(BSTR *pVal)
{
	*pVal = m_strServer;
	return S_OK;
}

STDMETHODIMP CFTPCopyControl::put_strServer(BSTR newVal)
{
	m_strServer = newVal;
	return S_OK;
}

STDMETHODIMP CFTPCopyControl::get_strLogin(BSTR *pVal)
{
	*pVal = m_strLogin;
	return S_OK;
}

STDMETHODIMP CFTPCopyControl::put_strLogin(BSTR newVal)
{
	m_strLogin = newVal;
	return S_OK;
}

STDMETHODIMP CFTPCopyControl::get_strPassword(BSTR *pVal)
{
	*pVal = m_strPassword;
	return S_OK;
}

STDMETHODIMP CFTPCopyControl::put_strPassword(BSTR newVal)
{
	m_strPassword = newVal;
	return S_OK;
}



//the following two functions do not need a thread or connection
STDMETHODIMP CFTPCopyControl::DeleteLocalFile(BSTR strLocalFileName)
{
	USES_CONVERSION;
	HRESULT hr = S_FALSE;
	CComBSTR strError;
	LPTSTR pLocalFileName		= OLE2T(strLocalFileName);
	HANDLE hLocalFile;
	//we use this function because we do not want directory finds
	hLocalFile = CreateFile( pLocalFileName, GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE != hLocalFile )
	{
		CloseHandle( hLocalFile );
		if( FALSE == DeleteFile( pLocalFileName ) )
		{
			PROCESS_ERROR1( "could not delete file" )
		}
		else
		{
			strError = "file deleted successfully";
			hr = S_OK;
		}
	}
	else
	{
		strError = "file did not exist. success";
		hr = S_OK;
	}
	Fire_OnCompleted( OperationType_DeleteLocalFile, hr, strError );
	return hr;
}

STDMETHODIMP CFTPCopyControl::DoesLocalFileExist(BSTR strLocalFileName, BOOL *pbResult)
{
	USES_CONVERSION;
	CComBSTR strError;
	LPTSTR pLocalFileName		= OLE2T(strLocalFileName);
	HANDLE hLocalFile;
	//we use this function because we do not want directory finds
	hLocalFile = CreateFile( pLocalFileName, GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	strError = "file : ";
	strError += pLocalFileName;
	if( INVALID_HANDLE_VALUE != hLocalFile )
	{
		CloseHandle( hLocalFile );
		strError += " exists on ftp server";
		*pbResult = TRUE;
	}
	else
	{
		strError += " does not exist on ftp server";
		*pbResult = FALSE;
	}
	Fire_OnCompleted( OperationType_DoesLocalFileExist, *pbResult, strError );
	return S_OK;
}

STDMETHODIMP CFTPCopyControl::Connect()
{
	HRESULT hr = S_OK;
	if( NULL == m_hConnection )
	{
		m_OperationType		= OperationType_Connect;
		
		hr = StartCopyWorkerThread();
	}
	return hr;
}

STDMETHODIMP CFTPCopyControl::Disconnect()
{
	HRESULT hr = S_OK;
	if( NULL != m_hConnection )
	{
		m_OperationType		= OperationType_Disconnect;
		
		hr = StartCopyWorkerThread();
	}
	return hr;
}

//these two functions connect if we are not
STDMETHODIMP CFTPCopyControl::GetFile(BSTR strLocalDestination, BSTR strRemoteSource)
{
	m_OperationType		= OperationType_Get;
	m_strSource			= strRemoteSource;
	m_strDestination	= strLocalDestination;
		
	return StartCopyWorkerThread();
}

STDMETHODIMP CFTPCopyControl::PutFile(BSTR strRemoteDestination, BSTR strLocalSource)
{
	m_OperationType		= OperationType_Put;
	m_strSource			= strLocalSource;
	m_strDestination	= strRemoteDestination;

	return StartCopyWorkerThread();
}

STDMETHODIMP CFTPCopyControl::MoveFTPFileToLocal(BSTR strLocalDestination, BSTR strRemoteSource)
{
	m_OperationType		= OperationType_MoveFTPFileToLocal;
	m_strSource			= strRemoteSource;
	m_strDestination	= strLocalDestination;
		
	return StartCopyWorkerThread();
}

STDMETHODIMP CFTPCopyControl::MoveLocalFileToFTP(BSTR strRemoteDestination, BSTR strLocalSource)
{
	m_OperationType		= OperationType_MoveLocalFileToFTP;
	m_strSource			= strLocalSource;
	m_strDestination	= strRemoteDestination;

	return StartCopyWorkerThread();
}

STDMETHODIMP CFTPCopyControl::DeleteFTPFile(BSTR strFTPFileName)
{
	m_OperationType		= OperationType_DeleteFTPFile;
	m_strDestination	= strFTPFileName;
	m_strSource			= "Dummy";

	return StartCopyWorkerThread();
}

STDMETHODIMP CFTPCopyControl::DoesFTPFileExist(BSTR strFTPFileName)
{
	m_OperationType		= OperationType_DoesFTPFileExist;
	m_strDestination	= strFTPFileName;
	m_strSource			= "Dummy";

	return StartCopyWorkerThread();
}



HRESULT CFTPCopyControl::StartCopyWorkerThread()
{
	HRESULT hr = S_FALSE;
	unsigned long unThreadAddr;

	m_WorkerThread = CreateThread(
		NULL,						// pointer to security attributes
		0,							// initial thread stack size
		ThreadProc,					// pointer to thread function
		this,                       // argument for new thread
		0,							// creation flags
		&unThreadAddr               // pointer to receive thread ID
		);

	if( NULL == m_WorkerThread )
	{
		CComBSTR strError;
		hr = GetLastError();
		ProcessError( hr, "cannot start worker thread", strError );
		Fire_OnError( hr, strError );
	}
	else
	{
		hr = S_OK;
	}
	return hr;
}

void CFTPCopyControl::TerminateThread()
{
	//this function gets called when the app terminates only
	if( m_WorkerThread != NULL )
	{
		//so wait only about 2 minutes, otherwise abort!
		::WaitForSingleObject( m_WorkerThread, 120000 );

		//disconnect
		if( NULL != m_hConnection )
			InternetCloseHandle( m_hConnection );
		if( NULL != m_hInternet )
			InternetCloseHandle( m_hInternet );

		m_hConnection	= NULL;
		m_hInternet		= NULL;
		m_WorkerThread	= NULL;
	}
}


// static method
unsigned long __stdcall CFTPCopyControl::ThreadProc( void * pVoid )
{
	//get calling thread pointer
	CFTPCopyControl * pThread = static_cast<CFTPCopyControl *>(pVoid);
	if( NULL == pThread )
	{
		Beep( 90, 400 ); //weird!
		return S_FALSE;
	}

	HRESULT	hr = S_FALSE;
	CComBSTR strError;

	//initialize COM
	hr = CoInitializeEx( NULL,	COINIT_APARTMENTTHREADED | 
							COINIT_SPEED_OVER_MEMORY );
	if( S_OK != hr )
	{
		PROCESS_ERROR( "could Initialize COM" )
	}

	HINTERNET				hInternet		= NULL;
	HINTERNET				hConnection		= NULL;

	CopyOperationModes		CopyMode		= CopyOperationMode_Overwrite;
	CopyOperationTypeFlags	Flags			= CopyOperationTypeFlag_BINARY;
	OperationTypes			OperationType	= OperationType_Invalid;

	USES_CONVERSION;

	//copy thread data 
	//so we don't bother the calling thread anymore
	pThread->Lock();

	hInternet			= pThread->m_hInternet;
	hConnection			= pThread->m_hConnection;
	OperationType		= pThread->m_OperationType;
	CopyMode			= pThread->m_CopyMode;
	Flags				= pThread->m_Flags;

	//here some space is allocated in the stack!
	//but it gets freed when the function ends, duh!
	LPTSTR pServer		= OLE2T(pThread->m_strServer);
	LPTSTR pLogin		= OLE2T(pThread->m_strLogin);
	LPTSTR pPassword	= OLE2T(pThread->m_strPassword);
	LPTSTR pSource		= OLE2T(pThread->m_strSource);
	LPTSTR pDestination = OLE2T(pThread->m_strDestination);

	pThread->Unlock();

	//connect if we are not, and we are not disconnecting
	if( OperationType != OperationType_Disconnect &&
		hConnection == NULL )
	{
		hInternet = 
			InternetOpen(	"ESFTPCopyControl", 
							INTERNET_OPEN_TYPE_DIRECT,
							NULL, NULL, 0 );
		if( NULL != hInternet )
		{
			hConnection = 
				InternetConnect(	hInternet, pServer,
									INTERNET_DEFAULT_FTP_PORT,
									pLogin, pPassword,
									INTERNET_SERVICE_FTP,
									INTERNET_FLAG_PASSIVE,
									NULL );
			if( NULL != hConnection )
			{
				//copy internet handles to calling thread
				//bother the thread a little bit
				pThread->Lock();
				pThread->m_hInternet = hInternet;
				pThread->m_hConnection = hConnection;
				pThread->Unlock();
				hr = S_OK;
			}
			else
			{
				PROCESS_ERROR( "could not connect to server" )
			}
		}
		else
		{
			PROCESS_ERROR( "could not open connection to the internet" )
		}
	}

	switch( OperationType )
	{
	//CONNECT
	case OperationType_Connect:
		//connected above
		break;
	//DISCONNECT
	case OperationType_Disconnect:
		{
			if( NULL != hConnection )
				InternetCloseHandle( hConnection );
			if( NULL != hInternet )
				InternetCloseHandle( hInternet );

			pThread->Lock();
			pThread->m_hInternet = NULL;
			pThread->m_hConnection = NULL;
			pThread->Unlock();

			hr = S_OK;
		}
		break;
	//GET FILE
	case OperationType_MoveFTPFileToLocal:
		//we'll try to delete later
	case OperationType_Get:
		if( !CheckFileParameters( pSource, pDestination ) )
		{
			PROCESS_ERROR_INVALID_FILE_PARAMETER
		}
		else
		{
			BOOL bRet = FtpGetFile( hConnection, pSource, pDestination, 
							FALSE, INTERNET_FLAG_RELOAD, 
							Flags, 0 );
			if( TRUE == bRet )
			{
				hr = S_OK;
			}
			else
			{
				PROCESS_ERROR( "could not get ftp file" )
			}
		}
		break;
	//PUT FILE
	case OperationType_MoveLocalFileToFTP:
		//we'll try to delete later
	case OperationType_Put:
		if( !CheckFileParameters( pSource, pDestination ) )
		{
			PROCESS_ERROR_INVALID_FILE_PARAMETER
		}
		else
		{
			BOOL bRet = FtpPutFile( hConnection, pSource, pDestination, 
							Flags, 0 );
			if( TRUE == bRet )
			{
				hr = S_OK;
			}
			else
			{
				PROCESS_ERROR( "could not put ftp file" )
			}
		}
		break;
	}
	switch( OperationType )
	{
	case OperationType_MoveFTPFileToLocal:
		if( hr == S_OK )
		{
			//file was already copied to local
			//verify local file
			HANDLE hLocalFile;
			hLocalFile = CreateFile( pSource, GENERIC_READ,
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
			if( INVALID_HANDLE_VALUE != hLocalFile )
			{
				CloseHandle( hLocalFile );
				//if it's there
				//delete file on ftp server
				if( TRUE == FtpDeleteFile( hConnection, pDestination ) )
				{
					hr = S_OK;
				}
				else
				{
					PROCESS_ERROR( "ftp file was copied to local, but could not delete ftp file afterward" )
					//this is just a warning
					hr = S_OK;
				}
			}
			else
			{
				strError = "ftp file just copied was not found on local drive";
				//this is an error
				hr = S_FALSE;
			}
		}
		break;
	case OperationType_MoveLocalFileToFTP:
		if( hr == S_OK )
		{
			//file was already copied to ftp
			//verify ftp file
			if( TRUE == FTPFileExists( hConnection, Flags, pDestination ) )
			{
				//if it's there
				//delete file on local drive
				if( FALSE == DeleteFile( pSource ) )
				{
					PROCESS_ERROR( "local file was copied to ftp, but could not delete local file afterward" )
					//this is just a warning
					hr = S_OK;
				}
				else
				{
					hr = S_OK;
				}
			}
			else
			{
				hr = S_FALSE;
				strError = "ftp file did not exist after copy operation";
			}
		}
		break;
	case OperationType_DeleteFTPFile:
		//find ftp file
		if( TRUE == FTPFileExists( hConnection, Flags, pDestination ) )
		{
			//if it's there delete it
			if( TRUE == FtpDeleteFile( hConnection, pDestination ) )
			{
				hr = S_OK;
				strError = "ftp file delete operation completed";
			}
			else
			{
				PROCESS_ERROR( "could not delete ftp file" )
			}
		}
		else
		{
			hr = S_OK;
			strError = "ftp file did not exist";
		}
		break;
	case OperationType_DoesFTPFileExist:
		//find ftp file
		hr = FTPFileExists( hConnection, Flags, pDestination );
		if( TRUE == hr )
		{
			strError = "file exists on ftp server";
		}
		else
		{
			strError += "file does not exist on ftp server";
		}
		break;
	}
	
	if( OperationType == OperationType_DeleteFTPFile )
	{
	}
	else if( OperationType == OperationType_DoesFTPFileExist )
	{
	}
	else
	{
		//copy a nice message
		if( S_OK == hr && !strError )
		{
			switch( OperationType )
			{
			case OperationType_Connect:
				strError = "Connection Completed Successfully";
				break;
			case OperationType_Disconnect:
				strError = "Disconnection Completed Successfuly";
				break;
			case OperationType_Get:
				strError = "File Copy Operation Get Completed";
				break;
			case OperationType_Put:
				strError = "File Copy Operation Put Completed";
				break;
			case OperationType_MoveFTPFileToLocal:
				strError = "File Move Operation to Local Completed";
				break;
			case OperationType_MoveLocalFileToFTP:
				strError = "File Move Operation to FTP Completed";
				break;
			}
		}
	}

	//we are now exiting
	//close the thread handle and invalidate it
	pThread->Lock();
	CloseHandle( pThread->m_WorkerThread );
	pThread->m_WorkerThread = NULL;
	pThread->Unlock();

	//notify thread of completion
	pThread->Fire_OnCompleted( OperationType, hr, strError );

	CoUninitialize();

	ExitThread( hr );

	return hr;
}

BOOL FTPFileExists( HINTERNET hConnection, DWORD dwFlags, LPCTSTR pFileName )
{
	HINTERNET hFTPFile;
	hFTPFile = FtpOpenFile( hConnection, pFileName,
					GENERIC_READ, dwFlags, NULL );
	if( NULL != hFTPFile )
	{
		InternetCloseHandle( hFTPFile );
		return TRUE;
	}
	return FALSE;
}

BOOL CheckFileParameters( LPCTSTR pSource, LPCTSTR pDestination )
{
	if( !pSource || !*pSource || !pDestination || !*pDestination )
		return FALSE;
	return TRUE;
}

void ProcessError( DWORD dwError, LPCTSTR pstrError, CComBSTR & strError )
{
	strError = pstrError;
	strError += ": ";
	if( dwError >= INTERNET_ERROR_BASE &&
		dwError <= INTERNET_ERROR_LAST )
	{
		if( dwError == ERROR_INTERNET_EXTENDED_ERROR )
		{
			get_extended_internet_error_string ( strError );
		}
		else
		{
			get_internet_error_string ( strError, dwError );
		}
	}
    else
	{
		get_system_error_string ( strError, dwError );
	}
}

void get_system_error_string ( CComBSTR & strError, DWORD dwError )
{
	LPSTR	pText = NULL;
	DWORD dwResult;
	dwResult = ::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
				   FORMAT_MESSAGE_FROM_SYSTEM | 
				   FORMAT_MESSAGE_IGNORE_INSERTS, 
				   NULL, dwError, 
				   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				   (LPTSTR)&pText, 0, NULL );
	if( dwResult )
	{
		if ( pText )
		{
			strError += pText;
			LocalFree( pText );
		}
	}
}

void get_internet_error_string ( CComBSTR & strError, DWORD dwError )
{
	LPSTR	pText = NULL;
	DWORD dwResult;
	dwResult = ::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
				   FORMAT_MESSAGE_FROM_HMODULE | 
				   FORMAT_MESSAGE_IGNORE_INSERTS, 
				   GetModuleHandle("wininet.dll"), dwError, 
				   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				   (LPTSTR)&pText, 0, NULL );
	if( dwResult )
	{
		if ( pText )
		{
			strError += pText;
			LocalFree( pText );
		}
	}
}

void get_extended_internet_error_string( CComBSTR & strError )
{
	TCHAR buffer[1024];
	DWORD dwLength = 1024;
	DWORD dwError = 0;

	if( TRUE == InternetGetLastResponseInfo( &dwError, buffer, &dwLength ) )
	{
		buffer[ dwLength ] = 0;
		strError += buffer;
	}
}


