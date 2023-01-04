#include "stdafx.h"
#include "CMAPI.h"
#include "PacketRouter.h"

//DLL file name and function names in MAPI dll
char szMAPI32DLL				[] = "MAPI32.DLL";
char szMAPIInitialize			[] = "MAPIInitialize";
char szMAPIUninitialize			[] = "MAPIUninitialize";
char szMAPILogonEx				[] = "MAPILogonEx";

char szMAPIFreeBuffer			[] = "MAPIFreeBuffer";
char szMAPIAllocateMore			[] = "MAPIAllocateMore";
char szMAPIAllocateBuffer		[] = "MAPIAllocateBuffer";
char szOpenStreamOnFile			[] = "OpenStreamOnFile";
char szFreePadrlist				[] = "FreePadrlist@4";
char szFreeProws				[] = "FreeProws@4";
char szHrGetOneProp				[] = "HrGetOneProp@12";
char szHrQueryAllRows			[] = "HrQueryAllRows@24";

char szMFUNC_CONSTRUCTOR			[] = "MAPI Constructor() - ";
char szMFUNC_DESTRUCTOR				[] = "MAPI Destructor() - ";
char szMFUNC_INITIALIZE				[] = "MAPI Initialize() - ";
char szMFUNC_LOGON					[] = "MAPI Logon() - ";
char szMFUNC_LOGOFF					[] = "MAPI Logoff() - ";
char szMFUNC_READMESSAGE			[] = "MAPI ReadMessage() - ";
char szMFUNC_SENDMESSAGE			[] = "MAPI SendMessage() - ";
char szMFUNC_SENDMAIL				[] = "MAPI SendMail() - ";
char szMFUNC_REGNOTIFICATIONS		[] = "MAPI ReadNotification() - ";

char szFUNC_NULL					[] = "";
char szMESSAGE_INITIALIZED			[] = "Initialized: ";
char szMESSAGE_LOGGEDON				[] = "Logged on as: '";
char szMESSAGE_LOGGEDOFF			[] = "Logged off ";
char szMESSAGE_SENT					[] = "Message was sent ";
char szMESSAGE_RED					[] = "Message was read ";

char szFUNC_NONE					[] = "Possible Assignment or initialization Error: ";
char szFUNC_LOADLIBRARY				[] = "LoadLibrary() : ";
char szFUNC_GETPROCADDRESS			[] = "GetProcAddress() : ";
char szFUNC_FREELIBRARY				[] = "FreeLibrary() : ";
char szFUNC_MAPIINIT				[] = "MAPIInitialize() : ";
char szFUNC_MAPIUNINIT				[] = "MAPIUninitialize() : ";
char szFUNC_MAPIINITUTIL			[] = "ScInitMapiUtil() : ";
char szFUNC_MAPIUNINITUTIL			[] = "DeinitMapiUtil() : ";
char szFUNC_MAPILOGON				[] = "MAPILogonEx() : ";
char szFUNC_STORELOGOFF				[] = "StoreLogoff() : ";
char szFUNC_SESSIONLOGOFF			[] = "Session Logoff() : ";
char szFUNC_ALLOCBUFF				[] = "MAPIAllocBuff() : ";
char szFUNC_FREEBUFF				[] = "MAPIFreeBuff() : ";
char szFUNC_OPENMSGSTORE			[] = "OpenMsgStore() : ";
char szFUNC_OPENADDRESSBOOK			[] = "OpenAddressBook() : ";
char szFUNC_OPENENTRY				[] = "OpenEntry() : ";
char szFUNC_QUERYIDENTITY			[] = "QueryIdentity() : ";
char szFUNC_HRGETONEPROP			[] = "HrGetOneProp() : ";
char szFUNC_HRQUERYALLROWS			[] = "HrQueryAllRows() : ";
char szFUNC_GETMSGSTORETABLES		[] = "GetMsgStoreTables() : ";
char szFUNC_SEEKROW					[] = "SeekRow() : ";
char szFUNC_SETCOLUMNS				[] = "SetColumns() : ";
char szFUNC_GETPROPS				[] = "GetProps() : ";
char szFUNC_SETPROPS				[] = "SetProps() : ";
char szFUNC_FREEPROWS				[] = "FreeProws() : ";
char szFUNC_GETRECEIVEFOLDER		[] = "GetReceiveFolder() : ";
char szFUNC_ADVISE					[] = "Advise() : ";
char szFUNC_UNADVISE				[] = "Unadvise() : ";
char szFUNC_PREPAREFORM				[] = "PrepareForm() : ";
char szFUNC_SHOWFORM				[] = "ShowForm() : ";
char szFUNC_CREATEMESSAGE			[] = "CreateMessage() : ";
char szFUNC_CREATEONEOFF			[] = "CreateOneOff() : ";
char szFUNC_CREATEATTACH			[] = "CreateAttach() : ";
char szFUNC_MODIFYRECIPIENTS		[] = "ModifyRecipients() : ";
char szFUNC_RESOLVENAME				[] = "ResolveNames() : ";
char szFUNC_SAVECHANGES				[] = "SaveChanges() : ";
char szFUNC_SUBMITMESSAGE			[] = "SubmitMessage() : ";
char szFUNC_SESSIONUNADVISE			[] = "Session Unadvise() : ";
char szFUNC_MSGSTOREUNDADVISE		[] = "MsgStore Unadvise() : ";
char szFUNC_ISWINDOWVALID			[] = "IsWindowValid() : ";
char szFUNC_ISPATHVALID				[] = "IsPathValid() : ";
char szFUNC_GETDEFAULTPROFILE		[] = "GetDefaultProfile() : Using default profile from system registry ";
char szFUNC_CHECK_INITIALIZED		[] = "Initialization check : ";
char szFUNC_CHECK_LOGGEDON			[] = "Logon check : ";
char szFUNC_CHECK_SEND_FOLDER		[] = "Send folder check : ";
char szFUNC_CHECK_RECEIVE_FOLDER	[] = "Receive folder check : ";
char szFUNC_CHECK_MESSAGE_STORE		[] = "Message store check : ";
char szFUNC_CHECK_ADDRESS_BOOK		[] = "Address book check: ";
char szFUNC_CHECK_MAPI_TABLE		[] = "MAPI table check : ";
char szFUNC_CHECK_MESSAGE			[] = "Message check : ";
char szFUNC_CHECK_ATTACHMENT		[] = "Attachment check : ";
char szFUNC_CHECK_ADDRESS_LIST		[] = "Address list check : ";
char szFUNC_GETCONTENSTABLE			[] = "GetContentsTable() : ";
char szFUNC_MUTEX					[] = "CreateMutex() : ";
char szFUNC_DELETEMESSAGES			[] = "DeleteMessages() : ";
char szFUNC_GETATTACHMENTTABLE		[] = "GetAttachmentTable() : ";
char szFUNC_OPENATTACHMENT			[] = "OpenAttachment() : ";
char szFUNC_OPENPROPERTY			[] = "OpenProperty() : ";
char szFUNC_OPENSTREAMONFILE		[] = "OpenStreamOnFile() : ";
char szFUNC_STAT					[] = "Stat() : ";
char szFUNC_COPYTO					[] = "CopyTo() : ";
char szFUNC_NOTIFICATION			[] = "MAPI Notification : ";
char szFUNC_ATTACHMENTCHECK			[] = "Attachment check : ";
char szFUNC_RECIPIENTLISTCHECK		[] = "Recipient list is empty or NULL ";
char szFUNC_MESSAGECHECK			[] = "Message CStringArray variable is NULL. Pass a valid variable and retry. ";

char szINVALID_MAIL_ADDRESS_TOOSMALL [] = "Invalid e-mail address, too small ";
char szINVALID_MAIL_ADDRESS_MISSINGTOKEN [] = "Invalid e-mail address, missing @ ";

	LPMAPIFREEBUFFER		lpfnMAPIFreeBuffer;

/////////////////////////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
/////////////////////////////////////////////////////////////////////////////
// INTERFACE FUNCTION - PUBLIC
// CMAPI CMAPI
// PURPOSE: Class constructor, uses default parameters,
// if profile is NULL gets default, if window is invalid assigns NULL
// tries to read path if it cannot obtains current directory
/////////////////////////////////////////////////////////////////////////////
CMAPI::CMAPI(void)
{
	if (!this) return;

//initialization
	m_lErrorLevel		= 0;
	m_szMemberFunction	= 0;
	m_szFunction		= 0;

	InitErrorVars( szMFUNC_CONSTRUCTOR );

	//library and dynamically allocated function
	hlibMAPI				= NULL;
	lpfnMAPIInitialize		= NULL;
	lpfnMAPIUninitialize	= NULL;
	lpfnMAPILogonEx			= NULL;
	m_pPacketRouter			= NULL;

	ZeroOutMAPIObjects();

	m_bInitError = true;
	m_bLoggedOn = false;

	return;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
/////////////////////////////////////////////////////////////////////////////
// INTERFACE FUNCTION - PUBLIC
// CMAPI ~CMAPI
// PURPOSE: Logoff, deinitialize MAPI, free DLL library
/////////////////////////////////////////////////////////////////////////////
CMAPI::~CMAPI()
{
	if (!this) return;
	if (m_bInitError) return;

	InitErrorVars( szMFUNC_DESTRUCTOR );

	Logoff();
	//deinitialize MAPI
	if ( lpfnMAPIUninitialize )
	{
		//DeinitMapiUtil();
		MAPIUninitialize();
	}
	//free MAPI32.DLL
	if( hlibMAPI )
		FreeLibrary( hlibMAPI );
	m_pPacketRouter = NULL;

	return;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
/////////////////////////////////////////////////////////////////////////////
// INTERFACE FUNCTION - PUBLIC
// CMAPI CMAPI
// PURPOSE: Class constructor, uses default parameters,
// if profile is NULL gets default, if window is invalid assigns NULL
// tries to read path if it cannot obtains current directory
/////////////////////////////////////////////////////////////////////////////
bool CMAPI::Initialize(
						LPCSTR	szProfileName,
						LPCSTR	szProfilePwd,
						LPCSTR	szAttachmentPath
					  )
{
	if (!this) return false;

	//declarations
	MAPIINIT_0		MAPIInitStruct ;
	HRESULT			hr;
	SCODE			sc;

	m_bInitError = true;

	if( IsInitialized() == true ) return true;

//initialization

	//return values
	hr	= 0;
	sc	= 0;

	//library and dynamically allocated function
	hlibMAPI				= NULL;
	lpfnMAPIInitialize		= NULL;
	lpfnMAPIUninitialize	= NULL;
	lpfnMAPILogonEx			= NULL;

	ZeroOutMAPIObjects();

	InitErrorVars( szMFUNC_INITIALIZE );

	//one time initialization
	m_bInitError			= false;
	m_bLoggedOn				= false;
	m_bIsDefaultProfile		= false;

	if ( !szProfileName || szProfileName[0] == 0 )
	{
		//get default profile from registry, and output warning
		GetDefaultProfile();
	}
	else
	{
		m_szProfileName = szProfileName;
	}
	//if logon fails we will also obtain default profile and try again once

	if ( szProfilePwd || szProfilePwd[0] == 0 )
	{
		m_szProfilePwd.Empty();
	}
	else
	{
		m_szProfilePwd = szProfilePwd;
	}

	if ( !szAttachmentPath || szAttachmentPath[0] == 0 )
	{
		//set save attachments path to app directory, output warning
		GetCurrentDirectory(MAX_PATH, (LPTSTR)(LPCTSTR)m_szAttachmentPath );
		m_szAttachmentPath += szATTACHMENT_DIR;
		CreateDirectory ( m_szAttachmentPath, NULL );
		m_szAttachmentPath += "\\";
	}
	else
	{
		m_szAttachmentPath = szAttachmentPath;
		//check if path is valid
		if ( IsPathValid() == false )
		{
			//if it's not, get current directory, output warning
			GetCurrentDirectory(MAX_PATH, (LPTSTR)(LPCTSTR)m_szAttachmentPath );
			m_szAttachmentPath += szATTACHMENT_DIR;
			CreateDirectory ( m_szAttachmentPath, NULL );
			m_szAttachmentPath += "\\";
		}
	}

//code

	//load MAPI DLL
    if (!(hlibMAPI = LoadLibrary( szMAPI32DLL )))
	{
		m_lErrorLevel			= LEVEL_ERROR;
		m_szFunction			= szFUNC_LOADLIBRARY;
		m_szCurrentError		= szMAPI32DLL;
		m_szCurrentError		+= " ";
		ProcessLastError();
		goto Error;
	}

    if (!(lpfnMAPIInitialize = (LPMAPIINITIALIZE) GetProcAddress (hlibMAPI, szMAPIInitialize)))
	{
		m_lErrorLevel			= LEVEL_ERROR;
		m_szFunction			= szFUNC_GETPROCADDRESS;
		m_szCurrentError		= szMAPIInitialize;
		m_szCurrentError		+= " ";
		ProcessLastError();
		goto Error;
	}
    if (!(lpfnMAPIUninitialize = (LPMAPIUNINITIALIZE) GetProcAddress (hlibMAPI, szMAPIUninitialize)))
	{
		m_lErrorLevel			= LEVEL_ERROR;
		m_szFunction			= szFUNC_GETPROCADDRESS;
		m_szCurrentError		= szMAPIUninitialize;
		m_szCurrentError		+= " ";
		ProcessLastError();
		goto Error;
	}
	if (!(lpfnMAPIFreeBuffer = (LPMAPIFREEBUFFER) GetProcAddress (hlibMAPI, szMAPIFreeBuffer)))
	{
		m_lErrorLevel			= LEVEL_ERROR;
		m_szFunction			= szFUNC_GETPROCADDRESS;
		m_szCurrentError		= szMAPIFreeBuffer;
		m_szCurrentError		+= " ";
		ProcessLastError();
		goto Error;
	}
	if (!(lpfnMAPIAllocateMore = (LPMAPIALLOCATEMORE) GetProcAddress (hlibMAPI, szMAPIAllocateMore)))
	{
		m_lErrorLevel			= LEVEL_ERROR;
		m_szFunction			= szFUNC_GETPROCADDRESS;
		m_szCurrentError		= szMAPIAllocateMore;
		m_szCurrentError		+= " ";
		ProcessLastError();
		goto Error;
	}
	if (!(lpfnMAPIAllocateBuffer = (LPMAPIALLOCATEBUFFER) GetProcAddress (hlibMAPI, szMAPIAllocateBuffer)))
	{
		m_lErrorLevel			= LEVEL_ERROR;
		m_szFunction			= szFUNC_GETPROCADDRESS;
		m_szCurrentError		= szMAPIAllocateBuffer;
		m_szCurrentError		+= " ";
		ProcessLastError();
		goto Error;
	}
	if (!(lpfnOpenStreamOnFile = (LPOPENSTREAMONFILE) GetProcAddress (hlibMAPI, szOpenStreamOnFile)))
	{
		m_lErrorLevel			= LEVEL_ERROR;
		m_szFunction			= szFUNC_GETPROCADDRESS;
		m_szCurrentError		= szOpenStreamOnFile;
		m_szCurrentError		+= " ";
		ProcessLastError();
		goto Error;
	}
	if (!(lpfnFreePadrlist = (LPFREEPADRLIST) GetProcAddress (hlibMAPI, szFreePadrlist)))
	{
		m_lErrorLevel			= LEVEL_ERROR;
		m_szFunction			= szFUNC_GETPROCADDRESS;
		m_szCurrentError		= szFreePadrlist;
		m_szCurrentError		+= " ";
		ProcessLastError();
		goto Error;
	}
	if (!(lpfnFreeProws = (LPFREEPROWS) GetProcAddress (hlibMAPI, szFreeProws)))
	{
		m_lErrorLevel			= LEVEL_ERROR;
		m_szFunction			= szFUNC_GETPROCADDRESS;
		m_szCurrentError		= szFreeProws;
		m_szCurrentError		+= " ";
		ProcessLastError();
		goto Error;
	}
	if (!(lpfnHrGetOneProp = (LPHRGETONEPROP) GetProcAddress (hlibMAPI, szHrGetOneProp)))
	{
		m_lErrorLevel			= LEVEL_ERROR;
		m_szFunction			= szFUNC_GETPROCADDRESS;
		m_szCurrentError		= szHrGetOneProp;
		m_szCurrentError		+= " ";
		ProcessLastError();
		goto Error;
	}
	if (!(lpfnHrQueryAllRows = (LPHRQUERYALLROWS) GetProcAddress (hlibMAPI, szHrQueryAllRows)))
	{
		m_lErrorLevel			= LEVEL_ERROR;
		m_szFunction			= szFUNC_GETPROCADDRESS;
		m_szCurrentError		= szHrQueryAllRows;
		m_szCurrentError		+= " ";
		ProcessLastError();
		goto Error;
	}
	if (!(lpfnMAPILogonEx = (LPMAPILOGONEX) GetProcAddress (hlibMAPI, szMAPILogonEx)))
	{
		m_lErrorLevel			= LEVEL_ERROR;
		m_szFunction			= szFUNC_GETPROCADDRESS;
		m_szCurrentError		= szMAPILogonEx;
		m_szCurrentError		+= " ";
		ProcessLastError();
		goto Error;
	}

	//MAPIInitStruct.ulFlags		= MAPI_MULTITHREAD_NOTIFICATIONS;
	MAPIInitStruct.ulFlags		= 0;
	MAPIInitStruct.ulVersion	= MAPI_INIT_VERSION;

	hr = MAPIInitialize ( (void*)&MAPIInitStruct );

	if( MAPIIsSuccessfull( szFUNC_MAPIINIT, hr ) == false ) goto Error;

	//initialize MAPI utilities
//	sc = ScInitMapiUtil(0);
//	if ( sc != S_OK )
//	{
//		m_lErrorLevel		= LEVEL_ERROR;
//		m_szFunction		= szFUNC_MAPIINITUTIL;
//		m_szCurrentError	= szINITUTIL;
//		goto Error;
//	}

	m_szTemp = szMESSAGE_INITIALIZED;
	m_szTemp += "Profile = '";
	m_szTemp += m_szProfileName;
	m_szTemp += "' Attachments path = '";
	m_szTemp += m_szAttachmentPath;
	m_szTemp += "'";
	InfoMessage( (LPSTR)(LPCTSTR)m_szTemp );

	return true;

//errors
Error:
	m_bInitError = true;
	OnError();
	return false;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
/////////////////////////////////////////////////////////////////////////////
// INTERFACE FUNCTION - PUBLIC
// CMAPI Logon
// PURPOSE: logon to MAPI with specified profile or default profile
// if specified profile fails, will use default profile and retry logon
/////////////////////////////////////////////////////////////////////////////
bool CMAPI::Logon(void)
{
	if (!this) return false;
	if (m_bInitError) return false;

//declarations
	HRESULT hr;
	ULONG	ulResult;
	FLAGS	flFlag;

//initialization

	InitErrorVars( szMFUNC_LOGON );

//code

	if( IsInitialized() == false ) goto Error;
	if ( IsLoggedOff() == false ) goto Error;

	//logon
	if ( m_lpSession == NULL )
	{
		//MAPI_LOGON_UI MAPI_NT_SERVICE MAPI_NO_MAIL | MAPI_FORCE_DOWNLOAD
		flFlag = MAPI_EXPLICIT_PROFILE | MAPI_EXTENDED | MAPI_ALLOW_OTHERS;
		//ulResult = MAPILogonEx( (ULONG)(void *)m_hWindow, 
		ulResult = MAPILogonEx( (ULONG)0, 
								(LPTSTR)(LPCTSTR)m_szProfileName, 
								(LPTSTR)(LPCTSTR)m_szProfilePwd, 
								flFlag, 
								(LPMAPISESSION FAR *) &m_lpSession);

		if(ulResult)
		{
			m_lpSession		= NULL;
			m_lErrorLevel	= LEVEL_ERROR;
			m_szFunction		= szFUNC_MAPILOGON;
			m_lError		= ulResult;
			goto Error;
		}
	}

	hr = m_lpSession->OpenAddressBook(NULL, NULL, AB_NO_DIALOG, &m_lpAddrBook);
	if( MAPIIsSuccessfull( szFUNC_OPENADDRESSBOOK, hr ) == false ) goto Error;

	m_bLoggedOn = true;

	if ( RegisterNotifications() == true )
	{
		m_bLoggedOn = true;
		m_szTemp += szMESSAGE_LOGGEDON;
		m_szTemp += m_szCurrentName;
		m_szTemp += "'";
		InfoMessage( (LPSTR)(LPCTSTR)m_szTemp );
	}
	else
	{
		m_bLoggedOn = false;
	}

	return( m_bLoggedOn );

//errors
Error:
	OnError();
	return( false );
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
/////////////////////////////////////////////////////////////////////////////
// INTERFACE FUNCTION - PUBLIC
// CMAPI Logoff
// PURPOSE: logoff MAPI, unregister notifications, invalidate all MAPI
// objects, zero out connection references
/////////////////////////////////////////////////////////////////////////////
bool CMAPI::Logoff()
{
	if (!this) return false;
	if (m_bInitError) return false;

//declarations
	HRESULT		hr;
	ULONG		ulFlags;

//initialization
	hr = 0;
	ulFlags = LOGOFF_PURGE;

	InitErrorVars( szMFUNC_LOGOFF );

//code

	if( IsLoggedOn() == false )
	{
		return true;
	}

	if( IsInitialized()	== false )
	{
		goto Error;
	}

	//if we have a valid session
	if ( m_lpSession )
	{
		//unadvise session sinks
		if ( m_ulConnSessionExt )
		{
			hr = m_lpSession->Unadvise(m_ulConnSessionExt);
			UnadviseFailiureWarning( hr );
		}
		if ( m_ulConnSessionErr )
		{
			hr = m_lpSession->Unadvise(m_ulConnSessionErr);
			UnadviseFailiureWarning( hr );
		}

		if ( m_lpMDB )
		{
			//unadvise message store object sinks
			if ( m_ulConnMsgStoreNewMail )
			{
				hr = m_lpMDB->Unadvise(m_ulConnMsgStoreNewMail);
				UnadviseFailiureWarning( hr );
			}
			if ( m_ulConnMsgStoreExt )
			{
				hr = m_lpMDB->Unadvise(m_ulConnMsgStoreExt);
				UnadviseFailiureWarning( hr );
			}
			if ( m_ulConnMsgStoreErr )
			{
				hr = m_lpMDB->Unadvise(m_ulConnMsgStoreErr);
				UnadviseFailiureWarning( hr );
			}

			//release folder objects
			if ( m_lpReceiveFolder )
			{
				m_lpReceiveFolder->Release();
				m_lpReceiveFolder = NULL;
			}
			if ( m_lpSendFolder )
			{
				m_lpSendFolder->Release();
				m_lpSendFolder = NULL;
			}

			//logoff message store
			hr = m_lpMDB->StoreLogoff(&ulFlags);    
			if( MAPIIsSuccessfull( szFUNC_STORELOGOFF, hr ) == false ) goto Error;
			m_lpMDB->Release();
			m_lpMDB = NULL;
		}

		if ( m_lpAddrBook)
		{
			m_lpAddrBook->Release();
			m_lpAddrBook = NULL;
		}

		//logoff session
		hr = m_lpSession->Logoff(
								(ULONG)(void *)0, //m_hWindow,
								0, //MAPI_LOGOFF_SHARED MAPI_LOGOFF_UI 
								0
								);
		if( MAPIIsSuccessfull( szFUNC_SESSIONLOGOFF, hr ) == false ) goto Error;
		m_lpSession->Release();
		m_lpSession = NULL;
	}

	m_bLoggedOn = false;
	ZeroOutMAPIObjects();

	InfoMessage( szMESSAGE_LOGGEDOFF );
	return( true );

//errors
Error:
	OnError();
	return( false );
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
/////////////////////////////////////////////////////////////////////////////
// INTERFACE FUNCTION - PUBLIC
// CMAPI SendMessage
// PURPOSE: Send message
/////////////////////////////////////////////////////////////////////////////
bool CMAPI::SendMail(void)
{
	if (!this) return false;
	if (m_bInitError) return false;

//declarations
	HRESULT		hr = 0;
    LPMESSAGE pmsgOutgoing = NULL;
	ULONG ulMsgToken = 0;

//initialization

	InitErrorVars( szMFUNC_SENDMAIL );

//code

	if( IsLoggedOn() == false )
	{
		return false;
	}

	if( IsInitialized()	== false )
	{
		goto Error;
	}

	//if we have a valid session
	if ( m_lpSession && m_lpSendFolder && m_lpMDB )
	{
		hr = m_lpSendFolder->
			CreateMessage(	NULL, 
							MAPI_DEFERRED_ERRORS,
							&pmsgOutgoing );
		if( MAPIIsSuccessfull( szFUNC_CREATEMESSAGE, hr ) == false ) goto Error;

		hr = m_lpSession->
			PrepareForm(	NULL, 
							pmsgOutgoing, 
							(LPULONG) &ulMsgToken );
		if( MAPIIsSuccessfull( szFUNC_PREPAREFORM, hr ) == false ) goto Error;

		pmsgOutgoing->Release();

		hr = m_lpSession->
			ShowForm(		(ULONG)0, 
							m_lpMDB, 
							m_lpSendFolder, 
							NULL, 
							ulMsgToken,
							NULL, 
							MAPI_NEW_MESSAGE, 
							0, 
							MSGFLAG_UNSENT | MSGFLAG_READ, 
							0, 
							"IPM.Note" );
		if ( hr != MAPI_E_USER_CANCEL )
			if( MAPIIsSuccessfull( szFUNC_SHOWFORM, hr ) == false ) goto Error;
		return( true );
	}

//errors
Error:
	OnError();
	return( false );
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
/////////////////////////////////////////////////////////////////////////////
// INTERFACE FUNCTION - PUBLIC
// CMAPI SendMessage
// PURPOSE: Send message to recipients in string array, with optional
// attachments in string array, optional subject and body, pass NULL for
// defaults: szSubject = %appName%, szBody = "" empty
/////////////////////////////////////////////////////////////////////////////
bool CMAPI::SendMessage(	
						CStringArray*	psaRecipients,		//in
						CStringArray*	psaAttachments,		//in
						LPSTR			szSubject,			//in
						LPSTR			szBody				//in
						)
{
	if (!this) return false;
	if (m_bInitError) return false;

//declarations
	bool			bErrors					= true;
	int				i						= 0;
	int				iLen					= 0;
	int				iArrayCount				= 0;
	int				iPos					= 0;
	char *			pszTemp					= NULL;
	HANDLE			hSearch					= NULL;
	POSITION		pos;
	WIN32_FIND_DATA fd;
	CString			sAttachment;
	CStringList		lString;

    HRESULT         hr						= hrSuccess;
	LPMESSAGE		lpMessage               = NULL;
	LPATTACH		lpAttachment			= NULL;
	LPADRLIST		lpAddressList			= NULL;
	LPSPropValue	lpvMessageProps			= NULL;
	LPSPropValue	lpvAttachmentProps		= NULL;
	ULONG			ulAttachNum				= 0;
    ULONG           ulResult				= 0;
	LPSTR			szMsgSubject			= 0;
	LPSTR			szMsgBody				= 0;
	DWORD			dwWaitResult			= 0;
	
//initialization

	InitErrorVars( szMFUNC_SENDMESSAGE );

	if( IsInitialized()	== false ||
		IsLoggedOn()	== false )
	{
		goto Error;
	}

//code

//____________________________________________________________________

	if( IsValidObject( m_lpAddrBook, szFUNC_CHECK_ADDRESS_BOOK, ERR_NO_ADDRESS_BOOK ) 
							== false ||
		IsValidObject( m_lpSendFolder, szFUNC_CHECK_SEND_FOLDER, ERR_NO_SEND_FOLDER )
							== false )
	{
		goto Error;
	}

	if ( psaRecipients == NULL )
	{
		m_lErrorLevel	= LEVEL_ERROR;
		m_szFunction	= szFUNC_RECIPIENTLISTCHECK;
		goto Error;
	}

	if ( psaRecipients->GetSize() < 1 )
	{
		m_lErrorLevel	= LEVEL_ERROR;
		m_szFunction	= szFUNC_RECIPIENTLISTCHECK;
		goto Error;
	}

	//setup subject
	if ( szSubject )
	{
		szMsgSubject = (LPSTR)(LPCTSTR)szSubject;
	}
	else
	{
		szMsgSubject = (LPSTR)(LPCTSTR)m_szCurrentName;
	}
	//setup message body
	if ( szBody )
	{
		szMsgBody = (LPSTR)(LPCTSTR)szBody;
	}
	else
	{
		szMsgBody = (LPSTR)(LPCTSTR)m_szCurrentName;
	}

	// This change must be posted. Create a new IMessage object and set its properties
    // Do not use MAPI_DEFERRED_ERRORS
	hr = m_lpSendFolder->CreateMessage(NULL, 0, &lpMessage);
	if( MAPIIsSuccessfull( szFUNC_CREATEMESSAGE, hr ) == false ) goto Error;

	if( IsValidObject( lpMessage, szFUNC_CHECK_MESSAGE, ERR_NO_MESSAGE ) == false ) goto Error;

	// allocates space for the message properties.
	hr = MAPIAllocateBuffer(
							(sizeof(SPropValue) * PROPERTY_COUNT_MESSAGE ),
							(void**)&lpvMessageProps
						   );
	if( MAPIAllocateIsSuccessfull( hr ) == false ) goto Error;

	//set message properties
	lpvMessageProps[0].ulPropTag	= PR_CONVERSATION_TOPIC;
    lpvMessageProps[0].Value.lpszA	= szMsgSubject;
    lpvMessageProps[1].ulPropTag	= PR_MESSAGE_FLAGS;
    lpvMessageProps[1].Value.l		= MSGFLAG_UNMODIFIED | MSGFLAG_RN_PENDING;
    lpvMessageProps[2].ulPropTag    = PR_SENDER_ENTRYID;
    lpvMessageProps[2].Value.bin    = m_IdentityEID;
    lpvMessageProps[3].ulPropTag    = PR_SENDER_NAME;
    lpvMessageProps[3].Value.lpszA  = (LPSTR)(LPCTSTR)m_szCurrentName;
    lpvMessageProps[4].ulPropTag    = PR_SENT_REPRESENTING_ENTRYID;
    lpvMessageProps[4].Value.bin    = m_IdentityEID;
    lpvMessageProps[5].ulPropTag    = PR_SENT_REPRESENTING_NAME;
    lpvMessageProps[5].Value.lpszA  = lpvMessageProps[3].Value.lpszA;
    lpvMessageProps[6].ulPropTag    = PR_MESSAGE_CLASS;
    lpvMessageProps[6].Value.lpszA  = szEMAIL_CLASS;
    lpvMessageProps[7].ulPropTag    = PR_PRIORITY;
	lpvMessageProps[7].Value.l      = PRIO_NORMAL;
	lpvMessageProps[8].ulPropTag    = PR_SUBJECT;
	lpvMessageProps[8].Value.lpszA  = szMsgSubject;
    lpvMessageProps[9].ulPropTag    = PR_BODY;
	lpvMessageProps[9].Value.lpszA  = szMsgBody;
    lpvMessageProps[10].ulPropTag   = PR_CLIENT_SUBMIT_TIME;
    GetSystemTimeAsFileTime (&lpvMessageProps[10].Value.ft);	
    lpvMessageProps[11].ulPropTag   = PR_DELETE_AFTER_SUBMIT;
	lpvMessageProps[11].Value.b     = TRUE;

    lpvMessageProps[12].ulPropTag   = PR_READ_RECEIPT_REQUESTED;
	//lpvMessageProps[12].Value.b     = TRUE;
	lpvMessageProps[12].Value.b     = FALSE;
    lpvMessageProps[13].ulPropTag   = PR_READ_RECEIPT_ENTRYID;
	lpvMessageProps[13].Value.bin   = m_IdentityEID;

	//save
    hr = lpMessage->SetProps (PROPERTY_COUNT_MESSAGE, lpvMessageProps, NULL);
	if( MAPIIsSuccessfull( szFUNC_SETPROPS, hr ) == false ) goto Error;


//recipient list setup

	iArrayCount = psaRecipients->GetSize();
	for ( i=0; i<iArrayCount; i++ ) 
	{
		lString.AddTail( psaRecipients->GetAt(i) );
	}

	psaRecipients->RemoveAll();

	//make sure we don't have NULLS or empty strings and
	// the @ symbol is present
	pos = lString.GetHeadPosition();
	while (pos != NULL)
	{
		CString szString = lString.GetNext(pos);
		szString.TrimLeft();
		szString.TrimRight();
		//here we are commiting to SMTP type addresses
		if ( szString.GetLength() < EMAIL_ADDRESS_MIN_SIZE )
		{
			m_lErrorLevel	= LEVEL_ERROR;
			m_szFunction	= szFUNC_RECIPIENTLISTCHECK;
			m_szCurrentError = szINVALID_MAIL_ADDRESS_TOOSMALL;
			m_szCurrentError += szString;
			goto Error;
		}
		else
		{
			//make sure internet style address has @ symbol
			if ( szString.Find( "@", 1 ) == NOT_FOUND )
			{
				m_lErrorLevel	= LEVEL_ERROR;
				m_szFunction	= szFUNC_RECIPIENTLISTCHECK;
				m_szCurrentError = szINVALID_MAIL_ADDRESS_MISSINGTOKEN;
				m_szCurrentError += szString;
				goto Error;
			}
			else
			{
				psaRecipients->Add( (LPCTSTR)szString );
			}
		}
	}

	iArrayCount = psaRecipients->GetSize();

	// allocate and address list
	hr = MAPIAllocateBuffer( (sizeof(ADRLIST) * iArrayCount) +
							 (sizeof(ADRENTRY)* iArrayCount), 
							 (LPVOID*)&lpAddressList );
	if( MAPIAllocateIsSuccessfull( hr ) == false ) goto Error;
	if( IsValidObject( lpAddressList, szFUNC_CHECK_ADDRESS_LIST, ERR_NO_ADDRESS_LIST ) == false ) goto Error;

	lpAddressList->cEntries = iArrayCount;

	for ( i=0; i<iArrayCount; i++ ) 
	{
		//recipient properties buffer
		hr = MAPIAllocateBuffer(
								PROPERTY_COUNT_RECIPIENT * sizeof(SPropValue),
								(LPVOID *)&lpAddressList->aEntries[i].rgPropVals
							   );
		if( MAPIAllocateIsSuccessfull( hr ) == false ) goto Error;

		//set count of recipient properties
		lpAddressList->aEntries[i].cValues = PROPERTY_COUNT_RECIPIENT;

		//set recipient name
		lpAddressList->aEntries[i].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
		iLen = psaRecipients->GetAt(i).GetLength();
		hr = MAPIAllocateBuffer(
								(iLen + 1) * sizeof(char),
								(LPVOID FAR *)&pszTemp
							   );
		if( MAPIAllocateIsSuccessfull( hr ) == false ) goto Error;
		strncpy(pszTemp, (LPTSTR)(LPCTSTR)psaRecipients->GetAt(i), psaRecipients->GetAt(i).GetLength() + 1 );
		lpAddressList->aEntries[i].rgPropVals[0].Value.lpszA = pszTemp;
		
		//set recipient type = TO: (available TO, CC and BCC)
		lpAddressList->aEntries[i].rgPropVals[1].ulPropTag = PR_RECIPIENT_TYPE;
		lpAddressList->aEntries[i].rgPropVals[1].Value.ul = MAPI_TO;
		lpAddressList->aEntries[i].rgPropVals[1].Value.l = MAPI_TO;

		//set e-mail address
		lpAddressList->aEntries[i].rgPropVals[2].ulPropTag = PR_EMAIL_ADDRESS;
		hr = MAPIAllocateBuffer(
								(iLen + 1) * sizeof(char),
								(LPVOID FAR *)&pszTemp
							   );
		if( MAPIAllocateIsSuccessfull( hr ) == false ) goto Error;
		strncpy(pszTemp, (LPTSTR)(LPCTSTR)psaRecipients->GetAt(i), psaRecipients->GetAt(i).GetLength() + 1 );
		lpAddressList->aEntries[i].rgPropVals[2].Value.lpszA = pszTemp;

		//set address type
		lpAddressList->aEntries[i].rgPropVals[3].ulPropTag = PR_ADDRTYPE;
		hr = MAPIAllocateBuffer(
								sizeof(szEMAIL_TYPE),
								(LPVOID FAR *)&pszTemp
							   );
		if( MAPIAllocateIsSuccessfull( hr ) == false ) goto Error;
		strncpy( pszTemp, szEMAIL_TYPE, strlen(szEMAIL_TYPE) + 1 );
		lpAddressList->aEntries[i].rgPropVals[3].Value.lpszA = pszTemp;

	} //end for

	//name resolution, checks against address book
	hr = m_lpAddrBook->ResolveName((ULONG)0,NULL, NULL, lpAddressList);
	if( MAPIIsSuccessfull( szFUNC_RESOLVENAME, hr ) == false ) goto Error;

	//--TODO-- option to save recipients

	// add the recipients to the address list
	hr = lpMessage->ModifyRecipients( MODRECIP_ADD, lpAddressList );
	if( MAPIIsSuccessfull( szFUNC_MODIFYRECIPIENTS, hr ) == false ) goto Error;

//attachment creation
	// allocates space for the attachment properties.
	hr = MAPIAllocateBuffer(
							(sizeof(SPropValue) * PROPERTY_COUNT_ATTACHMENT ),
							(void**)&lpvAttachmentProps
						   );
	if( MAPIAllocateIsSuccessfull( hr ) == false ) goto Error;

	if ( psaAttachments )
	{
		iArrayCount = psaAttachments->GetSize();

		for ( i=0; i<iArrayCount; i++ ) 
		{    // for each attachment

			sAttachment = psaAttachments->GetAt(i);

			//check if attachment is valid
			hSearch = FindFirstFile( sAttachment, &fd );
			if ( hSearch == INVALID_HANDLE_VALUE )
			{
				m_lErrorLevel	= LEVEL_ERROR;
				m_szFunction	= szFUNC_ATTACHMENTCHECK;
				m_szCurrentError = szERR_ATTACHMENT;
				m_szCurrentError += sAttachment;
				OnError();
			}
			else
			{
				FindClose( hSearch );
				if ( fd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )
				{
					m_lErrorLevel	= LEVEL_ERROR;
					m_szFunction	= szFUNC_ATTACHMENTCHECK;
					m_szCurrentError = szERR_ATTACHMENT_ISDIR;
					m_szCurrentError += sAttachment;
					OnError();
				}
				else
				{
					//create new attachment
					hr = lpMessage->CreateAttach(NULL,0,&ulAttachNum,&lpAttachment);
					if( MAPIIsSuccessfull( szFUNC_CREATEATTACH, hr ) == false ) goto Error;

					if( IsValidObject( lpAttachment, szFUNC_CHECK_ATTACHMENT, ERR_NO_ATTACHMENT ) == false ) goto Error;

					iPos = sAttachment.ReverseFind( '\\' ) + 1;
					iLen = sAttachment.GetLength();
					sAttachment = sAttachment.Mid( iPos, iLen - iPos );

					//MessageBox(NULL, sAttachment.Left( iPos + 1 ), sAttachment.Right( iLen - iPos - 1 ), MB_OK);

					//set attachment properties
					lpvAttachmentProps[0].ulPropTag    = PR_ATTACH_METHOD;
					lpvAttachmentProps[0].Value.l      = ATTACH_BY_REF_RESOLVE;
					lpvAttachmentProps[1].ulPropTag    = PR_RENDERING_POSITION;
					lpvAttachmentProps[1].Value.l      = (LONG)-1;

					lpvAttachmentProps[2].ulPropTag    = PR_DISPLAY_NAME;
					lpvAttachmentProps[2].Value.lpszA  = (LPTSTR)(LPCTSTR)sAttachment;

					lpvAttachmentProps[3].ulPropTag    = PR_ATTACH_PATHNAME;
					lpvAttachmentProps[3].Value.lpszA  = (LPTSTR)(LPCTSTR)psaAttachments->GetAt(i);

					lpvAttachmentProps[4].ulPropTag    = PR_ATTACH_FILENAME;
					lpvAttachmentProps[4].Value.lpszA  = (LPTSTR)(LPCTSTR)sAttachment;
					lpvAttachmentProps[5].ulPropTag    = PR_ATTACH_LONG_FILENAME;
					lpvAttachmentProps[5].Value.lpszA  = (LPTSTR)(LPCTSTR)sAttachment;
					//save properties
					hr = lpAttachment->SetProps (PROPERTY_COUNT_ATTACHMENT-1, lpvAttachmentProps, NULL);
					if( MAPIIsSuccessfull( szFUNC_SETPROPS, hr ) == false ) goto Error;
					//save attachment
					hr = lpAttachment->SaveChanges(FORCE_SAVE);
					if( MAPIIsSuccessfull( szFUNC_SAVECHANGES, hr ) == false ) goto Error;
					if ( lpAttachment )
					{
						lpAttachment->Release();
						lpAttachment = NULL;
					}
				}
			}
		} //end for
	}

//message submission

	//submit message
	hr = lpMessage->SubmitMessage(0);
	if( MAPIIsSuccessfull( szFUNC_SUBMITMESSAGE, hr ) == false ) goto Error;

//______________________________________________________________________

	//if we get here, there were no errors
	bErrors = false;

//errors
Error:

//we always get here, if bErrors == false then there are no errors to process
// so we don't duplicate code

	//release buffers
	ReleaseMAPIBuffer( lpvMessageProps );
	ReleaseMAPIBuffer( lpvAttachmentProps );

	if ( lpAttachment ) lpAttachment->Release();
	if ( lpMessage ) lpMessage->Release();

	if ( bErrors )
	{
		OnError();
	}
	else
	{
		if ( lpAddressList ) FreePadrlist( lpAddressList );
		InfoMessage( szMESSAGE_SENT );
	}

	return( !bErrors );
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
/////////////////////////////////////////////////////////////////////////////
// INTERFACE FUNCTION - PUBLIC
// CMAPI ReadNextMessage
// PURPOSE: after logon succeeds, the session downloads all messages to
// the inbox, reads the first unread message it finds in the inbox,
// then deletes the message with DeleteMessage()
/////////////////////////////////////////////////////////////////////////////
bool CMAPI::ReadNextMessage( CStringArray*	psaMessage )
{
	if (!this) return false;
	if (m_bInitError) return false;

//declarations
	bool			bErrors					= true;

    HRESULT         hr						= hrSuccess;
    LPMAPITABLE		lpMAPITable				= NULL;
	LPMESSAGE		lpMessage               = NULL;
	LPATTACH		lpAttachment            = NULL;
    LPSRowSet		lpRows					= NULL;
	LPSRow			lpRow					= NULL;
    ULONG           ulResult				= 0;
    ULONG           ulObjType				= 0;
    ULONG           ulAttachmentNumber		= 0;
    UINT			nIndex					= 0;
	LPENTRYLIST     lpEntryList				= NULL;
    LPSPropValue    lpAttachmentProp		= NULL;
    LPSPropValue    lpMsgBody				= NULL;
    ULONG           cValues					= 0;
    SRestriction	restUnreadMessages;

	LPSTREAM		pStreamFile				= NULL;
	LPSTREAM		pStreamAtt				= NULL;
	int				iPos					= 0;
	STATSTG			StatInfo;
	CString			sAttachment;
	CString			sAttachmentFile;
	bool			bGetMsgBody				= false;

//initialization

	InitErrorVars( szMFUNC_READMESSAGE );

//______________________________________________________________________

	enum 
	{ 
		E_EID=0, 
		E_SUBJECT, 
		E_SENDER_NAME, 
		E_MSG_DEL_TIME, 
		E_FLAGS, 
		E_PRIORITY, 
		E_CONVERS_KEY,
        E_SEARCH_KEY, 
		E_CLASS, 
		E_RECORD_KEY, 
		E_DISPLAY_TO,
		E_BODY,
		E_PTINBOXDIM
	};

	//sort order
    SizedSSortOrderSet( 1, aSortOrder ) =
    {
		1, 0, 0, 
		{ 
			PR_MESSAGE_DELIVERY_TIME, 
			TABLE_SORT_ASCEND 
		} 
	};

	SizedSPropTagArray( E_PTINBOXDIM, aInboxProperties ) =
	{
		E_PTINBOXDIM,
		{
			PR_ENTRYID,
			PR_SUBJECT,
			PR_SENDER_NAME,
			PR_MESSAGE_DELIVERY_TIME,
			PR_MESSAGE_FLAGS,
			PR_PRIORITY,
			PR_CONVERSATION_KEY,
			PR_SEARCH_KEY,
			PR_MESSAGE_CLASS,
			PR_RECORD_KEY,
			PR_DISPLAY_TO,
			PR_BODY
		}
	};

	SizedSPropTagArray( 1, aAttachmentProperties ) =
	{
		1,
		{
			PR_ATTACH_NUM
		}
	};

    SPropTagArray sptaAttachmentProp =
    {
        1,
        {
            PR_ATTACH_LONG_FILENAME
        }
    };

    SPropTagArray sptMsgBody =
    {
        1,
        {
            PR_BODY
        }
    };
//code

	//if we are not given an array where to put message info... forget it
	if ( psaMessage == NULL )
	{
		m_lErrorLevel	= LEVEL_ERROR;
		m_szFunction	= szFUNC_MESSAGECHECK;
		goto Error;
	}

	//zero out psaMessage array
	psaMessage->RemoveAll();

	if( IsInitialized()		== false ||
		IsLoggedOn()		== false ||
		IsValidObject( m_lpReceiveFolder, szFUNC_CHECK_RECEIVE_FOLDER, ERR_NO_RECEIVE_FOLDER ) 
							== false )
	{
		goto Error;
	}

	//get MAPI table
    hr = m_lpReceiveFolder->GetContentsTable( 0, &lpMAPITable);
	if( MAPIIsSuccessfull( szFUNC_GETCONTENSTABLE, hr ) == false ) goto Error;
	if( IsValidObject( lpMAPITable, szFUNC_CHECK_MAPI_TABLE, ERR_NO_MAPI_TABLE ) == false ) goto Error;

	//we only want unread messages
    restUnreadMessages.rt = RES_BITMASK;
    restUnreadMessages.res.resBitMask.relBMR = BMR_EQZ;
    restUnreadMessages.res.resBitMask.ulPropTag = PR_MESSAGE_FLAGS;
    restUnreadMessages.res.resBitMask.ulMask = MSGFLAG_READ;

	hr = HrQueryAllRows(
						lpMAPITable, 
						(LPSPropTagArray)&aInboxProperties, 
						(LPSRestriction)&restUnreadMessages, 
						(LPSSortOrderSet)&aSortOrder, 
						0,
						&lpRows 
						);
	if( MAPIIsSuccessfull( szFUNC_HRQUERYALLROWS, hr ) == false ) goto Error;

	//if message count is < 1, there are no unread messages
	if ( lpRows->cRows < 1 ) 
	{
		if( lpRows ) FreeProws( lpRows );
		if( lpMAPITable )
		{
			lpMAPITable->Release();
			lpMAPITable = NULL;
		}
		return( false );
	}

	lpRow = lpRows->aRow;

	//fill array structure with: 1)Sender 2)Subject 3)Body >=4)Attachments

	//add sender name or NULL
	if( lpRow->lpProps[E_SENDER_NAME].ulPropTag == PR_SENDER_NAME )
	{
		if ( lpRow->lpProps[E_SENDER_NAME].Value.LPSZ )
		{
			psaMessage->Add( lpRow->lpProps[E_SENDER_NAME].Value.LPSZ );
		}
		else
		{
			psaMessage->Add( szNO_SENDER );
		}
	}

	//add message subject or NULL
	if( lpRow->lpProps[E_SUBJECT].ulPropTag == PR_SUBJECT )
	{
		if ( lpRow->lpProps[E_SUBJECT].Value.LPSZ )
		{
			psaMessage->Add( lpRow->lpProps[E_SUBJECT].Value.LPSZ );
		}
		else
		{
			psaMessage->Add( szNO_SUBJECT );
		}
	}

	//add message body or NULL
	if( lpRow->lpProps[E_BODY].ulPropTag == PR_BODY )
	{
		if ( lpRow->lpProps[E_BODY].Value.LPSZ )
		{
			sAttachment = lpRow->lpProps[E_BODY].Value.LPSZ;
			if ( sAttachment.GetLength() > 0 )
				bGetMsgBody = true;
		}
	}

	//add E_DISPLAY_TO or NULL
	if( lpRow->lpProps[E_DISPLAY_TO].ulPropTag == PR_DISPLAY_TO )
	{
		if ( lpRow->lpProps[E_DISPLAY_TO].Value.LPSZ )
		{
			psaMessage->Add( lpRow->lpProps[E_DISPLAY_TO].Value.LPSZ );
		}
		else
		{
			psaMessage->Add( "" );
		}
	}

	//allocate memory for entry list
	hr = MAPIAllocateBuffer(
							sizeof(ENTRYLIST),
							(void**)&lpEntryList
						   );
	if( MAPIAllocateIsSuccessfull( hr ) == false ) goto Error;
	//one message to delete
    lpEntryList->cValues = 1;
	hr = MAPIAllocateMore(
							sizeof(SBinary),
							lpEntryList,
							(void**)&lpEntryList->lpbin
						   );
	if( MAPIAllocateIsSuccessfull( hr ) == false ) goto Error;
    lpEntryList->lpbin[0].cb  = (ULONG) lpRow->lpProps[E_EID].Value.bin.cb;
	//allocate buffer for message entry id
	hr = MAPIAllocateMore(
							lpRow->lpProps[E_EID].Value.bin.cb,
							lpEntryList,
							(void**)&lpEntryList->lpbin[0].lpb
						   );
	if( MAPIAllocateIsSuccessfull( hr ) == false ) goto Error;
	//copy to entry list
    memcpy(lpEntryList->lpbin[0].lpb,
			  (LPENTRYID) lpRow->lpProps[E_EID].Value.bin.lpb,
			  (size_t) (ULONG) lpEntryList->lpbin[0].cb);

	//open the message
    hr = m_lpReceiveFolder->OpenEntry(
                        lpRow->lpProps[E_EID].Value.bin.cb,
                        (ENTRYID*)lpRow->lpProps[E_EID].Value.bin.lpb,
                        NULL,
                        MAPI_MODIFY,
                        &ulObjType,
                        (LPUNKNOWN*)(LPMAPIPROP*)&lpMessage);
	if( MAPIIsSuccessfull( szFUNC_OPENENTRY, hr ) == false ) goto Error;
	if( IsValidObject( lpMessage, szFUNC_CHECK_MESSAGE, ERR_NO_MESSAGE ) == false ) goto Error;

	//free rows and table so we can reuse them
	if( lpRows ) FreeProws( lpRows );
	if( lpMAPITable )
	{
		lpMAPITable->Release();
		lpMAPITable = NULL;
	}

	//open the message body
	if( bGetMsgBody == true )
	{
		hr = lpMessage->GetProps( &sptMsgBody,0,&cValues,&lpMsgBody );
		if( lpMsgBody )
		{
			psaMessage->Add( lpMsgBody->Value.lpszA );
			ReleaseMAPIBuffer( lpMsgBody );
			lpMsgBody = NULL;
		}
		else
		{
			psaMessage->Add( "" );
		}
	}
	else
	{
		psaMessage->Add( "" );
	}

	//open the message attachment table
	hr = lpMessage->GetAttachmentTable( 0, &lpMAPITable );
	if( MAPIIsSuccessfull( szFUNC_GETATTACHMENTTABLE, hr ) == false ) goto Error;
	if( IsValidObject( lpMAPITable, szFUNC_CHECK_MAPI_TABLE, ERR_NO_MAPI_TABLE ) == false ) goto Error;

	hr = lpMessage->SetReadFlag( 0 );
	if( MAPIIsSuccessfull( szFUNC_GETATTACHMENTTABLE, hr ) == false ) goto Error;

	//query all rows in the table
	hr = HrQueryAllRows(
						lpMAPITable, 
						(LPSPropTagArray)&aAttachmentProperties, 
						(LPSRestriction)NULL, 
						(LPSSortOrderSet)NULL, 
						0,
						&lpRows 
						);
	if( MAPIIsSuccessfull( szFUNC_HRQUERYALLROWS, hr ) == false ) goto Error;
	if( IsValidObject( lpMAPITable, szFUNC_CHECK_MAPI_TABLE, ERR_NO_MAPI_TABLE ) == false ) goto Error;

    //for ( nIndex=0; nIndex<lpRows->cRows; ++nIndex )
    for ( nIndex=0; nIndex<lpRows->cRows; nIndex++ )
    {
		lpRow = lpRows->aRow + nIndex;

		//create an attachment object for each table item
		hr = lpMessage->OpenAttach( lpRow->lpProps->Value.l,
									NULL,
									MAPI_BEST_ACCESS,
									&lpAttachment
								  );
		if( MAPIIsSuccessfull( szFUNC_OPENATTACHMENT, hr ) == false ) goto Error;

		//get the attachment properties
		hr = lpAttachment->GetProps( &sptaAttachmentProp, 0, &cValues, &lpAttachmentProp );
		if( hr )
		{
			if( hr != MAPI_W_ERRORS_RETURNED )
			{
				m_lErrorLevel	= LEVEL_ERROR;
				m_szFunction	= szFUNC_GETPROPS;
				m_lError		= hr;
				ProcessLastMAPIError();
				goto Error;
			}
		}
		else
		{
			//add attachment path and file name or NULL
			if ( lpAttachmentProp->Value.lpszA )
			{
				psaMessage->Add( lpAttachmentProp->Value.lpszA );

				sAttachment = lpAttachmentProp->Value.lpszA;
				iPos = sAttachment.ReverseFind( '\\' ) - 1;

				sAttachment = sAttachment.Right( sAttachment.GetLength() - iPos - 2 );
				//sAttachmentFile = m_szAttachmentPath + sAttachment;
				sAttachmentFile = m_szAttachmentPath;
				sAttachmentFile += sAttachment;

				//MAPI_E_NOT_FOUND -2147467259 x80004005
				//copy the data stream to a stream file object
				hr = OpenStreamOnFile (MAPIAllocateBuffer, MAPIFreeBuffer,
									   STGM_CREATE | STGM_READWRITE, (char*)(LPCTSTR)sAttachmentFile, 
									   NULL, &pStreamFile);
				if( MAPIIsSuccessfull( szFUNC_OPENSTREAMONFILE, hr ) == false ) goto Error;

				// Open the destination stream in the attachment object
				hr = lpAttachment->OpenProperty (PR_ATTACH_DATA_BIN, //PR_ATTACH_DATA_OBJ,
											&IID_IStream,
											0,
											0,
											(LPUNKNOWN *)&pStreamAtt);
				if( MAPIIsSuccessfull( szFUNC_OPENPROPERTY, hr ) == false ) goto Error;
				if( IsValidObject( pStreamAtt, szFUNC_CHECK_STREAM, ERR_NO_STREAM ) == false ) goto Error;

				hr = pStreamAtt->Stat (&StatInfo, STATFLAG_NONAME);
				if( MAPIIsSuccessfull( szFUNC_STAT, hr ) == false ) goto Error;
				hr = pStreamAtt->CopyTo (pStreamFile, StatInfo.cbSize, 
											   NULL, NULL);
				if( MAPIIsSuccessfull( szFUNC_COPYTO, hr ) == false ) goto Error;
				if ( pStreamAtt )
				{
					pStreamAtt->Commit( STGC_DEFAULT );
					pStreamAtt->Release();
					pStreamAtt = NULL;
				}
				if ( pStreamFile )
				{
					pStreamFile->Commit( STGC_DEFAULT );
					pStreamFile->Release();
					pStreamFile = NULL;
				}
			}
			else
			{
				psaMessage->Add( szATTACHMENT_IN_ERROR );
			}
		}
	}

//MessageBox(NULL,"wait","hi",MB_OK);
	//delete message
    hr = m_lpReceiveFolder->DeleteMessages( 
									lpEntryList,
									NULL, NULL, 0 );
	if( MAPIIsSuccessfull( szFUNC_DELETEMESSAGES, hr ) == false ) goto Error;

//______________________________________________________________________

	//if we get here, there were no errors
	bErrors = false;

//errors
Error:

//we always get here, if bErrors == false then there are no errors to process
// so we don't duplicate code

	//release buffers
	ReleaseMAPIBuffer( lpAttachmentProp );
	ReleaseMAPIBuffer( lpEntryList );
	
	if( lpRows ) FreeProws( lpRows );
	if( lpAttachment ) lpAttachment->Release();
	if( lpMessage ) lpMessage->Release();
	if( lpMAPITable ) lpMAPITable->Release();

	if ( bErrors == true )
	{
		OnError();
		return( false );
	}
	else
	{
		InfoMessage( szMESSAGE_RED );
		return( true );
	}
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
/////////////////////////////////////////////////////////////////////////////
// HELPER FUNCTION - PRIVATE
// CMAPI RegisterNotifications
// PURPOSE: after logon succeeds, the session downloads all messages to
// the inbox, then it runs this function to register for notifications
// m_bNotifyNewMail and/or m_bNotifyErrors default is true for both.
// It uses CMAPIAdviseSink class which has OnNotify(), use OnNotify() to
// add your code to respond to the notifications.
// If you are using a shared session register for extended notifications
// on the session object so you'll receive a message when a session owner
// logs off the session, otherwise you session would be invalid!
/////////////////////////////////////////////////////////////////////////////
bool CMAPI::RegisterNotifications()
{
	if (!this) return false;
	if (m_bInitError) return false;

//declarations
	LPCTSTR			szOldMemberFunction	= NULL;
	bool			bErrors				= true;

    HRESULT         hr					= hrSuccess;
    LPENTRYID       lpEntryID           = NULL;
    LPMAPITABLE		lpMAPITable         = NULL;
    LPSRowSet		lpRows				= NULL;
    ULONG           ulResult			= 0;
    ULONG           ulEntryID           = 0;
    ULONG           ulObjType           = 0;
    ULONG           iRow				= 0;
    LONG            lSelection			= 0;
    ULONG           cValues             = 0;
    ULONG           ulFlags				= MAPI_BEST_ACCESS;
	ULONG           ulAccess			= MAPI_BEST_ACCESS;
    LPMAILUSER		pUserObj            = NULL;
    LPSPropValue    lpMyName            = NULL;
    LPSTR           lpszExplicitClass   = NULL;
	CMAPIAdviseSink * psink				= NULL;

	CString szAddress;

//initialization

	szOldMemberFunction = m_szMemberFunction;
	InitErrorVars( szMFUNC_REGNOTIFICATIONS );

//code

//CORE CODE
//______________________________________________________________

	//set property array for table
    SizedSPropTagArray(4,sptaStores) =
    {
        4,
        {
            PR_DEFAULT_STORE,
            PR_DISPLAY_NAME,
            PR_PROVIDER_DISPLAY,
            PR_ENTRYID
        }
    };

   // proptag array used by GetProps for getting entryid of IPM subtree
    SPropTagArray sptaIPMOutbox =
    {
        1,
        {
            PR_IPM_OUTBOX_ENTRYID
        }
    };

    SizedSPropTagArray(2, sptaEMail) =
    {
        2,
        {
            PR_ADDRTYPE,
			PR_EMAIL_ADDRESS
        }
    };

//	SPropTagArray sptaEMail =
//	{
//		2,
//		{
//			PR_ADDRTYPE,
//			PR_EMAIL_ADDRESS,
//		}
//	};

	if( IsInitialized()	== false ||
		IsLoggedOn()	== false )
	{
		goto Error;
	}

	//get table
	// 0 or MAPI_UNICODE
    hr = m_lpSession->GetMsgStoresTable(0, &lpMAPITable);
	if( MAPIIsSuccessfull( szFUNC_GETMSGSTORETABLES, hr ) == false ) goto Error;

	if( IsValidObject( lpMAPITable, szFUNC_CHECK_MAPI_TABLE, ERR_NO_MAPI_TABLE ) == false ) goto Error;

	//set table columns
	// 0 or TBL_ASYNC TBL_BATCH 
    hr = lpMAPITable->SetColumns( (LPSPropTagArray)&sptaStores, 0);
	if( MAPIIsSuccessfull( szFUNC_SETCOLUMNS, hr ) == false ) goto Error;

	//move to begining of the table
    hr = lpMAPITable->SeekRow( BOOKMARK_BEGINNING, 0, NULL );
	if( MAPIIsSuccessfull( szFUNC_SEEKROW, hr ) == false ) goto Error;

	hr = HrQueryAllRows(lpMAPITable, NULL, NULL, NULL, cMaxRows, &lpRows );
	if( MAPIIsSuccessfull( szFUNC_HRQUERYALLROWS, hr ) == false ) goto Error;

	// if there is at least one row, find column of EntryID and Default Store
    if(lpRows->cRows)
    {
        // find out which row has the Default Store and open it
        for(iRow = 0; iRow < lpRows->cRows; iRow++)
        {
            // if row is default store, OPEN IT
			if ( lpRows->aRow[iRow].lpProps[0].Value.b == 1 )
			{
				lSelection = iRow;
			}
        }
    }

	//open message store with write rights
    ulFlags	= MDB_WRITE | MDB_NO_DIALOG;
    //hr = m_lpSession->OpenMsgStore( (ULONG)(void *)m_hWindow,
    hr = m_lpSession->OpenMsgStore( (ULONG)0,
                    (ULONG)     lpRows->aRow[lSelection].lpProps[3].Value.bin.cb,
                    (LPENTRYID) lpRows->aRow[lSelection].lpProps[3].Value.bin.lpb,
                    NULL,
                    ulFlags,
                    &m_lpMDB );
	if( MAPIIsSuccessfull( szFUNC_OPENMSGSTORE, hr ) == false ) goto Error;

	if( IsValidObject( m_lpMDB, szFUNC_CHECK_MESSAGE_STORE, ERR_NO_MESSAGE_STORE ) == false ) goto Error;

	// Lets get the name we have assigned as our primary identity. This is
    // the name that will go out in the FROM line of messages
    hr = m_lpSession->QueryIdentity (&m_IdentityEID.cb,
                                         (LPENTRYID *)&m_IdentityEID.lpb);
	if( MAPIIsSuccessfull( szFUNC_QUERYIDENTITY, hr ) == true )
	{
		//open logged on user object
		hr = m_lpSession->OpenEntry (m_IdentityEID.cb,
										 (LPENTRYID)m_IdentityEID.lpb, 
										 NULL,
										 0,
										 &ulObjType,
										 (LPUNKNOWN *)&pUserObj);
		if( MAPIIsSuccessfull( szFUNC_OPENENTRY, hr ) == false ) goto Error;

		//get user name
		HrGetOneProp (pUserObj, PR_DISPLAY_NAME, &lpMyName);
		//hr = HrGetOneProp (pUserObj, PR_DISPLAY_NAME, &lpMyName);
		//if( MAPIIsSuccessfull( szFUNC_HRGETONEPROP, hr ) == false ) goto Error;

		if( lpMyName )
		{
			m_szCurrentName = lpMyName->Value.lpszA;
			ReleaseMAPIBuffer( lpMyName );
			lpMyName = NULL;
		}
		//release user object
		if( pUserObj )
		{
			pUserObj->Release();
			pUserObj = NULL;
		}
	}
	else
	{
		m_szCurrentName = "Unknown user";
		m_lErrorLevel		= LEVEL_WARNING;
		m_szFunction		= szFUNC_QUERYIDENTITY;
		m_lError			= ERR_NOOUTBOX;
		ProcessLastMAPIError();
		OnError();
	}

	//register for ERROR NOTIFICATIONS on session object
	psink = new CMAPIAdviseSink;
	hr = m_lpSession->Advise(
			(ULONG)0,
			(LPENTRYID)NULL,
			fnevCriticalError,
			psink, 
			&m_ulConnSessionErr);
	if( MAPIIsSuccessfull( szFUNC_ADVISE, hr ) == false ) goto Error;
	if ( psink )
	{
		psink->SetCMAPIClass( this );
		psink->Release();
		psink = NULL;
	}

	//register for EXTENDED NOTIFICATIONS on session object
	psink = new CMAPIAdviseSink;
	hr = m_lpSession->Advise(
			(ULONG)0,
			(LPENTRYID)NULL,
			fnevExtended,
			psink, 
			&m_ulConnSessionExt);
	if( MAPIIsSuccessfull( szFUNC_ADVISE, hr ) == false ) goto Error;
	if ( psink )
	{
		psink->SetCMAPIClass( this );
		psink->Release();
		psink = NULL;
	}

	//register for NEW MAIL NOTIFICATIONS on receive folder
	psink = new CMAPIAdviseSink;
	hr = m_lpMDB->Advise(
			(ULONG)0,
			(LPENTRYID)NULL,
			fnevNewMail,
			psink, 
			&m_ulConnMsgStoreNewMail);
	if( MAPIIsSuccessfull( szFUNC_ADVISE, hr ) == false ) goto Error;
	if ( psink )
	{
		psink->SetCMAPIClass( this );
		psink->Release();
		psink = NULL;
	}

	//register for ERROR NOTIFICATIONS on receive folder
	psink = new CMAPIAdviseSink;
	hr = m_lpMDB->Advise(
			(ULONG)0,
			(LPENTRYID)NULL,
			fnevCriticalError,
			psink, 
			&m_ulConnMsgStoreErr);
	if( MAPIIsSuccessfull( szFUNC_ADVISE, hr ) == false ) goto Error;
	if ( psink )
	{
		psink->SetCMAPIClass( this );
		psink->Release();
		psink = NULL;
	}

	//register for EXTENDED NOTIFICATIONS on receive folder
	psink = new CMAPIAdviseSink;
	hr = m_lpMDB->Advise(
			(ULONG)0,
			(LPENTRYID)NULL,
			fnevExtended,
			psink, 
			&m_ulConnMsgStoreExt);
	if( MAPIIsSuccessfull( szFUNC_ADVISE, hr ) == false ) goto Error;
	if ( psink )
	{
		psink->SetCMAPIClass( this );
		psink->Release();
		psink = NULL;
	}

	//get receive folder id
	ulFlags = 0; //MAPI_UNICODE
    hr = m_lpMDB->GetReceiveFolder(
					(LPSTR)szMESSAGE_CLASS,
                    ulFlags,
                    &ulEntryID,
                    &lpEntryID, //pointer to the receive folder id
                    &lpszExplicitClass );
	if( MAPIIsSuccessfull( szFUNC_GETRECEIVEFOLDER, hr ) == false ) goto Error;

	//open receive folder
    hr = m_lpMDB->OpenEntry(
                        ulEntryID,
                        lpEntryID,
                        NULL,
                        ulAccess,
                        &ulObjType,
                        (LPUNKNOWN*)(LPMAPIPROP *) &m_lpReceiveFolder);
	if( MAPIIsSuccessfull( szFUNC_OPENENTRY, hr ) == false ) goto Error;

    // open up the object described, determine if folder, and open folder dlg
    hr = m_lpMDB->GetProps( &sptaIPMOutbox,0,&cValues,&lpMyName );
	if( MAPIIsSuccessfull( szFUNC_GETPROPS, hr ) == false ) goto Error;

    if( lpMyName[0].ulPropTag != PR_IPM_OUTBOX_ENTRYID )
    {
		//Message Store has no valid PR_IPM_OUTBOX_ENTRYID
		m_lErrorLevel		= LEVEL_ERROR;
		m_szFunction		= szFUNC_GETPROPS;
		m_lError			= ERR_NOOUTBOX;
		ProcessLastMAPIError();
		goto Error;
    }

	//open send folder
    // open up the object described, determine if folder, and open folder dlg
    hr = m_lpMDB->OpenEntry((ULONG)lpMyName[0].Value.bin.cb,
                            (LPENTRYID) lpMyName[0].Value.bin.lpb,
                            NULL,
                            ulAccess,
                            &ulObjType,
                            (LPUNKNOWN*)(LPMAPIPROP *)&m_lpSendFolder);
	if( MAPIIsSuccessfull( szFUNC_OPENENTRY, hr ) == false ) goto Error;
    if( ulObjType != MAPI_FOLDER )
    {
		//ulObjType of OpenEntry on m_lpSendFolder != MAPI_FOLDER
		m_lErrorLevel		= LEVEL_ERROR;
		m_szFunction		= szFUNC_OPENENTRY;
		m_lError			= ERR_NOTAFOLDER;
		ProcessLastMAPIError();
		goto Error;
    }

	bErrors = false;

//errors
Error:

/*
	if( lpMyName )
	{
		ReleaseMAPIBuffer( lpMyName );
		lpMyName = NULL;
	}
	hr = pUserObj->GetProps( (SPropTagArray*)&sptaEMail,0,&cValues,&lpMyName );
	if( lpMyName )
	{
		szAddress = lpMyName[0].Value.lpszA;
		szAddress = lpMyName[1].Value.lpszA;
		ReleaseMAPIBuffer( lpMyName );
		lpMyName = NULL;
	}
*/

	//free rows buffer
	if( lpRows ) FreeProws(lpRows);
	// release table
	if( lpMAPITable ) lpMAPITable->Release();

	ReleaseMAPIBuffer( lpMyName );
	ReleaseMAPIBuffer( lpEntryID );
	ReleaseMAPIBuffer( lpszExplicitClass );

//_____________________________________________________________________

	m_szMemberFunction = szOldMemberFunction;
	if ( bErrors == true )
	{
		OnError();
		return( false );
	}
	else
	{
		return( true );
	}
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// INTERFACE FUNCTION - PUBLIC optional
// CMAPI SetAttachmentPath
// PURPOSE: Sets attachment path and checks to see if it's valid
/////////////////////////////////////////////////////////////////////////////
bool CMAPI::SetAttachmentPath( LPCSTR szAttachmentPath )
{
//declarations

//code

	if ( szAttachmentPath )
	{
		m_szAttachmentPath = szAttachmentPath;
		return( IsPathValid() );
	}
	else
	{
		if ( GetCurrentDirectory(MAX_PATH, (LPTSTR)(LPCTSTR)m_szAttachmentPath ) )
		{
			return( false );
		}
		else
		{   //make sure we don't overwrite char string
			m_szAttachmentPath += szATTACHMENT_DIR;
			CreateDirectory ( (LPTSTR)(LPCTSTR)m_szAttachmentPath, NULL );
			m_szAttachmentPath += "\\";
			return( true );
		}
	}
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// INTERFACE FUNCTION - PUBLIC optional
// CMAPI SetProfile
// PURPOSE: Sets profile name and/or password
// to use the new profile you must logoff and logon if you are already
/////////////////////////////////////////////////////////////////////////////
bool CMAPI::SetProfile( LPCSTR szProfile, LPCSTR szPwd )
{
//declarations

//code

	if ( szProfile )
	{
		m_szProfileName = szProfile;
	}

	if ( szPwd )
	{
		m_szProfilePwd = szPwd;
	}

	return( true );

}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// HELPER FUNCTION - PRIVATE, error handlers
// CMAPI OnError
// PURPOSE: Processes errors, messages and warnings
// uses a 4k buffer and clears it when it's full, otherwise the buffer is
// only cleared when the main app calls ClearBuffer()
// the app is notified of new messages through Windows API
/////////////////////////////////////////////////////////////////////////////
bool CMAPI::OnError(void)
{
	m_szTemp.Empty();
	
	//member function
	if ( m_szMemberFunction )
	{
		m_szTemp += m_szMemberFunction;
	}
	//function
	if ( m_szFunction )
	{
		m_szTemp += m_szFunction;
	}
	//other info
	if ( m_szCurrentError )
	{
		m_szTemp += m_szCurrentError;
	}
	//translate error number to string
	GetError( m_szCurrentError, (ULONG)m_lError );
	if ( m_szCurrentError )
	{
		m_szTemp += m_szCurrentError;
	}

	if ( m_pLog )
	{
		m_pLog->Log( m_szTemp, (long)m_lErrorLevel );
	}

	m_szTemp.Empty();
	m_szCurrentError.Empty();
	m_szFunction			= szFUNC_NONE;
	m_lErrorLevel			= LEVEL_MESSAGE;
	m_lError				= 0;

	return( true );

}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// HELPER FUNCTION - PRIVATE, error handlers
// CMAPI ProcessLastError
// PURPOSE: Used to process an OS error, it puts the string in
// m_szCurrentError which OnError() uses, OnError() then clears it
/////////////////////////////////////////////////////////////////////////////
void CMAPI::ProcessLastError(void)
{
//declarations
	DWORD dwError;
	char pBuffer[260];

//code

	dwError = GetLastError();
	if ( ::FormatMessage(	FORMAT_MESSAGE_FROM_SYSTEM | 
							FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
							dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							pBuffer, 255, NULL ) == 0 )
	{
		m_szCurrentError += szERR_NO_OS_ERROR;
	}
	else
	{
		m_szCurrentError += pBuffer;
	}

	if ( pBuffer )
	{
		LocalFree( pBuffer );
	}

	m_lError = 0;

	return;

}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// HELPER FUNCTION - PRIVATE, error handlers
// CMAPI ProcessLastMAPIError
// PURPOSE: Similar to ProcessLastError() but it obtains the error string
// from the MAPI system
/////////////////////////////////////////////////////////////////////////////
void CMAPI::ProcessLastMAPIError( int iObjType )
{
//declarations
	HRESULT hr;

//initialization
	m_pMAPIError	= NULL;
	hr				= 0;

//code

	switch ( iObjType )
	{
	case OBJ_SESSION:
		hr = m_lpSession->GetLastError(	(HRESULT)m_lError,
									0,
									&m_pMAPIError);
		break;
	case OBJ_MSGSTORE:
		hr = m_lpMDB->GetLastError(	(HRESULT)m_lError,
									0,
									&m_pMAPIError);
		break;
	case OBJ_SENDFOLDER:
		hr = m_lpSendFolder->GetLastError(	(HRESULT)m_lError,
									0,
									&m_pMAPIError);
		break;
	case OBJ_RECEIVEFOLDER:
		hr = m_lpReceiveFolder->GetLastError(	(HRESULT)m_lError,
									0,
									&m_pMAPIError);
		break;
	}

	if ( m_pMAPIError == NULL )
	{
		m_szCurrentError = szERR_NO_MAPI_ERROR;
	}
	else
	{
		if ( hr == S_OK )
		{
			if ( m_pMAPIError->lpszComponent != NULL )
			{
				m_szCurrentError = m_pMAPIError->lpszComponent;
				if ( m_pMAPIError->lpszError != NULL )
				{
					m_szCurrentError += m_pMAPIError->lpszError;
				}
				else
				{
					m_szCurrentError += szERR_NO_MAPI_ERROR;
				}
			}
			else
			{
				if ( m_pMAPIError->lpszError != NULL )
				{
					m_szCurrentError = m_pMAPIError->lpszError;
				}
				else
				{
					m_szCurrentError = szERR_NO_MAPI_ERROR;
				}
			}
		}
		else
		{
			m_szCurrentError = szERR_NO_MAPI_ERROR;
		}
		MAPIFreeBuffer( (LPVOID)m_pMAPIError ) ;
	}

	return;

}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// HELPER FUNCTION - PRIVATE, variable assertion
// CMAPI IsPathValid
// PURPOSE: Calls FindFirstFile() API to check for the existence of
// file or directory, if it doesn't exist or isn't a directory
// it returns false (error)
// called by the class constructor and by SetAttachmentPath()
/////////////////////////////////////////////////////////////////////////////
bool CMAPI::IsPathValid(void)
{
//declarations
	HANDLE hSearch;
	WIN32_FIND_DATA fd;
	int iLen;

//code
	
	iLen = m_szAttachmentPath.GetLength()-1;
	if ( m_szAttachmentPath.Right(1) == '\\' )
	{
		//m_szAttachmentPath.SetAt( iLen, 0 );
		m_szAttachmentPath = m_szAttachmentPath.Mid( 0, m_szAttachmentPath.GetLength()-1 );
	}

	CreateDirectory ( m_szAttachmentPath, NULL );

	hSearch = FindFirstFile( m_szAttachmentPath, &fd );
	if ( hSearch == INVALID_HANDLE_VALUE )
	{
		goto Error;
	}
	else
	{
		FindClose( hSearch );
		if ( fd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY )
		{
			goto Error;
		}
	}

	if ( m_szAttachmentPath.Right(1) != '\\' )
	{
		m_szAttachmentPath += '\\';
	}

	return( true );

//errors
Error:
	m_lErrorLevel	= LEVEL_ERROR;
	m_szFunction		= szFUNC_ISPATHVALID;
	ProcessLastError();
	m_lError = 0;
	m_szCurrentError += szWARNING_PATH;
	m_szCurrentError += m_szAttachmentPath;
	m_szAttachmentPath.Empty();
	OnError();
	return( false );
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// HELPER FUNCTION - PRIVATE, variable assertion
// CMAPI GetDefaultProfile
// PURPOSE: Gets default profile name from the system registry
// it uses DRegistry class and checks for OS type to determine registry path
/////////////////////////////////////////////////////////////////////////////
LPCTSTR CMAPI::GetDefaultProfile(void)
{
//declarations
	CString			szProfileName;
	CRegistry		dRegistry;
	OSVERSIONINFO	VersionInformation;

//code

	VersionInformation.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx( &VersionInformation );
	if ( VersionInformation.dwPlatformId == VER_PLATFORM_WIN32_NT )
	{
		dRegistry.SetKeyName( szWINPROFILEKEY_WNT );
	}
	else
	{
		dRegistry.SetKeyName( szWINPROFILEKEY_W95 );
	}
	szProfileName		= dRegistry.GetString(szDEFAULTPROFILE);
	m_szProfileName		= szProfileName;

	m_bIsDefaultProfile = true;

	m_szMemberFunction	= szFUNC_NULL;
	m_lErrorLevel		= LEVEL_WARNING;
	m_szFunction		= szFUNC_GETDEFAULTPROFILE;
	goto Error;

//errors
Error:
//	OnError();
	return (LPCTSTR)m_szProfileName;
}
/////////////////////////////////////////////////////////////////////////////
LPCTSTR CMAPI::GetMAPIProfile(void)
{
	return (LPCTSTR)m_szProfileName;
}



//###########################################################################
//###########################################################################
//###########################################################################

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CMAPIAdviseSink OnNotify
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG) CMAPIAdviseSink::OnNotify (ULONG cNotif, LPNOTIFICATION pNotif)
{
	if (!this) return -1;

	m_pntfExtended	= NULL;
	m_pntfError		= NULL;
	CStringArray psaMsg;

	for (unsigned int i = 0; i < cNotif; i++)	{

		switch ((ULONG)pNotif[i].ulEventType)	{
		case fnevNewMail:			// new mail arrived
			if ( m_pCMAPIClass )
				if ( m_pCMAPIClass->m_pPacketRouter )
					m_pCMAPIClass->m_pPacketRouter->OnReceiveMail( &psaMsg );

			break;

		case fnevExtended:			// extended notification
			//usually other client logged off
	        m_pntfExtended = &pNotif->info.ext;
			m_pCMAPIClass->m_szMemberFunction = szFUNC_NOTIFICATION;
			m_pCMAPIClass->m_lErrorLevel = LEVEL_WARNING;
			m_pCMAPIClass->m_szFunction	= szWARNING_EXT_NOTIF;
			m_pCMAPIClass->m_lError = 0;
			m_pCMAPIClass->m_szCurrentError = szWARNING_EXT_NOTIF;
			m_pCMAPIClass->OnError();
			break;

		case fnevCriticalError:		// critical error (low memory, ....)
	        m_pntfError = &pNotif->info.err;
			m_pCMAPIClass->m_szMemberFunction = szFUNC_NOTIFICATION;
			m_pCMAPIClass->m_lErrorLevel = LEVEL_ERROR;
			m_pCMAPIClass->m_szFunction	= szFUNC_NOTIFICATION;
			m_pCMAPIClass->m_lError = 0;
			if ( m_pntfError == NULL )
			{
				m_pCMAPIClass->m_szCurrentError = szERR_NO_MAPI_ERROR;
			}
			else
			{
				m_pCMAPIClass->m_lError = m_pntfError->scode;
				if ( m_pntfError->lpMAPIError->lpszComponent != NULL )
				{
					m_pCMAPIClass->m_szCurrentError = m_pntfError->lpMAPIError->lpszComponent;
					if ( m_pntfError->lpMAPIError->lpszError != NULL )
					{
						m_pCMAPIClass->m_szCurrentError += m_pntfError->lpMAPIError->lpszError;
					}
					else
					{
						m_pCMAPIClass->m_szCurrentError += szERR_NO_MAPI_ERROR;
					}
				}
				else
				{
					if ( m_pntfError->lpMAPIError->lpszError != NULL )
					{
						m_pCMAPIClass->m_szCurrentError = m_pntfError->lpMAPIError->lpszError;
					}
					else
					{
						m_pCMAPIClass->m_szCurrentError = szERR_NO_MAPI_ERROR;
					}
				}
				MAPIFreeBuffer( (LPVOID)m_pntfError ) ;
			}
			m_pCMAPIClass->OnError();
			break;

		default:
			break;
		}
	}
	m_pntfExtended	= NULL;
	m_pntfError		= NULL;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////


//FUNCTIONS

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void FormatFILETIME(FILETIME *pft, LPSTR szTime)
{
    FILETIME        ft;
    SYSTEMTIME      systime;

    FileTimeToLocalFileTime(pft, &ft);
    FileTimeToSystemTime(&ft, &systime);
    wsprintf(szTime,
        "%04.4d/%02.2d/%02.2d %02.2d:%02.2d",
        systime.wYear, systime.wMonth, systime.wDay,
        systime.wHour, systime.wMinute);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void GetError( CString & szString, ULONG ulError )
{
    LPTSTR  szErr;

    switch(ulError)
    {     
		
	case 0 : szErr = ""; break;

		//our MAPI class errors
		case ERR_NOOUTBOX			: szErr = szERR_NOOUTBOX;			break;
		case ERR_NOTAFOLDER			: szErr = szERR_NOTAFOLDER;			break;
		case ERR_NOT_INITIALIZED    : szErr = szERR_NOT_INITIALIZED;	break;
		case ERR_NOT_LOGGEDON		: szErr = szERR_NOT_LOGGEDON;		break;
		case ERR_NOT_LOGGEDOFF		: szErr = szERR_NOT_LOGGEDOFF;		break;
		case ERR_NO_SEND_FOLDER		: szErr = szERR_NO_SEND_FOLDER;		break;
		case ERR_NO_RECEIVE_FOLDER  : szErr = szERR_NO_RECEIVE_FOLDER;	break;
		case ERR_NO_MESSAGE_STORE   : szErr = szERR_NO_MESSAGE_STORE;	break;
		case ERR_NO_ADDRESS_BOOK    : szErr = szERR_NO_ADDRESS_BOOK;	break;
		case ERR_NO_MAPI_TABLE		: szErr = szERR_NO_MAPI_TABLE;		break;
		case ERR_NO_MESSAGE			: szErr = szERR_NO_MESSAGE;			break;
		case ERR_NO_ATTACHMENT		: szErr = szERR_NO_ATTACHMENT;		break;
		case ERR_NO_ADDRESS_LIST    : szErr = szERR_NO_ADDRESS_LIST;	break;

		//MAPI definitions and windows stuff
		//we used these in case we can't obtain OS or MAPI error strings
		// at least we'll have an idea of what's going on

		case E_FAIL                             : szErr = "E_FAIL";                         break;
        case E_INVALIDARG                       : szErr = "E_INVALIDARG";                   break;
        case E_NOINTERFACE                      : szErr = "E_NOINTERFACE";                  break;
        case E_OUTOFMEMORY                      : szErr = "E_OUTOFMEMORY";                  break;
        case E_ACCESSDENIED                     : szErr = "E_ACCESSDENIED";                 break;

        case MAPI_E_NO_SUPPORT                  : szErr = "MAPI_E_NO_SUPPORT";              break;
        case MAPI_E_BAD_CHARWIDTH               : szErr = "MAPI_E_BAD_CHARWIDTH";           break;
        case MAPI_E_STRING_TOO_LONG             : szErr = "MAPI_E_STRING_TOO_LONG";         break;
        case MAPI_E_UNKNOWN_FLAGS               : szErr = "MAPI_E_UNKNOWN_FLAGS";           break;
        case MAPI_E_INVALID_ENTRYID             : szErr = "MAPI_E_INVALID_ENTRYID";         break;
        case MAPI_E_INVALID_OBJECT              : szErr = "MAPI_E_INVALID_OBJECT";          break;
        case MAPI_E_OBJECT_CHANGED              : szErr = "MAPI_E_OBJECT_CHANGED";          break;
        case MAPI_E_OBJECT_DELETED              : szErr = "MAPI_E_OBJECT_DELETED";          break;
        case MAPI_E_BUSY                        : szErr = "MAPI_E_BUSY";                    break;
        case MAPI_E_NOT_ENOUGH_DISK             : szErr = "MAPI_E_NOT_ENOUGH_DISK";         break;
        case MAPI_E_NOT_ENOUGH_RESOURCES        : szErr = "MAPI_E_NOT_ENOUGH_RESOURCES";    break;
        case MAPI_E_NOT_FOUND                   : szErr = "MAPI_E_NOT_FOUND";               break;
        case MAPI_E_VERSION                     : szErr = "MAPI_E_VERSION";                 break;
        case MAPI_E_LOGON_FAILED                : szErr = "MAPI_E_LOGON_FAILED";            break;
        case MAPI_E_SESSION_LIMIT               : szErr = "MAPI_E_SESSION_LIMIT";           break;
        case MAPI_E_USER_CANCEL                 : szErr = "MAPI_E_USER_CANCEL";             break;
        case MAPI_E_UNABLE_TO_ABORT             : szErr = "MAPI_E_UNABLE_TO_ABORT";         break;
        case MAPI_E_NETWORK_ERROR               : szErr = "MAPI_E_NETWORK_ERROR";           break;
        case MAPI_E_DISK_ERROR                  : szErr = "MAPI_E_DISK_ERROR";              break;
        case MAPI_E_TOO_COMPLEX                 : szErr = "MAPI_E_TOO_COMPLEX";             break;
        case MAPI_E_BAD_COLUMN                  : szErr = "MAPI_E_BAD_COLUMN";              break;
        case MAPI_E_EXTENDED_ERROR              : szErr = "MAPI_E_EXTENDED_ERROR";          break;
        case MAPI_E_COMPUTED                    : szErr = "MAPI_E_COMPUTED";                break;
        case MAPI_E_CORRUPT_DATA                : szErr = "MAPI_E_CORRUPT_DATA";            break;
        case MAPI_E_UNCONFIGURED                : szErr = "MAPI_E_UNCONFIGURED";            break;
        case MAPI_E_FAILONEPROVIDER             : szErr = "MAPI_E_FAILONEPROVIDER";         break;
        case MAPI_E_END_OF_SESSION              : szErr = "MAPI_E_END_OF_SESSION";          break;
        case MAPI_E_UNKNOWN_ENTRYID             : szErr = "MAPI_E_UNKNOWN_ENTRYID";         break;
        case MAPI_E_MISSING_REQUIRED_COLUMN     : szErr = "MAPI_E_MISSING_REQUIRED_COLUMN"; break;
        case MAPI_W_NO_SERVICE                  : szErr = "MAPI_W_NO_SERVICE";              break;
        case MAPI_E_BAD_VALUE                   : szErr = "MAPI_E_BAD_VALUE";               break;
        case MAPI_E_INVALID_TYPE                : szErr = "MAPI_E_INVALID_TYPE";            break;
        case MAPI_E_TYPE_NO_SUPPORT             : szErr = "MAPI_E_TYPE_NO_SUPPORT";         break;
        case MAPI_E_UNEXPECTED_TYPE             : szErr = "MAPI_E_UNEXPECTED_TYPE";         break;
        case MAPI_E_TOO_BIG                     : szErr = "MAPI_E_TOO_BIG";                 break;
        case MAPI_E_DECLINE_COPY                : szErr = "MAPI_E_DECLINE_COPY";            break;
        case MAPI_E_UNEXPECTED_ID               : szErr = "MAPI_E_UNEXPECTED_ID";           break;
        case MAPI_W_ERRORS_RETURNED             : szErr = "MAPI_W_ERRORS_RETURNED";         break;
        case MAPI_E_UNABLE_TO_COMPLETE          : szErr = "MAPI_E_UNABLE_TO_COMPLETE";      break;
        case MAPI_E_TIMEOUT                     : szErr = "MAPI_E_TIMEOUT";                 break;
        case MAPI_E_TABLE_EMPTY                 : szErr = "MAPI_E_TABLE_EMPTY";             break;
        case MAPI_E_TABLE_TOO_BIG               : szErr = "MAPI_E_TABLE_TOO_BIG";           break;
        case MAPI_E_INVALID_BOOKMARK            : szErr = "MAPI_E_INVALID_BOOKMARK";        break;
        case MAPI_W_POSITION_CHANGED            : szErr = "MAPI_W_POSITION_CHANGED";        break;
        case MAPI_W_APPROX_COUNT                : szErr = "MAPI_W_APPROX_COUNT";            break;
        case MAPI_E_WAIT                        : szErr = "MAPI_E_WAIT";                    break;
        case MAPI_E_CANCEL                      : szErr = "MAPI_E_CANCEL";                  break;
        case MAPI_E_NOT_ME                      : szErr = "MAPI_E_NOT_ME";                  break;
        case MAPI_W_CANCEL_MESSAGE              : szErr = "MAPI_W_CANCEL_MESSAGE";          break;
        case MAPI_E_CORRUPT_STORE               : szErr = "MAPI_E_CORRUPT_STORE";           break;
        case MAPI_E_NOT_IN_QUEUE                : szErr = "MAPI_E_NOT_IN_QUEUE";            break;
        case MAPI_E_NO_SUPPRESS                 : szErr = "MAPI_E_NO_SUPPRESS";             break;
        case MAPI_E_COLLISION                   : szErr = "MAPI_E_COLLISION";               break;
        case MAPI_E_NOT_INITIALIZED             : szErr = "MAPI_E_NOT_INITIALIZED";         break;
        case MAPI_E_NON_STANDARD                : szErr = "MAPI_E_NON_STANDARD";            break;
        case MAPI_E_NO_RECIPIENTS               : szErr = "MAPI_E_NO_RECIPIENTS";           break;
        case MAPI_E_SUBMITTED                   : szErr = "MAPI_E_SUBMITTED";               break;
        case MAPI_E_HAS_FOLDERS                 : szErr = "MAPI_E_HAS_FOLDERS";             break;
        case MAPI_E_HAS_MESSAGES                : szErr = "MAPI_E_HAS_MESSAGES";            break;
        case MAPI_E_FOLDER_CYCLE                : szErr = "MAPI_E_FOLDER_CYCLE";            break;
        case MAPI_W_PARTIAL_COMPLETION          : szErr = "MAPI_W_PARTIAL_COMPLETION";      break;
        case MAPI_E_AMBIGUOUS_RECIP             : szErr = "MAPI_E_AMBIGUOUS_RECIP";         break;
        
        #ifndef MAPI_E_UNKNOWN_CPID
        #define MAPI_E_UNKNOWN_CPID             MAKE_MAPI_E( 0x11E )
        #define MAPI_E_UNKNOWN_LCID             MAKE_MAPI_E( 0x11F )
        #endif
        case MAPI_E_UNKNOWN_CPID                : szErr = "MAPI_E_UNKNOWN_CPID";            break;
        case MAPI_E_UNKNOWN_LCID                : szErr = "MAPI_E_UNKNOWN_LCID";            break;

        case STG_E_INVALIDFUNCTION              : szErr = "STG_E_INVALIDFUNCTION";          break;
        case STG_E_FILENOTFOUND                 : szErr = "STG_E_FILENOTFOUND";             break;
        case STG_E_PATHNOTFOUND                 : szErr = "STG_E_PATHNOTFOUND";             break;
        case STG_E_TOOMANYOPENFILES             : szErr = "STG_E_TOOMANYOPENFILES";         break;
        case STG_E_ACCESSDENIED                 : szErr = "STG_E_ACCESSDENIED";             break;
        case STG_E_INVALIDHANDLE                : szErr = "STG_E_INVALIDHANDLE";            break;
        case STG_E_INSUFFICIENTMEMORY           : szErr = "STG_E_INSUFFICIENTMEMORY";       break;
        case STG_E_INVALIDPOINTER               : szErr = "STG_E_INVALIDPOINTER";           break;
        case STG_E_NOMOREFILES                  : szErr = "STG_E_NOMOREFILES";              break;
        case STG_E_DISKISWRITEPROTECTED         : szErr = "STG_E_DISKISWRITEPROTECTED";     break;
        case STG_E_SEEKERROR                    : szErr = "STG_E_SEEKERROR";                break;
        case STG_E_WRITEFAULT                   : szErr = "STG_E_WRITEFAULT";               break;
        case STG_E_READFAULT                    : szErr = "STG_E_READFAULT";                break;
        case STG_E_SHAREVIOLATION               : szErr = "STG_E_SHAREVIOLATION";           break;
        case STG_E_LOCKVIOLATION                : szErr = "STG_E_LOCKVIOLATION";            break;
        case STG_E_FILEALREADYEXISTS            : szErr = "STG_E_FILEALREADYEXISTS";        break;
        case STG_E_INVALIDPARAMETER             : szErr = "STG_E_INVALIDPARAMETER";         break;
        case STG_E_MEDIUMFULL                   : szErr = "STG_E_MEDIUMFULL";               break;
        case STG_E_ABNORMALAPIEXIT              : szErr = "STG_E_ABNORMALAPIEXIT";          break;
        case STG_E_INVALIDHEADER                : szErr = "STG_E_INVALIDHEADER";            break;
        case STG_E_INVALIDNAME                  : szErr = "STG_E_INVALIDNAME";              break;
        case STG_E_UNKNOWN                      : szErr = "STG_E_UNKNOWN";                  break;
        case STG_E_UNIMPLEMENTEDFUNCTION        : szErr = "STG_E_UNIMPLEMENTEDFUNCTION";    break;
        case STG_E_INVALIDFLAG                  : szErr = "STG_E_INVALIDFLAG";              break;
        case STG_E_INUSE                        : szErr = "STG_E_INUSE";                    break;
        case STG_E_NOTCURRENT                   : szErr = "STG_E_NOTCURRENT";               break;
        case STG_E_REVERTED                     : szErr = "STG_E_REVERTED";                 break;
        case STG_E_CANTSAVE                     : szErr = "STG_E_CANTSAVE";                 break;
        case STG_E_OLDFORMAT                    : szErr = "STG_E_OLDFORMAT";                break;
        case STG_E_OLDDLL                       : szErr = "STG_E_OLDDLL";                   break;
        case STG_E_SHAREREQUIRED                : szErr = "STG_E_SHAREREQUIRED";            break;
        case STG_E_NOTFILEBASEDSTORAGE          : szErr = "STG_E_NOTFILEBASEDSTORAGE";      break;
        case STG_S_CONVERTED                    : szErr = "STG_S_CONVERTED";                break;


        case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) :         szErr = "WIN32 - ERROR_FILE_NOT_FOUND";         break;
        case HRESULT_FROM_WIN32(ERROR_INVALID_DRIVE) :          szErr = "WIN32 - ERROR_INVALID_DRIVE";          break;
        case HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE) :         szErr = "WIN32 - ERROR_INVALID_HANDLE";         break;
        case HRESULT_FROM_WIN32(ERROR_SEEK) :                   szErr = "WIN32 - ERROR_SEEK";                   break;
        case HRESULT_FROM_WIN32(ERROR_SECTOR_NOT_FOUND) :       szErr = "WIN32 - ERROR_SECTOR_NOT_FOUND";       break;
        case HRESULT_FROM_WIN32(ERROR_WRITE_FAULT) :            szErr = "WIN32 - ERROR_WRITE_FAULT";            break;
        case HRESULT_FROM_WIN32(ERROR_READ_FAULT) :             szErr = "WIN32 - ERROR_READ_FAULT";             break;
        case HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION) :      szErr = "WIN32 - ERROR_SHARING_VIOLATION";      break;
        case HRESULT_FROM_WIN32(ERROR_LOCK_VIOLATION) :         szErr = "WIN32 - ERROR_LOCK_VIOLATION";         break;
        case HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED) :          szErr = "WIN32 - ERROR_NOT_SUPPORTED";          break;
        case HRESULT_FROM_WIN32(ERROR_NO_SUCH_USER) :           szErr = "WIN32 - ERROR_NO_SUCH_USER";           break;
        case HRESULT_FROM_WIN32(ERROR_NO_SUCH_GROUP) :          szErr = "WIN32 - ERROR_NO_SUCH_GROUP";          break;
        case HRESULT_FROM_WIN32(ERROR_WRONG_PASSWORD) :         szErr = "WIN32 - ERROR_WRONG_PASSWORD";         break;
        case HRESULT_FROM_WIN32(ERROR_INVALID_PASSWORD) :       szErr = "WIN32 - ERROR_INVALID_PASSWORD";       break;
        case HRESULT_FROM_WIN32(ERROR_INVALID_FLAGS) :          szErr = "WIN32 - ERROR_INVALID_FLAGS";          break;
        case HRESULT_FROM_WIN32(ERROR_BAD_USERNAME) :           szErr = "WIN32 - ERROR_BAD_USERNAME";           break;
        case HRESULT_FROM_WIN32(ERROR_BROKEN_PIPE) :            szErr = "WIN32 - ERROR_BROKEN_PIPE";            break;
        case HRESULT_FROM_WIN32(ERROR_PIPE_BUSY) :              szErr = "WIN32 - ERROR_PIPE_BUSY";              break;
        case HRESULT_FROM_WIN32(ERROR_PIPE_NOT_CONNECTED):      szErr = "WIN32 - ERROR_PIPE_NOT_CONNECTED";     break;
        case HRESULT_FROM_WIN32(ERROR_PIPE_CONNECTED):          szErr = "WIN32 - ERROR_PIPE_CONNECTED";         break;
        case HRESULT_FROM_WIN32(ERROR_STATIC_INIT) :            szErr = "WIN32 - ERROR_STATIC_INIT";            break;
        case HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION) :       szErr = "WIN32 - ERROR_INVALID_FUNCTION";       break;
        case HRESULT_FROM_WIN32(ERROR_EXCEPTION_IN_SERVICE):    szErr = "WIN32 - ERROR_EXCEPTION_IN_SERVICE";   break;
        case HRESULT_FROM_WIN32(ERROR_CANCELLED):               szErr = "WIN32 - ERROR_CANCELLED";              break;
        case HRESULT_FROM_WIN32(ERROR_PARTIAL_COPY):            szErr = "WIN32 - ERROR_PARTIAL_COPY";           break;
        case HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER):     szErr = "WIN32 - ERROR_INSUFFICIENT_BUFFER";    break;
        case HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION):  szErr = "WIN32 - ERROR_NO_UNICODE_TRANSLATION"; break;
        case HRESULT_FROM_WIN32(ERROR_INVALID_ACCOUNT_NAME):    szErr = "WIN32 - ERROR_INVALID_ACCOUNT_NAME";   break;
        case HRESULT_FROM_WIN32(ERROR_PRIVILEGE_NOT_HELD):      szErr = "WIN32 - ERROR_INVALID_ACCOUNT_NAME";   break;
        case HRESULT_FROM_WIN32(ERROR_BAD_NET_NAME):            szErr = "WIN32 - ERROR_BAD_NET_NAME";           break;
        case HRESULT_FROM_WIN32(ERROR_SERVICE_NO_THREAD):       szErr = "WIN32 - ERROR_SERVICE_NO_THREAD";      break;
        case HRESULT_FROM_WIN32(ERROR_SHUTDOWN_IN_PROGRESS):    szErr = "WIN32 - ERROR_SHUTDOWN_IN_PROGRESS";   break;
        case HRESULT_FROM_WIN32(ERROR_CONNECTION_INVALID):      szErr = "WIN32 - ERROR_CONNECTION_INVALID";     break;
        case HRESULT_FROM_WIN32(ERROR_HANDLE_EOF):              szErr = "WIN32 - ERROR_HANDLE_EOF";             break;
        case HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS):          szErr = "WIN32 - ERROR_ALREADY_EXISTS";         break;
        case HRESULT_FROM_WIN32(ERROR_HOST_UNREACHABLE):        szErr = "WIN32 - ERROR_HOST_UNREACHABLE";       break;
        case HRESULT_FROM_WIN32(ERROR_FILE_EXISTS):             szErr = "WIN32 - ERROR_FILE_EXISTS";            break;
        case HRESULT_FROM_WIN32(ERROR_IO_PENDING):              szErr = "WIN32 - ERROR_IO_PENDING";             break;
        case HRESULT_FROM_WIN32(ERROR_ILL_FORMED_PASSWORD):     szErr = "WIN32 - ERROR_ILL_FORMED_PASSWORD";    break;
        case HRESULT_FROM_WIN32(ERROR_PASSWORD_RESTRICTION):    szErr = "WIN32 - ERROR_PASSWORD_RESTRICTION";   break;
        case HRESULT_FROM_WIN32(ERROR_LOGON_FAILURE):           szErr = "WIN32 - ERROR_LOGON_FAILURE";          break;
        case HRESULT_FROM_WIN32(ERROR_ACCOUNT_RESTRICTION):     szErr = "WIN32 - ERROR_ACCOUNT_RESTRICTION";    break;
        case HRESULT_FROM_WIN32(ERROR_INVALID_LOGON_HOURS):     szErr = "WIN32 - ERROR_INVALID_LOGON_HOURS";    break;
        case HRESULT_FROM_WIN32(ERROR_INVALID_WORKSTATION):     szErr = "WIN32 - ERROR_INVALID_WORKSTATION";    break;
        case HRESULT_FROM_WIN32(ERROR_PASSWORD_EXPIRED):        szErr = "WIN32 - ERROR_PASSWORD_EXPIRED";       break;
        case HRESULT_FROM_WIN32(ERROR_ACCOUNT_DISABLED):        szErr = "WIN32 - ERROR_ACCOUNT_DISABLED";       break;
        case HRESULT_FROM_WIN32(ERROR_NONE_MAPPED):             szErr = "WIN32 - ERROR_NONE_MAPPED";            break;
        case HRESULT_FROM_WIN32(ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) : szErr = "WIN32 - ERROR_FAILED_SERVICE_CONTROLLER_CONNECT"; break;

        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_CALL_FAILED):              szErr = "RPC Error - RPC_S_CALL_FAILED";            break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_CALL_FAILED_DNE):          szErr = "RPC Error - RPC_S_CALL_FAILED_DNE";        break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_PROTOCOL_ERROR):           szErr = "RPC Error - RPC_S_PROTOCOL_ERROR";         break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_UNSUPPORTED_TRANS_SYN):    szErr = "RPC Error - RPC_S_UNSUPPORTED_TRANS_SYN";  break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_UNSUPPORTED_TYPE):         szErr = "RPC Error - RPC_S_UNSUPPORTED_TYPE";       break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_INVALID_TAG):              szErr = "RPC Error - RPC_S_INVALID_TAG";            break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_INVALID_BOUND):            szErr = "RPC Error - RPC_S_INVALID_BOUND";          break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_NO_ENTRY_NAME):            szErr = "RPC Error - RPC_S_NO_ENTRY_NAME";          break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_INVALID_NAME_SYNTAX):      szErr = "RPC Error - RPC_S_INVALID_NAME_SYNTAX";    break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_UNSUPPORTED_NAME_SYNTAX):  szErr = "RPC Error - RPC_S_UNSUPPORTED_NAME_SYNTAX";break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_DUPLICATE_ENDPOINT):       szErr = "RPC Error - RPC_S_DUPLICATE_ENDPOINT";     break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_INVALID_STRING_BINDING):   szErr = "RPC Error - RPC_S_INVALID_STRING_BINDING"; break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_WRONG_KIND_OF_BINDING):    szErr = "RPC Error - RPC_S_WRONG_KIND_OF_BINDING";  break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_INVALID_BINDING):          szErr = "RPC Error - RPC_S_INVALID_BINDING";        break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_PROTSEQ_NOT_SUPPORTED):    szErr = "RPC Error - RPC_S_PROTSEQ_NOT_SUPPORTED";  break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_INVALID_RPC_PROTSEQ):      szErr = "RPC Error - RPC_S_INVALID_RPC_PROTSEQ";    break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_INVALID_STRING_UUID):      szErr = "RPC Error - RPC_S_INVALID_STRING_UUID";    break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_INVALID_ENDPOINT_FORMAT):  szErr = "RPC Error - RPC_S_INVALID_ENDPOINT_FORMAT";break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_INVALID_NET_ADDR):         szErr = "RPC Error - RPC_S_NO_ENDPOINT_FOUND";      break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_NO_ENDPOINT_FOUND):        szErr = "RPC Error - RPC_S_DUPLICATE_ENDPOINT";     break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_INVALID_TIMEOUT):          szErr = "RPC Error - RPC_S_INVALID_TIMEOUT";        break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_OBJECT_NOT_FOUND):         szErr = "RPC Error - RPC_S_OBJECT_NOT_FOUND";       break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_ALREADY_REGISTERED):       szErr = "RPC Error - RPC_S_ALREADY_REGISTERED";     break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_TYPE_ALREADY_REGISTERED):  szErr = "RPC Error - RPC_S_TYPE_ALREADY_REGISTERED";break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_ALREADY_LISTENING):        szErr = "RPC Error - RPC_S_ALREADY_LISTENING";      break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_NO_PROTSEQS_REGISTERED):   szErr = "RPC Error - RPC_S_NO_PROTSEQS_REGISTERED"; break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_NOT_LISTENING):            szErr = "RPC Error - RPC_S_NOT_LISTENING";          break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_UNKNOWN_MGR_TYPE):         szErr = "RPC Error - RPC_S_UNKNOWN_MGR_TYPE";       break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_UNKNOWN_IF):               szErr = "RPC Error - RPC_S_UNKNOWN_IF";             break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_NO_BINDINGS):              szErr = "RPC Error - RPC_S_NO_BINDINGS";            break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_NO_PROTSEQS):              szErr = "RPC Error - RPC_S_NO_PROTSEQS";            break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_CANT_CREATE_ENDPOINT):     szErr = "RPC Error - RPC_S_CANT_CREATE_ENDPOINT";   break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_OUT_OF_RESOURCES):         szErr = "RPC Error - RPC_S_OUT_OF_RESOURCES";       break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_SERVER_UNAVAILABLE):       szErr = "RPC Error - RPC_S_SERVER_UNAVAILABLE";     break;
        case MAKE_HRESULT(1, FACILITY_RPC, RPC_S_SERVER_TOO_BUSY):          szErr = "RPC Error - RPC_S_SERVER_TOO_BUSY";        break;
        default : szErr = "UNKNOWN ERROR!!!"; break;
    }   
    szString = szErr;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// HELPER FUNCTION - PRIVATE, inline
// CMAPI ZeroOutMAPIObjects
// PURPOSE: 
/////////////////////////////////////////////////////////////////////////////
void CMAPI::ZeroOutMAPIObjects(void)
{

	/*invalidate all MAPI objects*/
	m_lpSession				= NULL;
	m_lpMDB					= NULL;
	m_lpAddrBook			= NULL;
    m_lpReceiveFolder		= NULL;
    m_lpSendFolder			= NULL;
	/*zero out all connection references*/
	m_ulConnSessionErr		= NULL;
	m_ulConnSessionExt		= NULL;
	m_ulConnMsgStoreErr		= NULL;
	m_ulConnMsgStoreExt		= NULL;
	m_ulConnMsgStoreNewMail	= NULL; 

	return;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// HELPER FUNCTION - PRIVATE, inline
// CMAPI InitErrorVars
// PURPOSE: 
/////////////////////////////////////////////////////////////////////////////
void CMAPI::InitErrorVars ( LPSTR szMemberFunction )
{

	m_szMemberFunction		= szMemberFunction;
	m_szFunction			= szFUNC_NONE;
	m_lErrorLevel			= LEVEL_MESSAGE;
	m_lError				= 0;
	m_szCurrentError.Empty();

	return;
}
/////////////////////////////////////////////////////////////////////////////

/*_______________________________________________________________________________________________*/
/*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯*/
bool CMAPI::IsInitialized()
{
	if ( m_bInitError == true )
	{
		m_lErrorLevel	= LEVEL_ERROR;
		m_szFunction	= szFUNC_CHECK_INITIALIZED;
		m_lError		= ERR_NOT_INITIALIZED;
		return false;
	}
	return true;
}
/*...............................................................................................*/

/*_______________________________________________________________________________________________*/
/*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯*/
bool CMAPI::IsLoggedOn()
{
	if ( m_bLoggedOn == false )
	{
		return false;
	}
	return true;
}

/*_______________________________________________________________________________________________*/
/*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯*/
bool CMAPI::IsLoggedOff()
{
	if ( m_bLoggedOn == true || m_lpSession != NULL )
	{
		m_lErrorLevel	= LEVEL_WARNING;
		m_szFunction	= szFUNC_CHECK_LOGGEDON;
		m_lError		= ERR_NOT_LOGGEDOFF;
		return false;
	}
	return true;
}
/*...............................................................................................*/

/*_______________________________________________________________________________________________*/
/*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯*/
bool CMAPI::IsValidObject( LPVOID lpObject, LPSTR szFunction, long lError )
{
	if ( lpObject == NULL )
	{
		m_lErrorLevel	= LEVEL_ERROR;
		m_szFunction	= szFunction;
		m_lError		= lError;
		return false;
	}
	return true;
}
/*...............................................................................................*/


/*_______________________________________________________________________________________________*/
/*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯*/
bool CMAPI::MAPIIsSuccessfull( LPSTR szFunction, HRESULT hr )
{
    if( hr )
    {
		m_lErrorLevel	= LEVEL_ERROR;
		m_szFunction	= szFunction;
		m_lError		= hr;
		ProcessLastMAPIError();
		return false;
    }
	return true;
}
/*...............................................................................................*/

/*_______________________________________________________________________________________________*/
/*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯*/
void CMAPI::ReleaseMAPIBuffer( LPVOID lpBuffer )
{
	ULONG ulResult;
	if( lpBuffer )
	{
		ulResult = MAPIFreeBuffer( lpBuffer );
		lpBuffer = NULL;
		if( ulResult )
		{
			m_lErrorLevel	= LEVEL_WARNING;
			m_szFunction	= szFUNC_FREEBUFF;
			m_lError		= ulResult;
			ProcessLastMAPIError();
			OnError();
		}
	}
}
/*...............................................................................................*/

/*_______________________________________________________________________________________________*/
/*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯*/
void CMAPI::UnadviseFailiureWarning( HRESULT hr )
{
	if ( hr != S_OK )
	{
		m_lErrorLevel	= LEVEL_WARNING;
		m_szFunction	= szFUNC_UNADVISE;
		m_lError		= hr;
		ProcessLastMAPIError();
		OnError();
	}
}
/*...............................................................................................*/

/*_______________________________________________________________________________________________*/
/*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯*/
bool CMAPI::MAPIAllocateIsSuccessfull( HRESULT hr )
{
	if( hr )
	{
		m_lErrorLevel	= LEVEL_ERROR;
		m_szFunction	= szFUNC_ALLOCBUFF;
		m_lError		= hr;
		ProcessLastMAPIError();
		return false;
	}
	return true;
}
/*...............................................................................................*/

/*_______________________________________________________________________________________________*/
/*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯*/
void CMAPI::InfoMessage( LPSTR szMessage )
{
	m_szFunction		= szFUNC_NULL;
	m_lErrorLevel		= LEVEL_MESSAGE;
	m_szCurrentError	= szMessage;
	m_lError			= 0;
	OnError();
}
/*...............................................................................................*/

/*_______________________________________________________________________________________________*/
/*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯*/
bool CMAPI::MakeDefaults(void)
{
	char charBuffer[MAX_PATH];
	GetDefaultProfile();
	GetCurrentDirectory(MAX_PATH, charBuffer );
	m_szAttachmentPath = charBuffer;
	m_szAttachmentPath += szATTACHMENT_DIR;
	CreateDirectory ( m_szAttachmentPath, NULL );
	m_szAttachmentPath += "\\";
	return true;
}
/*...............................................................................................*/


