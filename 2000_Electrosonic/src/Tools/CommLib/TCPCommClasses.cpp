#include "stdafx.h"
#include "resource.h"
#include "CommDLL.h"

ErrorTxtType CWinwallSocket::m_ErrorTable[] = {
	{ WSAEINTR,				IDS_ERR_WSAEINTR },
	{ WSAEBADF,				IDS_ERR_WSAEBADF },
	{ WSAEACCES,			IDS_ERR_WSAEACCES },
	{ WSAEFAULT,			IDS_ERR_WSAEFAULT },
	{ WSAEINVAL,			IDS_ERR_WSAEINVAL },
	{ WSAEMFILE,			IDS_ERR_WSAEMFILE },
/*
 * Windows Sockets definitions of regular Berkeley error constants
 */
	{ WSAEWOULDBLOCK,		IDS_ERR_WSAEWOULDBLOCK },
	{ WSAEINPROGRESS,		IDS_ERR_WSAEINPROGRESS },
	{ WSAEALREADY,			IDS_ERR_WSAEALREADY },
	{ WSAENOTSOCK,			IDS_ERR_WSAENOTSOCK },
	{ WSAEDESTADDRREQ,		IDS_ERR_WSAEDESTADDRREQ },
	{ WSAEMSGSIZE,			IDS_ERR_WSAEMSGSIZE },
	{ WSAEPROTOTYPE,		IDS_ERR_WSAEPROTOTYPE },
	{ WSAENOPROTOOPT,		IDS_ERR_WSAENOPROTOOPT },
	{ WSAEPROTONOSUPPORT,	IDS_ERR_WSAEPROTONOSUPPORT },
	{ WSAESOCKTNOSUPPORT,	IDS_ERR_WSAESOCKTNOSUPPORT },
	{ WSAEOPNOTSUPP,		IDS_ERR_WSAEOPNOTSUPP },
	{ WSAEPFNOSUPPORT,		IDS_ERR_WSAEPFNOSUPPORT },
	{ WSAEAFNOSUPPORT,		IDS_ERR_WSAEAFNOSUPPORT },
	{ WSAEADDRINUSE,		IDS_ERR_WSAEADDRINUSE },
	{ WSAEADDRNOTAVAIL,		IDS_ERR_WSAEADDRNOTAVAIL },
	{ WSAENETDOWN,			IDS_ERR_WSAENETDOWN },
	{ WSAENETUNREACH,		IDS_ERR_WSAENETUNREACH },
	{ WSAENETRESET,			IDS_ERR_WSAENETRESET },
	{ WSAECONNABORTED,		IDS_ERR_WSAECONNABORTED },
	{ WSAECONNRESET,		IDS_ERR_WSAECONNRESET },
	{ WSAENOBUFS,			IDS_ERR_WSAENOBUFS },
	{ WSAEISCONN,			IDS_ERR_WSAEISCONN },
	{ WSAENOTCONN,			IDS_ERR_WSAENOTCONN },
	{ WSAESHUTDOWN,			IDS_ERR_WSAESHUTDOWN },
	{ WSAETOOMANYREFS,		IDS_ERR_WSAETOOMANYREFS },
	{ WSAETIMEDOUT,			IDS_ERR_WSAETIMEDOUT },
	{ WSAECONNREFUSED,		IDS_ERR_WSAECONNREFUSED },
	{ WSAELOOP,				IDS_ERR_WSAELOOP },
	{ WSAENAMETOOLONG,		IDS_ERR_WSAENAMETOOLONG },
	{ WSAEHOSTDOWN,			IDS_ERR_WSAEHOSTDOWN },
	{ WSAEHOSTUNREACH,		IDS_ERR_WSAEHOSTUNREACH },
	{ WSAENOTEMPTY,			IDS_ERR_WSAENOTEMPTY },
	{ WSAEPROCLIM,			IDS_ERR_WSAEPROCLIM },
	{ WSAEUSERS,			IDS_ERR_WSAEUSERS },
	{ WSAEDQUOT,			IDS_ERR_WSAEDQUOT },
	{ WSAESTALE,			IDS_ERR_WSAESTALE },
	{ WSAEREMOTE,			IDS_ERR_WSAEREMOTE },
	{ WSAEDISCON,			IDS_ERR_WSAEDISCON },
/*
 * Extended Windows Sockets error constant definitions
 */
	{ WSASYSNOTREADY,		IDS_ERR_WSASYSNOTREADY },
	{ WSAVERNOTSUPPORTED,	IDS_ERR_WSAVERNOTSUPPORTED },
	{ WSANOTINITIALISED,	IDS_ERR_WSANOTINITIALISED },
	{ LAST_ERROR,	0 },
};


void GetErrorString ( DWORD dwError, CString& str )
{
	LPSTR	pText = NULL;

	if ( 0 == ::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
							   FORMAT_MESSAGE_FROM_SYSTEM | 
							   FORMAT_MESSAGE_IGNORE_INSERTS, 
							   NULL,
							   dwError, 
							   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							   (LPTSTR)&pText, 0, NULL ) )
	{
		pText = NULL;
	}

	str = pText;

	if ( pText ) LocalFree( pText );
}


void CWinwallSocket::GetErrorMsg( CString& String, int ErrorCode )
{
	ErrorTxtType* pError = m_ErrorTable;

	while( pError->Error != LAST_ERROR && pError->Error != ErrorCode )
		pError++;

	if( pError->Error == ErrorCode ) {
		CString str;
//		GetErrorString ( ErrorCode, str );
		str.LoadString( pError->ResourceErrorCode );
		String += str;
	} else {
		String += _T( "Unknown" );
	}
}

CWinwallSocket&	CWinwallSocket::operator=( const CWinwallSocket& pW )
{
	m_DeviceName = pW.m_DeviceName;
	return *this;
}


void CWinwallSocket::DisplayError( LPCTSTR pStr )
{
	if( m_DeviceName.IsEmpty() ) {
		::MessageBox( NULL, pStr, _T("Network Error"), MB_ICONEXCLAMATION | MB_OK );
	} else {
		::MessageBox( NULL, pStr, m_DeviceName, MB_ICONEXCLAMATION | MB_OK );
	}
}

void CWinwallSocket::DisplayError( UINT ResId )
{
	CString str;
	VERIFY( str.LoadString( ResId ));
	DisplayError( str );
}


void CWinwallSocket::DisplayLastError( CString& String, int ErrorCode )
{
	GetErrorMsg( String, ErrorCode );
	DisplayError( String );
}

void CWinwallSocket::DisplayLastError( CString& String )
{
	DisplayLastError( String, GetLastError() );
}

void CWinwallSocket::DisplayLastError( int ErrorCode )
{
	CString str;
	DisplayLastError( str, ErrorCode );
}

void CWinwallSocket::DisplayLastError( void )
{
	CString str;
	DisplayLastError( str, GetLastError() );
}


void CWinwallSocket::SetDeviceName( LPCTSTR pName )
{
	m_DeviceName = pName;
}

void CWinwallSocket::GetDeviceName( CString& pName )
{
	pName = m_DeviceName;
}

LPCTSTR CWinwallSocket::GetDeviceName( void )
{
	return m_DeviceName;
}


UDPCommPort::UDPCommPort()
{
	m_Port					= 0;
	m_pData					= NULL;
	m_ReceivedPacketSize	= DEFAULT_DGRAM_SIZE;

	m_State					= STATE_CLOSED;
}

UDPCommPort::~UDPCommPort()
{
	ClosePort();

	if( m_pData ) {
		delete m_pData;
	}
}

void UDPCommPort::SetReceiveBufferSize( int Size )
{
	m_ReceivedPacketSize = Size;

	if( m_pData ) {
		delete m_pData;
	}

	m_pData = new BYTE[ m_ReceivedPacketSize ];
}


BOOL UDPCommPort::OpenListenPort( UINT Port )
{
	SetAddress( Port );
	return OpenListenPort();
}

BOOL UDPCommPort::OpenListenPort( void )
{
	BOOL Res = Create( m_Port, SOCK_DGRAM );

	if( !Res ) {
		DisplayLastError();
		m_State	= STATE_CLOSED;
	} else {
		m_State	= STATE_OPEN;
	}

	return Res;
}

BOOL UDPCommPort::OpenPort( void )
{
	// Pass call onto create function
	BOOL Res = Create( 0, SOCK_DGRAM );

	if( !Res ) {
		DisplayLastError();
	}
	return Res;
}

void UDPCommPort::SetAddress( UINT Port )
{
	m_Port = Port;
}

UINT UDPCommPort::RetrieveAddress( void )
{
	return m_Port;
}


BOOL UDPCommPort::ClosePort( void )
{
	BYTE Buffer[50];
	ShutDown();

	while(Receive(Buffer,50) > 0);
	
	Close();
	m_State = STATE_CLOSED;

	return TRUE;
}

BOOL UDPCommPort::SendPacket( LPCTSTR pHost, UINT Port, PBYTE pData, int Size )
{
	int Count = SendTo( pData, Size, Port, pHost );

	if( Count != Size ) {
		CString msg = ("UDP:SendPacket ");
		DisplayLastError();
		TRACE( "UDP Send Error\n\r" );
	}
		
	return Count == Size;
}


void UDPCommPort::OnConnect( int nErrorCode )
{
	TRACE( "On Connect\n\r" );
}

void UDPCommPort::OnAccept( int nErrorCode )
{
	TRACE( "On Accept\n\r" );
}


void UDPCommPort::ProcessReceivedPacket( LPCTSTR pAddress, UINT Port, PBYTE pData, int Count )
{
}


void UDPCommPort::OnReceive( int nErrorCode )
{
	BOOL	More = TRUE;
	CString	m_Address;
	UINT	m_Port;

	TRACE( "Data arrived\n\r" );

	while( More ) {
		if( !m_pData ) {
			SetReceiveBufferSize( DEFAULT_DGRAM_SIZE );
		}

		int Count = ReceiveFrom( m_pData, m_ReceivedPacketSize, m_Address, m_Port );

		if( Count == SOCKET_ERROR ) {
			int Error = GetLastError();
			if( Error && Error != WSAEWOULDBLOCK ) {
				CString	strError;
				strError.Format( "Receive Error: " );
				DisplayLastError( strError, Error );
			}
			More = FALSE;
		}
		else {
			ProcessReceivedPacket( m_Address, m_Port, m_pData, Count );
		}
	}
}


TCPCommPort::TCPCommPort()
{
	SetState( STATE_CLOSED );
	m_Port = 2100;
}

TCPCommPort::~TCPCommPort()
{
	BYTE Buffer[50];
	ShutDown();
	while(Receive(Buffer,50) > 0);
	Close();
}

TCPCommPort& TCPCommPort::operator=( const TCPCommPort& pW )
{
	CWinwallSocket::operator =( pW );

	m_State		= pW.m_State;
	m_Address	= pW.m_Address;
	m_Port		= pW.m_Port;

	return *this;
}


void TCPCommPort::SetDefaultOptions( void ) 
{
	BOOL bOpt = TRUE;

	if( !SetSockOpt( SO_DONTLINGER, &bOpt, sizeof( bOpt ))) {
		DisplayLastError( GetLastError() );
	}

	if( !SetSockOpt( SO_KEEPALIVE, &bOpt, sizeof( bOpt ))) {
		DisplayLastError( GetLastError() );
	}
}

void TCPCommPort::OpenListenPort( void )
{
}

void TCPCommPort::OpenListenPort( UINT Port )
{
	// Attempts to connect to the device on the requested 
	// address and port.
	ASSERT( Port );

	if( Port == 0 ) return;

	int		LastError = 0;
	CString strError;

	// The port is already initialized.
	switch( GetState() ) {
		case STATE_CLOSED: 
		{
			// The port has not been initialized yet.
			if( Create( Port ) ) {
				SetState( STATE_CREATED );
			} else {
				strError = _T("TCPCommPort:Create ");
			}
			break;
		}
		case STATE_CONNECTED: 
		case STATE_CONNECTING: 
		{
			// The socket is already talking to a device.
			// If the connection is to a different port 
			// close it down.
			if( m_Port == Port )
				return;
			Close();
			break;
		}
	}

	m_Port		= Port;

	if( strError.IsEmpty()) {
		if( Listen()) {
			// The socket has connected. 
			SetState( STATE_CONNECTED );
		} else {
			LastError = GetLastError();
			if( LastError == WSAEWOULDBLOCK ) {
				SetState( STATE_CONNECTING );
			} else {
				strError = _T("TCPCommPort:Connect ");
			}
		}
	}

	if( strError.GetLength() ) {
		if( LastError ) {
			DisplayLastError( strError, LastError );
		} else {
			DisplayLastError( strError );
		}
	}
}


void TCPCommPort::OpenPort( LPCTSTR pAddress, UINT Port )
{
	// Attempts to connect to the device on the requested 
	// address and port.
	ASSERT( Port );
	ASSERT( pAddress );
	ASSERT( *pAddress );

	int		LastError = 0;
	CString strError;

	// The port is already initialized.
	switch( GetState() ) {
		case STATE_CLOSED: 
		{
			// The port has not been initialized yet.
			if(Create()) {
				SetState( STATE_CREATED );
			} else {
				strError = _T("TCP ");
			}
			break;
		}
		case STATE_CONNECTED: 
		case STATE_CONNECTING: 
		{
			// The socket is already talking to a device.
			// If the connection is to a different port 
			// close it down.
			if( m_Address == pAddress && m_Port == Port )
				return;
			Close();
			break;
		}
	}

	m_Address	= pAddress;
	m_Port		= Port;

	if( strError.IsEmpty()) {
		if( Connect( m_Address, m_Port )) {
			// The socket has connected. 
			SetState( STATE_CONNECTED );
		} else {
			LastError = GetLastError();
			if( LastError == WSAEWOULDBLOCK ) {
				SetState( STATE_CONNECTING );
			} else {
				strError = _T("TCP ");
			}
		}
	}

	if( strError.GetLength() ) {
		strError.Empty();
		if( LastError ) {
			DisplayLastError( strError, LastError );
		} else {
			DisplayLastError( strError );
		}
	}
}


void TCPCommPort::OpenPort( void )
{
	int		LastError = 0;
	CString strError;

	if(		m_Port < 0 
		||	m_Port > 65535
		||	!m_Address.GetLength()
	) {
		return;
	}

	if( IsState(STATE_CLOSED) ) {
		// The port has not been initialized yet.
		if(Create() ) {
			SetState( STATE_CREATED );
		} else {
			strError = _T("TCPCommPort:Create ");
		}
	} else {
		// The port is already initialized.
		switch( GetState() ) {
			case STATE_CONNECTED: 
			{
				// The socket is already talking to a device
				// close it down.
				ClosePort();
				break;
			}
			case STATE_CONNECTING: 
			{
				// The socket is in the process of talking to a
				// device. Shut it down.
				ClosePort();
				break;
			}
		}
	}

	if( strError.IsEmpty() && GetState() != STATE_CREATED ) {
		// The port has not been initialized yet.
		if(Create() ) {
			SetState( STATE_CREATED );
		} else {
			strError = _T("TCPCommPort:Create ");
		}
	}

	if( GetState() == STATE_CREATED ) {
		if( Connect( m_Address, m_Port )) {
			// The socket has connected. 
			SetState( STATE_CONNECTED );
		} else {
			LastError = GetLastError();
			if( LastError == WSAEWOULDBLOCK ) {
				SetState( STATE_CONNECTING );
			} else {
				strError = _T("TCPCommPort:Connect ");
			}
		}
	}

	if( strError.GetLength() ) {
		if( LastError ) {
			DisplayLastError( strError, LastError );
		} else {
			DisplayLastError( strError );
		}
	}
}

void TCPCommPort::ClosePort( void )
{
	BYTE Buffer[50];
	ShutDown();
	while(Receive(Buffer,50) > 0);
	Close();
	SetState( STATE_CLOSED );
}


BOOL TCPCommPort::SendPacket( PBYTE pData, int Size )
{
	if( !IsOpen() ) {
		return FALSE;
	}

	int Count = Send( pData, Size );

	if( Count != Size ) {
		CString strError = _T( "TCPComm:SendPacket" );
		DisplayLastError( strError );
	}
		
	return Count == Size;
}

TCPCommPort::SocketState TCPCommPort::GetState( void )
{
	return m_State;
}

void TCPCommPort::OnClose( int nErrorCode )
{
	// Notification that the Socket has closed.
	// This means returning the state to not connected.
	switch( nErrorCode ) {
		case WSAENETDOWN:
		{
			CString strError = _T( "TCPComm:OnClose " );
			DisplayLastError( strError, nErrorCode );
			ClosePort();
			break;
		}
		default:
			ClosePort();
			break;
	}
}

void TCPCommPort::OnAccept( int nErrorCode )
{
	switch( nErrorCode ) {
		case 0:
			// A remote connection has occured.
			TRACE( " Client connected\n\r" );
			ClientConnected();
			break;
		default:
			CString strError;
			DisplayLastError( strError, nErrorCode );
	}
}


void TCPCommPort::ClientConnected( void )
{
	// called when a connection occurs. Don't do anything
}


void TCPCommPort::OnConnect( int nErrorCode )
{
	// Notification that the connection has completed.
	// Check for errors and mark the port as open if success.
	CString strError;
	switch( nErrorCode ) {
		case 0:
			SetState( STATE_CONNECTED );
			break;
		case WSAETIMEDOUT:
			DisplayError( IDS_ERR_NO_CONNECT );
			ClosePort();
			break;
		default:
			ClosePort();
			strError = _T( "TCP: " );
			DisplayLastError( strError, nErrorCode );
	}
}

void TCPCommPort::OnReceive( int nErrorCode )
{
	// Notification for received data. Default
	// implementation just flushes the buffer.
	BYTE Data[5];
	Receive(Data,5);

	CString str;

	str.Format( "%2x %2x %2x %2x %2x", 
		Data[0],Data[1],Data[2],Data[3],Data[4]
	);

	AfxMessageBox( str );
	CWinwallSocket::OnReceive( nErrorCode );
}

void TCPCommPort::RetrieveAddress( CString& Address, UINT& Port )
{
	Address		= m_Address;
	Port		= m_Port;
}

void TCPCommPort::SetAddress( CString& Address, UINT Port )
{
	m_Address	= Address;
	m_Port		= Port;
}

BOOL TCPCommPort::IsOpen( void )
{
	return m_State == STATE_CONNECTED;
}

void TCPCommPort::StateChanging( SocketState State )
{
	// notification to derived classes that
	// something has happened.
}

void TCPCommPort::SetState( SocketState State ) 
{ 
	m_State = State; 
	if( m_State == STATE_CREATED ) {
		SetDefaultOptions();
	}
	StateChanging( State );
}




TCPServerPort::TCPServerPort()
{
	m_pManager = NULL;
}

TCPServerPort::~TCPServerPort()
{
}

void TCPServerPort::ClosePort( void )
{
	TCPCommPort::ClosePort();

	TRACE0( "TCPServerPort closing down\r" );

	// notify the manager class that we are closing.
	if( m_pManager ) {
		m_pManager->PortClosed( this );
	}
}

void TCPServerPort::SetConnectedState( void )
{
	SetState( STATE_CONNECTED );
}

void TCPServerPort::ConnectionMade( void )
{
	// called when the connection is completely established
}




void TCPServerPort::OnReceive( int nErrorCode )
{
	BYTE	Data[50];
	BOOL	More = TRUE;

	while( More ) {
		int Count = Receive(Data,sizeof(Data));

		if( Count == SOCKET_ERROR ) {
			int Error = GetLastError();
			if( Error && Error != WSAEWOULDBLOCK ) {
				CString	strError;
				strError.Format( "Receive Error: " );
				DisplayLastError( strError, Error );
			}
			More = FALSE;
		}
		else 
		{
			if( Count < 1 )
				More = FALSE;
			else
				ProcessReceivedPacket( Data, Count );
		}
	}
}


void TCPServerPort::ProcessReceivedPacket( PBYTE Data, int Count )
{
}


TCPListenManagerPort::TCPListenManagerPort()
{
}

TCPListenManagerPort::~TCPListenManagerPort()
{
	ClosePort();
}

void TCPListenManagerPort::ClosePort( void )
{
	for( int i = 0; i < m_Clients.GetSize(); i++ ) {
		delete m_Clients[i];
	}

	m_Clients.RemoveAll();

	TCPCommPort::ClosePort();
}

 
void TCPListenManagerPort::OpenListenPort( UINT Port )
{
	TCPCommPort::OpenListenPort( Port );
}

void TCPListenManagerPort::ClientConnected( void )
{
	TRACE0( "TCPListenManager Client Connected\n\r" );

	bool More = true;

	while( More ) {
		TCPServerPort*	pClient			= NULL;

		// search for an empty server object.

		for( int i = 0; (pClient == NULL) && (i < m_Clients.GetSize()); i++ ) {
			if( m_Clients[i]->GetState() == STATE_CLOSED ) {
				pClient = m_Clients[i];
				ReuseServer( pClient );
			}
		}

		if( !pClient ) {
			TRACE0( "TCPListenManagerPort:: Create a new Server\n\r" );
			pClient = CreateServer();
			m_Clients.Add( pClient );
		}

		if( pClient ) {
			pClient->SetManager( this );

			if( Accept( *pClient ) == 0 ) {
				TRACE0( "TCPListenManagerPort:: Accept new server failed\n\r" );
	
				// we need to remove it from the client list
				for( int i = 0; i < m_Clients.GetSize(); i++ ) {
					if( m_Clients[i] == pClient ) {
						m_Clients.RemoveAt( i );
					}
				}
				
				delete pClient;	
				More = false;
			} else {
				// let the derived class do things when connection established.
				pClient->SetConnectedState();
				pClient->ConnectionMade();
				ClientListChanged();
			}
		}
	}
}


void TCPListenManagerPort::PortClosed( TCPServerPort* pPort )
{
	// this function is called by a TCPServer port when it shuts down.
	// This presents a bit of a problem because we can't delete the object
	// because it is still alive at this point. Instead this implementation
	// marks the object as available so that next time a connection is made
	// we simply reuse any closed socket objects. If none are available then
	// we create a new one.
	// 
	// To achieve the above action actually requires no work since the test for
	// closed comes completely from the object itself.

	ClientListChanged();
}


TCPServerPort* TCPListenManagerPort::CreateServer( void )
{
	return new TCPServerPort;
}

void TCPListenManagerPort::ReuseServer( TCPServerPort* pPort )
{
}


int TCPListenManagerPort::GetCount( void )
{
	return m_Clients.GetSize();
}

TCPServerPort* TCPListenManagerPort::operator[]( int idx )
{
	if( idx < m_Clients.GetSize() ) {
		return m_Clients[idx];
	}

	return NULL;
}


void TCPListenManagerPort::ClientListChanged( void )
{
}

