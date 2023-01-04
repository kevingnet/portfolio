#include "stdafx.h"
#include "ES4000Packet.h"
#include "PacketRouter.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CES4000Packet CES4000Packet - PUBLIC
// PURPOSE: Constructor
// CALLED BY: System
/////////////////////////////////////////////////////////////////////////////
CES4000Packet::CES4000Packet()
{
	if (!this) return;
	m_bIsValid = false;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CES4000Packet ~CES4000Packet - PUBLIC
// PURPOSE: Destructor
// CALLED BY: System
/////////////////////////////////////////////////////////////////////////////
CES4000Packet::~CES4000Packet()
{
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CES4000Packet Initialize - PUBLIC
// PURPOSE: Initializer
// CALLED BY: User
/////////////////////////////////////////////////////////////////////////////
bool CES4000Packet::Initialize( PBYTE	pData, 
							    int		iCount,
							    int		iSourceType,
							    int		iPacketType )
{
	if (!this) return false;

	m_bIsValid					= false;
	m_iCommand					= INVALID_COMMAND;
	m_iHeaderCount				= INIT_HEADER_COUNT;   
	m_iSmallHeaderCount			= INIT_HEADER_COUNT;   

	m_pData						= pData;
	m_iCount					= iCount;   
	m_iSourceType				= iSourceType;
	m_PacketType				= iPacketType;

	if ( !m_pData ) return false;
	if ( m_iCount <= PACKET_DATA_LOCATION )
		return false;

	//check for ES4000 packet validity
	if ( m_pData[0]						!= PACKET_START			||
		 !(isdigit( (int)m_pData[1] )) ||
		 !(isdigit( (int)m_pData[2] )) ||
		 !(isdigit( (int)m_pData[3] )) ||
		 !(isdigit( (int)m_pData[4] )) ||
		 m_pData[PACKET_DATA_LOCATION-1]!= PACKET_DATA_START	||
		 m_pData[m_iCount-1]			!= PACKET_END
	   )
	{
		return false;
	}

	//convert command string to int command ID
	m_i = m_pData[5];
	m_pData[5] = NULL_TERMINATOR;
	m_iCommand = atoi(LPCTSTR(m_pData+1));
	m_pData[5] = m_i;
	if ( m_iCommand < 0 || m_iCommand > MAX_VALID_COMMAND ) return false;

	m_iDataCount = m_iCount-PACKET_DATA_LOCATION;

	if ( m_pData[m_iHeaderCount+1] == NULL_TERMINATOR )
	{	//we did not get source in header
		m_iSmallHeaderCount++;
		m_iHeaderCount++;
	}
	else
	{	//got the source from the packet header...count it...
		for ( m_i=m_iHeaderCount; m_i<MAX_HEADER+m_iHeaderCount; m_i++ )
		{
			if ( m_pData[m_i] == NULL_TERMINATOR )
			{
				m_iSmallHeaderCount = m_i+1;
				m_iHeaderCount = m_iSmallHeaderCount;
				break;
			}
		}
	}
	if ( m_pData[m_iHeaderCount] == NULL_TERMINATOR )
	{	//we did not get a destination...count the NULL
		m_iHeaderCount++;
	}
	else
	{	//got the destination from the packet header...count it...
		for ( m_i=m_iHeaderCount; m_i<MAX_HEADER; m_i++ )
		{
			if ( m_pData[m_i] == NULL_TERMINATOR )
			{
				m_iHeaderCount = m_i+1;
				break;
			}
		}
	}

	m_pData[m_iHeaderCount++] = PACKET_DATA_START;

	m_pTemp = m_pData+PACKET_DATA_LOCATION-m_iHeaderCount;
	for ( m_i=0; m_i<m_iHeaderCount; m_i++ )
	{
		m_pTemp[m_i] = m_pData[m_i];
	}

	if ( m_iHeaderCount > MAX_HEADER || m_iHeaderCount < MIN_HEADER ) 
		return false;

	m_bIsValid = true;
	return true;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CES4000Packet SetDestination - PUBLIC
// PURPOSE: 
// CALLED BY: User
/////////////////////////////////////////////////////////////////////////////
bool CES4000Packet::SetDestination( LPCTSTR	szDestination, int iType )
{
	if (!this) return false;
	if (!szDestination) return false;
	if (!*szDestination) return false;

	if ( !szDestination )
	{
		m_pData[m_iHeaderCount] = 0;
		m_iDestinationType = iType;
		m_iDestinationProxyCount = 0;
	}
	else
	{
		//copy the destination to the first header
		m_iDestinationProxyCount = 0;
		for ( m_i=m_iSmallHeaderCount; m_i<MAX_HEADER; m_i++ )
		{
			m_pData[m_i] = szDestination[m_i-m_iSmallHeaderCount];
			if ( m_pData[m_i] == ':' && m_iDestinationProxyCount == 0 )
				m_iDestinationProxyCount = m_i-m_iSmallHeaderCount+1;
			if ( m_pData[m_i] == NULL_TERMINATOR )
			{
				m_iHeaderCount = m_i;
				m_iDestinationType = iType;
				break;
			}
		}
		m_pData[m_iHeaderCount++] = NULL_TERMINATOR;
	}
	if ( m_iHeaderCount > MAX_HEADER ) return false;
	//copy the header just before the packet data
	m_pTemp = m_pData+PACKET_DATA_LOCATION-m_iHeaderCount-1;
	for ( m_i=0; m_i<m_iHeaderCount; m_i++ )
	{
		m_pTemp[m_i] = m_pData[m_i];
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CES4000Packet SetSource - PUBLIC
// PURPOSE: 
// CALLED BY: User
/////////////////////////////////////////////////////////////////////////////
bool CES4000Packet::SetSource( LPCTSTR	szSource )
{
	if (!this) return false;
	if (!szSource) return false;
	if (!*szSource) return false;
	
	//save destination
	CString szDestination = GetDestination();
	if ( szDestination.GetLength() >= MAX_NAME ) return false;

	//copy source
	for ( m_i=PACKET_SOURCE_LOCATION; m_i<MAX_HEADER; m_i++ )
	{
		m_pData[m_i] = szSource[m_i-PACKET_SOURCE_LOCATION];
		if ( m_pData[m_i] == NULL_TERMINATOR )
		{
			m_iSmallHeaderCount = m_i;
			break;
		}
	}
	m_pData[m_iSmallHeaderCount++] = NULL_TERMINATOR;
	if ( szDestination == "" )
	{
		m_iHeaderCount = m_iSmallHeaderCount;
		m_pData[m_iHeaderCount++] = NULL_TERMINATOR;
	}
	else
	{
		//copy destination
		m_iHeaderCount = m_iSmallHeaderCount+szDestination.GetLength();
		for ( m_i=m_iSmallHeaderCount; m_i<m_iHeaderCount; m_i++ )
		{
			m_pData[m_i] = szDestination[m_i-m_iSmallHeaderCount];
			if ( m_pData[m_i] == NULL_TERMINATOR )
			{
				break;
			}
		}
		m_pData[m_iHeaderCount++] = NULL_TERMINATOR;
	}
	if ( m_iHeaderCount >= MAX_HEADER ) return false;
	//copy the header just before the packet data
	m_pTemp = m_pData+PACKET_DATA_LOCATION-m_iHeaderCount-1;
	for ( m_i=0; m_i<m_iHeaderCount; m_i++ )
	{
		m_pTemp[m_i] = m_pData[m_i];
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////
