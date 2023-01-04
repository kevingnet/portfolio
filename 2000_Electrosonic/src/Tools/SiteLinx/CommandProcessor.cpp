#include "stdafx.h"
#include "CommandProcessor.h"
#include "CMAPI.h"
#include "TCPIPServer.h"
#include "TCPIPClient.h"
#include "PacketRouter.h"
#include "ES4000Packet.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CCommandProcessor CCommandProcessor - PUBLIC
// PURPOSE: Constructor
// CALLED BY: System
/////////////////////////////////////////////////////////////////////////////
CCommandProcessor::CCommandProcessor()
{
	if (!this) return;
	m_bIsInitialized		= false;
	m_bIsRunning			= false;
	m_pLog					= NULL;
	m_pPacketRouter			= NULL;
	m_pProductsTable		= NULL;
	m_pMAPI					= NULL;
	m_pTCPIP				= NULL;
	m_szClassName.LoadString(IDS_COMMANDPROCESSOR_CLASSNAME);
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CCommandProcessor ~CCommandProcessor - PUBLIC
// PURPOSE: Destructor
// CALLED BY: System
/////////////////////////////////////////////////////////////////////////////
CCommandProcessor::~CCommandProcessor()
{
	if (!this) return;
	try
	{
		if ( m_pPacketRouter )
		{
			delete m_pPacketRouter;
		}
		if ( m_pMAPI )
		{
			delete m_pMAPI;
		}
		if ( m_pTCPIP ) 
		{
			delete m_pTCPIP;
		}
		if ( m_pProductsTable )
		{
			delete m_pProductsTable;
		}
		if ( m_pLog )
		{
			delete m_pLog;
		}
		m_saNotifyAddresses.RemoveAll();
	}
	catch ( CException * e )
	{
		m_pLog->LogException(
					 e,
					 IDS_COMMANDPROCESSOR_DESTRUCTOR, 
					 LOGTYPE_ERROR, 
					 m_szClassName );
		e->Delete();
		return;
	}
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CCommandProcessor MakeDefaults - PUBLIC
// PURPOSE: 
// CALLED BY: 
/////////////////////////////////////////////////////////////////////////////
bool CCommandProcessor::MakeDefaults()
{
	if (!this) return false;
	try
	{
		m_pMAPI->MakeDefaults();
		m_Defaults.iPortNumber = 2000;
		m_Defaults.szAttachmentPath = m_pMAPI->GetAttachmentPath();
		m_Defaults.szProfileName = m_pMAPI->GetMAPIProfile();
		m_Defaults.szProfilePwd = "";
		m_Defaults.szLocalName = "Local";
		m_Defaults.szLocalAddress = "YourAddress@YourDomain.com???";
		m_Defaults.bDisableRemoteNodes = true;
		m_Defaults.bAutoSave = false;
	}
	catch ( CException * e )
	{
		m_pLog->LogException(
					 e,
					 IDS_COMMANDPROCESSOR_MAKEDEFAULTS, 
					 LOGTYPE_ERROR, 
					 m_szClassName );
		e->Delete();
		return false;
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CCommandProcessor Initialize - PUBLIC
// PURPOSE: Initializer
// CALLED BY: User
/////////////////////////////////////////////////////////////////////////////
bool CCommandProcessor::Initialize()
{
	if (!this) return false;

	m_pPacketRouter		= NULL;
	m_pPacketRouter->IncrementInvalidPackets();

	try
	{
		//allocate new modules
		m_pLog				= new CLog;
		m_pPacketRouter		= new CPacketRouter;
		m_pProductsTable	= new CProductsTable;
		m_pMAPI				= new CMAPI;
		m_pTCPIP			= new CTCPIPServer;

		//make sure they are valid
		if ( ! m_pLog ) return false;
		if ( ! m_pPacketRouter ) return false;
		if ( ! m_pProductsTable ) return false;
		if ( ! m_pMAPI ) return false;
		if ( ! m_pTCPIP ) return false;

		m_pLog->Initialize();
		m_pLog->SetCommandProcessor( this );

		m_pMAPI->SetLog( m_pLog );
		m_pTCPIP->SetLog( m_pLog );
		m_pProductsTable->SetLog( m_pLog );
		m_pPacketRouter->SetLog( m_pLog );

		m_pMAPI->SetPacketRouter( m_pPacketRouter );
		m_pTCPIP->SetPacketRouter( m_pPacketRouter );

		m_pPacketRouter->SetCommandProcessor( this );
		m_pPacketRouter->SetProductsTable( m_pProductsTable );
		m_pPacketRouter->SetCMAPI( m_pMAPI );
		m_pPacketRouter->SetTCPIPServer( m_pTCPIP );

		m_pPacketRouter->Initialize();

		m_szBaseFileName = GetCommandLine();
		m_szBaseFileName = m_szBaseFileName.Mid( 1, m_szBaseFileName.Find( "\"", 1 ) - 4 );

		m_bIsInitialized = true;

	}
	catch ( CException * e )
	{
		m_pLog->LogException(
					 e,
					 IDS_COMMANDPROCESSOR_INITIALIZE, 
					 LOGTYPE_ERROR, 
					 m_szClassName );
		e->Delete();
		return false;
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CCommandProcessor Startup - PUBLIC
// PURPOSE: Initializer
// CALLED BY: User
/////////////////////////////////////////////////////////////////////////////
bool CCommandProcessor::Startup( bool bDisableRemoteNodes )
{
	if (!this) return false;
	//MessageBox(NULL,"wait"," ",MB_OK);

	try
	{
		if ( bDisableRemoteNodes == false )
		{
			if ( m_pMAPI->Initialize( m_Defaults.szProfileName,
									   m_Defaults.szProfilePwd,
									   m_Defaults.szAttachmentPath
									  ) == false )
			{
				//return false;
			}
			else
			{
				if ( m_pMAPI->Logon() == false )
				{
					//return false;
				}
			}
		}

		m_pTCPIP->OpenListenPort( m_Defaults.iPortNumber );

		m_pPacketRouter->Startup();
		m_pPacketRouter->SetLocalName( m_Defaults.szLocalName );

		PostMessage( GetWindowHandle(), WM_CP_UPDATE_LIST, FALSE, 0 );
		m_bIsRunning = true;
	}
	catch ( CException * e )
	{
		m_pLog->LogException(
					 e,
					 IDS_COMMANDPROCESSOR_STARTUP, 
					 LOGTYPE_ERROR, 
					 m_szClassName );
		e->Delete();
		return false;
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CCommandProcessor Startup - PUBLIC
// PURPOSE: Initializer
// CALLED BY: User
/////////////////////////////////////////////////////////////////////////////
bool CCommandProcessor::Shutdown()
{
	if (!this) return false;
	try
	{
		m_pMAPI->Logoff();
		m_bIsRunning = false;
	}
	catch ( CException * e )
	{
		m_pLog->LogException(
					 e,
					 IDS_COMMANDPROCESSOR_SHUTDOWN, 
					 LOGTYPE_ERROR, 
					 m_szClassName );
		e->Delete();
		return false;
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////

//####################################################################
//####################################################################
//####################################################################

//####################################################################
//####################################################################
//####################################################################

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CCommandProcessor OnProductReceived - PUBLIC
// PURPOSE: Called as a result of subscribing to CProductRouter
//  and CProductRouter is sending us the packets (product)
// This is based on the Producer/Consumer model, here we decide what
//  to do with the packet; display, execute, discard, etc...
// CALLED BY: CProductRouter
/////////////////////////////////////////////////////////////////////////////
bool CCommandProcessor::OnProductReceived( PBYTE pPacket, int iCount )
{
	//TODO: here we will process commands or process the product
	return true;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CCommandProcessor OnMailCommand - PUBLIC
// PURPOSE: Called when someone sent a message with COMMAND as subject
//  can be used to send logs and other stuff
// this function generates mail or command packets
// this option gives us flexibility to send textual commands to the 
//  system via e-mail only, for via TCP/IP use our protocol
// CALLED BY: CPacketRouter
// text command protocol: name=CmdName;parameters=CmdParameters (comma
//  separated);
/////////////////////////////////////////////////////////////////////////////
bool CCommandProcessor::OnMailCommand( CString& szCommandName, CString& szSource )
{
	CStringArray	saTemp;
	CString			szToken;
	CString			szCommand;
	CString			szParameters;
	int				iType;
	SYSTEMTIME		st;

	try
	{
		//by default is time isn't given we'll produce a report for
		// this month, starting on day 1
		GetSystemTime( &st );
		st.wDay = 1;

		//remove any \n chars
		szCommandName.Replace( 13, ';' );
		szCommandName.Replace( 10, ';' );
		StringToArray ( szCommandName, saTemp, ";" );

		GetCommandNameAndParameters( saTemp, szCommand, szParameters );
		StringToArray ( szParameters, saTemp, "," );

		if ( szCommand == "" ) return false;
		szCommand.MakeUpper();

		//check if command is send log
		if ( COMMAND_IS_SENDLOG )
		{
			//get parameters
			if ( szParameters.GetLength() > 0 )
			{
				//get type
				szToken = saTemp.GetAt(0);
				GetLogType( szToken, &iType, LOGTYPE_ALL );
			}
			if ( saTemp.GetSize() >= 2 )
			{
				//get date
				szToken = saTemp.GetAt(1);
				StringToTimeDate( szToken, &st );
			}
			//SEND LOG
			return OnCmdSendLog( szSource, iType, &st );
		}

		//check if command is send report
		if ( COMMAND_IS_SENDREPORT )
		{
			//get parameters
			if ( szParameters.GetLength() > 0 )
			{
				//get type
				szToken = saTemp.GetAt(0);
				GetReportType( szToken, &iType, REPORTTYPE_ALL );
			}
			if ( saTemp.GetSize() >= 2 )
			{
				//get date
				szToken = saTemp.GetAt(1);
				StringToTimeDate( szToken, &st );
			}
			//SEND REPORT
			return OnCmdSendReport( szSource, iType, &st );
		}
	}
	catch ( CException * e )
	{
		m_pLog->LogException(
					 e,
					 IDS_COMMANDPROCESSOR_ONMAILCOMMAND, 
					 LOGTYPE_ERROR, 
					 m_szClassName );
		e->Delete();
		return false;
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CCommandProcessor OnMail - PUBLIC
// PURPOSE: Regular mail processing, handle undeliverable mail
// CALLED BY: CPacketRouter
/////////////////////////////////////////////////////////////////////////////
bool CCommandProcessor::OnMail( CStringArray * psaMessage )
{
	CString szFileName;
	CString szLine;
	CString szBody;
	CTime	cTime; 

	CStdioFile				m_File;
	CString					m_szMsgFileName;
	CString					m_szUndRepFileName;

	try
	{
		m_szMsgFileName = m_szBaseFileName;
		m_szUndRepFileName = m_szBaseFileName;
		m_szMsgFileName += "msg";
		m_szUndRepFileName += "und";


		szLine = psaMessage->GetAt( MESSAGE_SUBJECT );
		szLine.MakeUpper();
		if ( szLine.Find( szUNDELIVERABLE ) == -1 )
		{
			//regular message
			szFileName = m_szMsgFileName;
			if ( m_pLog )
				m_pLog->Log( "Received mail message", 
							 LOGTYPE_MESSAGE, 
							 m_Defaults.szLocalName );
		}
		else
		{
			//undeliverable message
			szFileName = m_szUndRepFileName;
			if ( m_pLog )
				m_pLog->Log( "Received undeliverable mail! ", 
							 LOGTYPE_WARNING, 
							 m_Defaults.szLocalName );
		}

		cTime = CTime::GetCurrentTime();
		szLine = "________________________________________________________\n¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\n";
		szLine += cTime.Format("%Y.%m.%d %H:%M:%S %A, %B %d %Y, %Z");

		szLine += "\nSender:           ";
		szLine += psaMessage->GetAt( MESSAGE_SENDER );
		szLine += "\nRecipients:       ";
		szLine += psaMessage->GetAt( MESSAGE_DESTINATION );
		szLine += "\nSubject:          ";
		szLine += psaMessage->GetAt( MESSAGE_SUBJECT );

		if ( MESSAGE_ATTACHMENTS_START < psaMessage->GetSize() )
		{
			szLine += "\nAttachment Path:  ";
			szLine += m_Defaults.szAttachmentPath;
			szLine += "\nAttachments:      ";

			for ( int i=MESSAGE_ATTACHMENTS_START; i<psaMessage->GetSize(); i++ )
			{
				szLine += psaMessage->GetAt(i);
				szLine += ",";
			}
		}
		szBody = psaMessage->GetAt( MESSAGE_BODY );
		if ( szBody.GetLength() > 0 )
		{
			szLine += "\n\nMessage: \n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n";
			szLine += szBody;
			szLine += "\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^";
		}
		szLine += "\n________________________________________________________\n¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\n\n";

		m_File.Open( szFileName, 
					 CFile::modeWrite | 
					 CFile::typeText | 
					 CFile::shareDenyWrite |
					 CFile::modeNoTruncate |
					 CFile::modeCreate );

		m_File.SeekToEnd();
		szLine += "\n";
		m_File.WriteString( szLine );
		m_File.Flush();
		m_File.Close();
	}
	catch ( CException * e )
	{
		m_pLog->LogException(
					 e,
					 IDS_COMMANDPROCESSOR_ONMAIL, 
					 LOGTYPE_ERROR, 
					 m_szClassName );
		e->Delete();
		return false;
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CCommandProcessor OnCmdSendLog - PUBLIC
// PURPOSE: 
// CALLED BY: 
/////////////////////////////////////////////////////////////////////////////
bool CCommandProcessor::OnCmdSendLog( CString& szDestination, int iLogType, SYSTEMTIME * pSysTime )
{
	try
	{
		CStringArray saTo;
		CStringArray saAttachments;
		CString szSubject;
		CString szBody;
		CString szAttachmentName;
		CString szLine;
		CStdioFile fLogReport;
		CStdioFile * pfLogFile = m_pLog->GetLogFile();
		BOOL bOpenLog;
		BOOL bOpenReportFile;

		COleDateTime dtReport;
		COleDateTime dtLogEntry;
		WORD wYear, wMonth, wDay;

		CString szFindTime;
		CString szWarning = szWARNING;
		CString szError = szERROR;

		CTime cTime( (SYSTEMTIME)*pSysTime );

		CString szReportFile = m_Defaults.szAttachmentPath;
		szReportFile += "SENDLOG.TXT";


		dtReport.SetTime(0,0,0);
		dtLogEntry.SetTime(0,0,0);


		dtReport.SetDate(   (int)pSysTime->wYear, 
							(int)pSysTime->wMonth, 
							(int)pSysTime->wDay );

	//n=log,p=3

		bOpenLog = pfLogFile->Open(  pfLogFile->GetFilePath(), 
									 CFile::modeRead | 
									 CFile::typeText | 
									 CFile::shareDenyNone );
		if ( bOpenLog )
		{
			while( pfLogFile->ReadString(szLine) == TRUE )
			{
				if ( szLine.GetLength() > 11 )
				{
					szBody = szLine.Mid(0,4);
					wYear = (WORD)atol(szBody);
					szBody = szLine.Mid(5,2);
					wMonth = (WORD)atol(szBody);
					szBody = szLine.Mid(8,2);
					wDay = (WORD)atol(szBody);

					dtLogEntry.SetDate(	(int)wYear, 
										(int)wMonth, 
										(int)wDay );

					if ( dtLogEntry >= dtReport )
						break;
				}
			}

			bOpenReportFile = fLogReport.Open(   szReportFile, 
												 CFile::modeWrite | 
												 CFile::typeText | 
												 CFile::shareDenyWrite |
												 CFile::modeCreate );

			if ( bOpenReportFile )
			{
				while( pfLogFile->ReadString(szLine) == TRUE )
				{
					szLine += "\n";
					switch ( iLogType )
					{
						case LOGTYPE_WARNING:
							if ( szLine.Find(szError,10) != -1 ||
								 szLine.Find(szWarning,10) != -1 )
								fLogReport.WriteString(szLine);
							break;
						case LOGTYPE_ERROR:
							if ( szLine.Find(szError,10) != -1 )
								fLogReport.WriteString(szLine);
							break;
						case LOGTYPE_ALL:
						case LOGTYPE_MESSAGE:
						default:
							fLogReport.WriteString(szLine);
							break;
					}
				}
				fLogReport.Flush();
				fLogReport.Close();
				saAttachments.Add( szReportFile );
			}
			pfLogFile->Close();
		}


		szSubject	= "LOG";
		switch ( iLogType )
		{
			case LOGTYPE_ALL:
				szBody = szALL;
				break;
			case LOGTYPE_MESSAGE:
				szBody = szMESSAGE;
				break;
			case LOGTYPE_WARNING:
				szBody = szWARNING;
				break;
			case LOGTYPE_ERROR:
				szBody = szERROR;
				break;
		}

		szBody += " LOG FROM: ";
		szBody += m_pMAPI->GetLoggedOnName();
		szBody += cTime.Format( " FOR: %A, %B %d %Y" );

	//DEBUG
	if ( m_Defaults.szLocalName == szDestination )
		saTo.Add( m_Defaults.szLocalAddress );
	else
		saTo.Add( szDestination );

		m_pMAPI->SendMessage(  &saTo, &saAttachments,
							   (LPSTR)(LPCTSTR)szSubject,
							   (LPSTR)(LPCTSTR)szBody );
	}
	catch ( CException * e )
	{
		m_pLog->LogException(
					 e,
					 IDS_COMMANDPROCESSOR_ONCMDSENDLOG, 
					 LOGTYPE_ERROR, 
					 m_szClassName );
		e->Delete();
		return false;
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CCommandProcessor OnCmdSendReport - PUBLIC
// PURPOSE: 
// CALLED BY: 
/////////////////////////////////////////////////////////////////////////////
bool CCommandProcessor::OnCmdSendReport( CString& szDestination, int iReportType, SYSTEMTIME * pSysTime )
{
	CStringArray saTo;
	CStringArray saAttachments;
	CString szSubject;
	CString szBody;
	CString szAttachmentName;

	try
	{
		CTime cTime( (SYSTEMTIME)*pSysTime );

		szSubject	= "REPORT";

		switch ( iReportType )
		{
			case REPORTTYPE_ALL:
				szBody = szALL;
				break;
			case REPORTTYPE_MESSAGE:
				szBody = szMESSAGE;
				break;
			case REPORTTYPE_WARNING:
				szBody = szWARNING;
				break;
			case REPORTTYPE_ERROR:
				szBody = szERROR;
				break;
		}

		szBody += " REPORT FROM: ";
		szBody += m_pMAPI->GetLoggedOnName();
		szBody += cTime.Format( " FOR: %A, %B %d %Y" );

		saTo.Add( szDestination );
		m_pMAPI->SendMessage(  &saTo, &saAttachments,
							   (LPSTR)(LPCTSTR)szSubject,
							   (LPSTR)(LPCTSTR)szBody );
	}
	catch ( CException * e )
	{
		m_pLog->LogException(
					 e,
					 IDS_COMMANDPROCESSOR_ONCMDSENDRPT, 
					 LOGTYPE_ERROR, 
					 m_szClassName );
		e->Delete();
		return false;
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CCommandProcessor OnAppLogError - PUBLIC
// PURPOSE: 
// CALLED BY: 
/////////////////////////////////////////////////////////////////////////////
bool CCommandProcessor::OnAppLogError( LPCTSTR szError )
{
	try
	{
		if ( m_Defaults.bDisableRemoteNodes == true )
			return true;

		CString szSubject	= "APPLICATION ERORR or WARNING";
		m_pMAPI->SendMessage(  &m_saNotifyAddresses, NULL,
							   (LPSTR)(LPCTSTR)szSubject,
							   (LPSTR)szError );
	}
	catch ( CException * e )
	{
		m_pLog->LogException(
					 e,
					 IDS_COMMANDPROCESSOR_ONCMDSENDRPT, 
					 LOGTYPE_ERROR, 
					 m_szClassName );
		e->Delete();
		return false;
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CCommandProcessor Send - PUBLIC
// PURPOSE: 
// CALLED BY: 
/////////////////////////////////////////////////////////////////////////////
bool CCommandProcessor::Send()
{
	return true;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CCommandProcessor GetCommandNameAndParameters - PRIVATE
// PURPOSE: very specific to OnMailCommand
// CALLED BY: 
/////////////////////////////////////////////////////////////////////////////
void CCommandProcessor::GetCommandNameAndParameters( CStringArray& saTokens, CString& szCommand, CString& szParameters )
{
	CString			szToken;
	int				i;
	int				iStart;
	try
	{
		for ( i=0; i<saTokens.GetSize(); i++ )
		{
			szToken = saTokens.GetAt( i );
			switch ( szToken[0] )
			{
			case 'n':
			case 'N':
				//get command name
				iStart = szToken.Find('=');
				//if we do not get a valid command name exit
				if ( iStart == -1 ) return;
				szCommand = szToken.Right( szToken.GetLength() - iStart-1 );
				break;
			case 'p':
			case 'P':
				//get command parameters
				iStart = szToken.Find('=');
				if ( iStart != -1 )
				{
					szParameters = szToken.Right( szToken.GetLength() - iStart-1 );
				}
				break;
			}
		}
	}
	catch ( CException * e )
	{
		m_pLog->LogException(
					 e,
					 IDS_COMMANDPROCESSOR_SEND, 
					 LOGTYPE_ERROR, 
					 m_szClassName );
		e->Delete();
		return;
	}
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CCommandProcessor GetLogType - PRIVATE
// PURPOSE: very specific to SendLog
// CALLED BY: 
/////////////////////////////////////////////////////////////////////////////
void CCommandProcessor::GetLogType( CString& szToken, int * piType, int iDefault )
{
	*piType = iDefault;
	try
	{
		if ( szToken.GetLength() > 0 )
		{
			szToken.MakeUpper();

			if ( TokenToID ( szToken, 
							 piType, 
							 szALL, 
							 LOGTYPE_ALL
							) == false )
				if ( TokenToID ( szToken, 
								 piType, 
								 szMESSAGE, 
								 LOGTYPE_MESSAGE
								) == false )
					if ( TokenToID ( szToken, 
									 piType, 
									 szWARNING, 
									 LOGTYPE_WARNING
									) == false )
						TokenToID ( szToken, 
									piType, 
									szERROR, 
									LOGTYPE_ERROR
								   );
		}
	}
	catch ( CException * e )
	{
		m_pLog->LogException(
					 e,
					 IDS_COMMANDPROCESSOR_GETLOGTYPE, 
					 LOGTYPE_ERROR, 
					 m_szClassName );
		e->Delete();
		return;
	}
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CCommandProcessor GetReportType - PRIVATE
// PURPOSE: very specific to SendReport
// CALLED BY: 
/////////////////////////////////////////////////////////////////////////////
void CCommandProcessor::GetReportType( CString& szToken, int * piType, int iDefault )
{
	*piType = iDefault;
	try
	{
		if ( szToken.GetLength() > 0 )
		{
			szToken.MakeUpper();

			if ( TokenToID ( szToken, 
							 piType, 
							 szALL, 
							 REPORTTYPE_ALL
							) == false )
				if ( TokenToID ( szToken, 
								 piType, 
								 szMESSAGE, 
								 REPORTTYPE_MESSAGE
								) == false )
					if ( TokenToID ( szToken, 
									 piType, 
									 szWARNING, 
									 REPORTTYPE_WARNING
									) == false )
						TokenToID ( szToken, 
									piType, 
									szERROR, 
									REPORTTYPE_ERROR
								   );
		}
	}
	catch ( CException * e )
	{
		m_pLog->LogException(
					 e,
					 IDS_COMMANDPROCESSOR_GETRPTTYPE, 
					 LOGTYPE_ERROR, 
					 m_szClassName );
		e->Delete();
		return;
	}
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// StringToTimeDate
// PURPOSE: 
// CALLED BY: 
/////////////////////////////////////////////////////////////////////////////
bool StringToTimeDate ( CString& szTimeDate, SYSTEMTIME * pSysTime )
{
	try
	{
		COleVariant		inDateString;
		COleVariant		outDate;
		if ( szTimeDate.GetLength() > 2 )
		{
			
			inDateString = szTimeDate;
			VariantChangeTypeEx( &outDate,
								 &inDateString,
								 LOCALE_USER_DEFAULT,
								 0,
								 VT_DATE );

			VariantTimeToSystemTime( outDate.date, pSysTime );
			return true;
		}
	}
	catch ( CException * e )
	{
		e->Delete();
		return false;
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TokenToID
// PURPOSE: 
// CALLED BY: 
/////////////////////////////////////////////////////////////////////////////
bool TokenToID ( CString& szToken, int * piID, LPCTSTR szFind, int iSet )
{
	int iChar;
	try
	{
		if ( isdigit( (int)szToken[0] ) != 0 )
		{
			iChar = (int)szToken[0] - 48;
			if ( iSet == (int)szToken[0] - 48 )
			{
				*piID = iSet;
				return true;
			}
		}
		else if ( szToken.Find( szFind ) != -1 )
		{
			*piID = iSet;
			return true;
		}
	}
	catch ( CException * e )
	{
		e->Delete();
		return false;
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// StringToArray
// PURPOSE: 
// CALLED BY: 
/////////////////////////////////////////////////////////////////////////////
bool StringToArray ( CString& szString, CStringArray& saTokens, LPCTSTR szDelimiter )
{
	char			pData[2048] ;
	CHAR *			token;

	try
	{
		saTokens.RemoveAll();

		if ( szString.GetLength() > 0 )
		{
			//tokenize command string
			strcpy( pData, szString );
			token = strtok( pData, szDelimiter );   
			while( token != NULL )
			{
				if ( token != 0 )
					saTokens.Add ( (LPCTSTR)token ) ;
				token = strtok( NULL, szDelimiter ) ;
			}
			return true;
		}
	}
	catch ( CException * e )
	{
		e->Delete();
		return false;
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// StringToArray
// PURPOSE: 
// CALLED BY: 
/////////////////////////////////////////////////////////////////////////////
bool StringToArray2 ( CString& szString, CStringArray& saTokens, LPCTSTR szDelimiter )
{
	char			pData[2048] ;
	CHAR *			token;
	CString			szTemp;

	try
	{
		saTokens.RemoveAll();

		if ( szString.GetLength() > 0 )
		{
			//tokenize command string
			strcpy( pData, szString );
			token = strtok( pData, szDelimiter );
			while( token != NULL )
			{
				if ( token != 0 )
					szTemp = (LPCTSTR)token;

				szTemp.TrimLeft();

				if ( szTemp != "" )
					saTokens.Add ( (LPCTSTR)token );
				token = strtok( NULL, szDelimiter );
			}
			return true;
		}
	}
	catch ( CException * e )
	{
		e->Delete();
		return false;
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// operator <<
// PURPOSE: Store
// CALLED BY: 
/////////////////////////////////////////////////////////////////////////////
CArchive& operator <<( CArchive& ar, CCommandProcessor* pCP )
{
	if (!pCP) return ar;
	CLog * pLog = pCP->GetLog();
	try
	{
		CString szLoad;
		ES_DEFAULTS * pDefaults = pCP->GetDefaults();
		CProductsTable * pPT = pCP->GetProductsTable();
		CStringArray * pNA = pCP->GetNotifyAddresses();

		if (!pDefaults) return ar;

		CString szTemp;
		CString szCRLF = "\r\n";

		//store defaults
		szTemp = "PROGRAM SETTINGS\r\n";
		ar.WriteString( szTemp );

		szTemp = "Local Name: ";
		ar.WriteString( szTemp );
		ar.WriteString( pDefaults->szLocalName );
		ar.WriteString( szCRLF );
		
		szTemp = "Local E-Mail Address: ";
		ar.WriteString( szTemp );
		ar.WriteString( pDefaults->szLocalAddress );
		ar.WriteString( szCRLF );
		
		szTemp = "Mail Profile: ";
		ar.WriteString( szTemp );
		ar.WriteString( pDefaults->szProfileName );
		ar.WriteString( szCRLF );
		
		szTemp = "Mail Password: ";
		ar.WriteString( szTemp );
		ar.WriteString( pDefaults->szProfilePwd );
		ar.WriteString( szCRLF );
		
		szTemp = "Attachment Path: ";
		ar.WriteString( szTemp );
		ar.WriteString( pDefaults->szAttachmentPath );
		ar.WriteString( szCRLF );

		szTemp.Format( "TCP/IP Listen Port: %d\r\n", pDefaults->iPortNumber );
		ar.WriteString( szTemp );

		szTemp = "Disable Remote Nodes: ";
		ar.WriteString( szTemp );
		if ( pDefaults->bDisableRemoteNodes == true )
			szTemp = "True";
		else
			szTemp = "False";
		ar.WriteString( szTemp );
		ar.WriteString( szCRLF );

		szTemp = "Auto Save: ";
		ar.WriteString( szTemp );
		if ( pDefaults->bAutoSave == true )
			szTemp = "True";
		else
			szTemp = "False";
		ar.WriteString( szTemp );
		ar.WriteString( szCRLF );

		ar.WriteString( szCRLF );
		ar.WriteString( szCRLF );

		//store notification addresses
		szLoad.LoadString(IDS_COMMANDPROCESSOR_ARCHIVESTORE_NA);
		szTemp.Format( szLoad, pNA->GetSize() );
		ar.WriteString( szTemp );

		for( int i=0; i<pNA->GetSize(); i++ )   
		{
			ar.WriteString( pNA->GetAt(i) );
			ar.WriteString( szCRLF );
		}
		ar.WriteString( szCRLF );
		ar.WriteString( szCRLF );

		if (!pPT) return ar;
		ar << pPT;
	}
	catch ( CException * e )
	{
		if ( pLog )
			pLog->LogException(
						 e,
						 IDS_COMMANDPROCESSOR_ARCHIVESTORE, 
						 LOGTYPE_ERROR, 
						 "Archive Load" );
		e->Delete();
		return ar;
	}
	return ar;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// operator >>
// PURPOSE: Load
// CALLED BY: 
/////////////////////////////////////////////////////////////////////////////
CArchive& operator >>( CArchive& ar, CCommandProcessor* pCP )
{
	if (!pCP) return ar;
	CLog * pLog = pCP->GetLog();
	try
	{
		int i, iCount;
		CString szLoad;
		ES_DEFAULTS * pDefaults = pCP->GetDefaults();
		CProductsTable * pPT = pCP->GetProductsTable();
		CStringArray * pNA = pCP->GetNotifyAddresses();
		if (!pDefaults) return ar;

		CString szTemp;
		//load defaults
		for ( i=0; i<7; i++ )
		{
			ar.ReadString( szTemp );
			if ( szTemp.Find(":",5) != -1 )
				break;
		}

		szTemp = szTemp.Mid( szTemp.Find(":",5)+1 );
		szTemp.TrimLeft();
		szTemp.TrimRight();
		pDefaults->szLocalName = szTemp;

		ar.ReadString( szTemp );
		szTemp = szTemp.Mid( szTemp.Find(":",5)+1 );
		szTemp.TrimLeft();
		szTemp.TrimRight();
		pDefaults->szLocalAddress = szTemp;

		ar.ReadString( szTemp );
		szTemp = szTemp.Mid( szTemp.Find(":",5)+1 );
		szTemp.TrimLeft();
		szTemp.TrimRight();
		pDefaults->szProfileName = szTemp;

		ar.ReadString( szTemp );
		szTemp = szTemp.Mid( szTemp.Find(":",5)+1 );
		szTemp.TrimLeft();
		szTemp.TrimRight();
		pDefaults->szProfilePwd = szTemp;

		ar.ReadString( szTemp );
		szTemp = szTemp.Mid( szTemp.Find(":",5)+1 );
		szTemp.TrimLeft();
		szTemp.TrimRight();
		pDefaults->szAttachmentPath = szTemp;

		ar.ReadString( szTemp );
		szTemp = szTemp.Mid( szTemp.Find(":",5)+1 );
		szTemp.TrimLeft();
		szTemp.TrimRight();
		pDefaults->iPortNumber = atoi(szTemp);

		ar.ReadString( szTemp );
		szTemp = szTemp.Mid( szTemp.Find(":",5)+1 );
		szTemp.TrimLeft();
		szTemp.TrimRight();
		szTemp.MakeUpper();
		if ( szTemp == "FALSE" || szTemp == "0" || szTemp == "" )
			pDefaults->bDisableRemoteNodes = false;
		else
			pDefaults->bDisableRemoteNodes = true;

		ar.ReadString( szTemp );
		szTemp = szTemp.Mid( szTemp.Find(":",5)+1 );
		szTemp.TrimLeft();
		szTemp.TrimRight();
		szTemp.MakeUpper();
		if ( szTemp == "FALSE" || szTemp == "0" || szTemp == "" )
			pDefaults->bAutoSave = false;
		else
			pDefaults->bAutoSave = true;

#define MAX_FILE_INVALID_LINES 15

		//load notification addresses
		for ( i=0; i<MAX_FILE_INVALID_LINES; i++ )
		{
			ar.ReadString( szTemp );
			szLoad.LoadString(IDS_COMMANDPROCESSOR_F1);
			if ( szTemp.Find(szLoad,0) != -1 )
				break;
		}
		szTemp = szTemp.Mid( szTemp.Find(":",3)+1 );
		szTemp.TrimLeft();
		szTemp.TrimRight();
		iCount = atoi(szTemp);

		for ( i=0; i<MAX_FILE_INVALID_LINES; i++ )
		{
			ar.ReadString( szTemp );
			if ( szTemp.Find("@",0) != -1 )
				break;
		}

		pNA->RemoveAll();
		for ( i=0; i<iCount; i++ )
		{
			szTemp.TrimLeft();
			szTemp.TrimRight();
			pNA->Add(szTemp);
			ar.ReadString( szTemp );
		}

		if (!pPT) return ar;
		ar >> pPT;
	}
	catch ( CException * e )
	{
		if ( pLog )
			pLog->LogException(
						 e,
						 IDS_COMMANDPROCESSOR_ARCHIVELOAD, 
						 LOGTYPE_ERROR, 
						 "Archive Load" );
		e->Delete();
		return ar;
	}
	return ar;
}
/////////////////////////////////////////////////////////////////////////////

HWND CCommandProcessor::GetWindowHandle		(void){ if (!this) return NULL; return m_pLog->GetWindowHandle(); }
bool CCommandProcessor::SetWindowHandle		( HWND hWnd ){ 	if (!this) return false; return m_pLog->SetWindowHandle( hWnd ); }
bool CCommandProcessor::SetLogListCtrl		( CListCtrl * pListCtrl ){ if (!this) return false; return m_pLog->SetLogListCtrl( pListCtrl ); }
bool CCommandProcessor::IsInitialized		(){ if (!this) return false; return m_bIsInitialized; }
bool CCommandProcessor::IsRunning			(){ if (!this) return false; return m_bIsRunning; }

bool CCommandProcessor::SetProductViewListCtrl( CListCtrl * pListCtrl ){ if (!this) return false; return m_pProductsTable->SetProductViewListCtrl( pListCtrl ); }
void CCommandProcessor::SetCurrentView		( CString& szViewNode, int iViewType ){ if (!this) return; m_pProductsTable->SetCurrentView( szViewNode, iViewType ); }
void CCommandProcessor::SetCurrentView		( int iView ){ if (!this) return; m_pProductsTable->SetCurrentView( iView ); }
