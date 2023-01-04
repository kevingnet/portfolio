#include "stdafx.h"
#include "smtp.h"

#define EOL  "\r\n"



//////////////// Implementation //////////////////////////////////////
CSMTPSocket :: CSMTPSocket ()
{
	m_hSocket = INVALID_SOCKET ; //default to an invalid scoket descriptor

	//DEBUG
	hFile = CreateFile("C:\\Socket.con",           /* create MYFILE.TXT  */
				 GENERIC_WRITE,                /* open for writing   */
				 0,                            /* do not share       */
				 (LPSECURITY_ATTRIBUTES) NULL, /* no security        */
				 CREATE_ALWAYS,                /* overwrite existing */
				 FILE_ATTRIBUTE_NORMAL,        /* normal file        */
				 (HANDLE) NULL);               /* no attr. template  */
	WriteFile(hFile, "Windows Sockets Debug File\r\n\r\n  ", 30, &dwBytesWritten, NULL);

}

CSMTPSocket :: ~CSMTPSocket ()
{
	Close () ;

	//DEBUG
	CloseHandle(hFile);
	
}

BOOL CSMTPSocket :: Create ()
{
	char	szError[125];
	char	szTemp[20];
	int		iError;
	WORD	wVersionRequested ;
	WSADATA wsaData ;
	int		err ; 
	wVersionRequested = MAKEWORD( 2, 0 ) ; 

	err = WSAStartup( wVersionRequested, &wsaData ) ;
	if ( err != 0 )
	{
		
		//DEBUG
		strcpy(szError, "The Socked DLL is all fucked up\r\n");
		WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);

		return FALSE ;
	} 
	/* Confirm that the WinSock DLL supports 2.0.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.0 in addition to 2.0, it will still return */
	/* 2.0 in wVersion since that is the version we      */
	/* requested.                                        */ 
	if ( LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 0 ) 
	{
		/* Tell the user that we couldn't find a usable WinSock DLL*/    
		WSACleanup( ) ;

		//DEBUG
		strcpy(szError, "The Socked DLL is all fucked up\r\n");
		WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);
		
		return FALSE; 
	}

	strcpy(szError, "Socket was created successfully!\r\n");
	m_hSocket = socket ( AF_INET, SOCK_STREAM, 0 ) ;
	if ( m_hSocket == INVALID_SOCKET )
	{
		iError = WSAGetLastError () ;

		switch ( iError )
		{
		case WSANOTINITIALISED :
			strcpy(szError, "A successful WSAStartup must occur before using this function.\r\n");
			break;
		case WSAENETDOWN :
			strcpy(szError, "The network subsystem or the associated service provider has failed.\r\n");
			break;
		case WSAEAFNOSUPPORT :
			strcpy(szError, "The specified address family is not supported.\r\n");
			break;
		case WSAEINPROGRESS :
			strcpy(szError, "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.\r\n");
			break;
		case WSAEMFILE :
			strcpy(szError, "No more socket descriptors are available.\r\n");
			break;
		case WSAENOBUFS :
			strcpy(szError, "No buffer space is available. The socket cannot be created.\r\n");
			break;
		case WSAEPROTONOSUPPORT :
			strcpy(szError, "The specified protocol is not supported.\r\n");
			break;
		case WSAEPROTOTYPE :
			strcpy(szError, "The specified protocol is the wrong type for this socket.\r\n");
			break;
		case WSAESOCKTNOSUPPORT :
			strcpy(szError, "The specified socket type is not supported in this address family.\r\n");
			break;
		default:
			strcpy(szError, "A really fucked up error just happened!!!\r\n");
		}
		return FALSE ;
	}
	else
	{
		WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);

		strcpy(szError, "wVersion = ");
		strcat(szError, ltoa(wsaData.wVersion, szTemp, 10));
		strcat(szError, "\r\n");
		WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);

		strcpy(szError, "wHighVersion = ");
		strcat(szError, ltoa(wsaData.wHighVersion, szTemp, 10));
		strcat(szError, "\r\n");
		WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);

		strcpy(szError, "iMaxSockets = ");
		strcat(szError, itoa(wsaData.iMaxSockets, szTemp, 10));
		strcat(szError, "\r\n");
		WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);

		strcpy(szError, "Description : ");
		strcat(szError, wsaData.szDescription);
		strcat(szError, "\r\n");
		WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);

		strcpy(szError, "SystemStatus : ");
		strcat(szError, wsaData.szSystemStatus);
		strcat(szError, "\r\n");
		WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);

		strcpy(szError, "VendorInfo : ");
		strcat(szError, wsaData.lpVendorInfo);
		strcat(szError, "\r\n");
		WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);

		return TRUE ;
	}
}

BOOL CSMTPSocket :: Connect ( LPCTSTR pszHostAddress, int nPort )
{
	char	szError[125];
	//int		iError;

	USES_CONVERSION ;

	if ( m_hSocket == INVALID_SOCKET )
	{
		return FALSE ;
	}

	LPSTR lpszAscii = T2A ( (LPTSTR)pszHostAddress ) ;

	strcpy(szError, "Connecting the socket\r\n");
	WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);

	strcpy(szError, "HostAddress = ");
	WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);
	WriteFile(hFile, pszHostAddress, strlen(pszHostAddress), &dwBytesWritten, NULL);

	strcpy(szError, "Ascii HostAddress = ");
	WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);
	WriteFile(hFile, lpszAscii, strlen(lpszAscii), &dwBytesWritten, NULL);
	
	strcpy(szError, "\r\n\r\n");
	WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);

	//Determine if the address is in dotted notation
	SOCKADDR_IN sockAddr ;
	ZeroMemory ( & sockAddr, sizeof(sockAddr) ) ;
	sockAddr.sin_family = AF_INET ;
	sockAddr.sin_port = htons ( (u_short)nPort ) ;
	sockAddr.sin_addr.s_addr = inet_addr ( lpszAscii ) ;

	//If the address is not dotted notation, then do a DNS lookup of it.
	if ( sockAddr.sin_addr.s_addr == INADDR_NONE )
	{
		LPHOSTENT lphost ;
		lphost = gethostbyname ( lpszAscii ) ;
		if ( lphost != NULL )
		{
			sockAddr.sin_addr.s_addr = ( (LPIN_ADDR)lphost->h_addr)->s_addr ;
		}
		else
		{
			WSASetLastError ( WSAEINVAL ) ; 
			return FALSE ;
		}
	}

	//Call the protected version which takes an address 
	//in the form of a standard C style struct.
	return Connect ( (SOCKADDR*)&sockAddr, sizeof(sockAddr) ) ; 
}

BOOL CSMTPSocket :: Connect ( const SOCKADDR* lpSockAddr, int nSockAddrLen )
{
	char	szError[255];
	int		iError;
	
	iError = connect ( m_hSocket, lpSockAddr, nSockAddrLen ) ;
	if ( iError == 0 )
	{
		return TRUE ;
	}
	else
	{
		iError = WSAGetLastError () ;

		switch ( iError )
		{
		case WSANOTINITIALISED :
			strcpy(szError, "A successful WSAStartup must occur before using this function. \r\n");
			break;
		case WSAENETDOWN :
			strcpy(szError, "The network subsystem has failed. \r\n");
			break;
		case WSAEADDRINUSE :
			strcpy(szError, "The specified address is already in use. \r\n");
			break;
		case WSAEINTR :
			strcpy(szError, "The (blocking) call was canceled through WSACancelBlockingCall. \r\n");
			break;
		case WSAEINPROGRESS :
			strcpy(szError, "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. \r\n");
			break;
		case WSAEALREADY :
			strcpy(szError, "A nonblocking connect call is in progress on the specified socket. \r\n");
			break;
		case WSAEADDRNOTAVAIL :
			strcpy(szError, "The specified address is not available from the local machine. \r\n");
			break;
		case WSAEAFNOSUPPORT :
			strcpy(szError, "Addresses in the specified family cannot be used with this socket. \r\n");
			break;
		case WSAECONNREFUSED :
			strcpy(szError, "The attempt to connect was forcefully rejected. \r\n");
			break;
		case WSAEFAULT :
			strcpy(szError, "The name or the namelen parameter is not valid , is too small, or incorrect \r\n");
			break;
		case WSAEINVAL :
			strcpy(szError, "The parameter s is a listening socket, or the destination address specified is not consistent with that of the constrained group the socket belongs to. \r\n");
			break;
		case WSAEISCONN :
			strcpy(szError, "The socket is already connected (connection-oriented sockets only). \r\n");
			break;
		case WSAENETUNREACH :
			strcpy(szError, "The network cannot be reached from this host at this time. \r\n");
			break;
		case WSAENOBUFS :
			strcpy(szError, "No buffer space is available. The socket cannot be connected. \r\n");
			break;
		case WSAENOTSOCK :
			strcpy(szError, "The descriptor is not a socket. \r\n");
			break;
		case WSAETIMEDOUT :
			strcpy(szError, "Attempt to connect timed out without establishing a connection. \r\n");
			break;
		case WSAEWOULDBLOCK :
			strcpy(szError, "The socket is marked as nonblocking and the connection cannot be completed immediately. \r\n");
			break;
		case WSAEACCES :
			strcpy(szError, "Attempt to connect datagram socket to broadcast address failed because setsockopt option SO_BROADCAST is not enabled. \r\n");
			break;
		default:
			strcpy(szError, "A really fucked up error just happened!!!\r\n");
		}
		WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);
		return FALSE ;
	}
}

BOOL CSMTPSocket :: Send ( LPCSTR pszBuf, int nBuf )
{
	if ( m_hSocket == INVALID_SOCKET )
	{
		return FALSE ;
	}

	char	szError[255];
	char	szTemp[25];
	int		iError;

	strcpy(szError, "Sending Data to Server\r\n");
	WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);

	//DEBUG
	strcpy(szError, "------------------------------------------------------------------------------------\r\n");
	WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);
	WriteFile(hFile, pszBuf, nBuf, &dwBytesWritten, NULL);
	WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);

	iError = send ( m_hSocket, pszBuf, nBuf, 0 ) ;
	if ( iError != SOCKET_ERROR )
	{
		strcpy(szError, "Bytes sent = ");
		WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);
		itoa(iError, szTemp, 10);
		WriteFile(hFile, szTemp, strlen(szTemp), &dwBytesWritten, NULL);
		strcpy(szError, "\r\n");
		WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);

		strcpy(szError, "Sent Data to Server\r\n");
		WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);

		return TRUE ;
	}
	else
	{
		iError = WSAGetLastError () ;

		switch ( iError )
		{
		case WSANOTINITIALISED :
			strcpy(szError, "A successful WSAStartup must occur before using this function. \r\n");
			break;
		case WSAENETDOWN :
			strcpy(szError, "The network subsystem has failed. \r\n");
			break;
		case WSAEACCES :
			strcpy(szError, "The requested address is a broadcast address, but the appropriate flag was not set. \r\n");
			break;
		case WSAEINTR :
			strcpy(szError, "The (blocking) call was canceled through WSACancelBlockingCall. \r\n");
			break;
		case WSAEINPROGRESS :
			strcpy(szError, "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. \r\n");
			break;
		case WSAEFAULT :
			strcpy(szError, "The buf parameter is not totally contained in a valid part of the user address space. \r\n");
			break;
		case WSAENETRESET :
			strcpy(szError, "The connection has been broken due to the remote host resetting. \r\n");
			break;
		case WSAENOBUFS :
			strcpy(szError, "No buffer space is available. \r\n");
			break;
		case WSAENOTCONN :
			strcpy(szError, "The socket is not connected. \r\n");
			break;
		case WSAENOTSOCK :
			strcpy(szError, "The descriptor is not a socket. \r\n");
			break;
		case WSAEOPNOTSUPP :
			strcpy(szError, "MSG_OOB was specified, but the socket is not stream style such as type SOCK_STREAM\r\n");
			break;
		case WSAESHUTDOWN :
			strcpy(szError, "The socket has been shut down; it is not possible to send on a socket after shutdown has been invoked with how set to SD_SEND or SD_BOTH. \r\n");
			break;
		case WSAEWOULDBLOCK :
			strcpy(szError, "The socket is marked as nonblocking and the requested operation would block. \r\n");
			break;
		case WSAEMSGSIZE :
			strcpy(szError, "The socket is message oriented, and the message is larger than the maximum supported by the underlying transport. \r\n");
			break;
		case WSAEHOSTUNREACH :
			strcpy(szError, "The remote host cannot be reached from this host at this time. \r\n");
			break;
		case WSAEINVAL :
			strcpy(szError, "The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled. \r\n");
			break;
		case WSAECONNABORTED :
			strcpy(szError, "The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable. \r\n");
			break;
		case WSAECONNRESET :
			strcpy(szError, "The virtual circuit was reset by the remote side executing a -hard- or -abortive- close. \r\n");
			break;
		case WSAETIMEDOUT :
			strcpy(szError, "The connection has been dropped, because of a network failure or because the system on the other end went down without notice. \r\n");
			break;
		default:
			strcpy(szError, "A really fucked up error just happened!!!\r\n");
		}
		WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);
		
		strcpy(szError, "Sent Data to Server, ERRORS!!!\r\n");
		WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);

		return FALSE ;
	}
}

int CSMTPSocket :: Receive ( LPSTR pszBuf, int nBuf )
{
	if ( m_hSocket == INVALID_SOCKET )
	{
		return FALSE ;
	}

	int		iError;
	iError = recv ( m_hSocket, pszBuf, nBuf, 0 ) ; 
	if ( iError == 0 )
	{
		return FALSE ;
	}

	char	szError[255];
	char	szTemp[25];

	strcpy(szError, "Receiving Data from Server\r\n");
	WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);
	if ( iError != SOCKET_ERROR )
	{
		strcpy(szError, "Bytes received = ");
		WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);
		itoa(iError, szTemp, 10);
		WriteFile(hFile, szTemp, strlen(szTemp), &dwBytesWritten, NULL);
		strcpy(szError, "\r\n");
		WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);

		//DEBUG
		strcpy(szError, "------------------------------------------------------------------------------------\r\n");
		WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);
		WriteFile(hFile, pszBuf, nBuf, &dwBytesWritten, NULL);
		WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);

		strcpy(szError, "Received Data from Server\r\n");
		WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);

		return TRUE ;
	}
	else
	{
		iError = WSAGetLastError () ;

		switch ( iError )
		{
		case WSANOTINITIALISED :
			strcpy(szError, "A successful WSAStartup must occur before using this function. \r\n");
			break;
		case WSAENETDOWN :
			strcpy(szError, "The network subsystem has failed. \r\n");
			break;
		case WSAEFAULT :
			strcpy(szError, "The buf parameter is not totally contained in a valid part of the user address space. \r\n");
			break;
		case WSAENOTCONN :
			strcpy(szError, "The socket is not connected. \r\n");
			break;
		case WSAEINTR :
			strcpy(szError, "The (blocking) call was canceled through WSACancelBlockingCall. \r\n");
			break;
		case WSAEINPROGRESS :
			strcpy(szError, "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. \r\n");
			break;
		case WSAENETRESET :
			strcpy(szError, "The connection has been broken due to the remote host resetting. \r\n");
			break;
		case WSAENOTSOCK :
			strcpy(szError, "The descriptor is not a socket. \r\n");
			break;
		case WSAEOPNOTSUPP :
			strcpy(szError, "MSG_OOB was specified, but the socket is not stream style such as type SOCK_STREAM, out-of-band data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations. \r\n");
			break;
		case WSAESHUTDOWN :
			strcpy(szError, "The socket has been shut down; it is not possible to recv on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH. \r\n");
			break;
		case WSAEWOULDBLOCK :
			strcpy(szError, "The socket is marked as nonblocking and the receive operation would block. \r\n");
			break;
		case WSAEMSGSIZE :
			strcpy(szError, "The message was too large to fit into the specified buffer and was truncated. \r\n");
			break;
		case WSAEINVAL :
			strcpy(szError, "The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative. \r\n");
			break;
		case WSAECONNABORTED :
			strcpy(szError, "The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable. \r\n");
			break;
		case WSAETIMEDOUT :
			strcpy(szError, "The connection has been dropped because of a network failure or because the peer system failed to respond. \r\n");
			break;
		case WSAECONNRESET :
			strcpy(szError, "The virtual circuit was reset by the remote side executing a -hard- or -abortive- close. The application should close the socket as it is no longer usable. On a UDP datagram socket this error would indicate that a previous send operation resulted in an ICMP -Port Unreachable' message. \r\n");
			break;
		default:
			strcpy(szError, "A really fucked up error just happened!!!\r\n");
		}
		WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);
		
		strcpy(szError, "Received Data from Server, ERRORS!!!\r\n");
		WriteFile(hFile, szError, strlen(szError), &dwBytesWritten, NULL);

		return FALSE ;
	}
}

void CSMTPSocket :: Close ()
{
	if ( m_hSocket != INVALID_SOCKET )
	{
		if ( closesocket ( m_hSocket ) == SOCKET_ERROR )
		{
			WSACleanup( ) ;
			return ;
		}
		WSACleanup( ) ;
		m_hSocket = INVALID_SOCKET ;
	}
}

BOOL CSMTPSocket :: IsReadible ( BOOL& bReadible )
{
	timeval timeout = { 0, 0 } ;
	fd_set fds ;
	FD_ZERO ( &fds ) ;
	FD_SET ( m_hSocket, &fds ) ;
	int nStatus = select ( 0, &fds, NULL, NULL, &timeout ) ;
	if ( nStatus == SOCKET_ERROR )
	{
		return FALSE ;
	}
	else
	{
		bReadible = ! ( nStatus == 0 ) ;
		return TRUE ;
	}
}

/*
CComBSTR CSMTPMessage::GetHeader() const
{

	//Create the "Date:" part of the header
	CTime now(CTime::GetCurrentTime());
	CComBSTR sDate(now.Format(_T("%a, %d %b %Y %H:%M:%S %Z")));
    char tmpbuf [ 128 ] ;
	time_t ltime ;
	struct _timeb tstruct ;
	struct tm today ;
	strftime( tmpbuf, 128, "%a, %d %b %Y %H:%M:%S %Z", today );

	//Create the "To:" part of the header
	CComBSTR sTo;
	for (int i=0; i<GetNumberOfRecipients(); i++)
	{
		CSMTPAddress recipient = GetRecipient(i);
		if (i)
		{
			sTo += _T(",");
		}
		sTo += recipient.GetRegularFormat();
	}

	//Stick everything together
	CComBSTR sBuf;
	sBuf.Format(_T("From: %s\r\n")\
	_T("To: %s\r\n")\
			_T("Subject: %s\r\n")\
			_T("Date: %s\r\n")\
			_T("X-Mailer: %s\r\n"), 
			m_From.GetRegularFormat(),
	sTo, 
			m_sSubject,
			sDate,
			m_sXMailer);


	//Add the optional Reply-To Field
	if (m_ReplyTo.m_sEmailAddress.GetLength())
	{
	CComBSTR sReply;
	sReply.Format(_T("Reply-To: %s\r\n"), m_ReplyTo.GetRegularFormat());
	sBuf += sReply;
	}

	//Add the optional fields if attachments are included
	if (m_Attachments.GetSize())
	sBuf += _T("MIME-Version: 1.0\r\nContent-type: multipart/mixed; boundary=\"#BOUNDARY#\"\r\n");

	sBuf += _T("\r\n");

	//Return the result
	return sBuf;
}

void CSMTPMessage :: FixSingleDot ( CComBSTR & sBody )
{
	int nFind = strstr ( & sBody, "\n." ) ;
	if ( nFind != NULL )
	{
		& sBody [ nFind ] = ' ' ;
		FixSingleDot ( sBody ) ;
	}
}

void CSMTPMessage :: setHost ( const CComBSTR & sVal )
{
	m_Host = sVal ;
}
void CSMTPMessage :: setFrom ( const CComBSTR & sVal )
{
	m_From = sVal ;
}
void CSMTPMessage :: setSender ( const CComBSTR & sVal )
{
	m_Sender = sVal ;
}
void CSMTPMessage :: setTo ( const CComBSTR & sVal )
{
	m_To = sVal ;
}
void CSMTPMessage :: setRecipient ( const CComBSTR & sVal )
{
	m_Recipient = sVal ;
}
void CSMTPMessage :: setSubject ( const CComBSTR & sVal )
{
	m_Subject = sVal ;
}
void CSMTPMessage :: setMessage ( const CComBSTR & sVal )
{
	m_Message = sVal ;
	FixSingleDot ( m_Message ) ;

}
*/

CSMTPConnection :: CSMTPConnection ()
{
		m_bConnected = FALSE ;
	#ifdef _DEBUG
		m_dwTimeout = 20000 ; //default timeout of 20 seconds when debugging
	#else
		m_dwTimeout = 2000 ;  //default timeout of 2 seconds for normal release code
	#endif
}

CSMTPConnection :: ~CSMTPConnection ()
{
	if ( m_bConnected )
	{
		Disconnect () ;
	}
}

BOOL CSMTPConnection :: Connect ( LPCTSTR pszHostName, int nPort )
{
	USES_CONVERSION;

	if ( !pszHostName )
	{
		return FALSE ;
	}

	//Create the socket
	if ( !m_SMTP.Create () )
	{
		//TRACE(_T("Failed to create client socket\n"));
		return FALSE ;
	}

	//Connect to the SMTP Host
	if ( ! m_SMTP.Connect ( pszHostName, nPort ) )
	{
		//TRACE(_T("Could not connect to the SMTP server %s on port %d\n"), pszHostName, nPort);
		return FALSE ;
	}
	else
	{
		m_bConnected = TRUE ;

		//check the response to the login
		if ( ! ReadCommandResponse ( 220 ) )
		{
			Disconnect () ;
			return FALSE ;
		}

		//retreive the localhost name
		char sHostName [ 255 ] ;
		gethostname ( sHostName, sizeof(sHostName) ) ;
		TCHAR* pszHostName = A2T ( sHostName ) ;

		//Send the HELO command
		char sBuf[285] ;
		strcpy ( sBuf, "HELO " ) ;
		strcat ( sBuf, pszHostName ) ;
		strcat ( sBuf, "\r\n" ) ;
		LPCSTR pszData = T2A ( (LPTSTR)(LPCTSTR) sBuf ) ;
		int nCmdLength = strlen ( pszData ) ;
		if ( !m_SMTP.Send ( pszData, nCmdLength ) )
		{
			Disconnect () ;
			return FALSE ;
		}
		//check the response to the HELO command
		if ( ! ReadCommandResponse ( 250 ) )
		{
			Disconnect () ;
			return FALSE ;
		} 

		return TRUE ;
	}
}

BOOL CSMTPConnection :: Disconnect ()
{
	BOOL bSuccess = FALSE ;      

	if ( m_bConnected )
	{
		char sBuf [ 10 ] ;
		strcpy ( sBuf, "QUIT\r\n" ) ;
		int nCmdLength = strlen ( sBuf ) ;
		if ( ! m_SMTP.Send ( sBuf, nCmdLength ) )
		{
			//TRACE(_T("Failed in call to send QUIT command\n"));
		}

		//Check the reponse
		bSuccess = ReadCommandResponse ( 221 ) ;
		if ( ! bSuccess )
		SetLastError ( ERROR_BAD_COMMAND ) ;

		//Reset all the state variables
		m_bConnected = FALSE ;
	}
	else
	{
		//TRACE(_T("Already disconnected\n"));
	}

	//free up our socket
	m_SMTP.Close () ;

	return bSuccess ;
}


BOOL CSMTPConnection :: ReadCommandResponse ( int nExpectedCode )
{
	char sBuf [ 1000 ] ;
	return ReadResponse ( sBuf, sizeof(sBuf), "\r\n", nExpectedCode ) ;
}

BOOL CSMTPConnection :: ReadResponse ( LPSTR pszBuffer, int nBuf, LPSTR pszTerminator, int nExpectedCode )
{
	//paramater validity checking
	if ( !pszBuffer )
	{
		return FALSE ;
	}
	if ( !nBuf )
	{
		return FALSE ;
	}

	if ( !m_bConnected )
	{
		return FALSE ;
	}

	//retrieve the reponse using until we
	//get the terminator or a timeout occurs
	BOOL bFoundTerminator = FALSE ;
	int nReceived = 0 ;
	DWORD dwStartTicks = :: GetTickCount () ;
	while ( ! bFoundTerminator )
	{
		//timeout has occured
		if ( ( :: GetTickCount() - dwStartTicks ) >	m_dwTimeout )
		{
			pszBuffer [ nReceived ] = '\0' ;
			SetLastError ( WSAETIMEDOUT ) ;
			m_sLastCommandResponse = pszBuffer ; //Hive away the last command reponse
			return FALSE ;
		}

		//check the socket for readability
		BOOL bReadible ;
		if ( ! m_SMTP.IsReadible ( bReadible ) )
		{
			pszBuffer [ nReceived ] = '\0' ;
			m_sLastCommandResponse = pszBuffer ; //Hive away the last command reponse
			return FALSE ;
		}
		else if ( ! bReadible ) //no data to receive, just loop around
		{
			Sleep ( 100 ) ; //Wait for 100 ms prior to looping around, 
			//helps to improve performance of system
			continue ;
		}

		//receive the data from the socket
		int nData = m_SMTP.Receive ( pszBuffer+nReceived, nBuf-nReceived ) ;
		if ( nData == SOCKET_ERROR )
		{
			pszBuffer [ nReceived ] = '\0' ;
			m_sLastCommandResponse = pszBuffer ; //Hive away the last command reponse
			return FALSE ;  
		}
		else
		{
			if ( nData )
			{
				dwStartTicks = :: GetTickCount () ; //Reset the idle timeout
			}
			nReceived += nData ;							   //Increment the count of data received
		}

		pszBuffer [ nReceived ] = '\0' ;	//temporarily NULL terminate the string
														//so that strstr works
		bFoundTerminator = ( strstr ( pszBuffer, pszTerminator ) != NULL ) ;
	}

	//Remove the terminator from the response data
	pszBuffer [ nReceived - strlen( pszTerminator ) ] = '\0' ;

	//determine if the response is an error
	char sCode [ 4 ] ;
	strncpy ( sCode, pszBuffer, 3 ) ;
	sCode [ 3 ] = '\0' ;
	sscanf ( sCode, "%d", &m_nLastCommandResponseCode ) ;
	BOOL bSuccess = ( m_nLastCommandResponseCode == nExpectedCode ) ;

	if ( ! bSuccess )
	{
		SetLastError ( WSAEPROTONOSUPPORT ) ;
		m_sLastCommandResponse = pszBuffer ; //Hive away the last command reponse
	}

	return bSuccess ;
}


long CSMTPConnection :: SendTestMessage ()
{
	USES_CONVERSION;
	if ( !m_bConnected )
	{
		return 1 ;
	}
	m_SMTP.Send ( "MAIL FROM:<kevin_guerra@studio.disney.com>\r\n", strlen("MAIL FROM:<kevin_guerra@studio.disney.com>\r\n") ) ;
	if ( !ReadCommandResponse (250) )
	{
		return 2;
	} 
	m_SMTP.Send ( "RCPT TO:<kevin_guerra@studio.disney.com>\r\n", strlen("MAIL FROM:<kevin_guerra@studio.disney.com>\r\n") ) ;
	if ( !ReadCommandResponse (250) )
	{
		return 3;
	} 
	m_SMTP.Send ( "DATA\r\n", strlen("DATA\r\n") ) ;
	if ( !ReadCommandResponse (354) )
	{
		//return 4;
	} 
	m_SMTP.Send ( "DATE IS NULL, From: kevin_guerra@studio.disney.com\r\nTo: kevin_guerra@studio.disney.com\r\nDate:  NULL DATE\r\nX-Mailer: TESTER\r\nReply-To: kevin_guerra@studio.disney.com\r\nMIME-Version: 1.0\r\nContent-type: multipart/mixed; boundary=\'#BOUNDARY#\'\r\n\r\n", strlen("DATE IS NULL, From: kevin_guerra@studio.disney.com\r\nTo: kevin_guerra@studio.disney.com\r\nDate:  NULL DATE\r\nX-Mailer: TESTER\r\nReply-To: kevin_guerra@studio.disney.com\r\nMIME-Version: 1.0\r\nContent-type: multipart/mixed; boundary=\'#BOUNDARY#\'\r\n\r\n") ) ;
	m_SMTP.Send ( "TEST\r\n", strlen("TEST\r\n") ) ;

	m_SMTP.Send ( "\r\n.\r\n", strlen("\r\n.\r\n") ) ;
	if ( !ReadCommandResponse (250) )
	{
		return 5 ;
	} 
	return 0 ;
}
