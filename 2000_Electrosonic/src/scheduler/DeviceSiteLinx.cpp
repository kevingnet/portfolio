#include "stdafx.h"
#include "resource.h"
#include "DeviceSiteLinx.h"
#include "DlgCueSiteLinx.h"
#include "ppgIPAddress.h"
#include "ppgSiteLinxActions.h"
#include "Scheduler.h"
#include "SchedulerDoc.h"
#include "SchedulerTreeView.h"
#include "Utility.h"
#include "ByteArrayHelpers.h"
#include "EMailGroupsCollection.h"

#include "FileVersion.h"

PCHAR szSendCommand = "Send e-mail ";
PCHAR szMessage = "Message";
PCHAR szWarning = "Warning";
PCHAR szError = "Error";

#define NumberOf(x)	(sizeof(x)/sizeof(x[0]))

CString CDeviceSiteLinx::m_szGlobal = "*";
PCHAR CDeviceSiteLinx::m_DeviceType = (LPSTR)(LPCTSTR)g_szSiteLinxName;

CDeviceSiteLinx::CDeviceSiteLinx()
{
	if ( !this ) return;
	m_iByteLast				= 0;
//	m_paPtrArray			= NULL;
//	m_pConsumerRegistration	= NULL;
	m_pSchedulerDoc			= NULL;
	m_bConnected			= false;
	m_bReloadDocument		= false;
	m_bCanReloadDocument	= true;

	m_iLast	= 0;
}

CDeviceSiteLinx::~CDeviceSiteLinx()
{
	//DeleteTables();
}

void CDeviceSiteLinx::GetDeviceType( CString& str )
{
	str = m_DeviceType;
}

void CDeviceSiteLinx::Idle( int iActiveType )
{
	static int Counter = gpApp->m_Defaults.iSiteLinxConnectionRetry;
	if( !IsOpen() && IsConnectionResolved() && (++Counter >= gpApp->m_Defaults.iSiteLinxConnectionRetry) )
	{
		GoOnline(true);
		Counter = 0;
	}
	CBaseDevice::Idle( iActiveType );
}

void CDeviceSiteLinx::Serialize( CArchive& ar )
{
	CBaseDevice::Serialize( ar );

	enum {
		VERSION_BASE,
		VERSION_CURRENT = VERSION_BASE,
	};
	CVersion V( ar, VERSION_CURRENT );

	if ( ar.IsLoading() ) {
		CString IPAddress = gpApp->GetProfileString( gpApp->m_szRegName, "Address", "" );
		UINT	Port		= gpApp->GetProfileInt( gpApp->m_szRegName, "TCPIP Port", 0 );
		SetAddress( IPAddress, Port );
		m_slDefaults.szNodeName =
			gpApp->GetProfileString( gpApp->m_szRegName, "Node Name", "configure me now" );
		SetDeviceName( m_slDefaults.szNodeName );
	}

}

bool CDeviceSiteLinx::EditCue( CByteArray& Cue )
{
	CDlgCueSiteLinx dlgCueSL( m_pSchedulerDoc );
	//cue contains 1) byType 2) szTo 3) szTitle 4) szText
	if( Cue.GetSize() ) 
	{
		int iIdx=1;
		dlgCueSL.m_cType = Cue[0];
		iIdx = CopyData2( dlgCueSL.m_szTo, iIdx, Cue );
		iIdx = CopyData2( dlgCueSL.m_szTitle, iIdx, Cue );
		iIdx = CopyData2( dlgCueSL.m_szText, iIdx, Cue );
	}
	if( dlgCueSL.DoModal() == IDOK ) 
	{
		if( dlgCueSL.m_szTo != "" &&
			dlgCueSL.m_szTitle != "" )
		{
			Cue.SetSize( 1 );
			Cue.SetAtGrow( 0, dlgCueSL.m_cType );
			AppendData( Cue, dlgCueSL.m_szTo );
			AppendData( Cue, dlgCueSL.m_szTitle );
			AppendData( Cue, dlgCueSL.m_szText );
			return true;
		}
	}
	return false;
}

bool CDeviceSiteLinx::GetCueText( CByteArray& Cue, CString& text )
{
	if( Cue.GetSize() )  
	{
		text = szSendCommand;
		switch( Cue[0] )
		{
		case 0:
			text += szMessage;
			break;
		case 1:
			text += szWarning;
			break;
		case 2:
			text += szError;
			break;
		}
		int iIdx=1;
		CString szTo;
		CString szTitle;
		//CString szText;
		iIdx = CopyData2( szTo, iIdx, Cue );
		iIdx = CopyData2( szTitle, iIdx, Cue );
		text += " : ";
		text += szTitle;
		text += " To: ";
		text += szTo;
		//iIdx = CopyData2( szText, iIdx, Cue );
	}
	else
	{
		text = "Invalid Command";
		return false;
	}
	return true;
}

void CDeviceSiteLinx::OnRenameEmailGroup( CByteArray& baData, LPCTSTR lpOldName, LPCTSTR lpNewName)
{
	if( lpOldName && *lpOldName && 
		lpNewName && *lpNewName &&
		baData.GetSize() > 4 )
	{
		int iIdx=1;
		BYTE bCommandType = baData[0];
		CString szTo;
		CString szTitle;
		CString szText;
		iIdx = CopyData2( szTo, iIdx, baData );
		if( szTo == lpOldName )
		{
			szTo = lpNewName;
			iIdx = CopyData2( szTitle, iIdx, baData );
			iIdx = CopyData2( szText, iIdx, baData );
			baData.RemoveAll();
			baData.SetSize( 1 );
			baData.SetAtGrow( 0, bCommandType );
			AppendData( baData, szTo );
			AppendData( baData, szTitle );
			AppendData( baData, szText );
		}
	}
}

bool CDeviceSiteLinx::FireEvent( CByteArray& Cue, LPCTSTR lpTriggeringDevice )
{
	int iType = LEVEL_DEVICE_SEND;
	if( m_pSchedulerDoc && Cue.GetSize() )  
	{
		CString szTemp;
		CString szTitle = "ESCAN ";
		CString szText;
		CString szTo;
		switch( Cue[0] )
		{
		case 0:
			szTitle += szMessage;
			break;
		case 1:
			szTitle += szWarning;
			break;
		case 2:
			szTitle += szError;
			break;
		}
		szTitle += " :";
		int iIdx=1;
		iIdx = CopyData2( szTo, iIdx, Cue );
		iIdx = CopyData2( szTemp, iIdx, Cue );
		iIdx = CopyData2( szText, iIdx, Cue );
		szTitle += szTemp;

		CStringArray saUsers;
		CEMailGroupsCollection * pColl = m_pSchedulerDoc->GetEMailGroupsCollection();
		pColl->GetUserNames( &saUsers, szTo );

		CString str;
		if( !IsOpen() )
		{
			iType = LEVEL_DEVICE_OFFLINE;
			str = "Off line, could not send e-mail: ";
			str += szTitle;
		}
		else
		{
			str = "SiteLinx sent e-mail: ";
			str += szTitle;
		}
		if( lpTriggeringDevice && *lpTriggeringDevice )
		{
			iType = LEVEL_EXTERNAL_TRIGGER;
			str += " by Device: ";
			str += lpTriggeringDevice;
		}
		SendToLog( str, iType );
		SendEMail( szTitle, szText, saUsers );
	}
	if( iType == LEVEL_DEVICE_SEND )
		return true;
	return false;
}

void CDeviceSiteLinx::SendEMail( CString& szTitle, CString& szText, CStringArray& saTo )
{
	BYTE Buff[10];
	Buff[0] = 1;
	Buff[1] = '9';
	Buff[2] = '9';
	Buff[3] = '9';
	Buff[4] = '9';
	Buff[5] = 0;
	Buff[6] = 0;
	Buff[7] = 2;
	SendPacket( Buff, 8 );

	BYTE bSeparator = 4;
	SendPacket( (PBYTE)(LPCTSTR)szTitle, szTitle.GetLength() );
	SendPacket( &bSeparator, 1 );
	SendPacket( (PBYTE)(LPCTSTR)szText, szText.GetLength() );
	SendPacket( &bSeparator, 1 );

	for( int i=0; i<saTo.GetSize(); i++ )
	{
		SendPacket( (PBYTE)(LPCTSTR)saTo.GetAt(i), saTo.GetAt(i).GetLength() );
		SendPacket( &bSeparator, 1 );
	}

	bSeparator = 0;
	SendPacket( &bSeparator, 1 );
	BYTE b = 3;
	SendPacket( &b, 1 );
	return;
}

void CDeviceSiteLinx::AddPropertyPages( CPropertySheet& Sheet )
{
	CppgIPAddress * pPPG = (CppgIPAddress*)Sheet.GetPage(0);
	m_ppgActions = new CppgSiteLinxActions(this, pPPG );
	Sheet.AddPage( m_ppgActions );
}

void CDeviceSiteLinx::DeletePropertyPages( bool /*bUpdate*/ )
{
	if( m_ppgActions ) 
	{
		delete m_ppgActions;
		m_ppgActions = NULL;
	}
}

bool CDeviceSiteLinx::Configure( void )
{
	char Buff[70];
	DWORD dwBuffSize = sizeof(Buff);
	GetComputerName( Buff, &dwBuffSize );

	CString szAddress	= gpApp->GetProfileString( gpApp->m_szRegName, "Address", Buff );
	UINT	uiPort		= gpApp->GetProfileInt( gpApp->m_szRegName, "TCPIP Port", 2000 );

	m_slDefaults.szAddress = szAddress;
	m_slDefaults.iPortNumber = uiPort;

	m_slDefaults.szNodeName =
		gpApp->GetProfileString( gpApp->m_szRegName, "Node Name", "SchedulerSL" );

	m_slDefaults.bAutoUpdate = true;
	if ( (int)gpApp->GetProfileInt( gpApp->m_szRegName, "Auto Update", 1 ) == 0 )
		m_slDefaults.bAutoUpdate = false;

	bool bRet = CBaseDevice::Configure();

	RetrieveAddress( szAddress, uiPort );

	m_slDefaults.szAddress		= szAddress;
	m_slDefaults.iPortNumber	= uiPort;

	gpApp->WriteProfileString( gpApp->m_szRegName,
		"Address", m_slDefaults.szAddress );

	gpApp->WriteProfileInt( gpApp->m_szRegName, 
		"TCPIP Port", m_slDefaults.iPortNumber );
	
	return bRet;
}

void CDeviceSiteLinx::OnReceiveProduct( int iType, LPCTSTR pSourceName, PBYTE pData, int iDataSize )
{
	CString szLog;
	UINT iFileType = 0;
	UINT iFileLen = 0;
	CString szFileName;
	PBYTE pProductData = 0;
	int iDataCount = 0;
	CByteArray baData;

	switch ( iType )
	{
		case PRODUCT_TYPE_REQUESTUPDATE:
		case PRODUCT_TYPE_RECEIVEUPDATE:
			iFileType = (UINT)*(pData)-FRAME_OFFSET_CHAR;
			iFileLen = (UINT)*(pData+2)-FRAME_OFFSET_CHAR;
			szFileName = (LPCTSTR)(pData+FRAME_OFFSET_CHAR);
			pProductData = (pData+iFileLen+FILE_NAME_LOCATION);
			iDataCount = iDataSize - iFileLen - FILE_NAME_LOCATION;
			break;

		case PRODUCT_TYPE_SENDLOG:
		case PRODUCT_TYPE_SITELINXDEVICENAME:
		case PRODUCT_TYPE_SETCONNECTIONSTATE:
		case PRODUCT_TYPE_EXEC_SCHEDULER_COMMAND:
		case PRODUCT_TYPE_SENDACTIVESTATE:
		case PRODUCT_TYPE_GETACTIVESTATE:
		case PRODUCT_TYPE_SETACTIVESTATE:
		case PRODUCT_TYPE_DEVICESTATUS:
		case PRODUCT_TYPE_SEQUENCESTATUS:
		case PRODUCT_TYPE_SEQUENCE:
		case PRODUCT_TYPE_DEVICE:
		case PRODUCT_TYPE_TRIGGERGROUP:
		case PRODUCT_TYPE_EMAILGROUP:
		case PRODUCT_TYPE_VARIABLESGROUP:
		case PRODUCT_TYPE_ALLSTATUS:
		{
			pProductData = pData;
			iDataCount = iDataSize;
			CString szName = (LPCTSTR)(pProductData);
			//skip document name
			pProductData += szName.GetLength()+1;
			iDataCount -= szName.GetLength()+1;
			//cancel if DocName != this DocName
			if( !m_pSchedulerDoc || szName != GetDocName() )
				return;
		}
			break;

		default:
			pProductData = pData;
			iDataCount = iDataSize;
			break;
	}

	switch ( iType )
	{
		case PRODUCT_TYPE_REQUESTUPDATE:
			if ( iFileType == FILE_TYPE_SCHEDULER )
			{
				OnReceiveUpdate( pSourceName, pProductData, iDataCount, iFileType, szFileName );
			}
			else
			{
				szLog = "Requested update for non scheduler file";
			}
			break;
		case PRODUCT_TYPE_RECEIVEUPDATE:
			if ( iFileType == FILE_TYPE_SCHEDULER )
			{
				OnReceiveUpdate( pSourceName, pProductData, iDataCount, iFileType, szFileName );
			}
			else
			{
				szLog = "Received update for non scheduler file";
			}
			break;
		case PRODUCT_TYPE_SENDLOG:
			{
				CStringArray saLog;
				CString szLog = (LPCTSTR)(pProductData);
				StringToArray( szLog, saLog, "\4" );
				if( saLog.GetSize() != 4 )return;
				CLog::Write( GetDocName(), 
							 saLog.GetAt(LOGPOS_MESSAGE),
							 (BYTE)(szLog[0]-5), //Level
							 (BYTE)(szLog[1]-5), //Type
							 true,
							 saLog.GetAt(LOGPOS_TIME) );
			}
			break;
		case PRODUCT_TYPE_SITELINXDEVICENAME:
			m_pSchedulerDoc->
				SetDestinationSiteLinxDeviceName( 
					(LPCTSTR)pProductData );
			szLog = "Changing Remote Node Name to: ";
			szLog += (LPCTSTR)pProductData;
			break;
		case PRODUCT_TYPE_EXEC_SCHEDULER_COMMAND:
			m_pSchedulerDoc->
				ExecuteCommand( (BYTE)(pProductData[0]-5),
								(LPCTSTR)(pProductData+1) );
			break;
		case PRODUCT_TYPE_SETCONNECTIONSTATE:
			m_pSchedulerDoc->SetConnectionState( (BYTE)(pProductData[0]-5) );
			m_pSchedulerDoc->RemoteSendAllStatus();
			break;
		case PRODUCT_TYPE_SENDACTIVESTATE:
			m_pSchedulerDoc->SetActiveState( (pProductData[0]?true:false) );
			szLog = "Remote Requested Site ";
			pProductData[0]?szLog+="Activation":szLog+="Deactivation";
			break;
		case PRODUCT_TYPE_GETACTIVESTATE:
			SendActiveState();
			m_pSchedulerDoc->RemoteSendAllStatus();
			break;
		case PRODUCT_TYPE_SETACTIVESTATE:
			ReceiveSetActiveState((pProductData[0]?true:false));
			szLog = "Remote Requested Site ";
			pProductData[0]?szLog+="Activation":szLog+="Deactivation";
			break;
		case PRODUCT_TYPE_ALLSTATUS:
			m_pSchedulerDoc->RemoteSendAllStatus();
			break;
		case PRODUCT_TYPE_DEVICESTATUS:
			m_pSchedulerDoc->
				SetRemoteDeviceStatus(  (LPCTSTR)(pProductData+1), 
										pProductData[0]-5, 
										baData );
			szLog = "Received Status : ";
			switch( pProductData[0]-5 )
			{
			case REMOTE_STATUS_INACTIVE:
				szLog += "Inactive";
				break;
			case REMOTE_STATUS_OFFLINE:
				szLog += "Off Line";
				break;
			case REMOTE_STATUS_ONLINE:
				szLog += "On Line";
				break;
			case REMOTE_STATUS_INERROR:
				szLog += "In Error";
				break;
			case REMOTE_STATUS_DISABLED_TX:
				szLog += "Disabled Tx";
				break;
			case REMOTE_STATUS_DISABLED_RX:
				szLog += "Disabled Tx";
				break;
			case REMOTE_STATUS_DISABLED_TRX:
				szLog += "Disabled TRx";
				break;
			}

			szLog += " for Device: ";
			szLog += (LPCTSTR)(pProductData+1);
			break;
		case PRODUCT_TYPE_SEQUENCESTATUS:
			m_pSchedulerDoc->
				SetRemoteSequenceStatus(	(LPCTSTR)(pProductData+1), 
											pProductData[0]-5, 
											baData );
			szLog = "Received Status for Sequence: ";
			szLog += (LPCTSTR)(pProductData+1);
			break;
		case PRODUCT_TYPE_DEVICE:
		case PRODUCT_TYPE_SEQUENCE:
		case PRODUCT_TYPE_TRIGGERGROUP:
		case PRODUCT_TYPE_EMAILGROUP:
		case PRODUCT_TYPE_VARIABLESGROUP:
			if( pProductData[0] )
			{
				m_pSchedulerDoc->
					RemoteSendObjects( (BYTE)(pProductData[1]-5) );
			}
			else
			{
				DecodeES4000Data( baData, pProductData+3, iDataCount-3 );
				m_pSchedulerDoc->
					ReceiveObjects( (BYTE)(pProductData[1]-5), baData, pProductData[2]?true:false );
			}
			break;
		default:
			szLog.Format("Received product #: %i from %s, %i bytes", iType, pSourceName, iDataSize );
			break;
	}
	CLog::Write( GetDocName(), szLog );
}

bool CDeviceSiteLinx::SendHeader( int iPacketType )
{
	if( !m_pSchedulerDoc ) return false;
	if( !GetDocName() ) return false;

	//packet start
	m_Buffer[0] = 1;
	//packet type
	m_Buffer[4] = (BYTE)('0' + (iPacketType % 10));
	iPacketType /= 10;
	m_Buffer[3] = (BYTE)('0' + (iPacketType % 10));		
	iPacketType /= 10;
	m_Buffer[2] = (BYTE)('0' + (iPacketType % 10));		
	iPacketType /= 10;
	m_Buffer[1] = (BYTE)('0' + (iPacketType % 10));
	//empty source = us
	m_Buffer[5] = (BYTE)0;
	SendPacket( m_Buffer, 6 );
	//destination
	SendPacket( (PBYTE)(LPCTSTR)m_pSchedulerDoc->GetDestinationSiteLinxDeviceName(), 
				m_pSchedulerDoc->GetDestinationSiteLinxDeviceName().GetLength()+1 );
	//header end
	m_Buffer[0] = 2;
	SendPacket( m_Buffer, 1 );
	//send document name
	SendPacket( (PBYTE)(LPCTSTR)GetDocName(), 
				strlen(GetDocName())+1 );

	return true;
}

void CDeviceSiteLinx::SendFooter()
{
	m_Buffer[0] = 3;
	SendPacket( m_Buffer, 1 );
}

void CDeviceSiteLinx::SendLogMessage( LPCTSTR pMessageLog )
{
	if( !pMessageLog ) return;
	if( !SendHeader( PRODUCT_TYPE_SENDLOG ) ) return;
	SendPacket( (PBYTE)(LPCTSTR)pMessageLog, strlen(pMessageLog)+1 );
	SendFooter();
}

void CDeviceSiteLinx::TransferRemoteObject(	char cObjectType, 
											CByteArray& baEncodedData,
											bool bRequest,
											bool bDeleteAll )
{
	int iPacketType=0;
	switch( cObjectType )
	{
	case ITEM_SCHED_DEVICES:
		iPacketType = PRODUCT_TYPE_DEVICE;
		break;	
	case ITEM_SCHED_SCHEDULES:
	case ITEM_SCHED_SEQUENCES:
		iPacketType = PRODUCT_TYPE_SEQUENCE;
		break;	
	case ITEM_SCHED_TRIGGERS:
		iPacketType = PRODUCT_TYPE_TRIGGERGROUP;
		break;	
	case ITEM_SCHED_EMAILGROUPS:
		iPacketType = PRODUCT_TYPE_EMAILGROUP;
		break;	
	case ITEM_SCHED_VARIABLESGROUPS:
		iPacketType = PRODUCT_TYPE_VARIABLESGROUP;
		break;	
	}
	if( !SendHeader( iPacketType ) ) return;
	bRequest?m_Buffer[0]=(BYTE)-1:m_Buffer[0]=0;
	m_Buffer[1] = (BYTE)(cObjectType+5);
	bDeleteAll?m_Buffer[2]=(BYTE)-1:m_Buffer[2]=0;
	SendPacket( m_Buffer, 3 );
	SendPacket( baEncodedData.GetData(), baEncodedData.GetSize() );
	SendFooter();
}

void CDeviceSiteLinx::SendRemoteCommand( 	LPCTSTR pName, 
											char cCommand )
{
	if( !SendHeader( PRODUCT_TYPE_EXEC_SCHEDULER_COMMAND ) ) return;
	
	m_Buffer[0] = (BYTE)(cCommand+5);
	SendPacket( m_Buffer, 1 );
	if( pName )
	{
		SendPacket( (PBYTE)(LPCTSTR)pName, strlen(pName)+1 );
		SendFooter();
	}
	else
	{
		m_Buffer[0] = 0;
		m_Buffer[1] = 3;
		SendPacket( m_Buffer, 2 );
	}
	CString		szMessage = "Executing remote command: ";
	switch( cCommand )
	{
	case REMOTE_COMMAND_DEVICE_CONNECT:
		szMessage += "Connect Device";
		if( pName )
		{
			szMessage += " ";
			szMessage += pName;
		}
		else
		{
			szMessage += "s";
		}
		break;	
	case REMOTE_COMMAND_DEVICE_DISCONNECT:
		szMessage += "Disconnect Device";
		if( pName )
		{
			szMessage += " ";
			szMessage += pName;
		}
		else
		{
			szMessage += "s";
		}
		break;	
	case REMOTE_COMMAND_SCHEDULE_RELOAD:
		szMessage += "Reload Schedule";
		break;	
	case REMOTE_COMMAND_SCHEDULE_RUN:
		szMessage += "Run Schedule: ";
		szMessage += pName;
		break;	
	case REMOTE_COMMAND_SCHEDULE_STOP:
		szMessage += "Stop Schedule: ";
		szMessage += pName;
		break;	
	case REMOTE_COMMAND_SEQUENCE_RUN:
		szMessage += "Run Sequence: ";
		szMessage += pName;
		break;	
	case REMOTE_COMMAND_SEQUENCE_STOP:
		szMessage += "Stop Sequence: ";
		szMessage += pName;
		break;	
	}
	szMessage += " @";
	szMessage += GetDeviceName();
	CLog::Write( GetDocName(), szMessage );
}

void CDeviceSiteLinx::SendSiteLinxDeviceName()
{
	if( !SendHeader( PRODUCT_TYPE_SITELINXDEVICENAME ) ) return;
	SendPacket( (PBYTE)(LPCTSTR)GetDeviceName(), 
				strlen(GetDeviceName())+1 );
	SendFooter();
}

void CDeviceSiteLinx::SendFileConnectionState( char cState )
{
	if( !SendHeader( PRODUCT_TYPE_SETCONNECTIONSTATE ) ) return;
	m_Buffer[0] = (BYTE)(cState+5);
	SendPacket( m_Buffer, 1 );
	SendFooter();
}

void CDeviceSiteLinx::SendActiveState()
{
	if( !SendHeader( PRODUCT_TYPE_SENDACTIVESTATE ) ) return;
	m_pSchedulerDoc->IsActive()?m_Buffer[0]=(BYTE)-1:m_Buffer[0]=0;
	SendPacket( m_Buffer, 1 );
	SendFooter();
}

void CDeviceSiteLinx::GetAllStatus()
{
	if( !SendHeader( PRODUCT_TYPE_ALLSTATUS ) ) return;
	SendFooter();
}

void CDeviceSiteLinx::GetActiveState()
{
	if( !SendHeader( PRODUCT_TYPE_GETACTIVESTATE ) ) return;
	SendFooter();
}

void CDeviceSiteLinx::SendSetActiveState( bool bState )
{
	if( !SendHeader( PRODUCT_TYPE_SETACTIVESTATE ) ) return;
	bState?m_Buffer[0]=(BYTE)-1:m_Buffer[0]=0;
	SendPacket( m_Buffer, 1 );
	SendFooter();
}

void CDeviceSiteLinx::ReceiveSetActiveState( bool bState )
{
	if( bState )
		gpTreeView->SchedulerSetActive( GetDocName() );
	else
		gpTreeView->SchedulerSetInactive( GetDocName() );
}

void CDeviceSiteLinx::SendRemoteDeviceStatus( 	LPCTSTR pDeviceName, 
												char cStatusFlag, 
												CByteArray& baData )
{
	if( !pDeviceName ) return;
	if( !SendHeader( PRODUCT_TYPE_DEVICESTATUS ) ) return;
	CByteArray baEncodedData;
	m_Buffer[0] = (BYTE)(cStatusFlag+5);
	SendPacket( m_Buffer, 1 );
	SendPacket( (PBYTE)(LPCTSTR)pDeviceName, strlen(pDeviceName)+1 );
	EncodeES4000Data( baEncodedData, baData.GetData(), baData.GetSize() );
	SendPacket( baEncodedData.GetData(), baEncodedData.GetSize() );
	SendFooter();
}

void CDeviceSiteLinx::SendRemoteSequenceStatus( 	LPCTSTR pSequenceName, 
													char cStatusFlag, 
													CByteArray& baData )
{
	if( !pSequenceName ) return;
	CByteArray baEncodedData;
	if( !SendHeader( PRODUCT_TYPE_SEQUENCESTATUS ) ) return;
	m_Buffer[0] = (BYTE)(cStatusFlag+5);
	SendPacket( m_Buffer, 1 );
	SendPacket( (PBYTE)(LPCTSTR)pSequenceName, strlen(pSequenceName)+1 );
	EncodeES4000Data( baEncodedData, baData.GetData(), baData.GetSize() );
	SendPacket( baEncodedData.GetData(), baEncodedData.GetSize() );
	SendFooter();
}

bool CDeviceSiteLinx::OnRequestFile( LPCTSTR szDestination, int iFileType, LPCTSTR szFileName, bool bDocUpdate )
{
	int i,j;
	CByteArray baBuffer;
	CByteArray baPacketBuffer;
	PBYTE pByte;
	PBYTE pPacketByte;
	BYTE Buff[MAX_PATH];

	CString szCurDocFile = m_pSchedulerDoc->GetPathName();
	CString szFileToUse = szFileName;
	if ( szFileToUse == "" && bDocUpdate == true )
		szFileToUse = szCurDocFile;

	CFileStatus fs;
	//destination is requesting file name
	//look for file in given path, if not found search cur dir
	if ( !CFile::GetStatus( szFileToUse, fs ) )
	{
		CFile cFile;
		cFile.SetFilePath( szFileToUse );
		CString szFileNameOnly = cFile.GetFileName();
		::GetCurrentDirectory( MAX_PATH, (LPTSTR)&Buff[0] );
		CString szNewFile = Buff;
		szNewFile += szFileNameOnly;
		if ( !CFile::GetStatus( szNewFile, fs ) )
		{
			return false;
		}
		szFileToUse = szNewFile;
	}

	CFile cfile;
	if ( cfile.Open( szFileToUse, CFile::modeRead ) == FALSE )
	{
		CString szError = "Could not open file: ";
		szError += szFileToUse;
		CLog::Write( GetDocName(), szError );
		return false;
	}

	Buff[0] = 1;
	Buff[1] = '0';
	Buff[2] = '0';
	Buff[3] = '0';
	if ( bDocUpdate == false )
		Buff[4] = '5';
	else
		Buff[4] = '7';
	Buff[5] = 0;

	int iLen = strlen(szDestination);
	memcpy( &Buff[6], (void*)(LPCTSTR)szDestination, iLen+1 );
	Buff[iLen+6] = 0;
	Buff[iLen+7] = 2;

	int iFileLen = strlen(szFileToUse);
	int * pFileInfo = (int*)(&Buff[iLen+8]);
	*pFileInfo = iFileType + FRAME_OFFSET_CHAR;
	pFileInfo = (int*)(&Buff[iLen+10]);
	*pFileInfo = iFileLen + FRAME_OFFSET_CHAR;

	memcpy( &Buff[iLen+12], (void*)(LPCTSTR)szFileToUse, iFileLen+1 );

	//send header
	SendPacket( Buff, iLen+iFileLen+13 );

	//send file
	const DWORD iBytesToRead = 5000;
	DWORD iBytesRead = 0;

	baBuffer.SetSize( iBytesToRead+100 );
	baPacketBuffer.SetSize( iBytesToRead+500, 300 );

	void* pData = baBuffer.GetData();

	while ( iBytesRead )
	{
		iBytesRead = 
			cfile.ReadHuge(	pData, 
							iBytesToRead );
		if ( iBytesRead == 0 ) break;

		pPacketByte = baPacketBuffer.GetData();
		pByte = (PBYTE)pData;
		j=0;
		for( i=0; i<(int)iBytesRead; i++ )
		{
			switch( *pByte )
			{
				case 1:			
				case 2:			
				case 3:			
				case 27:			
				{
					*pPacketByte++ = (BYTE)27;
					j++;
					*pPacketByte++ = (BYTE)(0x80 | *pByte++);
					j++;
					break;
				}
				default:
					*pPacketByte++ = *pByte++;
					j++;
					break;
			}
		}
		pPacketByte = baPacketBuffer.GetData();
		SendPacket( pPacketByte, j );
	}
	cfile.Close();

	Buff[0] = (BYTE)3;
	//send end of packet
	SendPacket( Buff, 1 );

	CString szTempDest = szDestination;
	if ( szTempDest == "" )
		szTempDest = g_szSiteLinxName;
	CString szLog;
	if ( bDocUpdate == false )
		szLog = "Sent file: ";
	else
		szLog = "Sent update: ";
	szLog += szFileToUse;
	szLog += " to: ";
	szLog += szTempDest;
	CLog::Write( GetDocName(), szLog);

	return true;
}

bool CDeviceSiteLinx::OnReceiveUpdate( LPCTSTR szSource, PBYTE pData, int iCount, int iFileType, LPCTSTR szFileName )
{
	if( !m_pSchedulerDoc ) return false;

	CString szCurFileToSave = szFileName;
	if ( szCurFileToSave == "" )
	{
		return OnRequestFile( szSource, iFileType, szFileName, true );
	}

	CString szRemoteFileName = szCurFileToSave;
	szRemoteFileName = szRemoteFileName.Mid( szRemoteFileName.ReverseFind('\\')+1 );

	CString szLocalFileName = m_pSchedulerDoc->GetPathName();
	szLocalFileName = szLocalFileName.Mid( szLocalFileName.ReverseFind('\\')+1 );

	CString szLog;

	if( szLocalFileName != szRemoteFileName )
	{
		szLog = "Received invalid update file: ";
		szLog += szCurFileToSave;
		szLog += " from: ";
		szLog += szSource;
		szLog += ". File was ignored.";
		CLog::Write( GetDocName(), szLog);
		return false;
	}

	szLog = "Received update, file: ";
	szLog += szCurFileToSave;
	szLog += " from: ";
	szLog += szSource;
	CLog::Write( GetDocName(), szLog);

	CString szBackupFileName = szLocalFileName;
	szBackupFileName += ".bak";
	::CopyFile( szLocalFileName, szBackupFileName, FALSE );

	CFile cfile;
	if ( ! cfile.Open( szLocalFileName, 
					CFile::modeWrite | 
//					CFile::shareExclusive |
					CFile::modeCreate )
		)
	{
		return false;
	}

	cfile.WriteHuge( (void*)pData,
					 (DWORD)iCount );
	cfile.Flush();
	cfile.Close();

	m_bReloadDocument = true;

//	if ( m_slDefaults.bAutoUpdate == true )
//	{
//		gpApp->SaveAllModified();
//		gpApp->OpenDocumentFile( szCurFileToSave );
//	}

	return true;
}

//user menu requests

bool CDeviceSiteLinx::RequestFile( LPCTSTR szSource, int iFileType, LPCTSTR szFileName, bool bDocUpdate )
{
	BYTE Buff[MAX_PATH];
	int iFileLen = strlen(szFileName);
	int * pFileInfo = (int*)(&Buff[0]);
	*pFileInfo = iFileType + FRAME_OFFSET_CHAR;
	pFileInfo = (int*)(&Buff[2]);
	*pFileInfo = iFileLen + FRAME_OFFSET_CHAR;
	memcpy( &Buff[4], (void*)(LPCTSTR)szFileName, iFileLen+1 );
	int iProductType = 0;
	if ( bDocUpdate == false ) 
		iProductType = 4;
	else
		iProductType = 6;

	SendProduct( iProductType, NULL, szSource, Buff, iFileLen+FILE_NAME_LOCATION );
	return true;
}

bool CDeviceSiteLinx::SendUpdate( LPCTSTR szDestination, int iFileType, LPCTSTR szFileName )
{
	return OnRequestFile( szDestination, iFileType, szFileName, true );
}

//FROM SITELINX

void CDeviceSiteLinx::DisplayError( LPCTSTR pStr )
{
	if ( !this ) return;
	if ( !pStr ) return;

//	szError.LoadString(IDS_TCPIP_CLIENT_ERROR);
	CString szError = GetDeviceName();
	szError += ": ";
	szError += pStr;
	CLog::Write( GetDocName(), szError, LEVEL_WARNING, LOG_GeneralDeviceError );
}

void CDeviceSiteLinx::OnStateChanging( TCPCommPort::SocketState State )
{
	m_bConnected = false;
	switch( State ) 
	{
		case TCPCommPort::STATE_CREATED:
			break;
		case TCPCommPort::STATE_CONNECTING:
		{
			gpTreeView->OnUpdate( HINT_CONNECTION_STATE_CHANGE, m_pSchedulerDoc );
			break;
		}
		case TCPCommPort::STATE_CONNECTED:
		{
			SendName( GetDeviceName() );
			//RegisterProductsTable();
			gpTreeView->OnUpdate( HINT_CONNECTION_STATE_CHANGE, m_pSchedulerDoc );
			GetActiveState();
			m_bConnected = true;
			break;
		}
		case TCPCommPort::STATE_CLOSING:
			gpTreeView->OnUpdate( HINT_CONNECTION_STATE_CHANGE, m_pSchedulerDoc );
			break;
		case TCPCommPort::STATE_CLOSED:
			gpTreeView->OnUpdate( HINT_CONNECTION_STATE_CHANGE, m_pSchedulerDoc );
			break;
		default:
			break;
	}
	CBaseDevice::OnStateChanging( State );
}

void CDeviceSiteLinx::ClosePort( void )
{
	if ( !this ) return;
	try
	{
		TCPCommPort::ClosePort();
		m_bConnected = false;
	}
	catch( CException * e )
	{
		e->Delete();
	}
}

void CDeviceSiteLinx::SendName( LPCTSTR pName )
{
	BYTE Buff[100];
	Buff[0] = 1;
	Buff[1] = '0';
	Buff[2] = '0';
	Buff[3] = '0';
	Buff[4] = '1';

	int SourceLen = strlen(pName);
	if ( SourceLen > 95 )
		return;
	memcpy( &Buff[5], pName, SourceLen+1 );
	Buff[SourceLen + 6] = 0;
	Buff[SourceLen + 7] = 2;
	SendPacket( Buff, SourceLen+8 );
	SendPacket( (PBYTE)(&pName[0]), strlen(pName)+1 );
	BYTE b = 3;
	SendPacket( &b, 1 );
}

bool CDeviceSiteLinx::SendPacket( PBYTE pData, int Size )
{
	if ( !this ) return false;
	if ( !pData || Size < 1 ) return false;
	return TCPCommPort::SendPacket( pData, Size ) ? true : false;
}

void CDeviceSiteLinx::SendEncodedData( PBYTE pData, int iDataSize )
{
	if ( iDataSize < 1 ) return;

	PBYTE pDataBytes;
	PBYTE pCurByte;
	BYTE Byte27 = 27;
	int i,j, iPrevCount=0, iCurCount=0;

	pDataBytes = (PBYTE)pData;
	pCurByte = (PBYTE)pData;

	j=0;
	for ( i=0; i<(int)iDataSize; i++ )
	{
		switch( *pDataBytes )
		{
			case 1:			
			case 2:			
			case 3:			
			case 27:			
			{
				SendPacket( pCurByte, iCurCount );
				SendPacket( &Byte27, 1 );
				*pDataBytes |= 0x80;
				pCurByte = pDataBytes;
				iPrevCount = i;
				iCurCount = 1;
				j++;
				break;
			}
			default:
				iCurCount++;
				break;
		}
		*pDataBytes++;
	}
	iCurCount = i - iPrevCount;
	SendPacket( pCurByte, iCurCount );
}

void CDeviceSiteLinx::SendProduct( int iType, LPCTSTR pSourceName, LPCTSTR pDestinationName, PBYTE pData, int iDataSize )
{
	BYTE Buff[100];
	CString szDestination;
	int i, DestLen;

	szDestination = pDestinationName;

	Buff[0] = 1;
	Buff[4] = (BYTE)('0' + (iType % 10));		iType /= 10;
	Buff[3] = (BYTE)('0' + (iType % 10));		iType /= 10;
	Buff[2] = (BYTE)('0' + (iType % 10));		iType /= 10;
	Buff[1] = (BYTE)('0' + (iType % 10));

	if ( pSourceName )
	{
		int SourceLen = strlen(pSourceName);
		memcpy( &Buff[5], pSourceName, SourceLen+1 );

		Buff[SourceLen + 6] = 0;

		if ( szDestination == "" || szDestination == m_szGlobal )
		{
			Buff[SourceLen + 7] = 2;
			SendPacket( Buff, SourceLen+8 );
		}
		else
		{
			DestLen = strlen(pDestinationName);
			memcpy( &Buff[SourceLen+6], pDestinationName, DestLen+1 );
			Buff[SourceLen+6+DestLen] = 0;
			Buff[SourceLen+6+DestLen+1] = 2;
			SendPacket( Buff, SourceLen+6+DestLen+2 );
		}
	}
	else
	{
		Buff[5] = 0;
		DestLen = strlen(pDestinationName);
		memcpy( &Buff[6], pDestinationName, DestLen+1 );
		Buff[6+DestLen] = 0;
		Buff[6+DestLen+1] = 2;
		SendPacket( Buff, DestLen+8 );
	}

	int Pos = 0;
	for( i = 0; i < iDataSize; i++ ) 
	{
		switch( *pData ) 
		{
			case 1:			
			case 2:			
			case 3:			
			case 27:			
			{
				Buff[Pos++] = (BYTE)27;
				Buff[Pos++] = (BYTE)(0x80 | *pData);
				break;
			}

			default:
				Buff[Pos++] = *pData;
				break;
		}
		pData++;
		if( Pos > 90 ) 
		{
			SendPacket( Buff, Pos );
			Pos = 0;
		}
	}
	if( Pos ) 
	{
		SendPacket( Buff, Pos );
		Pos = 0;
	}
	BYTE b = 3;
	SendPacket( &b, 1 );
}

void CDeviceSiteLinx::SendMessage( int iErrorLevel, LPCTSTR szError )
{
	BYTE Buff[10];

	Buff[0] = 1;
	Buff[1] = '0';
	Buff[2] = '0';
	Buff[3] = '0';
	Buff[4] = '0';
	Buff[5] = 0;
	Buff[6] = 0;
	Buff[7] = 2;
	Buff[8] = (BYTE)(iErrorLevel+3);

	SendPacket( Buff, 9 );
	SendPacket( (PBYTE)szError, strlen(szError)+1 );
	BYTE b = 3;
	SendPacket( &b, 1 );
	return;
}

void CDeviceSiteLinx::ProcessMessage( PBYTE pData, int iCount )
{
	char cCmd[5];
	char * pSource;
	int iCmd;
	int i,j;
	CString szSource, szSourceProxy, szSourceName;
	CString szView;

	PBYTE pPacketData = pData;
	int iDataCount = 0;

	cCmd[0] = pData[1];
	cCmd[1] = pData[2];
	cCmd[2] = pData[3];
	cCmd[3] = pData[4];
	cCmd[4] = 0;
	pSource	= (char*)(pData+5);

	//convert command string to int command ID
	iCmd = atoi(cCmd);
	if ( iCmd < 0 || iCmd > MAX_VALID_COMMAND )
	{
		iCmd = INVALID_COMMAND;
	}
	if ( iCmd == INVALID_COMMAND )
		return;

	szSource = pSource;
	i = szSource.Find( ":", 0 );
	if ( i == NOT_FOUND )
	{
		szSourceName = szSource;
		szSourceProxy = m_szGlobal;
	}
	else
	{
		szSourceName = szSource.Mid( i+1 );
		szSourceProxy = szSource.Mid( 0, i );
	}

//	if ( LookUpRegistration( iCmd, szSourceProxy, szSourceName )
//			== false )
//	{
//		return;
//	}
	
	for ( i=0; i<iCount; i++ )
	{
		if ( *pPacketData++ == 2 )
		{
			iDataCount = iCount - i - 2;
			break;
		}
	}
	if ( iDataCount == 0 ) iDataCount = iCount;

	PBYTE pPacketStart = pPacketData;
	PBYTE pBackData = pPacketData;

	j=iDataCount;
	for ( i=0; i<iDataCount; i++ )
	{
		if ( *pPacketData == 27 )
		{
			pPacketData++;
			*pPacketData ^= 0x80;
			j--;
		}
		*pBackData++ = *pPacketData++;
	}
	iDataCount = j;

	OnReceiveProduct( iCmd, pSource, pPacketStart, iDataCount );
}

void CDeviceSiteLinx::ProcessReceivedPacket( PBYTE pBuffer, int iCount )
{
	if ( !this ) return;
	if ( !pBuffer || iCount < 1 ) return;
//	int j=0;
	int i=0;
	try
	{
		for( i=0; i<iCount; i++ ) 
		{
			switch( pBuffer[i] ) 
			{
				case PACKET_START:
					m_iByteLast = 0;
					m_baBuffer.SetAtGrow( m_iByteLast++, PACKET_START );
					break;
				case PACKET_DATA_START:
					m_baBuffer.SetAtGrow( m_iByteLast++, PACKET_DATA_START );
					break;
				case PACKET_END:
					m_baBuffer.SetAtGrow( m_iByteLast++, PACKET_END );
					m_baBuffer.SetAtGrow( (BYTE)m_iByteLast, (BYTE)-1 );
					ProcessMessage( m_baBuffer.GetData(), m_iByteLast );
					m_baBuffer.SetSize( 500, 500 );
					m_iByteLast	= 0;
					break;
				default:
					m_baBuffer.SetAtGrow( m_iByteLast++, pBuffer[i] );
			}
		}
	}
	catch ( CException * e )
	{
		TCHAR   szCause[255];
		CString szError;
		e->GetErrorMessage(szCause, 255);
		szError = "IDS_TCPIP_CLIENT_ERROR_MEMALLOC ";
		szError += szCause;
		CLog::Write( GetDocName(), szError );
		e->Delete();
	}
}

void CDeviceSiteLinx::OnReceive( int /*nErrorCode*/ )
{
	BOOL	bMore = TRUE;
	CByteArray baData;
	baData.SetSize( 100 );

	while( bMore ) 
	{
		int iCount = Receive(baData.GetData(), baData.GetSize() );
		if( iCount < 1 ) 
		{
			bMore = FALSE;
		}
		else 
		{
			m_baBuffer.SetSize( m_baBuffer.GetSize() + iCount + 105 );
			ProcessReceivedPacket( baData.GetData(), iCount );
			bMore = FALSE;
		}
	}
}

