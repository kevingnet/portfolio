#include "stdafx.h"
#include "TCPIPClient.h"
#include "TCPIPServer.h"
#include "Session.h"

CTCPIPServer * CTCPIPClient::sm_pServer = NULL;

CTCPIPClient::CTCPIPClient()
{
	m_pSession = NULL;
	m_iByteLast			= 0;
}

CTCPIPClient::~CTCPIPClient()
{
	m_pSession = NULL;
}

void CTCPIPClient::DisplayError( LPCTSTR a_pStr )
{
	g_pLog->Log( a_pStr, LOGTYPE_ERROR );
}

void CTCPIPClient::ConnectionMade( void )
{
	m_baBuffer.SetSize( 512, 512 );
	SendPacket( CTCPIPServer::sm_baSendOnConnect.GetData(),
				CTCPIPServer::sm_baSendOnConnect.GetSize() );
}

void CTCPIPClient::OnClose( int nErrorCode )
{
	UNREFERENCED_PARAMETER( nErrorCode );
	SetState( STATE_CLOSED );
	sm_pServer->ClientListChanged();
}

bool CTCPIPClient::SendPacket( PBYTE pData, int Size )
{
	if ( !pData || Size < 1 ) return false;
	return TCPServerPort::SendPacket( pData, Size ) ? true : false;
}

void CTCPIPClient::ProcessMessage()
{
	m_pSession->ProcessMessage( m_baBuffer );
}

void CTCPIPClient::ProcessReceivedPacket( PBYTE pBuffer, int iCount )
{
	if ( !pBuffer || iCount < 1 ) return;
	for( int i=0; i<iCount; i++ ) 
	{
		if( pBuffer[i] == CTCPIPServer::sm_byStartSeq && 
			CTCPIPServer::sm_bStartSeqIsSet ) 
		{
			m_iByteLast = 0;
			m_baBuffer.SetAtGrow(m_iByteLast++, CTCPIPServer::sm_byStartSeq );
		} 
		else 
		{
			m_baBuffer.SetAtGrow( m_iByteLast++, pBuffer[i] );
			if( pBuffer[i] == CTCPIPServer::sm_byEndSeq &&
				CTCPIPServer::sm_bEndSeqIsSet ) 
			{
				m_baBuffer.SetAtGrow( m_iByteLast++, '\0' );
				ProcessMessage();
				m_baBuffer.RemoveAll();
				m_iByteLast = 0;
			}
		}
	}
	if( ( !CTCPIPServer::sm_bStartSeqIsSet && !CTCPIPServer::sm_bEndSeqIsSet ) ||
		( CTCPIPServer::sm_bStartSeqIsSet && !CTCPIPServer::sm_bEndSeqIsSet )	)
	{
		m_baBuffer.SetAtGrow( m_iByteLast++, '\0' );
		m_baBuffer.SetSize( m_iByteLast );
		m_baBuffer.FreeExtra();
		ProcessMessage();
		m_baBuffer.RemoveAll();
		m_iByteLast = 0;
	}
}
