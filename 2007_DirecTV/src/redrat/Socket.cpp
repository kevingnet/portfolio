#include "Socket.h"

Logger * Socket::m_pLogger = 0;
fd_set Socket::m_ReadFDS;
char Socket::m_Buffer[64];

//------------------------------------------------------------------------------

SocketException::SocketException(char* error, int code): 
m_ErrorText(error),
m_Code(code)
{
	if( m_Code ) {
		m_ErrorText += " ";
		m_ErrorText += strerror(m_Code);
	}
}

SocketException::~SocketException(void)
{
}

//------------------------------------------------------------------------------

SocketClient::SocketClient(void) : m_IsBase(false), m_ID(0), m_RXCount(0), m_TXCount(0), m_ErrorCount(0)
{
}

SocketClient::~SocketClient(void)
{
}

void SocketClient::CheckRead()
{
	uint16_t code = 0;
	int count = ReadCode( code );
	switch( count ) {
	case SOCKET_DISCONNECT:
	case SOCKET_FAILURE:
		Disconnect();
		break;
	case SOCKET_RETRYREAD:
		break;
	default:
		OnReceive(code);
		break;
	}
}

int SocketClient::ReadCode( uint16_t & code ) 
{ 
	int count = recv( m_Socket, &code, sizeof(code), MSG_DONTWAIT );
	if ( count == SOCKET_FAILURE ) {
		switch( errno ) {
       		case EAGAIN:
			return SOCKET_RETRYREAD;
       		case EINTR:
			m_pLogger->Output("Warning: read was interrupted: %s ", strerror(errno));
			return SOCKET_RETRYREAD;
		case ENOMEM:
		case EFAULT:
        		throw SocketException("socket recv failed", errno);
		default:
			m_pLogger->Output("Error: read failed with error: %s", strerror(errno));
			return SOCKET_FAILURE;
		}
	} else if ( count == 0 ) {
		return SOCKET_DISCONNECT;
	}
	return count;
}

void SocketClient::OnConnect()
{
	m_RXCount = 0;
	m_TXCount = 0;
	m_ErrorCount = 0;
	m_ID = lrand48();
	socklen_t addrlen = sizeof(m_Address);
	int res = getpeername(m_Socket, (sockaddr*)&m_Address, &addrlen);
	if( res == SOCKET_SUCCESS ) {
		if(IsLocal())
			m_pLogger->Output("Info: new connection s#%d id: %d", m_Socket, m_ID );
		else
			m_pLogger->Output("Info: new connection s#%d id: %d addr: %s", m_Socket, m_ID, inet_ntoa(m_Address.sin_addr) );
	} else {
		m_pLogger->Output("Error: getpeername failed with error: %s", strerror(errno));
	}
	uint16_t * pShort = reinterpret_cast<uint16_t*>(m_Buffer);
	uint32_t * pLong = reinterpret_cast<uint32_t*>(m_Buffer+2);
	*pShort = m_Socket;
	*pLong = m_ID;
	res = write( m_Socket, &m_Buffer, 6 ); 
	m_TXCount++;
}

void SocketClient::OnDisconnect()
{
}

void SocketClient::Disconnect()
{
	close( m_Socket );
	m_pLogger->Output("Info: client disconnected s#%d id: %d rx: %d tx: %d err: %d", m_Socket, m_ID, m_RXCount, m_TXCount, m_ErrorCount );
	m_Socket = SocketDisconnected;
	m_ID = 0;
}

void SocketClient::OnReceive(uint16_t code)
{
	m_RXCount++;
	SendResponse(0);
	//SendResponse(0x8001);
}

void SocketClient::SendResponse( uint16_t response )
{
	size_t res = write( m_Socket, &response, 2 ); 
	m_TXCount++;
	//m_pLogger->Output("Info: sending response..." );
}

//------------------------------------------------------------------------------

SocketManager::SocketManager(void)
{
	m_Port              = 0;
	m_MaxSocket         = 0;
	m_MaxClients        = 0;
	m_ConnectionTimeOut = 0;
	for(int i=0; i<BASE_SOCKET_CLIENTS; ++i)
		m_BaseClients[i].SetBase();
}

SocketManager::~SocketManager(void)
{
	Shutdown();
}

void SocketManager::Initialize(int port, int maxclients, double conntimeout)
{
	m_Port                  = port;
	m_MaxClients            = maxclients;
	m_ConnectionTimeOut     = conntimeout;

	m_Socket = socket(AF_INET,SOCK_STREAM,0);
	if ( m_Socket == SOCKET_FAILURE )
		throw SocketException("create server socket failed", errno);

	m_MaxSocket = m_Socket;
	int opt = 1;
	int res = setsockopt( m_Socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt) ); 
	if ( res != SOCKET_SUCCESS )
		throw SocketException("set socket failed", errno);

	m_Address.sin_family 		= AF_INET;
	m_Address.sin_addr.s_addr 	= INADDR_ANY;
	m_Address.sin_port 		= htons(m_Port);

	res = bind( m_Socket, (struct sockaddr *)&m_Address, sizeof(m_Address) );
	if ( res != SOCKET_SUCCESS )
		throw SocketException("bind socket failed", errno);

	res = fcntl( m_Socket, F_SETFL, fcntl(m_Socket, F_GETFL) | O_NONBLOCK );
	if( res == SOCKET_FAILURE )
		throw SocketException("fcntl failed to set server to non-blocking mod", errno);

	m_Clients.reserve(BASE_SOCKET_CLIENTS + 8 );
}

void SocketManager::StartListening()
{
	if(m_Socket != SocketInvalid) {
		int res = listen( m_Socket, 3 );
		if ( res == SOCKET_FAILURE ) 
			switch( errno ) {
	       		case EBADF:
       			case EADDRINUSE:
			case ENOTSOCK:
			//case ENOTSUPP:
			default:
				throw SocketException("listen socket failed", errno);
			}
		else
			m_pLogger->Output("Server listening for connections");
	}
}

void SocketManager::StopListening()
{
	if(m_Socket != SocketInvalid) {
		int res = shutdown( m_Socket, 0 );
		if ( res == SOCKET_FAILURE )
			switch( errno ) {
	       		case EBADF:
			case ENOTSOCK:
			default:
				throw SocketException("shutdown socket failed", errno);
			case ENOTCONN:
				m_pLogger->Output("Warning: socket shutdown failed because socket is not connected");
				break;
			}
		else
			m_pLogger->Output("Server not accepting new connections");
	}
}

void SocketManager::OnConnect()
{
	if (!CanListen()) {
		StopListening();
		m_pLogger->Output("Maximum number of connections was reached");
	}

	socklen_t addrlen = sizeof(m_Address);
	int new_socket = accept( m_Socket, (struct sockaddr *)&m_Address, &addrlen );
	if ( new_socket == SOCKET_FAILURE ) {
		switch( errno ) {
		case EAGAIN: //EWOULDBLOCK:
		case ECONNABORTED:
		case EINTR:
			m_pLogger->Output("Warning: accept failed with error: %s", strerror(errno));
			return;
		case EBADF:
		case ENOTSOCK:
		case EOPNOTSUPP:
		case EMFILE:
		case ENFILE:
		case EPROTO:
		case EPERM:
		default:
			m_pLogger->Output("Error: accept failed with error: %s", strerror(errno));
			return;
		case ENOMEM:
		case ENOBUFS:
		case EFAULT:
			throw SocketException("accept socket failed", errno);
		case EINVAL:
			m_pLogger->Output("Warning: accept failed because server was not accepting connections. Client should retry...");
			StartListening();
			return;
		}
	}

	int res = fcntl( new_socket, F_SETFL, fcntl(new_socket, F_GETFL) | O_NONBLOCK );
	if( res == SOCKET_FAILURE ) {
		switch( errno ) {
		case EPROTONOSUPPORT:
		case EAFNOSUPPORT:
		case EACCES:
		case EINVAL:
		default:
			m_pLogger->Output("Error: accept failed with error: %s", strerror(errno));
			close( new_socket );
			break;
		case ENOBUFS:
		case ENOMEM:
		case ENFILE:
		case EMFILE:
			throw SocketException("failed to set socket to non-blocking mode", errno);
		}
	}

	//we need a socket object, see if we can find one from the base array,
	SocketClientPtr pClient = 0;
	for(int i=0; i<BASE_SOCKET_CLIENTS; ++i){
		if( m_BaseClients[i].IsUnused() )
			pClient = &m_BaseClients[i];
	}
	if( !pClient )
		pClient = new SocketClient();
		//pClient = new SocketClient(new_socket);

	m_Clients.push_back( pClient );

	pClient->SetSocket( new_socket );
	pClient->OnConnect();
	
	if( new_socket > m_MaxSocket )
		m_MaxSocket = new_socket;
}

void SocketManager::Shutdown()
{
	StopListening();
	SocketClientPtr pClient = 0;
	for_each(m_Clients.begin(), m_Clients.end(), mem_fun(&SocketClient::Shutdown));
	int size = m_Clients.size();
	for(int i=0; i<size; ++i){
		pClient = m_Clients[i];
		if( pClient && !pClient->IsBase() ) {
			delete pClient;
		}
	}
	m_Clients.clear();
	m_Port              = 0;
	m_MaxSocket         = 0;
	m_MaxClients        = 0;
	m_ConnectionTimeOut = 0;
}

SocketClientPtr SocketManager::GetUnusedBaseSocketClient()
{
	for(int i=0; i<BASE_SOCKET_CLIENTS; ++i){
		if( m_BaseClients[i].IsUnused() )
			return &m_BaseClients[i];
	}
}

void SocketManager::CleanupDisconnectedSockets()
{
	SocketClientPtr pClient = 0;
	int size = m_Clients.size();
	m_MaxSocket = m_Socket;
	for(int i=0; i<size; ++i){
		pClient = m_Clients[i];
		if( pClient ) {
			int socket = pClient->GetSocket();
			switch(socket){
			case SocketDisconnected:
				pClient->SetInvalid();
			//fall through
			case SocketInvalid:
				m_Clients.erase(m_Clients.begin() + i);
				if( !pClient->IsBase() ) {
					delete pClient;
					pClient = 0;
				}
				break;
			default:
				if( socket > m_MaxSocket )
					m_MaxSocket = socket;
				break;
			}
		}
	}
	if( size < m_MaxClients ) {
		int listening = 0;
		socklen_t len = sizeof(int);
		int res = getsockopt(m_Socket, SOL_SOCKET, SO_ACCEPTCONN, &listening, &len); 
		if ( !listening )
			StartListening();
	}
}

void SocketManager::Poll()
{
	struct timeval timeout;
	timeout.tv_usec = 250000; //half a second, so this thing in theory should process almost two calls per second
	timeout.tv_sec  = 0;
	FD_ZERO(&m_ReadFDS);
	//m_pLogger->Output("polling...");

	SetRead();
	for_each(m_Clients.begin(), m_Clients.end(), mem_fun(&SocketClient::SetRead));

	int res = select( m_MaxSocket + 3, &m_ReadFDS, NULL, NULL, &timeout );
	if( res == SOCKET_FAILURE ) {
		switch( errno ) {
		case EBADF:
		case EINVAL:
			m_pLogger->Output("Error: select failed with error: %s", strerror(errno));
			return;
		case EINTR:
			m_pLogger->Output("Warning: select failed with error: %s", strerror(errno));
			return;
		case ENOMEM:
		default:
        		throw SocketException("socket select failed", errno);
		}
	}

	if ( CanRead() )
		OnConnect();
	for_each(m_Clients.begin(), m_Clients.end(), mem_fun(&SocketClient::CheckRead));

	CleanupDisconnectedSockets();
}


