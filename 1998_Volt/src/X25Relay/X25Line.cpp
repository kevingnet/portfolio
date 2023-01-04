// X25Line.cpp : implementation file
//

/******************************************************************************************

For simplicity:

	the layers do not handle Xon or Xoff conditions, they should never happen
	if anyone layer should be disconnected, they will all disconnect and restart the 
		connection process or listening process


*******************************************************************************************/

#include "stdafx.h"
#include "X25Relay.h"
#include "X25Line.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CX25Line

CX25Line::CX25Line()
{
	m_ConnectionType	= CONNECTION_TYPE_DISABLED;
	m_LineState			= LINE_STATE_CLOSED;

	m_CIDPhysical		= lgo_INVALID_CID;
	m_CIDLink			= lgo_INVALID_CID;
	m_CIDPacket			= lgo_INVALID_CID;
	m_iPVCCount			= lgo_INVALID_CID;
	m_iLineNumber		= INVALID_VALUE;
	m_MajorId			= 0;

	m_PVCEvent			= 0;
	m_StateCode			= 0;

	m_iMaxBufferSize	= 0;
	m_iCount			= 0;
	m_Buffer			= NULL;
    m_pCTrace			= NULL;
}

CX25Line::~CX25Line()
{
	if( NULL != m_Buffer )
		delete m_Buffer;
}

/////////////////////////////////////////////////////////////////////////////
// CX25Line member functions

void CX25Line::ConnectLayer( CIDType iCIDType )
{
	CID cid = lgo_INVALID_CID;
	switch( iCIDType )
	{
		case CIDTYPE_PACKET:
			cid = m_CIDPacket;
			break;
		case CIDTYPE_LINK:
			cid = m_CIDLink;
			break;
		case CIDTYPE_PHYSICAL:
			cid = m_CIDPhysical;
			break;
	}
	m_Result = 0;
	if( STATE_OPEN == lgo_State( cid ) )
	{
		m_Result = lgo_ConnectRequest( cid, NULL, 0 );
		ReportError( "Connect Request", m_Result, iCIDType, true );
	}
}

void CX25Line::DisconnectLayer( CIDType iCIDType )
{
	CID cid = lgo_INVALID_CID;
	switch( iCIDType )
	{
		case CIDTYPE_PACKET:
			cid = m_CIDPacket;
			break;
		case CIDTYPE_LINK:
			cid = m_CIDLink;
			break;
		case CIDTYPE_PHYSICAL:
			cid = m_CIDPhysical;
			break;
	}
	m_Result = 0;
	m_StateCode = lgo_State( cid );
	if( STATE_DATA_TRANSFER_XON == m_StateCode ||
		STATE_RESETTING == m_StateCode )
	{
		m_Result = lgo_DisconnectRequest( cid, NULL, 0 );
		ReportError( "Disconnect Request", m_Result, iCIDType, true );
		m_Result = lgo_Reopen( cid );
		ReportError( "Reopen", m_Result, iCIDType );
	}
}

void CX25Line::ListenLayer( CIDType iCIDType )
{
	CID cid = lgo_INVALID_CID;
	switch( iCIDType )
	{
		case CIDTYPE_PACKET:
			cid = m_CIDPacket;
			break;
		case CIDTYPE_LINK:
			cid = m_CIDLink;
			break;
		case CIDTYPE_PHYSICAL:
			cid = m_CIDPhysical;
			break;
	}
	m_StateCode = lgo_State( cid );
	switch( m_StateCode )
	{
		case STATE_LISTENING:
			m_Result = 0;
			break;
		case STATE_OPEN:
			m_Result = lgo_Listen( cid );
			ReportError( "Listen", m_Result, iCIDType );
			break;

		case STATE_WAITING_FOR_REMOTE_ACCEPT:
		case STATE_WAITING_FOR_REMOTE_CONFIRMATION:

		case STATE_WAITING_FOR_LOCAL_CONFIRMATION:
		case STATE_WAITING_FOR_LOCAL_ACCEPT:

		case STATE_RESETTING:
		case STATE_DATA_TRANSFER_XOFF:
		case STATE_DATA_TRANSFER_XON:
			m_Result = lgo_Reopen( cid );
			ReportError( "Reopen", m_Result, iCIDType );
			m_Result = lgo_Listen( cid );
			ReportError( "Listen", m_Result, iCIDType );
			break;
	}
}

bool CX25Line::AllLayersOpen()
{
	if( STATE_OPEN == lgo_State( m_CIDPacket ) &&
		STATE_OPEN == lgo_State( m_CIDLink ) &&
		STATE_OPEN == lgo_State( m_CIDPhysical ) )
	{
		return true;
	}
	return false;
}

bool CX25Line::AllLayersXOn()
{
	if( STATE_DATA_TRANSFER_XON == lgo_State( m_CIDPacket ) &&
		STATE_DATA_TRANSFER_XON == lgo_State( m_CIDLink ) &&
		STATE_DATA_TRANSFER_XON == lgo_State( m_CIDPhysical ) )
	{
		return true;
	}
	return false;
}

void CX25Line::ListenAllLayers()
{
	ListenLayer( CIDTYPE_PHYSICAL );
	if( m_Result >= 0 )
	{
		ListenLayer( CIDTYPE_LINK );
		if( m_Result >= 0 )
		{
			ListenLayer( CIDTYPE_PACKET );
			if( m_Result >= 0 )
			{
				ChangeState( LINE_STATE_LISTENING );
			}
		}
	}
}

void CX25Line::DisconnectAllLayers()
{
	DisconnectLayer( CIDTYPE_PHYSICAL );
	DisconnectLayer( CIDTYPE_LINK );
	DisconnectLayer( CIDTYPE_PACKET );
}

void CX25Line::ReopenAllLayers()
{
	lgo_Reopen( m_CIDPacket );
	lgo_Reopen( m_CIDLink );
	lgo_Reopen( m_CIDPhysical );
}

void CX25Line::ResetAllLayers()
{
	m_Result = lgo_ResetRequest( m_CIDPacket, NULL, 0 );
	ReportError( "Reset Request", m_Result, CIDTYPE_PACKET, true );
	m_Result = lgo_ResetRequest( m_CIDLink, NULL, 0 );
	ReportError( "Reset Request", m_Result, CIDTYPE_LINK, true );
	m_Result = lgo_ResetRequest( m_CIDPhysical, NULL, 0 );
	ReportError( "Reset Request", m_Result, CIDTYPE_PHYSICAL, true );
}

void CX25Line::DoEvents()
{
	switch( m_ConnectionType )
	{
		case CONNECTION_TYPE_CONNECT:
			ProcessLineConnect();
			break;
		case CONNECTION_TYPE_LISTEN:
			ProcessLineListen();
			break;
		case CONNECTION_TYPE_DISABLED:
		default:
			break;
	}
}

StateCode CX25Line::GetCurrentLineState()
{
	StateCode state = lgo_State( m_CIDPhysical );
	switch( state )
	{
		case STATE_CLOSED:
		case STATE_OPEN:
		case STATE_RESETTING:
		case STATE_LISTENING:
		case STATE_WAITING_FOR_LOCAL_ACCEPT:
		case STATE_WAITING_FOR_LOCAL_CONFIRMATION:
		case STATE_WAITING_FOR_REMOTE_ACCEPT:
		case STATE_WAITING_FOR_REMOTE_CONFIRMATION:
		case STATE_DATA_TRANSFER_XOFF:
			return state;
			break;

		case STATE_DATA_TRANSFER_XON:
			break;
	}
	state = lgo_State( m_CIDLink );
	switch( state )
	{
		case STATE_CLOSED:
		case STATE_OPEN:
		case STATE_RESETTING:
		case STATE_LISTENING:
		case STATE_WAITING_FOR_LOCAL_ACCEPT:
		case STATE_WAITING_FOR_LOCAL_CONFIRMATION:
		case STATE_WAITING_FOR_REMOTE_ACCEPT:
		case STATE_WAITING_FOR_REMOTE_CONFIRMATION:
		case STATE_DATA_TRANSFER_XOFF:
			return state;
			break;

		case STATE_DATA_TRANSFER_XON:
			break;
	}
	state = lgo_State( m_CIDPacket );
	switch( state )
	{
		case STATE_CLOSED:
		case STATE_OPEN:
		case STATE_RESETTING:
		case STATE_LISTENING:
		case STATE_WAITING_FOR_LOCAL_ACCEPT:
		case STATE_WAITING_FOR_LOCAL_CONFIRMATION:
		case STATE_WAITING_FOR_REMOTE_ACCEPT:
		case STATE_WAITING_FOR_REMOTE_CONFIRMATION:
		case STATE_DATA_TRANSFER_XOFF:
			return state;
			break;

		case STATE_DATA_TRANSFER_XON:
			break;
	}
	return state;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
//
//	CONNECT
//
////////////////////////////////////////////////////////////////////////////////////////////
void CX25Line::ProcessLineConnect()
{
	//get physical layer STATE
	m_StateCode = lgo_State( m_CIDPhysical );
	ReportState( m_StateCode, CIDTYPE_PHYSICAL );
	switch( m_StateCode )
	{
		case STATE_OPEN:
			//if this layer is open...
			//disconnect the other layers, for resiliency...
			DisconnectLayer( CIDTYPE_LINK );
			DisconnectLayer( CIDTYPE_PACKET );

			//if line is supposed to be higher than open state...
			if( m_LineState > LINE_STATE_OPEN )
			{
				//set to open state
				ChangeState( LINE_STATE_OPEN );
			}

			//connect physical layer
			ConnectLayer( CIDTYPE_PHYSICAL );
			if( m_Result >= 0 )
			{
				ChangeState( LINE_STATE_CONNECTING_PHYSICAL );
			}
			break;
		case STATE_WAITING_FOR_REMOTE_ACCEPT:
			//we have issued a connect request and are waiting...
			break;

		case STATE_WAITING_FOR_LOCAL_CONFIRMATION:
			//handled automatically
			break;
		case STATE_WAITING_FOR_REMOTE_CONFIRMATION:
			//waiting for a disconnect, reset, Xon or Xoff
			break;

		case STATE_LISTENING:
			//this is obviously erroneous, however, this could happen when debugging
			//the user can issue this command through the GUI
			break;
		case STATE_WAITING_FOR_LOCAL_ACCEPT:
			//the user issued the command through the GUI and network connected to us
			break;

		case STATE_RESETTING:
			//handled automatically
			break;

		case STATE_DATA_TRANSFER_XOFF:
			//This can happen if Hardware Flow Control is enabled
			if( m_LineState == LINE_STATE_XON )
			{
				ChangeState( LINE_STATE_XOFF );
			}
			//ResetAllLayers();
			break;

		case STATE_DATA_TRANSFER_XON:
			//if line is supposed to be lower than connected physical...
			if( m_LineState < LINE_STATE_CONNECTED_PHYSICAL )
			{
				//we can now attempt to connect the link layer
				ChangeState( LINE_STATE_CONNECTED_PHYSICAL );
			}
			break;
	}

	//issue commands based on our current line state, line state is NOT layer state
	switch( m_LineState )
	{
		case LINE_STATE_OPEN:
			break;
		case LINE_STATE_CONNECTING_PHYSICAL:
			//transitory state, waiting for connect accept event 
			//which will change state to connected
			break;
		case LINE_STATE_CONNECTED_PHYSICAL:
			//connect link layer
			ConnectLayer( CIDTYPE_LINK );
			if( m_Result >= 0 )
			{
				ChangeState( LINE_STATE_CONNECTING_LINK );
			}
			break;
		case LINE_STATE_CONNECTING_LINK:
			//transitory state, waiting for connect accept event 
			//which will change state to connected
			break;
		case LINE_STATE_CONNECTED_LINK:
			//connect packet layer
			ConnectLayer( CIDTYPE_PACKET );
			if( m_Result >= 0 )
			{
				ChangeState( LINE_STATE_CONNECTING_PACKET );
			}
			break;
		case LINE_STATE_CONNECTING_PACKET:
			//transitory state, waiting for connect accept event 
			//which will change state to connected
			break;
		case LINE_STATE_CONNECTED_PACKET:
			//we are now ready to for the pvcs
			if( true == AllLayersXOn() )
			{
				ChangeState( LINE_STATE_XON );
			}
			break;
		case LINE_STATE_XON:
			//this is the desired state for pvc operation
			break;
	}

	//get physical layer EVENT
	m_iCount	= m_iMaxBufferSize;
	m_PVCEvent	= lgo_Event( m_CIDPhysical, m_Buffer, &m_iCount );
	ReportEvent( m_PVCEvent, CIDTYPE_PHYSICAL );
	if( m_PVCEvent >= 0 )
	{
		//note: changing line state to open will cause a disconnection of the other lines
		switch( m_PVCEvent )
		{
			case EVENT_CONNECT_ACCEPT:
				//remote accepted our connect request
				ChangeState( LINE_STATE_CONNECTED_PHYSICAL );
				break;
			case EVENT_DISCONNECT_CONFIRMATION:
				//we disconnected from remote, confirmation
			case EVENT_DISCONNECTED:
				//remote disconnected from us
				break;
			case EVENT_CONNECT_REQUEST:
				//we received a connect request by the remote, this is usually erroneous
				//since this line is in connect mode
				break;
			case EVENT_CONNECTED:
				break;
			case EVENT_CONNECT_TIMEOUT:
				//it will be retried, very soon...
				break;
			case EVENT_CONNECT_REFUSE:
				//it will be retried, very soon...
				break;
			case EVENT_DISCONNECT_REQUEST:
				//remote requested a disconnect
				break;
			case EVENT_DISCONNECTING:
				break;
			case EVENT_DISCONNECT_TIMEOUT:
				break;
			case EVENT_RESETTING:
				break;
			case EVENT_RESET_COMPLETED:
				break;
			case EVENT_RESET_TIMEOUT:
				break;
			case EVENT_XON_REQUEST:
				break;
			case EVENT_XOFF_REQUEST:
				break;
			case EVENT_PROTOCOL_MESSAGE:
			case EVENT_DATA_LENGTH_ERROR:
			case EVENT_DATA_CRC_ERROR:
			case EVENT_DATA_ABORTED_FRAME:
			case EVENT_DATA_SPECIAL:
			case EVENT_DATA_OK:
				//this is handled by the top layer
				break;
			default:
				break;
		}
	}

	//get link layer STATE
	m_StateCode = lgo_State( m_CIDLink );
	ReportState( m_StateCode, CIDTYPE_LINK );
	switch( m_StateCode )
	{
		case STATE_OPEN:
			//if this layer is open...
			//disconnect the other layer, for resiliency...
			DisconnectLayer( CIDTYPE_PACKET );
			//if line is supposed to be higher than connected to physical layer state
			if( m_LineState >= LINE_STATE_CONNECTED_PHYSICAL )
			{
				//obviously we are not,
				if( STATE_DATA_TRANSFER_XON == lgo_State( m_CIDPhysical ) )
				{
					//at least physical layer is connected
					ChangeState( LINE_STATE_CONNECTED_PHYSICAL );
				}
				else
				{
					//we are not even connected to the physical layer, darn it
					if( m_LineState > LINE_STATE_OPEN )
						ChangeState( LINE_STATE_OPEN );
				}
			}
			//else if it's ok for this layer to be open
			break;
		case STATE_WAITING_FOR_REMOTE_ACCEPT:
			//we have issued a connect request and are waiting...
			break;

		case STATE_WAITING_FOR_LOCAL_CONFIRMATION:
			//handled automatically
			break;
		case STATE_WAITING_FOR_REMOTE_CONFIRMATION:
			//waiting for a disconnect, reset, Xon or Xoff
			break;

		case STATE_LISTENING:
			//this is obviously erroneous, however, this could happen when debugging
			//the user can issue this command through the GUI
			break;
		case STATE_WAITING_FOR_LOCAL_ACCEPT:
			//the user issued the command through the GUI and network connected to us
			break;

		case STATE_RESETTING:
			//handled automatically
			break;
		case STATE_DATA_TRANSFER_XOFF:
			//This can happen if an RNR packet is received
			if( m_LineState == LINE_STATE_XON )
			{
				ChangeState( LINE_STATE_XOFF );
			}
			break;

		case STATE_DATA_TRANSFER_XON:
			//link layer is in data transfer
			if( m_LineState < LINE_STATE_CONNECTED_LINK )
			{
				if( STATE_DATA_TRANSFER_XON == lgo_State( m_CIDPhysical ) )
				{
					//physical and link layers are connected
					ChangeState( LINE_STATE_CONNECTED_LINK );
				}
				else
				{
					//physical layer is not connected
					//disconnect link layer
					DisconnectLayer( CIDTYPE_LINK );
					//restart connection process
					if( m_LineState > LINE_STATE_CONNECTING_PHYSICAL )
						ChangeState( LINE_STATE_CONNECTING_PHYSICAL );
				}
			}
			break;
	}

	//get link layer EVENT
	m_iCount	= m_iMaxBufferSize;
	m_PVCEvent	= lgo_Event( m_CIDLink, m_Buffer, &m_iCount );
	ReportEvent( m_PVCEvent, CIDTYPE_LINK );
	if( m_PVCEvent >= 0 )
	{
		switch( m_PVCEvent )
		{
			case EVENT_CONNECT_ACCEPT:
				//remote accepted our connect request
				ChangeState( LINE_STATE_CONNECTED_LINK );
				break;
			case EVENT_DISCONNECT_CONFIRMATION:
				//we disconnected from remote, confirmation
			case EVENT_DISCONNECTED:
				//remote disconnected from us
				break;
			case EVENT_CONNECT_REQUEST:
				//we received a connect request by the remote, this is usually erroneous
				//since this line is in connect mode
				break;
			case EVENT_CONNECTED:
				break;
			case EVENT_CONNECT_TIMEOUT:
				//it will be retried, very soon...
				break;
			case EVENT_CONNECT_REFUSE:
				//it will be retried, very soon...
				break;
			case EVENT_DISCONNECT_REQUEST:
				//remote requested a disconnect
				break;
			case EVENT_DISCONNECTING:
				break;
			case EVENT_DISCONNECT_TIMEOUT:
				break;
			case EVENT_RESETTING:
				break;
			case EVENT_RESET_COMPLETED:
				break;
			case EVENT_RESET_TIMEOUT:
				break;
			case EVENT_XON_REQUEST:
				break;
			case EVENT_XOFF_REQUEST:
				break;
			case EVENT_PROTOCOL_MESSAGE:
			case EVENT_DATA_LENGTH_ERROR:
			case EVENT_DATA_CRC_ERROR:
			case EVENT_DATA_ABORTED_FRAME:
			case EVENT_DATA_SPECIAL:
			case EVENT_DATA_OK:
				//this is handled by the top layer
				break;
			default:
				break;
		}
	}

	//get packet layer STATE
	m_StateCode = lgo_State( m_CIDPacket );
	ReportState( m_StateCode, CIDTYPE_PACKET );
	switch( m_StateCode )
	{
		case STATE_OPEN:
			//if line is supposed to be higher than connected to physical layer state
			if( m_LineState >= LINE_STATE_CONNECTED_LINK )
			{
				//obviously we are not,
				if( STATE_DATA_TRANSFER_XON == lgo_State( m_CIDPhysical ) )
				{
					//at least physical layer is connected
					if( STATE_DATA_TRANSFER_XON == lgo_State( m_CIDLink ) )
					{
						ChangeState( LINE_STATE_CONNECTED_LINK );
					}
					else
					{
						//link layer is not connected
						ChangeState( LINE_STATE_CONNECTED_PHYSICAL );
					}
				}
				else
				{
					//we are not even connected to the physical layer, darn it
					if( m_LineState > LINE_STATE_OPEN )
						ChangeState( LINE_STATE_OPEN );
				}
			}
			//else if it's ok for this layer to be open
			break;
		case STATE_WAITING_FOR_REMOTE_ACCEPT:
			//we have issued a connect request and are waiting...
			break;

		case STATE_WAITING_FOR_LOCAL_CONFIRMATION:
			//handled automatically
			break;
		case STATE_WAITING_FOR_REMOTE_CONFIRMATION:
			//waiting for a disconnect, reset, Xon or Xoff
			break;

		case STATE_LISTENING:
			//this is obviously erroneous, however, this could happen when debugging
			//the user can issue this command through the GUI
			break;
		case STATE_WAITING_FOR_LOCAL_ACCEPT:
			//the user issued the command through the GUI and network connected to us
			break;

		case STATE_RESETTING:
			//handled automatically
			break;
		case STATE_DATA_TRANSFER_XOFF:
			if( m_LineState == LINE_STATE_XON )
			{
				ChangeState( LINE_STATE_XOFF );
			}
			break;

		case STATE_DATA_TRANSFER_XON:
			//packet layer is in data transfer state
			if( m_LineState < LINE_STATE_CONNECTED_PACKET )
			{
				if( STATE_DATA_TRANSFER_XON == lgo_State( m_CIDPhysical ) )
				{
					//physical layer is connected

					if( STATE_DATA_TRANSFER_XON == lgo_State( m_CIDLink ) )
					{
						//link layer is connected
						ChangeState( LINE_STATE_CONNECTED_PACKET );
					}
					else
					{
						//link layer is not connected
						//what are the odds of these, almost zero
						//disconnect packet layer
						DisconnectLayer( CIDTYPE_PACKET );
						if( m_LineState > LINE_STATE_CONNECTING_LINK )
							ChangeState( LINE_STATE_CONNECTING_LINK );
					}
				}
				else
				{
					//physical layer is not connected
					DisconnectLayer( CIDTYPE_PACKET );
					DisconnectLayer( CIDTYPE_LINK );
					//restart connection process
					if( m_LineState > LINE_STATE_CONNECTING_PHYSICAL )
						ChangeState( LINE_STATE_CONNECTING_PHYSICAL );
				}
			}
			if( (m_LineState != LINE_STATE_XON) && (true == AllLayersXOn()) )
			{
				ChangeState( LINE_STATE_XON );
			}
			break;
	}

	//get packet layer EVENT
	m_iCount	= m_iMaxBufferSize;
	m_PVCEvent	= lgo_Event( m_CIDPacket, m_Buffer, &m_iCount );
	ReportEvent( m_PVCEvent, CIDTYPE_PACKET );
	if( m_PVCEvent >= 0 )
	{
		switch( m_PVCEvent )
		{
			case EVENT_CONNECT_ACCEPT:
				//remote accepted our connect request
				ChangeState( LINE_STATE_CONNECTED_PACKET );
				break;
			case EVENT_DISCONNECT_CONFIRMATION:
				//we disconnected from remote, confirmation
			case EVENT_DISCONNECTED:
				//remote disconnected from us
				break;
			case EVENT_CONNECT_REQUEST:
				//we received a connect request by the remote, this is usually erroneous
				//since this line is in connect mode
				break;
			case EVENT_CONNECTED:
				break;
			case EVENT_CONNECT_TIMEOUT:
				//it will be retried, very soon...
				break;
			case EVENT_CONNECT_REFUSE:
				//it will be retried, very soon...
				break;
			case EVENT_DISCONNECT_REQUEST:
				//remote requested a disconnect
				break;
			case EVENT_DISCONNECTING:
				break;
			case EVENT_DISCONNECT_TIMEOUT:
				break;
			case EVENT_RESETTING:
				break;
			case EVENT_RESET_COMPLETED:
				break;
			case EVENT_RESET_TIMEOUT:
				break;
			case EVENT_XON_REQUEST:
				break;
			case EVENT_XOFF_REQUEST:
				break;
			case EVENT_PROTOCOL_MESSAGE:
			case EVENT_DATA_LENGTH_ERROR:
			case EVENT_DATA_CRC_ERROR:
			case EVENT_DATA_ABORTED_FRAME:
			case EVENT_DATA_SPECIAL:
			case EVENT_DATA_OK:
				//this is handled by the top layer
				break;
			default:
				break;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
//
//	LISTEN
//
////////////////////////////////////////////////////////////////////////////////////////////
void CX25Line::ProcessLineListen()
{
	//issue commands based on our current line state, line state is NOT layer state
	switch( m_LineState )
	{
		case LINE_STATE_OPEN:
			//listen for connections
			ListenAllLayers();
			break;
		case LINE_STATE_CONNECTING_PHYSICAL:
			//transitory state, waiting for connect accept event 
			//which will change state to connected
			break;
		case LINE_STATE_CONNECTED_PHYSICAL:
			break;
		case LINE_STATE_CONNECTING_LINK:
			//transitory state, waiting for connect accept event 
			//which will change state to connected
			break;
		case LINE_STATE_CONNECTED_LINK:
			break;
		case LINE_STATE_CONNECTING_PACKET:
			//transitory state, waiting for connect accept event 
			//which will change state to connected
			break;
		case LINE_STATE_CONNECTED_PACKET:
			//we are now ready to for the pvcs
			if( true == AllLayersXOn() )
			{
				ChangeState( LINE_STATE_XON );
			}
			break;
		case LINE_STATE_XON:
			//this is the desired state for pvc operation
			break;
	}

	//get physical layer STATE
	m_StateCode = lgo_State( m_CIDPhysical );
	ReportState( m_StateCode, CIDTYPE_PHYSICAL );
	switch( m_StateCode )
	{
		case STATE_OPEN:
			//if this layer is open...
			if( m_LineState > LINE_STATE_OPEN )
			{
				ChangeState( LINE_STATE_OPEN );
			}
			break;
		case STATE_WAITING_FOR_REMOTE_ACCEPT:
			//we have issued a connect request and are waiting...
			break;

		case STATE_WAITING_FOR_LOCAL_CONFIRMATION:
			//handled automatically
			break;
		case STATE_WAITING_FOR_REMOTE_CONFIRMATION:
			//waiting for a disconnect, reset, Xon or Xoff
			break;

		case STATE_LISTENING:
			break;
		case STATE_WAITING_FOR_LOCAL_ACCEPT:
			break;

		case STATE_RESETTING:
			//handled automatically
			break;

		case STATE_DATA_TRANSFER_XOFF:
			if( m_LineState == LINE_STATE_XON )
			{
				ChangeState( LINE_STATE_XOFF );
			}
			break;

		case STATE_DATA_TRANSFER_XON:
			//if line is supposed to be lower than connected physical...
			if( m_LineState < LINE_STATE_CONNECTED_PHYSICAL )
			{
				//we can now attempt to connect the link layer
				ChangeState( LINE_STATE_CONNECTED_PHYSICAL );
			}
			break;
	}

	//get physical layer EVENT
	m_iCount	= m_iMaxBufferSize;
	m_PVCEvent	= lgo_Event( m_CIDPhysical, m_Buffer, &m_iCount );
	ReportEvent( m_PVCEvent, CIDTYPE_PHYSICAL );
	if( m_PVCEvent >= 0 )
	{
		//note: changing line state to open will cause a disconnection of the other lines
		switch( m_PVCEvent )
		{
			case EVENT_CONNECT_ACCEPT:
				//remote accepted our connect request
				ChangeState( LINE_STATE_CONNECTED_PHYSICAL );
				break;
			case EVENT_DISCONNECT_CONFIRMATION:
				//we disconnected from remote, confirmation
			case EVENT_DISCONNECTED:
				//remote disconnected from us
				//disconnect other layers
				DisconnectLayer( CIDTYPE_LINK );
				DisconnectLayer( CIDTYPE_PACKET );
				if( true == AllLayersOpen() )
					ChangeState( LINE_STATE_OPEN );
				break;
			case EVENT_CONNECT_REQUEST:
				//we received a connect request by the remote, this is usually erroneous
				//since this line is in connect mode
				break;
			case EVENT_CONNECTED:
				break;
			case EVENT_CONNECT_TIMEOUT:
				//it will be retried, very soon...
				break;
			case EVENT_CONNECT_REFUSE:
				//it will be retried, very soon...
				break;
			case EVENT_DISCONNECT_REQUEST:
				//remote requested a disconnect
				break;
			case EVENT_DISCONNECTING:
				break;
			case EVENT_DISCONNECT_TIMEOUT:
				break;
			case EVENT_RESETTING:
				break;
			case EVENT_RESET_COMPLETED:
				break;
			case EVENT_RESET_TIMEOUT:
				break;
			case EVENT_XON_REQUEST:
				break;
			case EVENT_XOFF_REQUEST:
				break;
			case EVENT_PROTOCOL_MESSAGE:
			case EVENT_DATA_LENGTH_ERROR:
			case EVENT_DATA_CRC_ERROR:
			case EVENT_DATA_ABORTED_FRAME:
			case EVENT_DATA_SPECIAL:
			case EVENT_DATA_OK:
				//this is handled by the top layer
				break;
			default:
				break;
		}
	}

	//get link layer STATE
	m_StateCode = lgo_State( m_CIDLink );
	ReportState( m_StateCode, CIDTYPE_LINK );
	switch( m_StateCode )
	{
		case STATE_OPEN:
			//if this layer is open...
			break;
		case STATE_WAITING_FOR_REMOTE_ACCEPT:
			//we have issued a connect request and are waiting...
			break;

		case STATE_WAITING_FOR_LOCAL_CONFIRMATION:
			//handled automatically
			break;
		case STATE_WAITING_FOR_REMOTE_CONFIRMATION:
			//waiting for a disconnect, reset, Xon or Xoff
			break;

		case STATE_LISTENING:
			break;
		case STATE_WAITING_FOR_LOCAL_ACCEPT:
			break;

		case STATE_RESETTING:
			//handled automatically
			break;
		case STATE_DATA_TRANSFER_XOFF:
			if( m_LineState == LINE_STATE_XON )
			{
				ChangeState( LINE_STATE_XOFF );
			}
			break;

		case STATE_DATA_TRANSFER_XON:
			//link layer is in data transfer
			if( m_LineState < LINE_STATE_CONNECTED_LINK )
			{
				if( STATE_DATA_TRANSFER_XON == lgo_State( m_CIDPhysical ) )
				{
					//physical and link layers are connected
					ChangeState( LINE_STATE_CONNECTED_LINK );
				}
				else
				{
					//physical layer is not connected
					DisconnectAllLayers();
				}
			}
			break;
	}

	//get link layer EVENT
	m_iCount	= m_iMaxBufferSize;
	m_PVCEvent	= lgo_Event( m_CIDLink, m_Buffer, &m_iCount );
	ReportEvent( m_PVCEvent, CIDTYPE_LINK );
	if( m_PVCEvent >= 0 )
	{
		switch( m_PVCEvent )
		{
			case EVENT_CONNECT_ACCEPT:
				//remote accepted our connect request
				ChangeState( LINE_STATE_CONNECTED_LINK );
				break;
			case EVENT_DISCONNECT_CONFIRMATION:
				//we disconnected from remote, confirmation
			case EVENT_DISCONNECTED:
				//remote disconnected from us
				//disconnect other layers
				DisconnectLayer( CIDTYPE_PACKET );
				DisconnectLayer( CIDTYPE_PHYSICAL );
				if( true == AllLayersOpen() )
					ChangeState( LINE_STATE_OPEN );
				break;
			case EVENT_CONNECT_REQUEST:
				//we received a connect request by the remote, this is usually erroneous
				//since this line is in connect mode
				break;
			case EVENT_CONNECTED:
				break;
			case EVENT_CONNECT_TIMEOUT:
				//it will be retried, very soon...
				break;
			case EVENT_CONNECT_REFUSE:
				//it will be retried, very soon...
				break;
			case EVENT_DISCONNECT_REQUEST:
				//remote requested a disconnect
				break;
			case EVENT_DISCONNECTING:
				break;
			case EVENT_DISCONNECT_TIMEOUT:
				break;
			case EVENT_RESETTING:
				break;
			case EVENT_RESET_COMPLETED:
				break;
			case EVENT_RESET_TIMEOUT:
				break;
			case EVENT_XON_REQUEST:
				break;
			case EVENT_XOFF_REQUEST:
				break;
			case EVENT_PROTOCOL_MESSAGE:
			case EVENT_DATA_LENGTH_ERROR:
			case EVENT_DATA_CRC_ERROR:
			case EVENT_DATA_ABORTED_FRAME:
			case EVENT_DATA_SPECIAL:
			case EVENT_DATA_OK:
				//this is handled by the top layer
				break;
			default:
				break;
		}
	}

	//get packet layer STATE
	m_StateCode = lgo_State( m_CIDPacket );
	ReportState( m_StateCode, CIDTYPE_PACKET );
	switch( m_StateCode )
	{
		case STATE_OPEN:
			//if this layer is open...
			break;
		case STATE_WAITING_FOR_REMOTE_ACCEPT:
			//we have issued a connect request and are waiting...
			break;

		case STATE_WAITING_FOR_LOCAL_CONFIRMATION:
			//handled automatically
			break;
		case STATE_WAITING_FOR_REMOTE_CONFIRMATION:
			//waiting for a disconnect, reset, Xon or Xoff
			break;

		case STATE_LISTENING:
			break;
		case STATE_WAITING_FOR_LOCAL_ACCEPT:
			break;

		case STATE_RESETTING:
			//handled automatically
			break;
		case STATE_DATA_TRANSFER_XOFF:
			if( m_LineState == LINE_STATE_XON )
			{
				ChangeState( LINE_STATE_XOFF );
			}
			break;

		case STATE_DATA_TRANSFER_XON:
			//packet layer is in data transfer state
			if( m_LineState < LINE_STATE_CONNECTED_PACKET )
			{
				if( STATE_DATA_TRANSFER_XON == lgo_State( m_CIDPhysical ) )
				{
					//physical layer is connected

					if( STATE_DATA_TRANSFER_XON == lgo_State( m_CIDLink ) )
					{
						//link layer is connected
						ChangeState( LINE_STATE_CONNECTED_PACKET );
					}
					else
					{
						//link layer is not connected
						//what are the odds of these, almost zero
						DisconnectAllLayers();
					}
				}
				else
				{
					//physical layer is not connected
					DisconnectAllLayers();
				}
			}
			if( (m_LineState != LINE_STATE_XON) && (true == AllLayersXOn()) )
			{
				ChangeState( LINE_STATE_XON );
			}
			break;
	}

	//get packet layer EVENT
	m_iCount	= m_iMaxBufferSize;
	m_PVCEvent	= lgo_Event( m_CIDPacket, m_Buffer, &m_iCount );
	ReportEvent( m_PVCEvent, CIDTYPE_PACKET );
	if( m_PVCEvent >= 0 )
	{
		switch( m_PVCEvent )
		{
			case EVENT_CONNECT_ACCEPT:
				//remote accepted our connect request
				ChangeState( LINE_STATE_CONNECTED_PACKET );
				break;
			case EVENT_DISCONNECT_CONFIRMATION:
				//we disconnected from remote, confirmation
			case EVENT_DISCONNECTED:
				//remote disconnected from us
				//disconnect other layers
				DisconnectLayer( CIDTYPE_LINK );
				DisconnectLayer( CIDTYPE_PHYSICAL );
				if( true == AllLayersOpen() )
					ChangeState( LINE_STATE_OPEN );
				break;
			case EVENT_CONNECT_REQUEST:
				//we received a connect request by the remote, this is usually erroneous
				//since this line is in connect mode
				break;
			case EVENT_CONNECTED:
				break;
			case EVENT_CONNECT_TIMEOUT:
				//it will be retried, very soon...
				break;
			case EVENT_CONNECT_REFUSE:
				//it will be retried, very soon...
				break;
			case EVENT_DISCONNECT_REQUEST:
				//remote requested a disconnect
				break;
			case EVENT_DISCONNECTING:
				break;
			case EVENT_DISCONNECT_TIMEOUT:
				break;
			case EVENT_RESETTING:
				break;
			case EVENT_RESET_COMPLETED:
				break;
			case EVENT_RESET_TIMEOUT:
				break;
			case EVENT_XON_REQUEST:
				break;
			case EVENT_XOFF_REQUEST:
				break;
			case EVENT_PROTOCOL_MESSAGE:
			case EVENT_DATA_LENGTH_ERROR:
			case EVENT_DATA_CRC_ERROR:
			case EVENT_DATA_ABORTED_FRAME:
			case EVENT_DATA_SPECIAL:
			case EVENT_DATA_OK:
				//this is handled by the top layer
				break;
			default:
				break;
		}
	}
}
	
bool CX25Line::IsLineUp()
{
	switch( m_LineState )
	{
		case LINE_STATE_XON:
			if( AllLayersXOn() )
				return true;
			break;
		case LINE_STATE_OPEN:
		case LINE_STATE_CONNECTING_PHYSICAL:
		case LINE_STATE_CONNECTED_PHYSICAL:
		case LINE_STATE_CONNECTING_LINK:
		case LINE_STATE_CONNECTED_LINK:
		case LINE_STATE_CONNECTING_PACKET:
		case LINE_STATE_CONNECTED_PACKET:
		case LINE_STATE_LISTENING:
		default:
			break;
	}
	return false;
}

void CX25Line::ChangeState( ModuleLineStates state )
{
	m_LineState = state;
	OnStateChanged();
}

void CX25Line::OnStateChanged()
{
	m_szString.Format( "[Line State Changed] %s", GetLineState() );
	ReportMessage( m_szString );
}

void CX25Line::ReportError( LPCTSTR pStr, Result Error, CIDType iCIDType, bool bDisplayIfNoError )
{
	if( Error < 0 || bDisplayIfNoError == true )
	{
		if( Error >= 0 )
		{
			OWS_3trace( (*m_pCTrace), LEVEL_INFO, "#%i [Info] %s : %s", m_iLineNumber+1, GetLayerType(iCIDType), pStr );
		}
		else if( Error != ERROR_TRANSMISSION_BLOCKED )
		{
			OWS_4trace( (*m_pCTrace), LEVEL_ERROR, "#%i [ERROR] %s : %s : %s", m_iLineNumber+1, GetLayerType(iCIDType), pStr, lgo_ErrorMessage(Error) );
		}
	}
}

void CX25Line::ReportState( StateCode StateValue, CIDType iCIDType )
{
	if( StateValue < 0 )
	{
		if( StateValue != ERROR_TRANSMISSION_BLOCKED )
		{
			OWS_3trace( (*m_pCTrace), LEVEL_ERROR, "#%i [ERROR] %s : %s", m_iLineNumber+1, GetLayerType(iCIDType), lgo_ErrorMessage(StateValue) );
		}
	}
	else
	{
		switch( StateValue )
		{
			case STATE_OPEN:
			case STATE_WAITING_FOR_REMOTE_ACCEPT:
			case STATE_DATA_TRANSFER_XON:
			case STATE_DATA_TRANSFER_XOFF:
			case STATE_LISTENING:
				break;
			default:
				OWS_3trace( (*m_pCTrace), LEVEL_EVENTSANDPOLLS, "#%i [Layer State] %s : %s", m_iLineNumber+1, GetLayerType(iCIDType), lgo_StateMessage(StateValue) );
				break;
		}
	}
}

void CX25Line::ReportEvent( EventCode EventValue, CIDType iCIDType )
{
	if( EventValue < 0 )
	{
		if( EventValue != ERROR_TRANSMISSION_BLOCKED &&
			EventValue != ERROR_NOTHING_WAITING )
		{
			OWS_3trace( (*m_pCTrace), LEVEL_ERROR, "#%i [ERROR] %s : %s", m_iLineNumber+1, GetLayerType(iCIDType), lgo_ErrorMessage(EventValue) );
		}
	}
	else
	{
		switch( EventValue )
		{
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
				OWS_3trace( (*m_pCTrace), LEVEL_EVENTSANDPOLLS, "#%i [Network Event] %s : %s", m_iLineNumber+1, GetLayerType(iCIDType), lgo_EventMessage(EventValue) );
				break;
		}
	}
}

void CX25Line::ReportMessage( LPCTSTR pStr )
{
	OWS_2trace( (*m_pCTrace), LEVEL_INFO, "#%i %s", m_iLineNumber+1, pStr );
}

LPCTSTR CX25Line::GetLineState()
{
	for( int i=0; i<LINE_STATE_MAX; i++ ) 
	{
		if( sm_ModuleLineStatesTable[i].state == m_LineState ) 
		{
			return sm_ModuleLineStatesTable[i].szText;
		}
	}
	return sm_ModuleLineStatesTable[0].szText;
}

LPCTSTR CX25Line::GetLayerType( CIDType iCIDType )
{
	for( int i=0; i<CIDTYPE_MAX; i++ ) 
	{
		if( sm_CIDTypesTable[i].type == iCIDType ) 
		{
			return sm_CIDTypesTable[i].szText;
		}
	}
	return sm_CIDTypesTable[0].szText;
}

CX25Line::ModuleLineStatesStruct CX25Line::sm_ModuleLineStatesTable[] = {
	{ LINE_STATE_CLOSED,				"Closed" },
	{ LINE_STATE_OPEN,					"Open" },
	{ LINE_STATE_LISTENING,				"Listening..." },
	{ LINE_STATE_CONNECTING_PHYSICAL,	"Physical Layer : Connecting..." },
	{ LINE_STATE_CONNECTED_PHYSICAL,	"Physical Layer : Connected" },
	{ LINE_STATE_CONNECTING_LINK,		"Link Layer : Connecting..." },
	{ LINE_STATE_CONNECTED_LINK,		"Link Layer : Connected" },
	{ LINE_STATE_CONNECTING_PACKET,		"Packet Layer : Connecting..." },
	{ LINE_STATE_CONNECTED_PACKET,		"Packet Layer : Connected" },
	{ LINE_STATE_XOFF,					"XOff" },
	{ LINE_STATE_XON,					"XOn" }
};

CX25Line::CIDTypesStruct CX25Line::sm_CIDTypesTable[] = {
	{ CIDTYPE_PVC,			"PVC" },
	{ CIDTYPE_PACKET,		"Packet Layer" },
	{ CIDTYPE_LINK,			"Link Layer" },
	{ CIDTYPE_PHYSICAL,		"Physical Layer" }
};

