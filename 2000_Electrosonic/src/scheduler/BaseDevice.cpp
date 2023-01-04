#include "stdafx.h"
#include "resource.h"
#include "BaseDevice.h"
#include "ppgIPAddress.h"
#include "SchedulerDoc.h"

#include "FileVersion.h"

CBaseDevice::CBaseDevice()
{
	m_hWndNotify		= NULL;
	m_iLast				= 0;
	m_bTerminatorSet	= false;
	m_bSOHSet			= false;
	m_bTxCommsActive	= true;
	m_bRxCommsActive	= true;
	m_baBuffer.SetSize( 0, 100 );
	m_iStatus			= REMOTE_STATUS_INACTIVE;
	m_bInError			= false;
	m_bConnected		= false;
	InitSubItemIcons();
}

void CBaseDevice::CopyDeviceStatus( CBaseDevice * pDevice )
{
	m_baBuffer.SetSize( 0, 100 );
	m_iLast				= 0;
	m_hWndNotify		= pDevice->m_hWndNotify;
	m_bTerminatorSet	= pDevice->m_bTerminatorSet;
	m_bSOHSet			= pDevice->m_bSOHSet;
	m_bTxCommsActive	= pDevice->m_bTxCommsActive;
	m_bRxCommsActive	= pDevice->m_bRxCommsActive;
	m_iStatus			= pDevice->m_iStatus;
	m_bInError			= pDevice->m_bInError;
	m_bConnected		= pDevice->m_bConnected;
}

CBaseDevice::~CBaseDevice()
{
}

void CBaseDevice::GetDeviceType( CString& str )
{
	str = "Base Device";
}

bool CBaseDevice::GoOnline( bool bOnline )
{
	if( bOnline ) {
		OpenPort();
		TRACE( "Attempting to open port\n\r" );
	} else {
		ClosePort();
	}
	return true;
}

bool CBaseDevice::DeviceInError( void )
{
	return false;
}

bool CBaseDevice::RemoteDeviceInError( void )
{
	return false;
}

void CBaseDevice::Serialize( CArchive& ar )
{
	enum {
		VERSION_BASE,
		VERSION_CURRENT = VERSION_BASE,
	};
	CVersion V( ar, VERSION_CURRENT );

	// serialize out the IP Address data.
	if( ar.IsStoring() ) {
		CString IP;
		UINT	Port=0;

		RetrieveAddress( IP, Port );
		ar << IP;
		ar << Port;

		GetDeviceName( IP );
		ar << IP;

	} else {
		CString IP;
		UINT	Port=0;

		ar >> IP;
		ar >> Port;
		SetAddress( IP, Port );

		ar >> IP;
		SetDeviceName( IP );
	}
}


void CBaseDevice::AddPropertyPages( CPropertySheet& /*Sheet*/ )
{
}

void CBaseDevice::DeletePropertyPages( bool /*bUpdate*/ )
{
}

bool CBaseDevice::Configure( void )
{
	CppgIPAddress	pPage;
	CString			name;
	CString			str;
	CString			IP;
	UINT			Port;


	RetrieveAddress( IP, Port );
	GetDeviceName( name );
	
	pPage.m_IPAddress	= IP;
	pPage.m_Port		= Port;
	GetDeviceType( pPage.m_DeviceType );

	AfxFormatString1( str, IDS_TITLE_PROPERTIES, (LPCTSTR)name );
	
	CPropertySheet	sheet( str );

	sheet.AddPage( &pPage );

	// let derived classes add sheets.
	AddPropertyPages( sheet );

	if( sheet.DoModal() == IDOK ) { 
		SetAddress( pPage.m_IPAddress, pPage.m_Port );
		// let derived classes update properties after
		// successful close.
		DeletePropertyPages( true );
		return true;
	} else {
		// let derived classes clean up properties after
		// cancelled properties sheet.
		DeletePropertyPages( false );
	}
	return false;
}

bool CBaseDevice::EditCue( CByteArray& /*baCue*/ )
{
	return false;
}

bool CBaseDevice::GetCueText( CByteArray& /*baCue*/, CString& /*text*/ )
{
	return false;
}

bool CBaseDevice::FireEvent( CByteArray& /*baCue*/, LPCTSTR /*lpTriggeringDevice*/ )
{
	return false;
}


void CBaseDevice::SetNotifyWindow( HWND hWnd )
{
	m_hWndNotify = hWnd;
}

void CBaseDevice::FireTrigger( int iTrigger )
{
	PCDeviceTriggerNotification pN = new CDeviceTriggerNotification();

	pN->m_pDevice		= this;
	pN->m_iCode			= iTrigger;
	pN->m_szDeviceName	= GetDeviceName();
	SendNotification( DEVNOTIFY_DEVICE_TRIGGER, pN );
}


void CBaseDevice::SendNotification( DeviceNotifyType Mess, void* Data )
{
	if( m_hWndNotify ) {
		::PostMessage( m_hWndNotify, WM_DEVICE_NOTIFY, (WPARAM)Mess, (LPARAM)Data );
	}
}


void CBaseDevice::SendToLog( LPCTSTR pText, int Level, int Type )
{
	CString str;
	GetDeviceName( str );
	str += ": ";
	str += pText;
	CLog::Write( GetDocName(), str, (BYTE)Level, (BYTE)Type );
}

void CBaseDevice::ReportBadMessage( void )
{
	CString str;
	GetDeviceName( str );
	str += ":  Bad Message Received";
	CLog::Write( GetDocName(), str, LEVEL_DEVICE_BAD_MESSAGE, LOG_BadResponse );
}

int CBaseDevice::GetFirstTrigger( void )
{
	return 0;
}

void CBaseDevice::GetTrigger( int& Pos, CString& /*Name*/ )
{
	Pos = 0;
}

int	CBaseDevice::GetFirstVariable( void )
{
	return 0;
}

ESVariable * CBaseDevice::GetVariable( int& Pos )
{
	Pos = 0;
	return NULL;
}

void CBaseDevice::DisplayError( LPCTSTR pStr )
{
	// report a network error back to the user
	SendToLog( pStr, LEVEL_WARNING );
}

//added by kg
bool CBaseDevice::IsConnectionResolved()
{
	if( !this || !*this ) return false;
	try
	{
		switch( GetState() ) {
			case TCPCommPort::STATE_CREATED:
			case TCPCommPort::STATE_CLOSED:
			case TCPCommPort::STATE_CONNECTED:
				return true;
				break;
			case TCPCommPort::STATE_CLOSING:
			case TCPCommPort::STATE_CONNECTING:
			default:
				return false;
				break;
		}
	}
	catch( CException * e )
	{
		e->Delete();
	}
	return false;
}

void CBaseDevice::Idle( int /*iActiveType*/ )
{
	if( DeviceInError() )
		m_iStatus = REMOTE_STATUS_INERROR;
}

void CBaseDevice::OnStateChanging( TCPCommPort::SocketState State )
{
	// the state of the socket has changed
	// do some status and deal with the transmission
	switch( State ) {
		case TCPCommPort::STATE_CREATED:
			break;
		case TCPCommPort::STATE_CONNECTING:
			CLog::Execute(  GetDocName(), 
							GetDeviceName(), 
							REMOTE_STATUS_OFFLINE,
							(BYTE)m_iStatus );
			break;
		case TCPCommPort::STATE_CONNECTED:
		{
			m_iStatus = REMOTE_STATUS_ONLINE;
			SendToLog( "Connected", LEVEL_DEVICE_CONNECTION, LOG_DeviceDisconnected );
			CLog::Execute(  GetDocName(), 
							GetDeviceName(), 
							REMOTE_COMMAND_SENDDEVICESTATUS,
							(BYTE)m_iStatus );
			break;
		}
		case TCPCommPort::STATE_CLOSING:
			m_iStatus = REMOTE_STATUS_OFFLINE;
			break;
		case TCPCommPort::STATE_CLOSED:
		{
			if( IsOpen() )
			{
				SendToLog( "Connection Closed", LEVEL_DEVICE_CONNECTION, LOG_DeviceConnected );
				CLog::Execute(  GetDocName(), 
								GetDeviceName(), 
								REMOTE_COMMAND_SENDDEVICESTATUS,
								REMOTE_STATUS_OFFLINE );
			}
			break;
		}
			break;
		default:
			break;
	}
	TCPCommPort::OnStateChanging( State );
}

void CBaseDevice::OnReceive( int /*nErrorCode*/ )
{
	BYTE	Data[50];
	BOOL	More = TRUE;
	CString	Str;

	while( More ) 
	{
		int Count = Receive(Data,sizeof(Data));
		if( Count < 1 )
		{
			More = FALSE;
		}
		else 
		{
			for( int i = 0; i < Count; i++ ) 
			{
				if( (Data[i] == m_bySOH) && m_bSOHSet ) 
				{
					m_iLast = 0;
					m_baBuffer.SetAtGrow(m_iLast++, m_bySOH );
				} 
				else 
				{
					m_baBuffer.SetAtGrow( m_iLast++, Data[i] );
					if( (Data[i] == m_byTerminator) && 
						m_bTerminatorSet )
						//||	Count == i+1 ) 
					{
						m_baBuffer.SetAtGrow( m_iLast++, '\0' );
						ProcessResponse( m_baBuffer.GetData() );
						m_iLast = 0;
					}
				}
			}
		}
	}
}


void CBaseDevice::SetTerminatingCharacter( BYTE Term )
{
	m_byTerminator	= Term;
	m_bTerminatorSet	= true;
}

void CBaseDevice::SetSOHCharacter( BYTE Term )
{
	m_bySOH		= Term;
	m_bSOHSet	= true;
}
	
void CBaseDevice::ProcessResponse( PBYTE pData )
{
	if( !pData || !*pData ) return;

	char cType = LEVEL_DEVICE_RESPONSE;
	CString szMessage;
	GetDeviceName( szMessage );

	const int iMAX_MSG_SIZE=30;
	char buff[iMAX_MSG_SIZE];
	for( int i=0; i<iMAX_MSG_SIZE; i++ )
	{
		if( i == m_iLast || pData[i] == 0 )
			break;
		buff[i] = pData[i];
	}
	buff[i] = 0;

	if( IsReceiveEnabled() )
	{
		szMessage += ":  Received message: ";
	}
	else
	{
		cType = LEVEL_DEVICE_OFFLINE;
		szMessage += ":  Receive is Disabled. Ignoring message: ";
	}
	szMessage += buff;
	CLog::Write( GetDocName(), szMessage, LEVEL_DEVICE_BAD_MESSAGE );
}

void CBaseDevice::ShowStatus( void )
{
	// display the current status of the device
}

