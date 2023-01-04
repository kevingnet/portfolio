#include "windows.h"
#include "NWConnect.h"


//DString szLine = "\r\n______________________________________________________________________________\r\n¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\r\n" ;
DString szBeginLine = "\r\n\r\n********************* NWConnect.dll *********************\r\n" ;
DString szEndLine	= "\r\n*********************************************************\r\n\r\n" ;

BOOL APIENTRY DllMain(HANDLE hInst, DWORD ul_reason_being_called, LPVOID lpReserved)
{
	DString::SetModuleInstance( hInst ) ;
    return 1;
        UNREFERENCED_PARAMETER(ul_reason_being_called);
        UNREFERENCED_PARAMETER(lpReserved);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: NWIsConnected
// PURPOSE:
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
LONG WINAPI NWIsConnected ( SERVER_INFO * si )
{
//      -----------------  DECLARATIONS  ------------------------

	DECLARE_GENERAL_VARIABLES
	CHAR				serverName		[NW_MAX_SERVER_NAME_LEN]	;
	CHAR				volName			[NW_MAX_VOLUME_NAME_LEN]	;
	CHAR				dirPath			[PATH_SIZE]					;
	NWCCConnInfo		connInfo									;

//      -----------------------  CODE  ------------------------
	INIT_SI_STRUCT
	log = szTemp.LoadString(IDS_LOG_FILE_ISCONNECTED) ;
	LOG_WRITE_HEADER
	NOVELL_INIT
	PARSE_PATH
	GET_CONNECTION_STATUS
	GET_NDS_TREE_STATUS
	LOG_WRITE_CONNECTION_STATUS
	NOVELL_TERM
	LOG_WRITE_FOOTER
	return ( bConnection ) ;
}
//////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: NWIsNDSLoggedIn
// PURPOSE:
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
LONG WINAPI NWIsNDSLoggedIn ( SERVER_INFO * si	)
{
//      -----------------  DECLARATIONS  ------------------------

	DECLARE_GENERAL_VARIABLES

//      -----------------------  CODE  ------------------------

	INIT_SI_STRUCT
	log = szTemp.LoadString(IDS_LOG_FILE_ISLOGGEDIN) ;
	LOG_WRITE_HEADER
	NOVELL_INIT
	GET_NDS_TREE_STATUS
	if ( si->lLoginState == LS_NOT_LOGGED_IN )
	{
		bConnection = FAILIURE ;
		log << szTemp.LoadString(IDS_LI_NONE) ;
	}
	else
	{
		bConnection = SUCCESS ;
		log << szTemp.LoadString(IDS_LI_NDS) ;
	}
	NOVELL_TERM
	LOG_WRITE_FOOTER
	return ( bConnection ) ;
}
//////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: NWNDSLogin
// PURPOSE:
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
LONG WINAPI NWNDSLogin ( SERVER_INFO * si	)
{
//      -----------------  DECLARATIONS  ------------------------

	DECLARE_GENERAL_VARIABLES

//      -----------------------  CODE  ------------------------

	INIT_SI_STRUCT
	log = szTemp.LoadString(IDS_LOG_FILE_LOGIN) ;
	LOG_WRITE_HEADER
	NOVELL_INIT
	GET_NDS_TREE_STATUS

	if ( si->lLoginState == LS_NOT_LOGGED_IN )
	{
		log << szTemp.LoadString(IDS_LI_NONE) ;
		NDS_LOGIN
		VERIFY_NDS_LOGIN
		if ( si->lLoginState == LS_NOT_LOGGED_IN )
		{
			log << szTemp.LoadString(IDS_LI_NONE) ;
			bConnection = FAILIURE ;
		}
		else
		{
			bConnection = SUCCESS ;
			log << szTemp.LoadString(IDS_LI_NDS) ;
		}
	}
	else
	{
		bConnection = SUCCESS ;
		log << szTemp.LoadString(IDS_LI_NDS) ;
	}

	NOVELL_TERM
	LOG_WRITE_FOOTER
	return ( bConnection ) ;
}
//////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: NWNDSLogout
// PURPOSE:
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
LONG WINAPI NWNDSLogout ( SERVER_INFO * si	)
{
//      -----------------  DECLARATIONS  ------------------------

	DECLARE_GENERAL_VARIABLES

//      -----------------------  CODE  ------------------------

	INIT_SI_STRUCT
	log = szTemp.LoadString(IDS_LOG_FILE_LOGOUT) ;
	LOG_WRITE_HEADER
	NOVELL_INIT
	GET_NDS_TREE_STATUS

	if ( si->lLoginState == LS_NOT_LOGGED_IN )
	{
		bConnection = SUCCESS ;
		log << szTemp.LoadString(IDS_LI_NONE) ;
	}
	else
	{
		log << szTemp.LoadString(IDS_LI_NDS) ;
		NDS_LOGOUT
		VERIFY_NDS_LOGIN
		if ( si->lLoginState == LS_NOT_LOGGED_IN )
		{
			bConnection = SUCCESS ;
			log << szTemp.LoadString(IDS_LI_NONE) ;
		}
		else
		{
			bConnection = FAILIURE ;
			log << szTemp.LoadString(IDS_LI_NDS) ;
		}
	}

	NOVELL_TERM
	LOG_WRITE_FOOTER
	return ( bConnection ) ;
}
//////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: NWNDSLogoutAndDisconnect
// PURPOSE:
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
LONG WINAPI NWNDSLogoutAndDisconnect ( SERVER_INFO * si	)
{
//      -----------------  DECLARATIONS  ------------------------

	DECLARE_GENERAL_VARIABLES
	UINT				i					=	0					;
	nuint32				iterator			=	0					;
	nuint32				connRef				=	0					;
	NWRCODE				rScanCode			=	0					;
	NWCCConnInfo		connInfo									;

//      -----------------------  CODE  ------------------------

	INIT_SI_STRUCT
	log = szTemp.LoadString(IDS_LOG_FILE_LOGOUT_DISCONNECT) ;
	LOG_WRITE_HEADER
	NOVELL_INIT
	GET_NDS_TREE_STATUS
	
	iterator = 0 ;
	log << szTemp.LoadString(IDS_CONN_DISCONNECT_ALL) ;
	rScanCode = NWCCScanConnRefs ( & iterator, & connRef ) ;
	if ( rScanCode != NO_MORE_ENTRIES )
	{
		NW_ERROR_IF( rScanCode, NWCCScanConnRefs )
	}
	while ( rScanCode == 0 )
	{
		rCode = NWCCGetAllConnRefInfo ( connRef					,
										NWCC_INFO_VERSION_1		,
										& connInfo				) ;
		NW_ERROR_IF( rCode, NWCCGetAllConnRefInfo )
		for ( i=0; i<=strlen(connInfo.treeName); i++ )
		{
			if ( connInfo.treeName[i] == '_' )
			{
				connInfo.treeName[i] = 0 ;
				break ;
			}
		}
		if ( szTree == ( _strupr( connInfo.treeName ) ) )
		{
			log << "\tDisconnecting: " << connInfo.serverName << "\r\n" ;
			rCode = NWCCSysCloseConnRef ( (nuint32)connRef ) ;
			NW_ERROR_IF ( rCode, NWCCSysCloseConnRef )
		}
		rScanCode = NWCCScanConnRefs ( & iterator, & connRef ) ;
		if ( rScanCode != NO_MORE_ENTRIES )
		{
			NW_ERROR_IF( rScanCode, NWCCScanConnRefs )
		}
	}

	if ( si->lLoginState == LS_NOT_LOGGED_IN )
	{
		bConnection = SUCCESS ;
		log << szTemp.LoadString(IDS_LI_NONE) ;
	}
	else
	{
		log << szTemp.LoadString(IDS_LI_NDS) ;
		NDS_LOGOUT
		VERIFY_NDS_LOGIN
		if ( si->lLoginState == LS_NOT_LOGGED_IN )
		{
			bConnection = SUCCESS ;
			log << szTemp.LoadString(IDS_LI_NONE) ;
		}
		else
		{
			bConnection = FAILIURE ;
			log << szTemp.LoadString(IDS_LI_NDS) ;
		}
	}

	NOVELL_TERM
	LOG_WRITE_FOOTER
	return ( bConnection ) ;
}
//////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: Connect
// PURPOSE:
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
LONG WINAPI NWConnect ( SERVER_INFO * si )
{
	LONG			bReturn					;

	bReturn = NWNDSConnectCurrentUser (si)  ;
	if ( bReturn == 0 )
	{
		bReturn = NWBinderyConnectUser (si) ;
		if ( bReturn == 0 )
		{
			bReturn = NWNDSConnectUser (si) ;
		}
	}
	return ( bReturn ) ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: NWBinderyConnectUser
// PURPOSE:
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
LONG WINAPI NWBinderyConnectUser ( SERVER_INFO * si )
{
//      -----------------  DECLARATIONS  ------------------------

	DECLARE_GENERAL_VARIABLES
	CHAR				serverName		[NW_MAX_SERVER_NAME_LEN]	;
	CHAR				volName			[NW_MAX_VOLUME_NAME_LEN]	;
	CHAR				dirPath			[PATH_SIZE]					;
	CHAR				userName		[NW_MAX_USER_NAME_LEN]		;
	nuint16				objType										;
	NWCCConnInfo		connInfo									;
	BOOL				bLogout						=	FALSE		;
	BOOL				bLogin						=	FALSE		;

//      -----------------------  CODE  ------------------------

	INIT_SI_STRUCT
	log = szTemp.LoadString(IDS_LOG_FILE_CONNECT_BINDERY) ;
	LOG_WRITE_HEADER
	NOVELL_INIT
	PARSE_PATH
	GET_CONNECTION_STATUS
	GET_NDS_TREE_STATUS
	LOG_WRITE_CONNECTION_STATUS
	si->lStatus = PROCESSED ;

	if ( connHandle )
	{
		switch ( connInfo.authenticationState )
		{
		case NWCC_AUTHENT_STATE_NONE :
			bLogin = TRUE ;
			break ;
		case NWCC_AUTHENT_STATE_BIND :
			log << szTemp.LoadString(IDS_CONN_GETTING_CREDENTIALS) ;
			cCode = NWGetObjectName (
									connHandle			,
									connInfo.userID		,
									(char*) & userName	,
									& objType			) ;
			if ( ! cCode )
			{	
				log << szTemp.LoadString(IDS_GOT_USER_NAME ) ;
				if ( szUser == ( strupr ( userName ) ) )
				{	
					log << szTemp.LoadString(IDS_USER_CREDENTIALS_MATCHED ) ;
				}
				else
				{	
					log << szTemp.LoadString(IDS_USER_CREDENTIALS_DID_NOT_MATCH ) ;
					bLogout = TRUE ;
					bLogin = TRUE ;
				}
			}
			else
			{	
				log << szTemp.LoadString(IDS_DID_NOT_GET_USER_NAME ) ;
				bLogout = TRUE ;
				bLogin = TRUE ;
			}
			break ;
		case NWCC_AUTHENT_STATE_NDS :
			log << szTemp.LoadString(IDS_CI_AS_NDS) ;
			bLogout = TRUE ;
			bLogin = TRUE ;
			log << szTemp.LoadString(IDS_CONN_UNLOCK) ;
			cCode = NWDSUnlockConnection ( connHandle ) ;
			if ( cCode != HANDLE_ALREADY_UNLICENSED )
			{
				NW_ERROR_IF ( cCode, NWDSUnlockConnection )
			}
			break ;
		}
	}
	else
	{
		log << szTemp.LoadString(IDS_CONN_CONNECT) ;
		rCode = NWCCOpenConnByName (
									0						,
									(pnstr8)serverName		,
									NWCC_NAME_FORMAT_BIND	,
									NWCC_OPEN_LICENSED		,
									NWCC_RESERVED			,
									& connHandle			) ;
		NW_ERROR_IF ( rCode, NWCCOpenConnByName				)
		bLogin = TRUE ;
	}

	if ( bLogout == TRUE )
	{
		if ( connInfo.licenseState != NWCC_NOT_LICENSED )
		{
			log << szTemp.LoadString(IDS_CONN_UNLICENSE) ;
			rCode = NWCCUnlicenseConn ( connHandle ) ;
			if ( rCode != HANDLE_ALREADY_UNLICENSED )
			{
				NW_ERROR_IF ( rCode, NWCCUnlicenseConn )
			}
			si->lLicenseState = 0 ;
		}
		log << szTemp.LoadString(IDS_CONN_BIND_LOGOUT) ;
		cCode = NWLogoutFromFileServer ( connHandle ) ;
	}

	if ( bLogin == TRUE )
	{
		log << szTemp.LoadString(IDS_CONN_BIND_LOGIN) ;
		cCode = NWLoginToFileServer (
									connHandle		,
									szUser,
									OT_USER			,
									szPassword	) ;
		NW_ERROR_IF ( cCode, NWLoginToFileServer	)
	}

	log << szTemp.LoadString(IDS_CONN_LICENSE) ;
	rCode = NWCCLicenseConn ( connHandle ) ;
	NW_ERROR_IF ( rCode, NWCCLicenseConn )

	log << szTemp.LoadString(IDS_CONN_MAKE_PERMANENT) ;
	rCode = NWCCMakeConnPermanent ( connHandle ) ;
	NW_ERROR_IF ( rCode, NWCCMakeConnPermanent	     )

	VERIFY_CONNECTION( NWCC_AUTHENT_STATE_BIND )
	GET_CONNECTION_STATUS
	LOG_WRITE_CONNECTION_STATUS
	DISCONNECT_IF_FAILED

	NOVELL_TERM
	LOG_WRITE_FOOTER
	return ( bConnection ) ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: NWNDSConnectCurrentUser
// PURPOSE:
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
LONG WINAPI NWNDSConnectCurrentUser ( SERVER_INFO * si	)
{
//      -----------------  DECLARATIONS  ------------------------

	DECLARE_GENERAL_VARIABLES
	CHAR				serverName		[NW_MAX_SERVER_NAME_LEN]	;
	CHAR				volName			[NW_MAX_VOLUME_NAME_LEN]	;
	CHAR				dirPath			[PATH_SIZE]					;
	NWCCConnInfo		connInfo									;

//      -----------------------  CODE  ------------------------

	INIT_SI_STRUCT
	log = szTemp.LoadString(IDS_LOG_FILE_CONNECT_CURRENT) ;
	LOG_WRITE_HEADER
	NOVELL_INIT
	PARSE_PATH
	GET_CONNECTION_STATUS
	GET_NDS_TREE_STATUS
	LOG_WRITE_CONNECTION_STATUS
	si->lStatus = PROCESSED ;

	if ( si->lLoginState == LS_NOT_LOGGED_IN )
	{
		bConnection = FAILIURE ;
		log << szTemp.LoadString(IDS_LI_NONE) ;
	}
	else
	{
		if ( connHandle )
		{
			if ( connInfo.serverVersion.major < VERSION_4_X_NDS )
			{
				log << szTemp.LoadString(IDS_CI_NDS_NOT) ;
			}
			else
			{
				switch ( connInfo.authenticationState )
				{
				case NWCC_AUTHENT_STATE_NONE :
					log << szTemp.LoadString(IDS_CONN_NDS_AUTHENTICATE) ;
					dscCode = NWDSAuthenticateConn ( dContext, connHandle ) ;
					NW_ERROR_IF ( dscCode, NWDSAuthenticateConn	)
					break ;
				case NWCC_AUTHENT_STATE_BIND :
					log << szTemp.LoadString(IDS_CI_AS_BIND) ;
					log << szTemp.LoadString(IDS_CONN_BIND_LOGOUT) ;
					cCode = NWLogoutFromFileServer ( connHandle ) ;
					NW_ERROR_IF ( cCode, NWLogoutFromFileServer	)

					log << szTemp.LoadString(IDS_CONN_NDS_AUTHENTICATE) ;
					dscCode = NWDSAuthenticateConn ( dContext, connHandle ) ;
					NW_ERROR_IF ( dscCode, NWDSAuthenticateConn	)
					break ;
				case NWCC_AUTHENT_STATE_NDS :
					log << szTemp.LoadString(IDS_CI_AS_NDS) ;
					break ;
				}
			}
		}
		else
		{
			log << szTemp.LoadString(IDS_CONN_CONNECT) ;
			rCode = NWCCOpenConnByName (
										0						,
										serverName				,
										NWCC_NAME_FORMAT_BIND	,
										NWCC_OPEN_LICENSED		,
										NWCC_TRAN_TYPE_IPX		,	
										& connHandle			) ;
			NW_ERROR_IF ( rCode, NWCCOpenConnByName				)
			dscCode = NWDSAuthenticateConn ( dContext, connHandle ) ;
			NW_ERROR_IF ( dscCode, NWDSAuthenticateConn	)
		}

		log << szTemp.LoadString(IDS_CONN_MAKE_PERMANENT) ;
		rCode = NWCCMakeConnPermanent ( connHandle ) ;
		NW_ERROR_IF ( rCode, NWCCMakeConnPermanent	     )

		VERIFY_CONNECTION( NWCC_AUTHENT_STATE_NDS )
		GET_CONNECTION_STATUS
		LOG_WRITE_CONNECTION_STATUS
		DISCONNECT_IF_FAILED
	}

	NOVELL_TERM
	LOG_WRITE_FOOTER
	return ( bConnection ) ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: NWNDSConnectUser
// PURPOSE:
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
LONG WINAPI NWNDSConnectUser ( SERVER_INFO * si	)
{
//      -----------------  DECLARATIONS  ------------------------

	DECLARE_GENERAL_VARIABLES
	NWCONN_HANDLE		tmpHandle									; 
	UINT				i					=	0					;
	nuint32				iterator			=	0					;
	nuint32				connRef				=	0					;
	NWRCODE				rScanCode			=	0					;
	NWCCConnInfo		connInfo									;
	CHAR				serverName		[NW_MAX_SERVER_NAME_LEN]	;
	CHAR				volName			[NW_MAX_VOLUME_NAME_LEN]	;
	CHAR				dirPath			[PATH_SIZE]					;
	CHAR				userName		[NW_MAX_USER_NAME_LEN]		;
	BOOL				bLogin						=	FALSE		;
	BOOL				bNDSAuthenticateServers		=	FALSE		;
	UINT				iNDSAuthenticatedServers	=	0			;
	DString				szMsgTitle									;
	DStringArray		aNDSAuthenticatedServers					;

//      -----------------------  CODE  ------------------------

	INIT_SI_STRUCT
	log = szTemp.LoadString(IDS_LOG_FILE_CONNECT_USER) ;
	LOG_WRITE_HEADER
	NOVELL_INIT
	PARSE_PATH
	SCAN_CURRENT_CONNECTIONS_SAVE
	GET_CONNECTION_STATUS
	GET_NDS_TREE_STATUS
	LOG_WRITE_CONNECTION_STATUS
	si->lStatus = PROCESSED ;

	if ( si->lLoginState == LS_NOT_LOGGED_IN )
	{
		bLogin = TRUE ;
	}
	else
	{
		log << szTemp.LoadString(IDS_CONN_CHECKING_CREDENTIALS) ;
		dscCode = NWDSWhoAmI ( dContext, userName ) ;
		NW_ERROR_IF ( dscCode, NWDSWhoAmI		  )
		if ( dscCode == 0 )
		{
			log << szTemp.LoadString(IDS_GOT_USER_NAME ) ;
			if ( _stricmp ( strupr ( userName ), strupr ( szUser ) ) == 0 )
			{	
				bConnection = SUCCESS ;
				log << szTemp.LoadString(IDS_USER_CREDENTIALS_MATCHED) ;
			}
			else
			{	
				bLogin = TRUE ;
				log << szTemp.LoadString(IDS_USER_CREDENTIALS_DID_NOT_MATCH) ;
			}
		}
		else
		{
			bLogin = TRUE ;
			log << szTemp.LoadString(IDS_DID_NOT_GET_USER_NAME) ;
		}

		if ( bLogin == TRUE && si->lLoginState == LS_LOGGED_IN )
		{
			switch ( si->lNDSCredentialsChange )
			{
				case 0:
					bLogin = FALSE ;
					bConnection = FAILIURE ;
					break ;
				case 1:
					bNDSAuthenticateServers = TRUE ;
					break ;
				default :
					if ( MessageBox ( NULL, szTemp.LoadString(IDS_NDS_LOGOUT_PROMPT), szMsgTitle.LoadString(IDS_NDS_LOGOUT_PROMPT_TITLE),
									  MB_YESNO | MB_ICONEXCLAMATION ) 
						 == IDNO ) 
					{
						bLogin = FALSE ;
						bConnection = FAILIURE ;
					}
					else
					{
						bNDSAuthenticateServers = TRUE ;
					}
					break ;
			}
		}
	}

	if ( bLogin == TRUE )
	{
		NDS_LOGOUT
		NDS_LOGIN
		VERIFY_NDS_LOGIN
		if ( si->lLoginState == LS_NOT_LOGGED_IN )
		{
			bConnection = FAILIURE ;
		}
		else
		{
			bConnection = SUCCESS ;
		}
	}

	if ( bConnection == FAILIURE )
	{
		log << szTemp.LoadString(IDS_LI_NONE) ;
	}
	else
	{
		log << szTemp.LoadString(IDS_LI_NDS) ;
		if ( connHandle == 0 )
		{
			log << szTemp.LoadString(IDS_CONN_CONNECT) ;
			rCode = NWCCOpenConnByName (
										0						,
										serverName				,
										NWCC_NAME_FORMAT_BIND	,
										NWCC_OPEN_LICENSED		,
										NWCC_TRAN_TYPE_IPX		,
										& connHandle			) ;
			NW_ERROR_IF ( rCode, NWCCOpenConnByName				)
			dscCode = NWDSAuthenticateConn ( dContext, connHandle ) ;
			NW_ERROR_IF ( dscCode, NWDSAuthenticateConn	)
		}
		else
		{
			switch ( (nuint)si->lAuthenticationState )
			{
			case NWCC_AUTHENT_STATE_NONE :
				log << szTemp.LoadString(IDS_CONN_NDS_AUTHENTICATE) ;
				dscCode = NWDSAuthenticateConn ( dContext, connHandle ) ;
				NW_ERROR_IF ( dscCode, NWDSAuthenticateConn	)
				break ;
			case NWCC_AUTHENT_STATE_BIND :
				log << szTemp.LoadString(IDS_CI_AS_BIND) ;
				log << szTemp.LoadString(IDS_CONN_BIND_LOGOUT) ;
				cCode = NWLogoutFromFileServer ( connHandle ) ;
				NW_ERROR_IF ( cCode, NWLogoutFromFileServer	)
				log << szTemp.LoadString(IDS_CONN_NDS_AUTHENTICATE) ;
				dscCode = NWDSAuthenticateConn ( dContext, connHandle ) ;
				NW_ERROR_IF ( dscCode, NWDSAuthenticateConn	)
				break ;
			case NWCC_AUTHENT_STATE_NDS :
				log << szTemp.LoadString(IDS_CI_AS_NDS) ;
				break ;
			}
		}

		log << szTemp.LoadString(IDS_CONN_MAKE_PERMANENT) ;
		rCode = NWCCMakeConnPermanent ( connHandle ) ;
		NW_ERROR_IF ( rCode, NWCCMakeConnPermanent	     )

		if ( bNDSAuthenticateServers == TRUE )
		{
			log << szTemp.LoadString(IDS_CONN_REAUTHENTICATE_ALL) ;
			for ( i = 0; i < iNDSAuthenticatedServers; i++ )
			{
				log << "\tObtaining connection handle for server: " << aNDSAuthenticatedServers[i] << "\r\n" ;
				tmpHandle = 0 ;
				rCode = NWCCOpenConnByName (
											0						,
											aNDSAuthenticatedServers[i],
											NWCC_NAME_FORMAT_BIND	,
											NWCC_OPEN_UNLICENSED	,
											NWCC_TRAN_TYPE_IPX		,	
											& tmpHandle				) ;
				NW_ERROR_IF ( rCode, NWCCOpenConnByName				)
				if ( rCode == 0 && connHandle != 0 )
				{
					log << szTemp.LoadString(IDS_CONN_NDS_AUTHENTICATE) ;
					dscCode = NWDSAuthenticate ( tmpHandle, 0, NULL ) ;
					NW_ERROR_IF ( dscCode, NWDSAuthenticate	 )
					log << szTemp.LoadString(IDS_CONN_MAKE_PERMANENT) ;
					rCode = NWCCMakeConnPermanent ( tmpHandle ) ;
					NW_ERROR_IF ( rCode, NWCCMakeConnPermanent	     )
				}
				else
				{
					log << szTemp.LoadString(IDS_CI_CONN_NOT) ;
				}
			}
		}
		VERIFY_CONNECTION( NWCC_AUTHENT_STATE_NDS )
		GET_CONNECTION_STATUS
		LOG_WRITE_CONNECTION_STATUS
		DISCONNECT_IF_FAILED
	}

	NOVELL_TERM
	LOG_WRITE_FOOTER
	return ( bConnection ) ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: Disconnect
// PURPOSE:
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
LONG WINAPI NWDisconnect ( SERVER_INFO * si	)
{
//      -----------------  DECLARATIONS  ------------------------

	DECLARE_GENERAL_VARIABLES
	UINT				i					=	0					;
	nuint32				iterator			=	0					;
	nuint32				connRef				=	0					;
	NWRCODE				rScanCode			=	0					;
	CHAR				serverName		[NW_MAX_SERVER_NAME_LEN]	;
	CHAR				volName			[NW_MAX_VOLUME_NAME_LEN]	;
	CHAR				dirPath			[PATH_SIZE]					;
	NWCCConnInfo		connInfo									;
	BOOL				bLogout						=	FALSE		;
	BOOL				bDisconnect					=	FALSE		;
	BOOL				bUnlicense					=	FALSE		;

//      -----------------------  CODE  ------------------------

	INIT_SI_STRUCT
	log = szTemp.LoadString(IDS_LOG_FILE_DISCONNECT) ;
	LOG_WRITE_HEADER
	NOVELL_INIT
	PARSE_PATH
	GET_CONNECTION_STATUS
	GET_NDS_TREE_STATUS
	LOG_WRITE_CONNECTION_STATUS

	bConnection = SUCCESS ;

	if ( connHandle == 0 )
	{
		log << szTemp.LoadString(IDS_CI_CONN_NOT) ;
	}
	else
	{
		if ( si->lStatus != PROCESSED )
		{
			bUnlicense = TRUE ;
			bDisconnect = TRUE ;
		}
		else
		{
			if ( si->lPriorLoginState == LS_NOT_LOGGED_IN  && si->lLoginState == LS_LOGGED_IN )
			{
				bLogout = TRUE ;
			}

			if ( si->lPriorConnectionState == CS_NOT_CONNECTED )
			{
				bUnlicense = TRUE ;
				bDisconnect = TRUE ;
			}
			else
			{
				if ( (LONG)si->lPriorLicenseState != NWCC_NOT_LICENSED )
				{
					bUnlicense = TRUE ;
				}
				switch ( (LONG)si->lPriorAuthenticationState )
				{
				case (LONG)NWCC_AUTHENT_STATE_NONE :
					break ;
				case (LONG)NWCC_AUTHENT_STATE_BIND :
					break ;
				case (LONG)NWCC_AUTHENT_STATE_NDS :
					break ;
				}
			}
		}
	}

	if ( bUnlicense == TRUE )
	{
		log << szTemp.LoadString(IDS_CONN_UNLICENSE) ;
		rCode = NWCCUnlicenseConn ( connHandle ) ;
		if ( rCode != HANDLE_ALREADY_UNLICENSED )
		{
			NW_ERROR_IF ( rCode, NWCCUnlicenseConn )
		}
		si->lLicenseState = 0 ;
	}
	else
	{
		log << szTemp.LoadString(IDS_FAILED_IGNORE_UNLICENSE) ;
	}
	
	if ( bDisconnect == TRUE )
	{
		log << szTemp.LoadString(IDS_CONN_DISCONNECT) ;
		switch ( connInfo.authenticationState )
		{
		case NWCC_AUTHENT_STATE_NONE :
			log << szTemp.LoadString(IDS_CI_AS_NONE) ;
			break ;
		case NWCC_AUTHENT_STATE_BIND :
			log << szTemp.LoadString(IDS_CI_AS_BIND) ;
			log << szTemp.LoadString(IDS_CONN_BIND_LOGOUT) ;
			cCode = NWLogoutFromFileServer ( connHandle ) ;
			NW_ERROR_IF ( cCode, NWLogoutFromFileServer	)
			break ;
		case NWCC_AUTHENT_STATE_NDS :
			log << szTemp.LoadString(IDS_CI_AS_NDS) ;
			break ;
		}
		log << szTemp.LoadString(IDS_CONN_CLOSE) ;
		rCode = NWCCSysCloseConnRef ( connInfo.connRef ) ;
		NW_WARNING_IF ( rCode, NWCCSysCloseConnRef )
		log << szTemp.LoadString(IDS_CONN_FREE_SLOT) ;
		cCode = NWFreeConnectionSlot ( connHandle, SYSTEM_DISCONNECT ) ;
		NW_WARNING_IF ( cCode, NWFreeConnectionSlot )
		log << szTemp.LoadString(IDS_CONN_DISCONNECT) ;
		rCode = NWCCCloseConn ( connHandle ) ;
		NW_WARNING_IF ( rCode, NWCCCloseConn )
		connHandle = 0 ;

		iterator = 0 ;
		bConnection = SUCCESS ;
		rScanCode = NWCCScanConnRefs ( & iterator, & connRef ) ;
		if ( rScanCode != NO_MORE_ENTRIES )
		{
			NW_ERROR_IF( rScanCode, NWCCScanConnRefs )
		}
		while ( rScanCode == 0 )
		{
			rCode = NWCCGetAllConnRefInfo ( connRef					,
											NWCC_INFO_VERSION_1		,
											& connInfo				) ;
			NW_ERROR_IF( rCode, NWCCGetAllConnRefInfo )
			if ( serverName == ( _strupr( connInfo.serverName ) ) )
			{
				bConnection = FAILIURE ;
			}
			rScanCode = NWCCScanConnRefs ( & iterator, & connRef ) ;
			if ( rScanCode != NO_MORE_ENTRIES )
			{
				NW_ERROR_IF( rScanCode, NWCCScanConnRefs )
			}
		}
	}
	else
	{
		log << szTemp.LoadString(IDS_FAILED_IGNORE_DISCONNECT) ;
	}
	
	if ( bLogout == TRUE )
	{
		if ( si->lIsNDSCapable == 1 )
		{
			log << szTemp.LoadString(IDS_FAILED_LOGOUT) ;
			NDS_LOGOUT
			VERIFY_NDS_LOGIN
			si->lLoginState = LS_NOT_LOGGED_IN ;
		}
	}
	else
	{
		log << szTemp.LoadString(IDS_FAILED_IGNORE_LOGOUT) ;
	}

	GET_CONNECTION_STATUS
	LOG_WRITE_CONNECTION_STATUS

	si->lStatus = UNPROCESSED ;
	NOVELL_TERM
	LOG_WRITE_FOOTER
	return ( bConnection ) ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
