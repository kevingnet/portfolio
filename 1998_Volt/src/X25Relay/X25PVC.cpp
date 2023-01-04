// X25PVC.cpp : implementation file
//

#include "stdafx.h"
#include "X25Relay.h"
#include "X25PVC.h"
#include "X25Line.h"
#include "ListenSocket.h"
#include "phys_api.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int	CX25PVC::sm_iBasePort = INVALID_VALUE;

#define CID_STATE_COUNT 11
#define SOCKET_STATE_COUNT 4

#define BUFFER_LOCATION m_pLine->m_Buffer
#define BUFFER_DATA_COUNT m_pLine->m_iCount
#define BUFFER_DATA_LOCATION (BUFFER_LOCATION+HEADER_BYTE_COUNT)
#define COPY_HEADER_TO_BUFFER \
							BUFFER_LOCATION[0] = MODULE_VERSION_CURRENT; \
							BUFFER_LOCATION[1] = HIBYTE(BUFFER_DATA_COUNT); \
							BUFFER_LOCATION[2] = LOBYTE(BUFFER_DATA_COUNT);

#define OOB_PROTO_MESSAGE_SIZE 3

#define BUFFER_LOCATION_OOB_PROTO_MESSAGE_TYPE		BUFFER_LOCATION[0]
#define BUFFER_LOCATION_OOB_PROTO_MESSAGE_CODE		BUFFER_LOCATION[1]
#define BUFFER_LOCATION_OOB_PROTO_MESSAGE_PARAMETER	BUFFER_LOCATION[2]

#define MIN_SOCKET_PACKET_SIZE 3

/////////////////////////////////////////////////////////////////////////////
// CX25PVC

CX25PVC::CX25PVC()
{
	m_pLine				= NULL;
	m_pListenSocket		= NULL;
    m_pCTrace			= NULL;
	m_CIDPVC			= lgo_INVALID_CID;
	m_ConnectionType	= CONNECTION_TYPE_DISABLED;

	m_iPVCNumber		= INVALID_VALUE;
	m_iMaxXOffBuffer	= MAXXOFFBUFFERKB;
	m_PVCEvent			= 0;
	m_StateCode			= 0;
	m_Result			= 0;

	m_LocalX25State			= LINE_STATE_DOWN;
	m_LocalSocketState		= LINE_STATE_DOWN;
	m_RemoteX25State		= LINE_STATE_DOWN;
	m_RemoteSocketState		= LINE_STATE_DOWN;
}

CX25PVC::~CX25PVC()
{
}

// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CX25PVC, TCPSocket)
	//{{AFX_MSG_MAP(CX25PVC)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CX25PVC member functions

//NOTE: we can work on local states when the PVC or socket where in line down modes

void CX25PVC::SetPVCDown()
{
	ChangeState( LINE_TYPE_REMOTE_PVC, LINE_STATE_DOWN );

	//in this case because line is down we can work on the local
	ChangeState( LINE_TYPE_LOCAL_PVC, LINE_STATE_DOWN );

	//so we don't get data from peer socket...
	ChangeState( LINE_TYPE_REMOTE_SOCKET, LINE_STATE_XOFF );
	//but we can still receive OOB data (proto msgs)
	//it's ok for local socket to be XOn, so we can send data out
	//we shouldn't really tinker with local socket transfer modes
	//they should be set by the peer

	//empty socket buffer, it contains data out to peer pvc
	m_baSocketBuffer.RemoveAll();
}

void CX25PVC::SetSocketDown()
{
	ChangeState( LINE_TYPE_REMOTE_SOCKET, LINE_STATE_DOWN );

	//in this case because line is down we can work on the local
	ChangeState( LINE_TYPE_LOCAL_SOCKET, LINE_STATE_DOWN );

	//so we don't get data from peer PVC...
	ChangeState( LINE_TYPE_REMOTE_PVC, LINE_STATE_XOFF );
	//it's ok for local PVC to be XOn, so we can send data out
	//we shouldn't really tinker with local PVC transfer modes
	//they should be set by the peer

	//empty pvc buffer, it contains data out to peer socket
	m_baPVCBuffer.RemoveAll();
}

void CX25PVC::SetPeerPVCXOff()
{
//	if( m_baPVCBuffer.GetSize() > m_iMaxXOffBuffer )
//	{
//		//buffer is full, set peer PVC to XOff
		ChangeState( LINE_TYPE_REMOTE_PVC, LINE_STATE_XOFF );
//	}
//	else
//	{
//		//it's ok for peer PVC to be XOn, if we receive data, we'll buffer it
//	}

	//if peer PVC was previously down, again, 
	//	we do not tinker with its transfer states
	if( LINE_STATE_DOWN == GetCurrentState( LINE_TYPE_LOCAL_PVC ) )
	{
		//set local to xoff
		ChangeState( LINE_TYPE_LOCAL_PVC, LINE_STATE_XOFF );
	}

	//because peer pvc requested XOff state,
	//we'll set remote socket to XOff so we do not get anymore data
	//ChangeState( LINE_TYPE_REMOTE_SOCKET, LINE_STATE_XOFF );

	//here, we do not tinker with local socket states
}

void CX25PVC::SetPeerSocketXOff()
{
	if( IsOpen() )
		ChangeState( LINE_TYPE_REMOTE_SOCKET, LINE_STATE_XOFF );
	else
		ChangeState( LINE_TYPE_REMOTE_SOCKET, LINE_STATE_DOWN );

	//if peer socket was previously down, again, 
	//we do not tinker with its transfer states
	if( LINE_STATE_DOWN == GetCurrentState( LINE_TYPE_LOCAL_SOCKET ) )
	{
		//set local to xon
		ChangeState( LINE_TYPE_LOCAL_SOCKET, LINE_STATE_XOFF );
	}

	//ChangeState( LINE_TYPE_REMOTE_PVC, LINE_STATE_XOFF );
	//it's ok for local PVC to be XOn, so we can send data out
	//we shouldn't really tinker with local PVC transfer modes
	//they should be set by the peer

	//here, we do not tinker with local socket states
}

void CX25PVC::SetPeerPVCXOn()
{
	ChangeState( LINE_TYPE_REMOTE_PVC, LINE_STATE_XON );

	//if peer PVC was not previously down, again, 
	//we do not tinker with its transfer states
	if( LINE_STATE_DOWN == GetCurrentState( LINE_TYPE_LOCAL_PVC ) )
	{
		//set local to xon
		ChangeState( LINE_TYPE_LOCAL_PVC, LINE_STATE_XON );
	}

	//because peer pvc requested XOn state,
	//we'll set remote socket to XOn so we can get data
	//ChangeState( LINE_TYPE_REMOTE_SOCKET, LINE_STATE_XON );

	//here, we do not tinker with local socket states
}

void CX25PVC::SetPeerSocketXOn()
{
	if( IsOpen() )
		ChangeState( LINE_TYPE_REMOTE_SOCKET, LINE_STATE_XON );
	else
		ChangeState( LINE_TYPE_REMOTE_SOCKET, LINE_STATE_DOWN );

	//if peer socket was previously down, again, 
	//we do not tinker with its transfer states
	if( LINE_STATE_DOWN == GetCurrentState( LINE_TYPE_LOCAL_SOCKET ) )
	{
		//set local to xon
		ChangeState( LINE_TYPE_LOCAL_SOCKET, LINE_STATE_XON );
	}

	//ChangeState( LINE_TYPE_REMOTE_PVC, LINE_STATE_XON );

	//here, we do not tinker with local socket states
}

void CX25PVC::DoEvents()
{
	//NOTE: Line state is not the same as PVC state

	if( m_pListenSocket->IsOpen() == FALSE )
	{
		if( IP_STATE_LISTENING != m_pListenSocket->GetState() )
		{
			if( m_pListenSocket->m_CommSocket.IsOpen() == FALSE )
			{
				m_pListenSocket->OpenListenPort( CX25PVC::sm_iBasePort+m_iPVCNumber );
			}
		}
	}

	switch( m_ConnectionType )
	{
		case CONNECTION_TYPE_CONNECT:
			break;
		case CONNECTION_TYPE_LISTEN:
			break;
		case CONNECTION_TYPE_DISABLED:
		default:
			return;
			break;
	}

	if( m_pLine->IsLineUp() == false )
	{

// LINE IS DOWN

		m_StateCode = lgo_State( m_CIDPVC );
		ReportState( m_StateCode );
		switch( m_StateCode )
		{
			case STATE_OPEN:
				break;
			case STATE_WAITING_FOR_REMOTE_ACCEPT:
			case STATE_RESETTING:
			case STATE_WAITING_FOR_LOCAL_ACCEPT:
			case STATE_WAITING_FOR_LOCAL_CONFIRMATION:
			case STATE_WAITING_FOR_REMOTE_CONFIRMATION:
			case STATE_LISTENING:
			case STATE_DATA_TRANSFER_XON:
			case STATE_DATA_TRANSFER_XOFF:
			default:
				SetPVCDown();
				m_Result = lgo_Reopen( m_CIDPVC );
				ReportError( "lgo_Reopen", m_Result );
				break;
		}
	}
	else
	{

/////////////////////////////////////////////////
// LINE IS UP
///////////////////////////////////////////////

		m_StateCode = lgo_State( m_CIDPVC );
		ReportState( m_StateCode );
		switch( m_StateCode )
		{
			case STATE_OPEN:
				if( m_ConnectionType == CONNECTION_TYPE_CONNECT )
				{
					m_Result = lgo_ConnectRequest( m_CIDPVC, NULL, 0 );
					ReportError( "lgo_ConnectRequest", m_Result, true );
				}
				else if( m_ConnectionType == CONNECTION_TYPE_LISTEN )
				{
					m_Result = lgo_Listen( m_CIDPVC );
					ReportError( "lgo_Listen", m_Result );
				}
				if( LINE_STATE_DOWN != GetCurrentState( LINE_TYPE_LOCAL_PVC ) )
				{
					SetPVCDown();
				}
				break;

			case STATE_RESETTING:
				break;

			case STATE_WAITING_FOR_REMOTE_ACCEPT:
				//we have issued a connect request and are waiting...
			case STATE_WAITING_FOR_LOCAL_CONFIRMATION:
				//handled automatically
			case STATE_WAITING_FOR_REMOTE_CONFIRMATION:
				//waiting for a disconnect, reset, Xon or Xoff
			case STATE_LISTENING:
			case STATE_WAITING_FOR_LOCAL_ACCEPT:

				if( LINE_STATE_DOWN != GetCurrentState( LINE_TYPE_LOCAL_PVC ) )
				{
					SetPVCDown();
				}

				break;

			case STATE_DATA_TRANSFER_XOFF:
				//local PVC is XOff, we cannot send to peer pvc

				//update local pvc state
				if( LINE_STATE_XOFF != GetCurrentState( LINE_TYPE_LOCAL_PVC ) )
				{
					ChangeState( LINE_TYPE_LOCAL_PVC, LINE_STATE_XOFF );
					//disable receives from peer socket
					SetPeerPVCXOff();
					SetPeerSocketXOff();
				}

				//if peer socket is XOn And peer pvc is not XOn
				if( LINE_STATE_XON == GetCurrentState( LINE_TYPE_REMOTE_SOCKET ) &&
					LINE_STATE_XON != GetCurrentState( LINE_TYPE_REMOTE_PVC ) )
				{
					//set peer socket to XOff
					//disable receives from peer socket
					SetPeerSocketXOff();
				}

				break;

			case STATE_DATA_TRANSFER_XON:
				//local PVC is XOn, we can send to peer pvc

				//update local pvc state
				if( LINE_STATE_XON != GetCurrentState( LINE_TYPE_LOCAL_PVC ) )
				{
					ChangeState( LINE_TYPE_LOCAL_PVC, LINE_STATE_XON );
					//enable receives from peer socket
					SetPeerPVCXOn();
					SetPeerSocketXOn();
				}

				//if peer pvc is XOn And peer socket is not XOn
				if( LINE_STATE_XON == GetCurrentState( LINE_TYPE_REMOTE_PVC ) &&
					LINE_STATE_XON != GetCurrentState( LINE_TYPE_REMOTE_SOCKET ) )
				{
					//set peer pvc to XOff
					//disable receives from peer socket
					SetPeerPVCXOff();
				}

				//any pending data for our peer pvc?
				if( 0 != m_baSocketBuffer.GetSize() )
				{
					SendSocketBufferedDataToPVC();
				}

				break;
		}

		//is peer socket connected
		if( IsOpen() )
		{
			//is local socket in XOn state
			if( LINE_STATE_XON != GetCurrentState( LINE_TYPE_LOCAL_SOCKET ) )
			{
				//any pending data for our peer socket
				if( 0 != m_baPVCBuffer.GetSize() )
				{
					SendPVCBufferedDataToSocket();
				}
			}
		}

		bool		bReadEvent = true;
		ULONG		ulBytesRead	= 0;
		m_PVCEvent = lgo_Poll( m_CIDPVC, &(BUFFER_DATA_COUNT) );
		if( m_PVCEvent >= 0 )
		{
			switch( m_PVCEvent )
			{
				case EVENT_DATA_OK:

					bReadEvent = false;

					//we received x25 data

					//can we send it to the peer socket?
					if( LINE_STATE_DOWN == GetCurrentState( LINE_TYPE_LOCAL_SOCKET ) )
					{
						//local socket is down (of course peer socket is too)
						
						//discard data
						m_Result = lgo_DiscardData( m_CIDPVC );
						ReportError( "lgo_DiscardData", m_Result, true );
						BUFFER_DATA_COUNT = 0;
					}
					else if( LINE_STATE_XOFF == GetCurrentState( LINE_TYPE_LOCAL_SOCKET ) )
					{
						//local socket is XOff

						//check if the data can fit in our buffer
						if( BUFFER_DATA_COUNT > m_pLine->m_iMaxBufferSize )
						{
							//packet is too large for our buffer
							m_Result = lgo_DiscardData( m_CIDPVC );
							ReportError( "lgo_DiscardData", m_Result, true );
							//issue error
							SendMessage( MESSAGE_TYPE_ERROR, ERROR_DATA, 0, "Packet is larger than buffer" );
						}
						else
						{
							//read data, to be buffered later
							ulBytesRead += lgo_Read( m_CIDPVC, BUFFER_LOCATION, BUFFER_DATA_COUNT );
							
							//can the buffer take more data?
							if( ( m_baPVCBuffer.GetSize() + BUFFER_DATA_COUNT ) > 
								m_iMaxXOffBuffer )
							{
								//cannot fit in buffer
								//empty the buffer, discard old data!
								m_baPVCBuffer.RemoveAll();
								//issue error
								SendMessage( MESSAGE_TYPE_ERROR, GET_LINE_STATE, LINE_STATE_XOFF, "Buffer was full, discarded all data!" );
							}
							//copy data to buffer
							AddQueueToBuffer( m_baPVCBuffer, BUFFER_LOCATION, BUFFER_DATA_COUNT );
						}

						BUFFER_DATA_COUNT = 0;
					}
					else
					{
						//check if the data can fit in our buffer
						if( BUFFER_DATA_COUNT > m_pLine->m_iMaxBufferSize )
						{
							//packet is too large for our buffer
							m_Result = lgo_DiscardData( m_CIDPVC );
							ReportError( "lgo_DiscardData", m_Result, true );
							//issue error
							SendMessage( MESSAGE_TYPE_ERROR, ERROR_DATA, 0, "Packet is larger than buffer" );
						}
						else
						{
							//copy header bytes
							COPY_HEADER_TO_BUFFER

							//read data
							ulBytesRead += lgo_Read( m_CIDPVC, BUFFER_DATA_LOCATION, BUFFER_DATA_COUNT );

							//TODO: check if ulBytesRead != BUFFER_DATA_COUNT
							
							//send data through socket
							Send( (PBYTE)BUFFER_LOCATION, ulBytesRead+HEADER_BYTE_COUNT );
						}

						BUFFER_DATA_COUNT = 0;
					}

					break;
			}
		}
		if( bReadEvent == true )
		{
			BUFFER_DATA_COUNT = m_pLine->m_iMaxBufferSize;
			m_PVCEvent = lgo_Event( m_CIDPVC, BUFFER_LOCATION, &(BUFFER_DATA_COUNT) );
			ReportEvent( m_PVCEvent );
			if( m_PVCEvent >= 0 )
			{
				switch( m_PVCEvent )
				{
					case EVENT_CONNECTED:
						SendMessage( MESSAGE_TYPE_INFO, INFO_DATA, 0, "X.25 Connected" );
						break;
					case EVENT_CONNECT_ACCEPT:
						SendMessage( MESSAGE_TYPE_INFO, INFO_DATA, 0, "X.25 Connection accepted" );
						break;
					case EVENT_RESET_COMPLETED:
						SendMessage( MESSAGE_TYPE_INFO, INFO_DATA, 0, "X.25 Reset completed" );
						break;
					case EVENT_RESETTING:
						SendMessage( MESSAGE_TYPE_ERROR, WARNING_DATA, 0, "X.25 Resetting" );
						break;
					case EVENT_DISCONNECT_CONFIRMATION:
						SendMessage( MESSAGE_TYPE_ERROR, WARNING_DATA, 0, "X.25 Disconnect confirmation" );
						break;
					case EVENT_DISCONNECTED:
						SendMessage( MESSAGE_TYPE_ERROR, WARNING_DATA, 0, "X.25 Disconnected" );
						break;
					case EVENT_DISCONNECT_REQUEST:
						SendMessage( MESSAGE_TYPE_ERROR, WARNING_DATA, 0, "X.25 Disconnect request" );
						break;
					case EVENT_DISCONNECTING:
						SendMessage( MESSAGE_TYPE_ERROR, WARNING_DATA, 0, "X.25 Disconnecting" );
						break;
					case EVENT_CONNECT_REFUSE:
						SendMessage( MESSAGE_TYPE_ERROR, ERROR_DATA, 0, "X.25 Connection refused" );
						break;
					case EVENT_XON_REQUEST:
						SendMessage( MESSAGE_TYPE_INFO, INFO_DATA, 0, "X.25 XOn Request" );
						break;
					case EVENT_XOFF_REQUEST:
						SendMessage( MESSAGE_TYPE_WARNING, WARNING_DATA, 0, "X.25 XOff Request" );
						break;
					case EVENT_DATA_LENGTH_ERROR:
						SendMessage( MESSAGE_TYPE_ERROR, ERROR_DATA, 0, "X.25 Data length error" );
						break;
					case EVENT_DATA_CRC_ERROR:
						SendMessage( MESSAGE_TYPE_ERROR, ERROR_DATA, 0, "X.25 CRC error" );
						break;
					case EVENT_DATA_ABORTED_FRAME:
						SendMessage( MESSAGE_TYPE_ERROR, ERROR_DATA, 0, "X.25 Aborted frame" );
						break;
					case EVENT_DATA_SPECIAL:
						SendMessage( MESSAGE_TYPE_INFO, ERROR_DATA, 0, "X.25 Data special ???" );
						break;

					case EVENT_PROTOCOL_MESSAGE:
						SendMessage( MESSAGE_TYPE_INFO, PROTOCOL_MESSAGE, 0, "X.25 Protocol message" );
						break;

					case EVENT_RESET_TIMEOUT:
						break;
					case EVENT_CONNECT_TIMEOUT:
						break;
					case EVENT_DISCONNECT_TIMEOUT:
						break;
					case EVENT_CONNECT_REQUEST:
						break;

					default:
						break;
				}
			}
		}
	}
}

//state functions

void CX25PVC::OnStateChanged( SocketState State )
{
    OWS_1trace( (*m_pCTrace), LEVEL_EVENTSANDPOLLS, "Socket state changed to: %s", GetStateString() );
	switch( State ) 
	{
		case IP_STATE_CONNECTED:
			//a peer socket has connected

			OWS_0trace( (*m_pCTrace), LEVEL_WARNING, "Network connected, disabling listen port" );
			
			//shutdown the listen socket, so we only accept one connection
			m_pListenSocket->ShutDown();
			m_pListenSocket->Close();
		
			ChangeState( LINE_TYPE_LOCAL_SOCKET, LINE_STATE_XON );
			//enable receives from peer pvc
			SetPeerPVCXOn();

			break;
		case IP_STATE_CLOSED:
			//peer socket has disconnected

			OWS_0trace( (*m_pCTrace), LEVEL_WARNING, "Network disconnected, enabling listen port" );
			
			//enable listen port
			m_pListenSocket->OpenListenPort( CX25PVC::sm_iBasePort+m_iPVCNumber );

			SetSocketDown();

		case IP_STATE_CREATED:
		case IP_STATE_CONNECTING:
		case IP_STATE_LISTENING:
		case IP_STATE_DISCONNECTING:
		default:
		
			break;
	}
}

void CX25PVC::OnStateChanged( X25LineTypes LineType )
{
	m_szString.Format( "[State Changed] %s : %s", GetLineTypeString(LineType), GetLineStateString(LineType) );
	ReportMessage( m_szString );
}

void CX25PVC::ChangeState( X25LineTypes LineType, LineStates state )
{
	switch( LineType )
	{
		case LINE_TYPE_LOCAL_PVC:
			switch( state )
			{
				case LINE_STATE_DOWN:
					SendMessage( MESSAGE_TYPE_ERROR, LINE_STATE, LINE_STATE_DOWN );
					break;
				case LINE_STATE_XON:
					SendMessage( MESSAGE_TYPE_INFO, LINE_STATE, LINE_STATE_XON );
					break;
				case LINE_STATE_XOFF:
					SendMessage( MESSAGE_TYPE_WARNING, LINE_STATE, LINE_STATE_XOFF );
					break;
			}
			m_LocalX25State = state;
			break;
		case LINE_TYPE_LOCAL_SOCKET:
			switch( state )
			{
				case LINE_STATE_DOWN:
					//we don't send message, if we are down, we can't really send, can we?
					break;
				case LINE_STATE_XON:
					SendMessage( MESSAGE_TYPE_INFO, LINE_STATE, LINE_STATE_XON );
					break;
				case LINE_STATE_XOFF:
					SendMessage( MESSAGE_TYPE_WARNING, LINE_STATE, LINE_STATE_XOFF );
					break;
			}
			m_LocalSocketState = state;
			break;
		case LINE_TYPE_REMOTE_PVC:
			switch( state )
			{
				case LINE_STATE_DOWN:
					break;
				case LINE_STATE_XON:
					m_Result = lgo_Xon( m_CIDPVC );
					ReportError( "lgo_Xon", m_Result, true );
					break;
				case LINE_STATE_XOFF:
					m_Result = lgo_Xoff( m_CIDPVC );
					ReportError( "lgo_Xoff", m_Result, true );
					break;
			}
			m_RemoteX25State = state;
			break;
		case LINE_TYPE_REMOTE_SOCKET:
			switch( state )
			{
				case LINE_STATE_DOWN:
					break;
				case LINE_STATE_XON:
					SendMessage( MESSAGE_TYPE_COMMAND, SET_LINE_STATE, LINE_STATE_XON );
					break;
				case LINE_STATE_XOFF:
					SendMessage( MESSAGE_TYPE_COMMAND, SET_LINE_STATE, LINE_STATE_XOFF );
					break;
			}
			m_RemoteSocketState = state;
			break;
		default:
			return;
	}
	OnStateChanged( LineType );
}

LineStates CX25PVC::GetCurrentState( X25LineTypes LineType )
{
	switch( LineType )
	{
		case LINE_TYPE_LOCAL_PVC:
			return( m_LocalX25State );
			break;
		case LINE_TYPE_LOCAL_SOCKET:
			return( m_LocalSocketState );
			break;
		case LINE_TYPE_REMOTE_PVC:
			return( m_RemoteX25State );
			break;
		case LINE_TYPE_REMOTE_SOCKET:
			return( m_RemoteSocketState );
			break;
	}
	return LINE_STATE_DOWN;
}


//receive data and messages functions

void CX25PVC::OnReceive( int /*nErrorCode*/ )
{
	//received data from peer socket

	int		iLastError = 0;

	int iCount = Receive( BUFFER_LOCATION, m_pLine->m_iMaxBufferSize );
	switch( iCount )
	{   
		case 0: //no data?, oh! well...
			return;
			break;   
		case SOCKET_ERROR:
			iLastError = GetLastError();
			if( iLastError != WSAEWOULDBLOCK )       
			{
				DisplayLastError( iLastError );
			}      
			return;
			break;
		default:      
			break;
	}

	//is the packet version correct?
	if( MODULE_VERSION_CURRENT != GetPacketVersion() )
	{
		SendMessage( MESSAGE_TYPE_ERROR, WRONG_VERSION, 0, "The version in the packet received is invalid or unsupported!" );
		return;
	}

	//did all the data arrive?
	if( iCount != GetPacketDataCount() )
	{
		SendMessage( MESSAGE_TYPE_ERROR, WRONG_PACKET_SIZE, 0, "The size in the packet header does not match with the size of the packet received!" );
		return;
	}


	//what do we do with the data
	switch( m_LocalX25State )
	{
		case LINE_STATE_DOWN:
			//discard it, and...
			//send an error message
			SendMessage( MESSAGE_TYPE_ERROR, LINE_STATE, LINE_STATE_DOWN, "Data was discarded" );
			//set peer socket to XOff
			SetPeerSocketXOff();
			break;
		case LINE_STATE_XOFF:
			//can the buffer take more data?
			if( ( m_baSocketBuffer.GetSize() + iCount ) > 
				m_iMaxXOffBuffer )
			{
				//cannot fit in buffer
				//empty the buffer, discard old data!
				m_baSocketBuffer.RemoveAll();
				//issue error
				SendMessage( MESSAGE_TYPE_ERROR, ERROR_DATA, 0, "Buffer was full, discarded all previously buffered data!" );
			}
			else
			{
				//send an warning message
				SendMessage( MESSAGE_TYPE_WARNING, LINE_STATE, LINE_STATE_XOFF, "PVC is XOff, data was saved to buffer" );
			}
			//copy data to buffer
			AddQueueToBuffer( m_baSocketBuffer, BUFFER_LOCATION, iCount );

			//set peer socket to XOff
			SetPeerSocketXOff();
			break;
		case LINE_STATE_XON:
			m_Result = lgo_Write( m_CIDPVC, BUFFER_DATA_LOCATION, iCount-HEADER_BYTE_COUNT );
			ReportError( "lgo_Write", m_Result );
			if( m_Result < 0)
			{
				/*
					failure generally means we are trying to write buffers
					faster than the transmitter can handle due to line speed
				*/
				switch( m_Result )
				{
					case ERROR_INVALID_CID:
					case ERROR_INVALID_COMMAND:
					case ERROR_INVALID_INPUT_BUFFER:
					case ERROR_BUFFER_SIZE:
					case ERROR_TRANSMISSION_BLOCKED:
					case ERROR_NO_SYSTEM_BUFFER:
					case ERROR_WRITE_QUEUE_FULL:
					case ERROR_INVALID_WRITE_DATA_SIZE:
					case ERROR_PROTOCOL_BLOCKING:
						SendMessage( MESSAGE_TYPE_ERROR, ERROR_DATA, 0, m_szString );
						break;
				}
			}
			break;
	}
}

int CX25PVC::GetPacketVersion()
{
	return BUFFER_LOCATION[0];
}

int CX25PVC::GetPacketDataCount()
{
	int iCount = 0;
	iCount			= (USHORT)BUFFER_LOCATION[1];
	iCount			<<= 8;
	iCount			+= (BYTE)BUFFER_LOCATION[2];
	return iCount;
}

void CX25PVC::OnOutOfBandData( int nErrorCode )
{
	//received protocol message from peer socket

	int		iLastError = 0;
	int iCount = Receive( BUFFER_LOCATION, m_pLine->m_iMaxBufferSize, MSG_OOB );

	switch( iCount )
	{   
		case 0: //no data?, oh! well...
			return;
			break;   
		case SOCKET_ERROR:
			iLastError = GetLastError();
			if( iLastError != WSAEWOULDBLOCK )       
			{
				DisplayLastError( iLastError );
			}      
			return;
			break;
		default:      
			break;
	}

	switch( BUFFER_LOCATION_OOB_PROTO_MESSAGE_TYPE )
	{
		case GET_LINE_STATE:
			SendMessage( MESSAGE_TYPE_REPLY, LINE_STATE, GetCurrentState( LINE_TYPE_LOCAL_SOCKET ) );
			break;
		case SET_LINE_STATE:
			switch( BUFFER_LOCATION_OOB_PROTO_MESSAGE_CODE )
			{
				case LINE_STATE_XON:
					ChangeState( LINE_TYPE_LOCAL_SOCKET, LINE_STATE_XON );
					SetPeerPVCXOn();
					break;
				case LINE_STATE_XOFF:
					ChangeState( LINE_TYPE_LOCAL_SOCKET, LINE_STATE_XOFF );
					SetPeerPVCXOff();
					break;
			}
			SendMessage( MESSAGE_TYPE_REPLY, LINE_STATE, GetCurrentState( LINE_TYPE_LOCAL_SOCKET ) );
			break;
		case GET_LINE_STATISTICS:

			m_szString = "LINE STAT\nPhysical Layer:\n";
			m_szString += GetStatistics( CIDTYPE_PHYSICAL );
			m_szString += "\nLink Layer:\n";
			m_szString += GetStatistics( CIDTYPE_LINK );
			m_szString += "\nPacket Layer:\n";
			m_szString += GetStatistics( CIDTYPE_PACKET );
			m_szString += "\nPVC:\n";
			m_szString += GetStatistics( CIDTYPE_PVC );

			SendMessage( MESSAGE_TYPE_REPLY, GET_LINE_STATISTICS, GetPVCState(), m_szString );
			break;
		default:
			//unkown request
			break;
	}
}


//helper functions

int	CX25PVC::GetPacketSize( PBYTE pData )
{
	int iCount		= 0;
	iCount			= (USHORT)pData[1];
	iCount			<<= 8;
	iCount			+= (BYTE)pData[2];
	return iCount;
}

void CX25PVC::SendPVCBufferedDataToSocket()
{
	int		iMaxBytes		= m_baPVCBuffer.GetSize();
	PBYTE	pData			= m_baPVCBuffer.GetData();
	int		iCount			= 0;
	int		iBytesSent		= 0;

	while( true )
	{
		if( iMaxBytes < (iBytesSent+MIN_SOCKET_PACKET_SIZE) )
			return;
		iCount = GetPacketSize( pData );
		
		Send( pData, iCount );
		
		iBytesSent += iCount;
		if( iBytesSent > iMaxBytes )
			return;
		pData += iCount;
	}
}

void CX25PVC::SendSocketBufferedDataToPVC()
{
	int		iMaxBytes		= m_baSocketBuffer.GetSize();
	PBYTE	pData			= m_baSocketBuffer.GetData();
	int		iCount			= 0;
	int		iBytesSent		= 0;

	while( true )
	{
		if( iMaxBytes < (iBytesSent+MIN_SOCKET_PACKET_SIZE) )
			return;
		iCount = GetPacketSize( pData );

		m_Result = lgo_Write( m_CIDPVC, pData, iCount );
		ReportError( "lgo_Write", m_Result );
		if( m_Result < 0 )
			SendMessage( MESSAGE_TYPE_ERROR, ERROR_DATA, 0, m_szString );

		iBytesSent += iCount;
		if( iBytesSent > iMaxBytes )
			return;
		pData += iCount;
	}
}

void CX25PVC::SendMessage( int iMessageType, int iCode, int iParameter, LPCTSTR szDetails )
{
	if( IsOpen() )
	{
		if( NULL != BUFFER_LOCATION )
		{
			BUFFER_LOCATION_OOB_PROTO_MESSAGE_TYPE			= iMessageType;
			BUFFER_LOCATION_OOB_PROTO_MESSAGE_CODE			= iCode;
			BUFFER_LOCATION_OOB_PROTO_MESSAGE_PARAMETER		= iParameter;
			BUFFER_DATA_COUNT								= OOB_PROTO_MESSAGE_SIZE;
			if( szDetails && *szDetails )
			{
				BUFFER_DATA_COUNT += strlen( szDetails );
				strcpy( (char*)(BUFFER_LOCATION+OOB_PROTO_MESSAGE_SIZE), szDetails );
			}
			SendOutOfBandData( (PBYTE)BUFFER_LOCATION, BUFFER_DATA_COUNT );
		}
	}
}

void CX25PVC::SendCommand( int iCommandType, int iCIDType )
{
	CID cid = lgo_INVALID_CID;
	switch( iCIDType )
	{
		case CIDTYPE_PVC:
			cid = m_CIDPVC;
			break;
		case CIDTYPE_PACKET:
			cid = m_pLine->m_CIDPacket;
			break;
		case CIDTYPE_LINK:
			cid = m_pLine->m_CIDLink;
			break;
		case CIDTYPE_PHYSICAL:
			cid = m_pLine->m_CIDPhysical;
			break;
	}

	CString szCommand;
	switch( iCommandType )
	{
		case COMMANDTYPE_SENDTESTDATA:
			szCommand = "Command: Send Test Data";
			strcpy( (char*)BUFFER_LOCATION, "TEST DATA - abc 123" );
			m_Result = lgo_Write( cid, BUFFER_LOCATION, m_pLine->m_iMaxBufferSize );
			break;
		case COMMANDTYPE_RESET:
			szCommand = "Command: Reset";
			m_Result = lgo_ResetRequest( cid, NULL, 0 );
			break;
		case COMMANDTYPE_XONREQUEST:
			szCommand = "Command: XOn Request";
			m_Result = lgo_Xon( cid );
			break;
		case COMMANDTYPE_XOFFREQUEST:
			szCommand = "Command: XOff Request";
			m_Result = lgo_Xoff( cid );
			break;
		case COMMANDTYPE_CLEARSTATISTICS:
			szCommand = "Command: Clear Statistics";
			m_Result = lgo_ClearStatistics( cid );
			break;
		case COMMANDTYPE_LISTEN:
			szCommand = "Command: Listen";
			m_Result = lgo_Listen( cid );
			break;
		case COMMANDTYPE_CONNECT:
			szCommand = "Command: Connect";
			m_Result = lgo_ConnectRequest( cid, NULL, 0 );
			break;
		case COMMANDTYPE_DISCONNECT:
			szCommand = "Command: Disconnect";
			m_Result = lgo_DisconnectRequest( cid, NULL, 0 );
			break;
		case COMMANDTYPE_REOPEN:
			szCommand = "Command: Reopen";
			m_Result = lgo_Reopen( m_pLine->m_CIDPhysical );
			m_Result = lgo_Reopen( m_pLine->m_CIDLink );
			m_Result = lgo_Reopen( m_pLine->m_CIDPacket );
			m_Result = lgo_Reopen( m_CIDPVC );
			break;
	}
	ReportError( szCommand, m_Result, true );
}

void CX25PVC::AddQueueToBuffer( CByteArray& baBuffer, const PBYTE pData, int iCount )
{
	int iArraySize = baBuffer.GetSize();
	baBuffer.SetSize( iArraySize + iCount + HEADER_BYTE_COUNT );
	
	//add version
	baBuffer.SetAt( iArraySize++, MODULE_VERSION_CURRENT );
	
	//add byte count
	baBuffer.SetAt( iArraySize++, HIBYTE(iCount) );
	baBuffer.SetAt( iArraySize++, LOBYTE(iCount) );
	
	//add data
	for( int i=0; i<iCount; i++ )
	{
		baBuffer.SetAt( iArraySize++, pData[i] );
	}
}

int CX25PVC::GetPVCState()
{
	m_StateCode = lgo_State( m_CIDPVC );
	ReportState( m_StateCode );
	switch( m_StateCode )
	{
		case STATE_OPEN:
		case STATE_WAITING_FOR_REMOTE_ACCEPT:
		case STATE_RESETTING:
		case STATE_WAITING_FOR_LOCAL_ACCEPT:
		case STATE_WAITING_FOR_LOCAL_CONFIRMATION:
		case STATE_WAITING_FOR_REMOTE_CONFIRMATION:
		case STATE_LISTENING:
			return LINE_STATE_DOWN;
			break;
		case STATE_DATA_TRANSFER_XOFF:
			return LINE_STATE_XOFF;
			break;
		case STATE_DATA_TRANSFER_XON:
			return LINE_STATE_XON;
			break;
	}
	return LINE_STATE_DOWN;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
// ERROR HANDLERS
//////////////////////////////////////////////////////////////////////////////////

void CX25PVC::DisplayError( LPCTSTR pStr )
{
	OWS_2trace( (*m_pCTrace), LEVEL_ERROR, "#%i TCP/IP Error: %s", m_iPVCNumber+1, pStr );
}

void CX25PVC::ReportError( LPCTSTR pStr, Result Error, bool bDisplayIfNoError )
{
	if( Error < 0 || bDisplayIfNoError == true )
	{
		if( Error >= 0 )
		{
			OWS_2trace( (*m_pCTrace), LEVEL_INFO, "#%i [Info] %s", m_iPVCNumber+1, pStr );
		}
		else if( Error != ERROR_TRANSMISSION_BLOCKED )
		{
			OWS_3trace( (*m_pCTrace), LEVEL_ERROR, "#%i [ERROR] %s : %s", m_iPVCNumber+1, pStr, lgo_ErrorMessage(Error) );
		}
	}
}

void CX25PVC::ReportState( StateCode StateValue )
{
	if( StateValue < 0 )
	{
		if( StateValue != ERROR_TRANSMISSION_BLOCKED )
		{
			OWS_2trace( (*m_pCTrace), LEVEL_ERROR, "#%i [ERROR] %s", m_iPVCNumber+1, lgo_ErrorMessage(StateValue) );
		}
	}
	else
	{
		switch( StateValue )
		{
			case STATE_DATA_TRANSFER_XON:
			case STATE_DATA_TRANSFER_XOFF:
			case STATE_WAITING_FOR_REMOTE_ACCEPT:
			case STATE_LISTENING:
			case STATE_OPEN:
				break;
			default:
				OWS_2trace( (*m_pCTrace), LEVEL_EVENTSANDPOLLS, "#%i [PVC State] %s", m_iPVCNumber+1, lgo_StateMessage(StateValue) );
				break;
		}
	}
}

void CX25PVC::ReportEvent( EventCode EventValue )
{
	if( EventValue < 0 )
	{
		if( EventValue != ERROR_TRANSMISSION_BLOCKED &&
			EventValue != ERROR_NOTHING_WAITING )
		{
			OWS_2trace( (*m_pCTrace), LEVEL_ERROR, "#%i [ERROR] %s : %s", m_iPVCNumber+1, lgo_ErrorMessage(EventValue) );
		}
	}
	else
	{
		switch( EventValue )
		{
			case EVENT_DATA_OK:
				break;
			case EVENT_CONNECT_REQUEST:
			case EVENT_CONNECTED:
			case EVENT_CONNECT_TIMEOUT:
			case EVENT_CONNECT_ACCEPT:
			case EVENT_CONNECT_REFUSE:
			case EVENT_DISCONNECT_REQUEST:
			case EVENT_DISCONNECTED:
			case EVENT_DISCONNECTING:
			case EVENT_DISCONNECT_TIMEOUT:
			case EVENT_DISCONNECT_CONFIRMATION:
			case EVENT_RESETTING:
			case EVENT_RESET_COMPLETED:
			case EVENT_RESET_TIMEOUT:
			case EVENT_XON_REQUEST:
			case EVENT_XOFF_REQUEST:
			case EVENT_DATA_LENGTH_ERROR:
			case EVENT_DATA_CRC_ERROR:
			case EVENT_DATA_ABORTED_FRAME:
			case EVENT_PROTOCOL_MESSAGE:
			case EVENT_DATA_SPECIAL:
			default:
				OWS_2trace( (*m_pCTrace), LEVEL_EVENTSANDPOLLS, "#%i [Network Event] %s", m_iPVCNumber+1, lgo_EventMessage(EventValue) );
				break;
		}
	}
}

void CX25PVC::ReportMessage( LPCTSTR pStr )
{
	OWS_2trace( (*m_pCTrace), LEVEL_INFO, "#%i %s", m_iPVCNumber+1, pStr );
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// GUI UPDATERS
//////////////////////////////////////////////////////////////////////////////////

LPCTSTR CX25PVC::GetLineTypeString( X25LineTypes type )
{
	for( int i=0; i<LINE_TYPE_MAX; i++ ) 
	{
		if( sm_LineTypesTable[i].type == type ) 
		{
			return sm_LineTypesTable[i].szText;
		}
	}
	return NULL;
}

LPCTSTR CX25PVC::GetLineStateString( X25LineTypes type )
{
	LineStates state;
	switch( type )
	{
		case LINE_TYPE_LOCAL_PVC:
			state = m_LocalX25State;
			break;
		case LINE_TYPE_LOCAL_SOCKET:
			state = m_LocalSocketState;
			break;
		case LINE_TYPE_REMOTE_PVC:
			state = m_RemoteX25State;
			break;
		case LINE_TYPE_REMOTE_SOCKET:
			state = m_RemoteSocketState;
			break;
	}
	switch( state )
	{
		case LINE_STATE_DOWN:
			return "Down";
			break;
		case LINE_STATE_XON:
			return "XOn";
			break;
		case LINE_STATE_XOFF:
			return "XOff";
			break;
	}
	return NULL;
}

UINT CX25PVC::GetLineIcon()
{
	if( m_ConnectionType == CONNECTION_TYPE_DISABLED ) return IDI_ICON_LINE_DISABLED;
	for( int i=0; i<CID_STATE_COUNT; i++ ) 
	{
		if( sm_LinesTable[i].iState == m_pLine->GetCurrentLineState() ) 
		{
			return sm_LinesTable[i].uiIcon;
		}
	}
	return IDI_ICON_LINE_DISABLED;
}

LPCTSTR CX25PVC::GetLineStateString()
{
	if( m_ConnectionType == CONNECTION_TYPE_DISABLED ) return sm_LinesTable[0].szText;
	for( int i=0; i<CID_STATE_COUNT; i++ ) 
	{
		if( sm_LinesTable[i].iState == m_pLine->GetCurrentLineState() ) 
		{
			return sm_LinesTable[i].szText;
		}
	}
	return sm_LinesTable[0].szText;
}

UINT CX25PVC::GetPVCIcon()
{
	if( m_ConnectionType == CONNECTION_TYPE_DISABLED ) return IDI_ICON_PVC_DISABLED;
	for( int i=0; i<CID_STATE_COUNT; i++ ) 
	{
		if( sm_PVCsTable[i].iState == lgo_State( m_CIDPVC ) ) 
		{
			return sm_PVCsTable[i].uiIcon;
		}
	}
	return IDI_ICON_PVC_DISABLED;
}

LPCTSTR CX25PVC::GetPVCStateString()
{
	if( m_ConnectionType == CONNECTION_TYPE_DISABLED ) return sm_LinesTable[0].szText;
	for( int i=0; i<CID_STATE_COUNT; i++ ) 
	{
		if( sm_PVCsTable[i].iState == lgo_State( m_CIDPVC ) ) 
		{
			return sm_PVCsTable[i].szText;
		}
	}
	return sm_PVCsTable[0].szText;
}

UINT CX25PVC::GetSocketIcon()
{
	if( m_ConnectionType == CONNECTION_TYPE_DISABLED ) return IDI_ICON_SOCKET_DISABLED;
	SocketState	state;
	for( int i=0; i<SOCKET_STATE_COUNT; i++ ) 
	{
		state = GetState();
		if( TCPSocket::IP_STATE_CLOSED == state )
		{
			state = m_pListenSocket->GetState();
		}
		if( sm_SocketsTable[i].iState == state ) 
		{
			if( TCPSocket::IP_STATE_CONNECTED == state )
			{
				if( LINE_STATE_XOFF == GetCurrentState( LINE_TYPE_LOCAL_SOCKET ) )
					return IDI_ICON_SOCKET_XOFF;
				else
					return sm_SocketsTable[i].uiIcon;
			}
			else
			{
				return sm_SocketsTable[i].uiIcon;
			}
		}
	}
	return IDI_ICON_SOCKET_DISABLED;
}

LPCTSTR CX25PVC::GetSocketStateString()
{
	if( m_ConnectionType == CONNECTION_TYPE_DISABLED ) return sm_SocketsTable[0].szText;
	SocketState	state;
	for( int i=0; i<SOCKET_STATE_COUNT; i++ ) 
	{
		state = GetState();
		if( TCPSocket::IP_STATE_CLOSED == state )
		{
			state = m_pListenSocket->GetState();
		}
		if( sm_SocketsTable[i].iState == state ) 
		{
			if( TCPSocket::IP_STATE_CONNECTED == state )
			{
				if( LINE_STATE_XOFF == GetCurrentState( LINE_TYPE_LOCAL_SOCKET ) )
					return sm_SocketsTable[4].szText;
				else
					return sm_SocketsTable[i].szText;
			}
			else
			{
				return sm_SocketsTable[i].szText;
			}
		}
	}
	return sm_SocketsTable[0].szText;
}

LPCTSTR CX25PVC::GetPVCText()
{
	char rxBuf[BUFFER_SIZE];
	BufferSize Count = (BufferSize)sizeof(rxBuf);
	Count = lgo_GetStatistics( m_CIDPVC, rxBuf, Count );
	if( Count < 0 )
	{
		m_szString = "no statistics available";
	}
	else
	{
		StatTable pStatsTable = (StatTable)rxBuf;
		m_szString.Format( "Tx: %11lu       Rx: %11lu", pStatsTable->bytesSent, pStatsTable->bytesRcvd );
	}
	return (LPCTSTR)m_szString;
}

void CX25PVC::ClearStatistics()
{
	lgo_ClearStatistics( m_pLine->m_CIDPhysical );
	lgo_ClearStatistics( m_pLine->m_CIDLink );
	lgo_ClearStatistics( m_pLine->m_CIDPacket );
	lgo_ClearStatistics( m_CIDPVC );
}

CString CX25PVC::GetStatistics( int iStatType  )
{
	CID cid = INVALID_VALUE;
	switch( iStatType )
	{
		case CIDTYPE_PHYSICAL:
			cid = m_pLine->m_CIDPhysical;
			break;
		case CIDTYPE_LINK:
			cid = m_pLine->m_CIDLink;
			break;
		case CIDTYPE_PACKET:
			cid = m_pLine->m_CIDPacket;
			break;
		case CIDTYPE_PVC:
			cid = m_CIDPVC;
			break;
	}

	char rxBuf[BUFFER_SIZE];
	BufferSize Count = (BufferSize)sizeof(rxBuf);
	Count = lgo_GetStatistics( cid, rxBuf, Count );

	CString szReturnStats;
	if( Count < 0 )
	{
		szReturnStats = "no statistics available";
	}
	else
	{
		StatTable pStatsTable = (StatTable)rxBuf;
		if( pStatsTable != NULL )
		{
			MessageTable    strings = lgo_StatisticMessages( lgo_Protocol( cid ) );

			szReturnStats.Format("%11lu : Sent\n%11lu : Received\n",
					pStatsTable->bytesSent, pStatsTable->bytesRcvd);

			CString szStats;
			for( int i=0; i<pStatsTable->statCount; i++ )
			{
				if ( strings && strings[i] )
				{
					szStats.Format( "%11u : %s\n", pStatsTable->stat[i], strings[i] );
				}
				else
				{
					szStats.Format( "%11u : %u\n", pStatsTable->stat[i], i );
				}
				szReturnStats += szStats;
			}
		}
	}
	return szReturnStats;
}

void CX25PVC::GetLEDStatus( char * pStatusArray )
{
	long		mask = 0;

	if( IsEnabled() )
	{
		BufferSize	size = sizeof(mask);
		m_Result = lgo_Ioctl(	m_pLine->m_CIDPhysical,
								phys_IOCTL_GET_SIGNALS,
								&mask,
								sizeof(mask),
								&mask,
								&size
								);
		ReportError( "lgo_Ioctl", m_Result );
	}

	PhysSignals signals = phys_SIGNAL_DATA_SIGNALS(mask);
	PhysSignals config = phys_SIGNAL_DATA_CONFIG(mask);

	if( config & phys_SIGNAL_DTR )
	{
		if( signals & phys_SIGNAL_DTR )
			pStatusArray[LED_DTR] = LEDSTATE_HIGH;
		else
			pStatusArray[LED_DTR] = LEDSTATE_LOW;
	}
	else
	{
		pStatusArray[LED_DTR] = LEDSTATE_OFF;
	}

	if( config & phys_SIGNAL_RTS )
	{
		if( signals & phys_SIGNAL_RTS )
			pStatusArray[LED_RTS] = LEDSTATE_HIGH;
		else
			pStatusArray[LED_RTS] = LEDSTATE_LOW;
	}
	else
	{
		pStatusArray[LED_RTS] = LEDSTATE_OFF;
	}

	if( config & phys_SIGNAL_DCD )
	{
		if( signals & phys_SIGNAL_DCD )
			pStatusArray[LED_DCD] = LEDSTATE_HIGH;
		else
			pStatusArray[LED_DCD] = LEDSTATE_LOW;
	}
	else
	{
		pStatusArray[LED_DCD] = LEDSTATE_OFF;
	}

	if( config & phys_SIGNAL_CTS )
	{
		if( signals & phys_SIGNAL_CTS )
			pStatusArray[LED_CTS] = LEDSTATE_HIGH;
		else
			pStatusArray[LED_CTS] = LEDSTATE_LOW;
	}
	else
	{
		pStatusArray[LED_CTS] = LEDSTATE_OFF;
	}

	if( config & phys_SIGNAL_DSR )
	{
		if( signals & phys_SIGNAL_DSR )
			pStatusArray[LED_DSR] = LEDSTATE_HIGH;
		else
			pStatusArray[LED_DSR] = LEDSTATE_LOW;
	}
	else
	{
		pStatusArray[LED_DSR] = LEDSTATE_OFF;
	}

	if( config & phys_SIGNAL_FLAG )
	{
		if( signals & phys_SIGNAL_FLAG )
			pStatusArray[LED_FLAGS] = LEDSTATE_HIGH;
		else
			pStatusArray[LED_FLAGS] = LEDSTATE_LOW;
	}
	else
	{
		pStatusArray[LED_FLAGS] = LEDSTATE_OFF;
	}

}

LPCTSTR CX25PVC::GetLineTitle()
{
	m_szString.Format("line: %i", m_pLine->m_iLineNumber+1 );
	return (LPCTSTR)m_szString;
}

LPCTSTR CX25PVC::GetPVCTitle()
{
	m_szString.Format("pvc: %i", m_iPVCNumber+1 );
	return (LPCTSTR)m_szString;
}

LPCTSTR CX25PVC::GetSocketTitle()
{
	m_szString.Format("port: %i", CX25PVC::sm_iBasePort+m_iPVCNumber );
	return (LPCTSTR)m_szString;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// TABLES
//////////////////////////////////////////////////////////////////////////////////

ConnectionState CX25PVC::sm_LinesTable[] = {
	{ STATE_DISABLED,						IDI_ICON_LINE_DISABLED,		"Disabled" },
	{ STATE_CLOSED,							IDI_ICON_LINE_CLOSED,		"Closed" },
	{ STATE_OPEN,							IDI_ICON_LINE_OPEN,			"Open" },
	{ STATE_RESETTING,						IDI_ICON_LINE_RESETING,		"Resetting..." },
	{ STATE_LISTENING,						IDI_ICON_LINE_LISTENING,	"Listening..." },
	{ STATE_WAITING_FOR_LOCAL_ACCEPT,		IDI_ICON_LINE_WAITINGFOR_LOCAL_ACCEPT,	"Wating for local accept" },
	{ STATE_WAITING_FOR_REMOTE_ACCEPT,		IDI_ICON_LINE_WAITINGFOR_REMOTE_ACCEPT,	"Wating for remote accept" },
	{ STATE_WAITING_FOR_LOCAL_CONFIRMATION,	IDI_ICON_LINE_WAITINGFOR_LOCAL_CONFIRM,	"Wating for local confirmation" },
	{ STATE_WAITING_FOR_REMOTE_CONFIRMATION,IDI_ICON_LINE_WAITINGFOR_REMOTE_CONFIRM,"Wating for remote confirmation" },
	{ STATE_DATA_TRANSFER_XON,				IDI_ICON_LINE_TRANSFER_XON,	"XOn" },
	{ STATE_DATA_TRANSFER_XOFF,				IDI_ICON_LINE_TRANSFER_XOFF,"XOff" }
};

ConnectionState CX25PVC::sm_PVCsTable[] = {
	{ STATE_DISABLED,						IDI_ICON_PVC_DISABLED,		"Disabled" },
	{ STATE_CLOSED,							IDI_ICON_PVC_CLOSED,		"Closed" },
	{ STATE_OPEN,							IDI_ICON_PVC_OPEN,			"Open" },
	{ STATE_RESETTING,						IDI_ICON_PVC_RESETING,		"Resetting..." },
	{ STATE_LISTENING,						IDI_ICON_PVC_LISTENING,		"Listening..." },
	{ STATE_WAITING_FOR_LOCAL_ACCEPT,		IDI_ICON_PVC_WAITING,		"Wating for local accept" },
	{ STATE_WAITING_FOR_REMOTE_ACCEPT,		IDI_ICON_PVC_WAITING,		"Wating for remote accept" },
	{ STATE_WAITING_FOR_LOCAL_CONFIRMATION,	IDI_ICON_PVC_WAITING,		"Wating for local confirmation" },
	{ STATE_WAITING_FOR_REMOTE_CONFIRMATION,IDI_ICON_PVC_WAITING,		"Wating for remote confirmation" },
	{ STATE_DATA_TRANSFER_XON,				IDI_ICON_PVC_XON,			"XOn" },
	{ STATE_DATA_TRANSFER_XOFF,				IDI_ICON_PVC_XOFF,			"XOff" }
};

ConnectionState CX25PVC::sm_SocketsTable[] = {
	{ STATE_DISABLED,										IDI_ICON_SOCKET_DISABLED,	"Disabled" },
	{ TCPSocket::IP_STATE_CLOSED,							IDI_ICON_SOCKET_CLOSED,		"Closed" },
	{ TCPSocket::IP_STATE_LISTENING,						IDI_ICON_SOCKET_LISTENING,	"Listening..." },
	{ TCPSocket::IP_STATE_CONNECTED,						IDI_ICON_SOCKET_XON,		"Connected" },
	{ STATE_XOFF,											IDI_ICON_SOCKET_XOFF,		"Blocking" }
};

CX25PVC::LineTypesStruct CX25PVC::sm_LineTypesTable[] = {
	{ CX25PVC::LINE_TYPE_LOCAL_PVC,			"Local PVC" },
	{ CX25PVC::LINE_TYPE_LOCAL_SOCKET,		"Local Socket" },
	{ CX25PVC::LINE_TYPE_REMOTE_PVC,		"Peer PVC" },
	{ CX25PVC::LINE_TYPE_REMOTE_SOCKET,		"Peer Socket" }
};

