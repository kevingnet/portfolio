#include "StdAfx.h"
#include ".\ocrserver.h"
#include "Options.h"
#include "common.h"
#include ".\timelapse.h"

OCRServer::OCRServer(void)
{
    m_MessageSequenceNumber = 1;
    m_IsConnected = false;
}

OCRServer::~OCRServer(void)
{
}

ACE_UINT32 OCRServer::GetAddress()
{
    return m_Address.get_ip_address();
}

bool OCRServer::Initialize()
{
    m_ErrorMessage.clear();

    ACE::init();

    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
     
    wVersionRequested = MAKEWORD( 3, 10 );
    err = WSAStartup( wVersionRequested, &wsaData );
    if ( err != 0 ) {
        m_ErrorMessage = "could not find a usable WinSock DLL";
        ACE_ERROR ((LM_WARNING, ACE_TEXT ("%p\n"), ACE_TEXT ("could not find a usable WinSock DLL")));
    }
    return true;
}

bool OCRServer::Connect(const char * host, int port)
{
    errno = 0;
    m_ErrorMessage.clear();

    if( ini_options->Debug() )
        ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%P|%t)"), ACE_TEXT ("Connecting...\n")));

    if( host && *host )
        m_Host = host;
    if( port )
        m_Port = port;

    m_Address.set( m_Port, m_Host, 0 );
    if (m_Address.addr_to_string (m_PeerAddress, sizeof(m_PeerAddress), 0) == 0)
    {
        ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%P|%t) Connecting to %s\n"), m_PeerAddress));
    }
    else
    {
        errno = ACE_OS::last_error();
        m_ErrorMessage = GetLastErrorMsg(errno);
        ACE_ERROR ((LM_WARNING, ACE_TEXT ("(%P|%t) Could not get address from: %s --- %s\n"), m_PeerAddress, m_ErrorMessage.c_str()));
    }

    ACE_Time_Value timeout (ini_options->ConnectionTimeout());
    ACE_OS::last_error(0);
    if (m_Connector.connect(m_Stream, m_Address, &timeout) == -1)
    {
        errno = ACE_OS::last_error();
        m_ErrorMessage = GetLastErrorMsg(errno);
        if (errno == ETIME)
        {
            ACE_ERROR ((LM_ERROR, ACE_TEXT ("(%P|%t) Timeout while ") ACE_TEXT ("connecting to log server\n")));
        }
        else
        {
            ACE_ERROR ((LM_ERROR, ACE_TEXT ("%p\n"), ACE_TEXT ("Could not connect to server --- %s"), m_ErrorMessage.c_str()));
        }
        return false;
    }
    m_IsConnected = true;
    return true;
}

bool OCRServer::Disconnect(bool clearError)
{
    if( clearError ) {
        errno = 0;
        m_ErrorMessage.clear();
    }
    if( ini_options->Debug() )
        ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%P|%t)"), ACE_TEXT ("Disconnecting...\n")));
    m_Stream.close_reader();
    m_Stream.close_writer();
    m_Stream.close();
    m_Connector.reset_new_handle( m_Stream.get_handle() );
    m_IsConnected = false;
    return true;
}

bool OCRServer::Ping()
{
    errno = 0;
    m_ErrorMessage.clear();

    TimeLapse       timer;
    timer.Start();

    if( ini_options->Debug() )
        ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%t)"), ACE_TEXT ("Pinging server...\n")));

    m_MessageRequest.Sequence               = htonl(m_MessageSequenceNumber);
    m_MessageRequest.Type                   = RequestPing;
    m_MessageRequest.Language               = 0;
    m_MessageRequest.CharacterProperties    = 0;
    m_MessageRequest.RecognitionFlags       = 0;
    m_MessageRequest.Length                 = htonl( 4 * sizeof(ACE_Byte) );
    m_MessageRequest.pBitmapData            = (ACE_Byte*)0;

    const char * pData = reinterpret_cast<char *>(&m_MessageRequest);

    iovec send[1];
    send[0].iov_base = reinterpret_cast<char *>(&m_MessageRequest);
    send[0].iov_len  = 16;

    ACE_Time_Value timeout (ini_options->SendTimeout());
    long total = 16;
    long bytes = m_Stream.sendv(send, 1, &timeout);
    if ( bytes == -1)
    {
        errno = ACE_OS::last_error();
        m_ErrorMessage = GetLastErrorMsg(errno);
        if (errno == ETIME)
        {
            ACE_ERROR ((LM_ERROR, ACE_TEXT ("(%P|%t) !!! Timeout while pinging ") ACE_TEXT ("query to status server\n")));
        }
        else
        {
            ACE_ERROR ((LM_ERROR, ACE_TEXT ("%p\n"), ACE_TEXT ("!!! ping sendv --- %s"), m_ErrorMessage.c_str()));
        }
        Disconnect(false);
        return false;
    } else if ( bytes != total ) {
        errno = ACE_OS::last_error();
        m_ErrorMessage = GetLastErrorMsg(errno);
        ACE_ERROR ((LM_ERROR, ACE_TEXT ("%p\n"), ACE_TEXT ("ping sendv invalid number of bytes sent! --- %s"), m_ErrorMessage.c_str()));
        Disconnect(false);
        return false;
    }

    memset((void*)&m_MessageReturn, 0, sizeof(m_MessageReturn) );
    ssize_t bc ;
    ACE_Time_Value recvTimeout (ini_options->ReceiveTimeout());

    char * pBuf = reinterpret_cast<char *>(&m_MessageReturn);

    bc = m_Stream.recv(pBuf, FRAME_SIZE, &recvTimeout);
    if ( bc == -1 )
    {
        errno = ACE_OS::last_error();
        m_ErrorMessage = GetLastErrorMsg(errno);
        if (errno == ETIME)
        {
            ACE_ERROR ((LM_ERROR, ACE_TEXT ("%p\n"), ACE_TEXT ("ping recv frame timed out! --- %s"), m_ErrorMessage.c_str()));
        }
        else
        {
            ACE_ERROR ((LM_ERROR, ACE_TEXT ("%p\n"), ACE_TEXT ("ping recv frame --- %s"), m_ErrorMessage.c_str()));
        }
        Disconnect(false);
        return false;
    } else if ( bc != FRAME_SIZE ) {
        errno = ACE_OS::last_error();
        m_ErrorMessage = GetLastErrorMsg(errno);
        ACE_ERROR ((LM_ERROR, ACE_TEXT ("%p\n"), ACE_TEXT ("ping recv frame, invalid number of bytes received! --- %s"), m_ErrorMessage.c_str()));
        Disconnect(false);
        return false;
    }
    
    m_MessageReturn.MagicWord = ntohl( m_MessageReturn.MagicWord );
    if( m_MessageReturn.MagicWord != MAGIC_WORD ) {
        ACE_ERROR ((LM_ERROR, ACE_TEXT ("(%P|%t) !!! Invalid Magic Word\n")));
        return false;
    }

    m_MessageReturn.Length = ntohl( m_MessageReturn.Length );

    m_MessageReturn.Sequence = ntohl( m_MessageReturn.Sequence );
    if( m_MessageReturn.Sequence != m_MessageSequenceNumber ) {
        ACE_ERROR ((LM_ERROR, ACE_TEXT ("(%P|%t) !!! Messages are out of sequence\n")));
        return false;
    }

    m_MessageReturn.Result             = ntohs( m_MessageReturn.Result );
    m_MessageSequenceNumber++;

    UINT32 pingTime = timer.End();

    if( ini_options->Debug() && ini_options->Verbose() )
    {
        ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "Ping --- times: %d ms \n" ), pingTime ));
    }
    return true;
}

int OCRServer::Recognize( DBitmap & bmp, ACE_Byte character_properties, ACE_Byte recognition, ACE_Byte type, ACE_Byte language )
{
    errno = 0;
    m_ErrorMessage.clear();

    if( ini_options->Debug() )
        ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%t)"), ACE_TEXT ("Recognizing bitmap...\n")));

    ACE_UINT32 h = abs(bmp.GetHeight());
    ACE_UINT32 w = abs(bmp.GetWidth());
    ACE_Byte bpp = bmp.GetBytesPerPixel();
    ACE_UINT32 bmp_size = h * w * bpp;
    if( !bmp_size )
    	bmp_size = (((w % 16) ? ((w/16)+1)*16 : w) / 8) * h; 

    m_MessageRequest.Sequence               = htonl(m_MessageSequenceNumber);
    m_MessageRequest.Type                   = type;
    m_MessageRequest.Language               = language;
    m_MessageRequest.CharacterProperties    = character_properties;
    m_MessageRequest.RecognitionFlags       = recognition;
    m_MessageRequest.Length                 = htonl( bmp_size + (4 * sizeof(ACE_Byte)) + sizeof(BITMAPINFO) );
    m_MessageRequest.pBitmapData            = (ACE_Byte*)bmp.GetBitmapBuffer();

    const char * pData = reinterpret_cast<char *>(&m_MessageRequest);

    iovec send[3];
    send[0].iov_base = reinterpret_cast<char *>(&m_MessageRequest);
    send[0].iov_len  = 16;
    send[1].iov_base = reinterpret_cast<char *>(const_cast<BITMAPINFO *>(bmp.GetBitmapInfo()));
    send[1].iov_len  = sizeof(BITMAPINFO);
    send[2].iov_base = reinterpret_cast<char *>(m_MessageRequest.pBitmapData);
    send[2].iov_len  = bmp_size;

    ACE_Time_Value timeout (ini_options->SendTimeout());
    long total = 16 + sizeof(BITMAPINFO) + bmp_size;
    long bytes = m_Stream.sendv(send, 3, &timeout);
    if ( bytes == -1)
    {
        errno = ACE_OS::last_error();
        m_ErrorMessage = GetLastErrorMsg(errno);
        if (errno == ETIME)
        {
            ACE_ERROR ((LM_ERROR, ACE_TEXT ("(%P|%t) !!! Timeout while sending bitmap ") ACE_TEXT ("query to status server\n")));
        }
        else
        {
            ACE_ERROR ((LM_ERROR, ACE_TEXT ("%p\n"), ACE_TEXT ("!!! recognition sendv --- %s"), m_ErrorMessage.c_str()));
        }
        return DTVOCRS_ERROR_NETWORK;
    } else if ( bytes != total ) {
        errno = ACE_OS::last_error();
        m_ErrorMessage = GetLastErrorMsg(errno);
        ACE_ERROR ((LM_ERROR, ACE_TEXT ("%p\n"), ACE_TEXT ("bitmap sendv invalid number of bytes sent! --- %s"), m_ErrorMessage.c_str()));
        return DTVOCRS_ERROR_NETWORK;
    }

    memset((void*)&m_MessageReturn, 0, sizeof(m_MessageReturn) );
    ssize_t bc ;
    ACE_Time_Value recvTimeout (ini_options->ReceiveTimeout());
    char * pBuf = reinterpret_cast<char *>(&m_MessageReturn);

    bc = m_Stream.recv(pBuf, FRAME_SIZE, &recvTimeout);
    if ( bc == -1 )
    {
        errno = ACE_OS::last_error();
        m_ErrorMessage = GetLastErrorMsg(errno);
        if (errno == ETIME)
        {
            ACE_ERROR ((LM_ERROR, ACE_TEXT ("%p\n"), ACE_TEXT ("recognition recv frame timed out! --- %s"), m_ErrorMessage.c_str()));
        }
        else
        {
            ACE_ERROR ((LM_ERROR, ACE_TEXT ("%p\n"), ACE_TEXT ("recognition recv frame --- %s"), m_ErrorMessage.c_str()));
        }
        return DTVOCRS_ERROR_NETWORK;
    } else if ( bc != FRAME_SIZE ) {
        errno = ACE_OS::last_error();
        m_ErrorMessage = GetLastErrorMsg(errno);
        ACE_ERROR ((LM_ERROR, ACE_TEXT ("%p\n"), ACE_TEXT ("recognition recv frame, invalid number of bytes received! --- %s"), m_ErrorMessage.c_str()));
        return DTVOCRS_ERROR_NETWORK;
    }
    
    m_MessageReturn.MagicWord = ntohl( m_MessageReturn.MagicWord );
    if( m_MessageReturn.MagicWord != MAGIC_WORD ) {
        m_ErrorMessage = "Invalid Magic Word";
        ACE_ERROR ((LM_ERROR, ACE_TEXT ("(%P|%t) !!! %s \n"), m_ErrorMessage.c_str()));
        return DTVOCRS_ERROR_PROTOCOL;
    }

    m_MessageReturn.Length = ntohl( m_MessageReturn.Length );

    m_MessageReturn.Sequence = ntohl( m_MessageReturn.Sequence );
    if( m_MessageReturn.Sequence != m_MessageSequenceNumber ) {
        m_ErrorMessage = "Messages are out of sequence";
        ACE_ERROR ((LM_ERROR, ACE_TEXT ("(%P|%t) !!! %s \n"), m_ErrorMessage.c_str()));
        return DTVOCRS_ERROR_PROTOCOL;
    }

    pBuf = reinterpret_cast<char *>(&m_MessageReturn) + FRAME_SIZE;

    bc = m_Stream.recv(pBuf, m_MessageReturn.Length, &recvTimeout);
    if ( bc == -1 )
    {
        errno = ACE_OS::last_error();
        m_ErrorMessage = GetLastErrorMsg(errno);
        if (errno == ETIME)
        {
            ACE_ERROR ((LM_ERROR, ACE_TEXT ("%p\n"), ACE_TEXT ("recognition recv data, timed out! --- %s"), m_ErrorMessage.c_str()));
        }
        else
        {
            ACE_ERROR ((LM_ERROR, ACE_TEXT ("%p\n"), ACE_TEXT ("recognition recv data --- %s"), m_ErrorMessage.c_str()));
        }
        return DTVOCRS_ERROR_NETWORK;
    } else if ( bc != m_MessageReturn.Length ) {
        errno = ACE_OS::last_error();
        m_ErrorMessage = GetLastErrorMsg(errno);
        ACE_ERROR ((LM_ERROR, ACE_TEXT ("%p\n"), ACE_TEXT ("recognition recv data, invalid number of bytes received! --- %s"), m_ErrorMessage.c_str()));
        return DTVOCRS_ERROR_NETWORK;
    }

    m_MessageReturn.Result             = ntohs( m_MessageReturn.Result );
    m_MessageReturn.ArrayLength        = ntohs( m_MessageReturn.ArrayLength );
    m_MessageSequenceNumber++;

    if( m_MessageReturn.Result == 0xFFFF ) {
        m_ErrorMessage = "Bad bitmap server error";
        ACE_ERROR(( LM_ERROR, ACE_TEXT( "(%P|%t) !!! %s \n" ), m_ErrorMessage.c_str()));
        return DTVOCRS_ERROR_BITMAP;
    }

    if( m_MessageReturn.Result == 0xFFFE ) {
        m_ErrorMessage = "Server exception error";
        m_ErrorMessage += (char*)m_MessageReturn.Buffer;
        ACE_ERROR(( LM_ERROR, ACE_TEXT( "(%P|%t) !!! %s \n" ), m_ErrorMessage.c_str() ));
        return DTVOCRS_ERROR_EXCEPTION;
    }

    if( ini_options->Debug() && ini_options->Verbose() )
    {
        ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "Array Length: %d \nBuffer '%W' \n" ), m_MessageReturn.ArrayLength, m_MessageReturn.Buffer ));
    }
    return DTVOCRS_SUCCESS;
}

BYTE * OCRServer::GetConfidenceArray() 
{ 
    if( !m_MessageReturn.ArrayLength || m_MessageRequest.RecognitionFlags & DiscardConfidenceArray )
        return 0;
    int mult = 2;
    return (BYTE*)((m_MessageReturn.Buffer + (m_MessageReturn.ArrayLength * mult)));
}

UINT16 * OCRServer::GetCharacterCoordinatesArray() 
{ 
    if( !m_MessageReturn.ArrayLength || !(m_MessageRequest.RecognitionFlags & ReturnCharacterCoordinatesArray) )
        return 0;
    int mult = 2;
    if( !(m_MessageRequest.RecognitionFlags & DiscardConfidenceArray) )
        mult += 1;
	return (UINT16*)((m_MessageReturn.Buffer + (m_MessageReturn.ArrayLength * mult)));
}

UINT16 * OCRServer::GetCharacterSizesArray() 
{ 
    if( m_MessageReturn.Type == ReturnPlainText || !m_MessageReturn.ArrayLength || !(m_MessageRequest.RecognitionFlags & ReturnCharacterSizesArray) )
        return 0;
    int mult = 2;
    if( !(m_MessageRequest.RecognitionFlags & DiscardConfidenceArray) )
        mult += 1;
    if( m_MessageRequest.RecognitionFlags & ReturnCharacterCoordinatesArray )
        mult += sizeof(UINT16) * 4;
	return (UINT16*)((m_MessageReturn.Buffer + (m_MessageReturn.ArrayLength * mult)));
}

BYTE * OCRServer::GetCharacterPropertiesArray() 
{ 
    if( m_MessageReturn.Type == ReturnPlainText || !m_MessageReturn.ArrayLength || !(m_MessageRequest.RecognitionFlags & ReturnCharacterPropertiesArray) )
        return 0;
    int mult = 2;
    if( !(m_MessageRequest.RecognitionFlags & DiscardConfidenceArray) )
        mult += 1;
    if( m_MessageRequest.RecognitionFlags & ReturnCharacterCoordinatesArray )
        mult += sizeof(UINT16) * 4;
    if( m_MessageRequest.RecognitionFlags & ReturnCharacterSizesArray )
        mult += sizeof(UINT16);
	return (BYTE*)((m_MessageReturn.Buffer + (m_MessageReturn.ArrayLength * mult)));
}

