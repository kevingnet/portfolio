/*					_asm        
					{  
						push ecx
						push edx

						mov		ecx, dword ptr[this]
						add		ecx, 74h
						push	ecx
						mov		edx, dword ptr[this]
						call	dword ptr[edx+64h]
						add		esp, 4
						mov		dword ptr[lReturn], eax

						pop edx
						pop ecx
					}
*/

#include "DConnection.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: Class Constructor
// PURPOSE:
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
DConnection::DConnection ( HANDLE hLogFile )
{
	m_bDLLIsLoaded = FALSE ;
	m_hLogFile = hLogFile ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: Class Destructor
// PURPOSE:
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
DConnection::~DConnection()
{
	if ( m_bDLLIsLoaded == TRUE )
	{
		if ( FreeLibrary ( hinstNWConnectLib ) == FALSE )
		{
			m_szLastError.GetErrorString( "\r\nWARNING! FreeLibrary NWConnect.dll ",
										 GetLastError() ) ;
			m_szLastError += "\r\n\r\n" ; 
			WriteLog(m_szLastError) ;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: LoadDLL
// PURPOSE:
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL DConnection::LoadDLL ()
{
	hinstNWConnectLib = LoadLibrary("CalWin32.dll");
	if ( hinstNWConnectLib == NULL )
	{
		WriteLog("\r\nNetware client was not found. Connecting via MS API\r\n") ;
		m_bDLLIsLoaded = FALSE ;
		return ( FALSE ) ;
	}
	FreeLibrary ( hinstNWConnectLib ) ;

	hinstNWConnectLib = LoadLibrary("NWConnect.dll");
	if ( hinstNWConnectLib == NULL )
	{
		WriteLog("\r\nERROR! LoadLibrary NWConnect.dll ") ;
		m_szLastError.GetErrorString( GetLastError() ) ;
		m_szLastError += "\r\n\r\n" ; 
		WriteLog(m_szLastError) ;
		m_bDLLIsLoaded = FALSE ;
		return ( FALSE ) ;
	}
	else
	{
		m_bDLLIsLoaded = TRUE ;
		NWNDSConnectCurrentUser = (NWNDSCONNECTCURRENTUSER) GetProcAddress ( hinstNWConnectLib, "NWNDSConnectCurrentUser" ) ;  
		if ( NWNDSConnectCurrentUser == NULL ) 
		{
			WriteLog("\r\nERROR! GetProcAddress NWNDSConnectCurrentUser ") ;
			m_szLastError.GetErrorString( GetLastError() ) ;
			m_szLastError += "\r\n\r\n" ; 
			WriteLog(m_szLastError) ;
			m_bDLLIsLoaded = FALSE ;
			return ( FALSE ) ;
		}
		NWBinderyConnectUser = (NWBINDERYCONNECTUSER) GetProcAddress ( hinstNWConnectLib, "NWBinderyConnectUser" ) ;  
		if ( NWBinderyConnectUser == NULL ) 
		{
			WriteLog("\r\nERROR! GetProcAddress NWBinderyConnectUser ") ;
			m_szLastError.GetErrorString( GetLastError() ) ;
			m_szLastError += "\r\n\r\n" ; 
			WriteLog(m_szLastError) ;
			m_bDLLIsLoaded = FALSE ;
			return ( FALSE ) ;
		}
		NWNDSConnectUser = (NWNDSCONNECTUSER) GetProcAddress ( hinstNWConnectLib, "NWNDSConnectUser" ) ;  
		if ( NWNDSConnectUser == NULL ) 
		{
			WriteLog("\r\nERROR! GetProcAddress NWNDSConnectUser ") ;
			m_szLastError.GetErrorString( GetLastError() ) ;
			m_szLastError += "\r\n\r\n" ; 
			WriteLog(m_szLastError) ;
			m_bDLLIsLoaded = FALSE ;
			return ( FALSE ) ;
		}
		NWDisconnect = (NWDISCONNECT) GetProcAddress ( hinstNWConnectLib, "NWDisconnect" ) ;  
		if ( NWDisconnect == NULL ) 
		{
			WriteLog("\r\nERROR! GetProcAddress NWDisconnect ") ;
			m_szLastError.GetErrorString( GetLastError() ) ;
			m_szLastError += "\r\n\r\n" ; 
			WriteLog(m_szLastError) ;
			m_bDLLIsLoaded = FALSE ;
			return ( FALSE ) ;
		}
	}
	return ( TRUE ) ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: Assignment Operator
// PURPOSE:
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
DConnection& DConnection::operator=( SERVER_INFO * si )
{
	CHAR szBuffer [MAX_COMPUTERNAME_LENGTH + 1] ;
	m_bMSConnect = FALSE ;

	m_si->lLogFileHandle			= (LONG)m_hLogFile	;
	m_si->lStatus					= 0					;
	m_si->lIsNDSCapable				= UNINITIALIZED		;
	m_si->lConnRef					= UNINITIALIZED		;
	m_si->lConnNum					= UNINITIALIZED		;
	m_si->lVersionMajor				= UNINITIALIZED		;
	m_si->lVersionMinor				= UNINITIALIZED		;
	m_si->lDistance					= UNINITIALIZED		;
	m_si->lPriorLoginState			= UNINITIALIZED		;
	m_si->lPriorConnectionState		= UNINITIALIZED		;
	m_si->lPriorAuthenticationState	= UNINITIALIZED		;
	m_si->lPriorLicenseState		= UNINITIALIZED		;
	m_si->lPriorBroadcastState		= UNINITIALIZED		;
	m_si->lLoginState				= UNINITIALIZED		;
	m_si->lConnectionState			= UNINITIALIZED		;
	m_si->lAuthenticationState		= UNINITIALIZED		;
	m_si->lLicenseState				= UNINITIALIZED		;
	m_si->lBroadcastState			= UNINITIALIZED		;

	m_si->lOptions					= si->lOptions		;
	m_si->szTitle					= si->szTitle		;
	m_si->szTree					= si->szTree		;
	m_si->szContext					= si->szContext		;
	m_si->szFullPath				= si->szFullPath	;
	m_si->szUser					= si->szUser		;
	m_si->szPassword				= si->szPassword	;

	if ( m_bDLLIsLoaded == FALSE )
	{
		m_si->lOptions = MS_NET_ONLY ;
	}

	if ( m_szComputerName == "" )
	{
		DWORD nBufferSize	;
		nBufferSize = MAX_COMPUTERNAME_LENGTH + 1 ;
		GetComputerName ( szBuffer, (LPDWORD)& nBufferSize ) ;
		m_szComputerName = szBuffer ;
		m_szComputerName.UpperCase() ;
	}

	DString szServer ;
	DStringArray sa ;
	szServer = m_si->szFullPath ;
	sa.TokenizeCompact(szServer, "\\") ;
	szServer = sa[0] ;
	szServer.UpperCase() ;
	if ( szServer == m_szComputerName )
	{
		m_bLocal = TRUE ;
		WriteLog(m_szLastError.LoadString(IDS_SERVER_IS_LOCAL)) ;
	}
	else
	{
		m_bLocal = FALSE ;
	}
	m_bMSConnect = FALSE ;
	return * this ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: Connect
// PURPOSE:
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL DConnection::Connect ()
{

//      -----------------  DECLARATIONS  ------------------------

	BOOL						bReturn						;
	LONG						lReturn						;

//      -----------------------  CODE  ------------------------

	if ( m_bLocal == TRUE )
	{
		return ( TRUE ) ;
	}

	switch ( m_si->lOptions )
	{
	case 0 :
		WriteLog("\r\n");
		WriteLog(m_szLastError.LoadString(IDS_CONNECTING_NDS_CUR)) ;
		lReturn = NWNDSConnectCurrentUser ( m_si ) ;
		if ( lReturn != SUCCESS )
		{
			WriteLog("\r\n");
			WriteLog(m_szLastError.LoadString(IDS_CONNECTING_BINDERY)) ;
			lReturn = NWBinderyConnectUser ( m_si ) ;
			if ( lReturn != SUCCESS )
			{
				WriteLog("\r\n");
				WriteLog(m_szLastError.LoadString(IDS_CONNECTING_MSAPI)) ;
				m_bMSConnect = TRUE ;
				bReturn = MSConnect ( m_si ) ;
				if ( bReturn == FALSE )
				{
					WriteLog("\r\n");
					WriteLog(m_szLastError.LoadString(IDS_CONNECTING_NDS)) ;
					m_bMSConnect = FALSE ;
					lReturn = NWNDSConnectUser ( m_si ) ;
				}
				else
				{
					lReturn = SUCCESS ;
				}
			}
		}
		break ;
	case 1 :
		WriteLog("\r\n");
		WriteLog(m_szLastError.LoadString(IDS_CONNECTING_MSAPI)) ;
		m_bMSConnect = TRUE ;
		bReturn = MSConnect ( m_si ) ;
		if ( bReturn == FALSE )
		{
			m_bMSConnect = FALSE ;
			WriteLog("\r\n");
			WriteLog(m_szLastError.LoadString(IDS_CONNECTING_NDS_CUR)) ;
			lReturn = NWNDSConnectCurrentUser ( m_si ) ;
			if ( lReturn != SUCCESS )
			{
				WriteLog("\r\n");
				WriteLog(m_szLastError.LoadString(IDS_CONNECTING_BINDERY)) ;
				lReturn = NWBinderyConnectUser ( m_si ) ;
				if ( lReturn != SUCCESS )
				{
					WriteLog("\r\n");
					WriteLog(m_szLastError.LoadString(IDS_CONNECTING_NDS)) ;
					lReturn = NWNDSConnectUser ( m_si ) ;
				}
			}
		}
		else
		{
			lReturn = SUCCESS ;
		}
		break ;
	case 2 :
		WriteLog("\r\n");
		WriteLog(m_szLastError.LoadString(IDS_CONNECTING_NDS_CUR)) ;
		lReturn = NWNDSConnectCurrentUser ( m_si ) ;

		if ( lReturn != SUCCESS )
		{
			WriteLog("\r\n");
			WriteLog(m_szLastError.LoadString(IDS_CONNECTING_BINDERY)) ;
			lReturn = NWBinderyConnectUser ( m_si ) ;
			if ( lReturn != SUCCESS )
			{
				WriteLog("\r\n");
				WriteLog(m_szLastError.LoadString(IDS_CONNECTING_NDS)) ;
				lReturn = NWNDSConnectUser ( m_si ) ;
			}
		}
		break ;
	case MS_NET_ONLY :
		WriteLog("\r\n");
		WriteLog(m_szLastError.LoadString(IDS_CONNECTING_MSAPI)) ;
		m_bMSConnect = TRUE ;
		bReturn = MSConnect ( m_si ) ;
		if ( bReturn == FALSE )
		{
			lReturn = FAILIURE ;
		}
		else
		{
			lReturn = SUCCESS ;
		}
		break ;
	case 4 :
		WriteLog("\r\n");
		WriteLog(m_szLastError.LoadString(IDS_CONNECTING_BINDERY)) ;
		lReturn = NWBinderyConnectUser ( m_si ) ;
		if ( lReturn != SUCCESS )
		{
			WriteLog("\r\n");
			WriteLog(m_szLastError.LoadString(IDS_CONNECTING_NDS)) ;
			lReturn = NWNDSConnectUser ( m_si ) ;
		}
		break ;
	case 5 :
		WriteLog("\r\n");
		WriteLog(m_szLastError.LoadString(IDS_CONNECTING_NDS)) ;
		lReturn = NWNDSConnectUser ( m_si ) ;
		break ;
	}
	if ( lReturn != SUCCESS )
	{
		return ( FALSE ) ;
	}
	else
	{
		return ( TRUE ) ;
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: Disconnect
// PURPOSE:
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL DConnection::Disconnect ()
{
//      -----------------  DECLARATIONS  ------------------------

	LONG						lReturn							;
	DString						szPrompt						;
	DString						szServerPath					;
	DWORD						dwResult						;
	DString						szTemp							;
	DString						szError							;
    BOOL						bTryMS		= FALSE				;  
	DString szServer ;

//      -----------------------  CODE  ------------------------

	if ( m_bLocal == TRUE )
	{
		return ( TRUE ) ;
	}

	if ( m_bMSConnect == FALSE )
	{
		WriteLog(m_szLastError.LoadString(IDS_DISCONNECTING_NW)) ;
		lReturn = NWDisconnect ( m_si ) ;
		if ( lReturn != SUCCESS )
		{
			WriteLog(m_szLastError.LoadString(IDS_FAILED_DISCONNECT_NW)) ;
			bTryMS = TRUE ;
		}
		else
		{
			return ( TRUE ) ;
		}
	}
	else
	{
		bTryMS = TRUE ;
	}
	
	WriteLog("\r\n");
	if ( bTryMS == TRUE )
	{
		WriteLog(m_szLastError.LoadString(IDS_DISCONNECTING_MSAPI)) ;

		szServer = m_si->szFullPath ;
		szServer.SetMid ( 2, szServer.Find ( '\\', 3 ) - 2 ) ;
		szServer.UpperCase() ;

		dwResult = WNetCancelConnection2( szServer, 0 , TRUE ) ;
		if( dwResult != NO_ERROR )
		{
			WriteLog("WNetCancelConnection2 ") ;
			WriteLog(GetMSNetError(m_dwLastError)) ;
			WriteLog(m_szLastError.LoadString(IDS_FAILED_DISCONNECT_MSAPI)) ;
			return ( FALSE ) ;
		}
		if ( IsConnected () == TRUE )
		{
			WriteLog(m_szLastError.LoadString(IDS_FAILED_DISCONNECT_MSAPI)) ;
			return ( FALSE ) ;
		}
		else
		{
			return ( TRUE ) ;
		}
	}
	else
	{
		return ( FALSE ) ;
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: IsConnected
// PURPOSE:
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL DConnection::IsConnected ()
{
	DString szTemp ;
	INT iStart = 0 ;
	if ( m_bLocal == TRUE )
	{
		return ( TRUE ) ;
	}
	m_bServerFound = FALSE;

	szTemp = m_si->szFullPath ;
	szTemp.SetMid ( 2, szTemp.Find ( '\\', 3 ) - 2 ) ;
	szTemp.UpperCase() ;
	strcpy(m_szFindServer,szTemp);

	EnumerateFunc ( NULL ) ;
	if ( m_bServerFound )
	{
		m_bServerFound = FALSE ;
		return ( TRUE ) ;
	}
	else
	{
		m_bServerFound = FALSE ;
		return ( FALSE ) ;
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: MSConnect
// PURPOSE:
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL DConnection::MSConnect ( SERVER_INFO_OUT * psio )
{
//      -----------------  DECLARATIONS  ------------------------

	DWORD					dwResult							;
	NETRESOURCE				nr									;
	DString					szServerPath						;
	DString					dbgBuffer							;
	DString szServer ;
	INT iStart = 0 ;

//      -----------------------  CODE  ------------------------

	if ( m_bLocal == TRUE )
	{
		return ( TRUE ) ;
	}

	if ( IsConnected () == TRUE )
	{
		psio->lPriorConnectionState = 1 ;
		return ( TRUE ) ;
	}
	
	szServer = psio->szFullPath ;
	szServer.SetMid ( 0, szServer.Find ( '\\', 4 ) ) ;
	szServer.UpperCase() ;

	nr.dwType			= RESOURCETYPE_DISK		;												
    nr.lpLocalName		= NULL					;												
	nr.lpRemoteName		= szServer				;												
    nr.lpProvider		= NULL					;			

	dwResult = WNetAddConnection2 ( 															
									& nr,														
									psio->szPassword,								
									psio->szUser ,									
									0 ) ;														
	if ( dwResult )
	{
		WriteLog("\r\n") ; 
		WriteLog("WNetAddConnection2 ") ; 
		WriteLog(GetMSNetError(dwResult)) ;
		WriteLog("\r\n") ; 
		dwResult = WNetAddConnection2 ( 															
										& nr,														
										NULL,								
										NULL,									
										0 ) ;														
		if ( dwResult )
		{
			WriteLog("\r\n") ; 
			WriteLog("WNetAddConnection2 ") ; 
			WriteLog(GetMSNetError(dwResult)) ;
			WriteLog("\r\n") ; 
			WriteLog(m_szLastError.LoadString(IDS_FAILED_CONNECT_MSAPI)) ;
			return ( FALSE ) ;
		}
		else
		{
			WriteLog("\r\nConnected via MS-API with current credentials.\r\n") ; 
			return ( TRUE ) ;
		}
	}
	else
	{
		WriteLog("\r\nConnected via MS-API.\r\n") ; 
		return ( TRUE ) ;
	}
}

// M S   C O D E

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: EnumerateFunc
// PURPOSE:
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL DConnection::EnumerateFunc( LPNETRESOURCE lpnr )
{

    DWORD			dwResult				;
    DWORD			dwResultEnum			;
    HANDLE			hEnum					;
    DWORD			cbBuffer = BUFFER_SIZE	;
    DWORD			cEntries = ALL_ENTRIES	;
    LPNETRESOURCE	lpnrLocal				;
    DWORD			i						;
	char *			pdest					;
	char			szThisServer [ MAX_COMPUTERNAME_LENGTH + 1 ] ;

 	dwResult = WNetOpenEnum(RESOURCE_CONNECTED, 
							RESOURCETYPE_ANY,         
							0, 
							lpnr, 
							&hEnum);
	if ( dwResult )	
	{
		return ( FALSE ) ;
	}

    do 
	{
		lpnrLocal = (LPNETRESOURCE) GlobalAlloc ( GPTR, cbBuffer ) ;
		if ( lpnrLocal == NULL )			
		{																	
			return ( FALSE ) ;
		}																	

        dwResultEnum = WNetEnumResource(
										hEnum		,
										& cEntries	,
										lpnrLocal	,
										& cbBuffer	) ;

        if ( dwResultEnum == NO_ERROR ) 
		{ 
			for ( i=0; i < cEntries; i++ ) 
			{
				strcpy ( szThisServer, _strupr ( lpnrLocal[i].lpRemoteName ) ) ;
				pdest = strstr ( szThisServer, m_szFindServer ) ;
				if( pdest != NULL )
				{
					m_bServerFound = TRUE ;
					GlobalFree ( (HGLOBAL) lpnrLocal ) ;
					dwResult = WNetCloseEnum ( hEnum ) ;
					if ( dwResult )	
					{
						return ( FALSE ) ;
					}
					return ( TRUE ) ;
				}
				if ( RESOURCEUSAGE_CONTAINER == ( lpnrLocal[i].dwUsage & RESOURCEUSAGE_CONTAINER ) )
				{
					if ( ! EnumerateFunc ( & lpnrLocal[i] ) )
					{
						return ( FALSE ) ;
					}
				}
            }         
		}  
        else if ( dwResultEnum != ERROR_NO_MORE_ITEMS ) 
		{ 
            break;         
		}     
    }
    while ( dwResultEnum != ERROR_NO_MORE_ITEMS ) ;

    GlobalFree ( (HGLOBAL) lpnrLocal) ;
    dwResult = WNetCloseEnum ( hEnum ) ;
    if ( dwResult ) 
	{
        return FALSE ;
    }
    return TRUE ;
}

/*

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: ListConnections
// PURPOSE:
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
void DConnection::ListConnections ()
{
	WriteLog("\r\nEnumerating current connections...\r\n") ;
	EnumerateConnections ( NULL ) ;
	WriteLog("\r\n\r\n") ;
}
//////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: EnumerateConnections
// PURPOSE:
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL DConnection::EnumerateConnections ( LPNETRESOURCE lpnr )
{

    DWORD			dwResult				;
    DWORD			dwResultEnum			;
    HANDLE			hEnum					;
    DWORD			cbBuffer = BUFFER_SIZE	;
    DWORD			cEntries = ALL_ENTRIES	;
    LPNETRESOURCE	lpnrLocal				;
    DWORD			i						;

	// RESOURCE_CONTEXT RESOURCE_GLOBALNET       RESOURCE_CONNECTED
 	dwResult = WNetOpenEnum(RESOURCE_CONNECTED, 
							RESOURCETYPE_ANY,         
							0, 
							lpnr, 
							&hEnum);
	if ( dwResult )	
	{
		return ( FALSE ) ;
	}

    do 
	{
		lpnrLocal = (LPNETRESOURCE) GlobalAlloc ( GPTR, cbBuffer ) ;
		if ( lpnrLocal == NULL )			
		{																	
			return ( FALSE ) ;
		}																	

        dwResultEnum = WNetEnumResource(
										hEnum		,
										& cEntries	,
										lpnrLocal	,
										& cbBuffer	) ;

        if ( dwResultEnum == NO_ERROR ) 
		{ 
			for ( i=0; i < cEntries; i++ ) 
			{
				WriteLog( "\t" ) ;
				WriteLog( lpnrLocal[i].lpRemoteName ) ;
				WriteLog( "\r\n" ) ;
				m_saConnections.Add( lpnrLocal[i].lpRemoteName ) ;
				if ( RESOURCEUSAGE_CONTAINER == ( lpnrLocal[i].dwUsage & RESOURCEUSAGE_CONTAINER ) )
				{
					if ( ! EnumerateConnections ( & lpnrLocal[i] ) )
					{
						return ( FALSE ) ;
					}
				}
            }         
		}  
        else if ( dwResultEnum != ERROR_NO_MORE_ITEMS ) 
		{ 
            break;         
		}     
    }
    while ( dwResultEnum != ERROR_NO_MORE_ITEMS ) ;

    GlobalFree ( (HGLOBAL) lpnrLocal) ;
    dwResult = WNetCloseEnum ( hEnum ) ;
    if ( dwResult ) 
	{
        return FALSE ;
    }
    return TRUE ;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////////////

DString & DConnection::GetMSNetError( LONG lError )
{
switch ( lError )
{
case ERROR_OUTOFMEMORY :
	m_szLastError = "Memory allocation error. Out of memory." ;
	break;
case ERROR_ACCESS_DENIED :
	m_szLastError = "Access to the network resource was denied." ;
	break;
case ERROR_ALREADY_ASSIGNED :
	m_szLastError = "The local device (drive letter) is already connected to a network resource." ;
	break;
case ERROR_BAD_DEV_TYPE :
	m_szLastError = "The type of local device and the type of network resource do not match." ;
	break;
case ERROR_BAD_DEVICE :
	m_szLastError = "The drive letter is invalid." ;
	break;
case ERROR_BAD_NET_NAME :
	m_szLastError = "The path is not acceptable to any network resource provider. The resource name is invalid, or the resource cannot be located." ;
	break;
case ERROR_BAD_PROFILE :
	m_szLastError = "The user profile is in an incorrect format." ;
	break;
case ERROR_BAD_PROVIDER :
	m_szLastError = "The value specified by lpProvider does not match any provider." ;
	break;
case ERROR_BUSY :
	m_szLastError = "The router or provider is busy, possibly initializing. The caller should retry." ;
	break;
case ERROR_CANCELLED :
	m_szLastError = "	The attempt to make the connection was cancelled by the user, or by a called resource." ;
	break;
case ERROR_CANNOT_OPEN_PROFILE :
	m_szLastError = "The system is unable to open the user profile to process persistent connections." ;
	break;
case ERROR_DEVICE_ALREADY_REMEMBERED :
	m_szLastError = "An entry for the drive letter specified is already in the user profile." ;
	break;
case ERROR_DEVICE_IN_USE :
	m_szLastError = "The device is in use by an active process and cannot be disconnected." ;
	break;
case ERROR_INVALID_HANDLE :
	m_szLastError = "hEnum is not a valid handle." ;
	break;
case ERROR_INVALID_PASSWORD :
	m_szLastError = "The specified password is invalid." ;
	break;
case ERROR_INVALID_PARAMETER :
	m_szLastError = "Either the fdwScope or fdwType parameter is invalid, or there is a bad combination of parameters." ;
	break;
case ERROR_NO_NET_OR_BAD_PATH :
	m_szLastError = "	A network component has not started, or the specified name could not be handled." ;
	break;
case ERROR_NO_NETWORK :
	m_szLastError = "There is no network present." ;
	break;
case ERROR_NOT_CONNECTED :
	m_szLastError = "The system is not currently connected to the device." ;
	break;
case ERROR_NOT_CONTAINER :
	m_szLastError = "The lpNetResource parameter does not point to a container." ;
	break;
case ERROR_OPEN_FILES :
	m_szLastError = "There are open files, and the fForce parameter is FALSE." ;
	break;

case ERROR_EXTENDED_ERROR :
default:

	DWORD dwWNetResult		= 0					;
	DWORD dwResult			= 0					;
    char szDescription	[256]	;
    char szProvider		[256]	;

	dwWNetResult = WNetGetLastError (
									& dwResult				,
									(LPSTR) szDescription	,
									sizeof( szDescription )	,
									(LPSTR) szProvider		,
									sizeof( szProvider )	) ;

	if( dwWNetResult ) 
	{
		m_szLastError = "Unable to obtain network error. I'm so sorry!" ;
	}
	else
	{
		m_szLastError = szDescription ;
		m_szLastError += " { NETWORK PROVIDER: " ;
		m_szLastError += szProvider ;
		m_szLastError += " }" ;
	} 
	break;
}

return m_szLastError ;

}
