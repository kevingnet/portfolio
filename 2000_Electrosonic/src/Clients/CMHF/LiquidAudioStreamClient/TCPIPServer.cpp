#include "stdafx.h"
#include "TCPIPServer.h"
#include "TCPIPClient.h"

BYTE CTCPIPServer::sm_byStartSeq=0;
BYTE CTCPIPServer::sm_byEndSeq=0;
bool CTCPIPServer::sm_bStartSeqIsSet=false;
bool CTCPIPServer::sm_bEndSeqIsSet=false;

CByteArray CTCPIPServer::sm_baSendOnConnect;

CTCPIPServer::CTCPIPServer(void)
{
	CTCPIPClient::SetServer(this);
}

inline CTCPIPServer::~CTCPIPServer()
{
}

void CTCPIPServer::DisplayError( LPCTSTR a_pStr )
{
	g_pLog->Log( a_pStr, LOGTYPE_ERROR );
}

void CTCPIPServer::Send( PBYTE pData, int iCount )
{
	CTCPIPClient * pClient=NULL;
	for( int i=0; i<m_Clients.GetSize(); i++ ) 
	{
		pClient = (CTCPIPClient*)m_Clients[i];
		if( CTCPIPServer::sm_bStartSeqIsSet )
			pClient->Send( &CTCPIPServer::sm_byStartSeq, 1 );
		pClient->Send( pData, iCount );
		if( CTCPIPServer::sm_bEndSeqIsSet )
			pClient->Send( &CTCPIPServer::sm_byEndSeq, 1 );
	}
}

inline void CTCPIPServer::ClientListChanged( void )
{
	TCPServerPort *	pClient	= NULL;
	for( int i=m_Clients.GetSize()-1; i>=0; i-- ) 
	{
		pClient = m_Clients[i];
		if( pClient && pClient->GetState() == STATE_CLOSED ) 
		{
			m_Clients.RemoveAt(i);
			delete pClient;
		}
	}
	m_pSession->UpdateConnections();
}

inline TCPServerPort* CTCPIPServer::CreateServer( void )
{
	CTCPIPClient* pServer = new CTCPIPClient;
	pServer->SetSession( m_pSession );
	return pServer;
}
