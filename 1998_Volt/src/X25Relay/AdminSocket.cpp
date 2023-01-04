// X25PVC.cpp : implementation file
//

#include "stdafx.h"
#include "X25Relay.h"
#include "AdminSocket.h"
#include "AdminListenSocket.h"
#include "X25Manager.h"
#include "X25PVC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

char		CAdminSocket::sm_Buffer[BUFFER_SIZE];
BufferSize	CAdminSocket::sm_Count = BUFFER_SIZE;

int	CAdminSocket::sm_iBasePort = INVALID_VALUE;

/////////////////////////////////////////////////////////////////////////////
// CAdminSocket

CAdminSocket::CAdminSocket()
{
    m_pCTrace = NULL;
	m_pListenAdminSocket = NULL;
	m_pX25Manager = NULL;
}

CAdminSocket::~CAdminSocket()
{
}

// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CAdminSocket, TCPSocket)
	//{{AFX_MSG_MAP(CAdminSocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CAdminSocket member functions

void CAdminSocket::OnStateChanged( SocketState State )
{
    OWS_1trace( (*m_pCTrace), LEVEL_EVENTSANDPOLLS, "Socket state changed to: %s", GetStateString() );
	switch( State ) 
	{
		case IP_STATE_CLOSED:
			OWS_0trace( (*m_pCTrace), LEVEL_INFO, "Network disconnected, enabling listen port" );
			m_pListenAdminSocket->OpenListenPort( CAdminSocket::sm_iBasePort );
			break;
		case IP_STATE_CONNECTED:
			OWS_0trace( (*m_pCTrace), LEVEL_INFO, "Network connected, disabling listen port" );
			m_pListenAdminSocket->ShutDown();
			m_pListenAdminSocket->Close();
			break;
		case IP_STATE_CREATED:
		case IP_STATE_CONNECTING:
		case IP_STATE_LISTENING:
		case IP_STATE_DISCONNECTING:
		default:
			break;
	}
}

void CAdminSocket::OnReceive( int /*nErrorCode*/ )
{
	int		iLastError = 0;
	sm_Count = Receive( sm_Buffer, sizeof(sm_Buffer) );
	switch( sm_Count )
	{   
		case 0: //no data?, oh! well...
			OWS_0trace( (*m_pCTrace), LEVEL_DATATRANSFERS, "Received 0 bytes, ???" );
			return;
			break;   
		case SOCKET_ERROR:
			iLastError = GetLastError();
			if( iLastError != WSAEWOULDBLOCK )       
			{
				OWS_0trace( (*m_pCTrace), LEVEL_ERROR, "Receive error" );
				DisplayLastError( iLastError );
			}
			else
			{
				OWS_0trace( (*m_pCTrace), LEVEL_WARNING, "Could not receive data, socket is currently blocking, will retry later." );
			}
			return;
			break;
		default:      
			OWS_1trace( (*m_pCTrace), LEVEL_DATATRANSFERS, "Received %s bytes", sm_Count );
			break;
	}

	int iPVCNumber = sm_Buffer[0];
	iPVCNumber <<= 8;
	iPVCNumber += sm_Buffer[1];

	LPX25PVC pPVC = m_pX25Manager->GetPVC( iPVCNumber );

	switch( sm_Buffer[2] )
	{
		case GET_LINE_STATE:
			OWS_1trace( (*m_pCTrace), LEVEL_OPERATIONS, "Remote requested line %i state", iPVCNumber );
			SendReplyMessage( iPVCNumber, GET_LINE_STATE, pPVC->GetPVCState() );
			break;
		case SET_LINE_STATE:
			OWS_1trace( (*m_pCTrace), LEVEL_OPERATIONS, "Remote requested line %i state change, cannot comply with request", iPVCNumber );
			//SendReplyMessage( iPVCNumber, GET_LINE_STATE, GetPVCStateString() );
			break;
		case GET_LINE_STATISTICS:
		{
			OWS_1trace( (*m_pCTrace), LEVEL_OPERATIONS, "Remote requested line %i statistics", iPVCNumber );

			CString szString;

			szString = "LINE STAT\nPhysical Layer:\n";
			szString += pPVC->GetStatistics( CIDTYPE_PHYSICAL );
			szString += "\nLink Layer:\n";
			szString += pPVC->GetStatistics( CIDTYPE_LINK );
			szString += "\nPacket Layer:\n";
			szString += pPVC->GetStatistics( CIDTYPE_PACKET );
			szString += "\nPVC:\n";
			szString += pPVC->GetStatistics( CIDTYPE_PVC );

			SendReplyMessage( iPVCNumber, GET_LINE_STATISTICS, pPVC->GetPVCState(), szString );
		}
			break;
		case CLEAR_LINE_STATISTICS:
			pPVC->ClearStatistics();
			break;
		default:
			OWS_1trace( (*m_pCTrace), LEVEL_WARNING, "Unknown request for line %i", iPVCNumber );
			break;
	}
}

void CAdminSocket::SendReplyMessage( int iLine, int iMessageType, int iCode, LPCTSTR szDetails )
{
	sm_Buffer[0]	= MESSAGE_TYPE_REPLY;
	sm_Buffer[1] = HIBYTE(iLine);
	sm_Buffer[2] = LOBYTE(iLine);
	sm_Buffer[3]	= iMessageType;
	sm_Buffer[4]	= iCode;
	sm_Count		= 5;
	if( szDetails && *szDetails )
	{
		sm_Count += strlen( szDetails );
		strcpy( sm_Buffer+5, szDetails );
	}
	Send( (PBYTE)sm_Buffer, sm_Count );
}

void CAdminSocket::OnOutOfBandData( int nErrorCode )
{
	//OWS_0trace( (*m_pCTrace), LEVEL_EVENTSANDPOLLS, "Received Out of Band Data" );
	OnReceive( nErrorCode );
}

void CAdminSocket::DisplayError( LPCTSTR pStr )
{
	OWS_1trace( (*m_pCTrace), LEVEL_ERROR, "TCP/IP Error: %s", pStr );
}

