#include <Pilot.h>
#include <FileStream.h>
#include <SerialMgr.h>
#include <SysEvtMgr.h>

#include "Globals.h"
#include "Com.h"

static UInt			s_SLRefNum;
//static char			s_szReceiveBuffer[ DEFAULT_RECEIVE_BUFFER_SIZE + HEADER_SIZE ];

int UploadFile( const char * p_szFileName, const int p_iRecordLength )
{
	Err	Error = 0;
	Err	ReturnError = SERIAL_SUCCESS;

	Boolean				bEndOfProcess 	= false;

	int					iState 			= STATE_UL_OPEN;
	int					iNextState 		= STATE_UL_OPEN;
	int					iLastState 		= STATE_UL_OPEN;
	int					iRetry 			= 0;
	int					iTimerEvent		= 0;

	long					lngFileLength	 	= 0L;
	long					lngFilePosition	= 0L;
	long					lngPacketNumber 	= 0L;
	long					lngPacketCount	= 0L;
	long					lngPacketSize		= 0L;
	unsigned long			lngBytes 			= 0L;
	
	DWord				dwAutoOffTime	= 0L;
	FileHand 				* hUploadFile		= NULL;
	
	char					szPacketData		[ DEFAULT_BUFFER_SIZE ];
	char					szPIDString		[ 10 ];

	if( g_Configuration.iPacketSize != DEFAULT_PACKET_SIZE )
	{
		g_Configuration.iPacketSize = DEFAULT_PACKET_SIZE;
	}
		
	dwAutoOffTime = PrefGetPreference( prefAutoOffDuration );
	
	lngPacketSize = (long)( g_Configuration.iPacketSize / p_iRecordLength );
	lngPacketSize *= p_iRecordLength;
	
	while( iState < STATE_UL_LAST && !bEndOfProcess )
	{
		if( iTimerEvent++ > ( dwAutoOffTime - 10 ) )
		{	// dont let the pilot turn off while we are in communications
			iTimerEvent = 0;
			EvtResetAutoOffTimer( );
		}
		
		ZEROOUT( szPacketData );
		switch( iState )
		{
			// OPEN FILE TO WRITE TO, VERIFY SERIAL ROUTINES EXIST and OPEN COM PORT
			case STATE_UL_OPEN:
			
				hUploadFile = FileOpen( 0, (char*)p_szFileName, 0, 0, 
										fileModeReadOnly | fileModeExclusive, 
										&Error );
				EXIT_AND_DISPLAY_IF_ERROR( Error )

				Error = FileSeek( hUploadFile, 0L, fileOriginEnd );	// move to eof
				EXIT_AND_DISPLAY_IF_ERROR( Error )
				
				lngFileLength = FileTell( hUploadFile, NULL, &Error );
				lngPacketCount = lngFileLength / lngPacketSize ;	// calc number of packets
				
				Error = FileSeek( hUploadFile, 0L, fileOriginBeginning );
				EXIT_AND_DISPLAY_IF_ERROR( Error )
				
				Error  = SerialOpen( );
				EXIT_AND_DISPLAY_IF_ERROR( Error )

				break;

			// SEND ENQ TO LOGON TO SYSTEM AND REQUEST DAT
			// PalmPilot send function will not send LF characters.
			// ENQ + C tells server to append LF when it finds a CR
			case STATE_UL_SEND_ENQ:
				FORMAT_PACKET1( "%c%03dENQB%c", g_Configuration.iHHTNumber );
				SEND_PACKET
				break;

			// CHECK ENQ AND USE DATE/TIME RETRIEVED
			case STATE_UL_CHECK_ENQ:
				RECEIVE_PACKET
				if( Error  == SERIAL_SUCCESS )
				{
					PID_STRING_SETUP

					if( PID_STRING_IS( szACK ) )
					{
						iNextState = STATE_UL_REQUEST_FILE;	// request file
						iRetry = 0;
					}
					else if( PID_STRING_IS( szNAK ) )
					{
						iState -= 2;	// repeat last step ( -2 coz iState++ down below )
						iRetry++;
					}
				}
				else
				{
					iNextState = STATE_UL_NAK;
				}
				
				break;

			// SEND REQUEST TO SEND FILENAME
			case STATE_UL_REQUEST_FILE:
				//rename files
				FORMAT_PACKET2( "%c%03dRTRR|%s%c", g_Configuration.iHHTNumber, p_szFileName );
				//overwrite
				//FORMAT_PACKET2( "%c%03dRTRO|%s%c", g_Configuration.iHHTNumber, p_szFileName );
				SEND_PACKET
				break;

			// GET ANSWER FROM SERVER
			case STATE_UL_CHECK_REQUEST_FILE:
				RECEIVE_PACKET
				if( Error  == SERIAL_SUCCESS )
				{
					PID_STRING_SETUP

					if( PID_STRING_IS( szACK ) )
					{
						iNextState = STATE_UL_SEND_DATA;	// start sending
						iRetry = 0;
					}
					else if( PID_STRING_IS( szNAK ) )
					{
						iState -= 2;	// repeat last step ( -2 coz iState++ down below )
						iRetry++;
					}
				}
				else
				{
					iNextState = STATE_UL_NAK;
				}
				
				break;

			// SEND CHUNKS OF FILE UNTIL EOF
			case STATE_UL_SEND_DATA:
			
				if( FileEOF( hUploadFile ) )
				{
					iNextState = STATE_UL_SEND_EOF;
					break;
				}

				//lngPacketNumber starts at zero, so first lngFilePosition is always zero, the beginning of the file
				lngFilePosition = lngPacketSize * lngPacketNumber;
				
				//avoid trying to read beyond lngFileLength, so adjust lngPacketSize
				if( ( lngFilePosition + lngPacketSize ) > lngFileLength )
				{
					//this is the last chunk to read 
					lngPacketSize = lngFileLength - lngFilePosition;
					iNextState = STATE_UL_SEND_EOF;
				}
				
				if( lngFilePosition == lngFileLength )
				{
					iNextState = STATE_UL_SEND_EOF;
					break;
				}

				Error = FileSeek( hUploadFile, lngFilePosition, fileOriginBeginning ); 
				EXIT_AND_DISPLAY_IF_ERROR( Error )

				FORMAT_PACKET_HEADER1( "%c%03dDAT", g_Configuration.iHHTNumber );
				FileRead( hUploadFile, szPacketData + PACKET_DATA_HEADER_SIZE, lngPacketSize, ONE_RECORD, &Error );
				EXIT_AND_DISPLAY_IF_ERROR( Error )
				szPacketData[ PACKET_DATA_HEADER_SIZE + lngPacketSize ] = EOM;
				szPacketData[ PACKET_DATA_HEADER_SIZE + lngPacketSize + 1 ] = 0;
				
				if( Error == SUCCESS )
				{
					lngPacketNumber++;
					SEND_PACKET
					
					if( iNextState != STATE_UL_SEND_EOF )
						iNextState = STATE_UL_CHECK_SEND;
				}
				else //we had some kind of file read error, bail out while there's time
				{
					iNextState = STATE_UL_SEND_EOF;
				}

				break;

			// CHECK TO MAKE SURE SERVER RECEIVED OUR LAST PACKET
			case STATE_UL_CHECK_SEND:
				RECEIVE_PACKET
				if( Error  == SERIAL_SUCCESS )
				{
					PID_STRING_SETUP

					if( PID_STRING_IS( szACK ) )
					{
						iNextState = STATE_UL_SEND_DATA;	// send next record
					}
					else if( PID_STRING_IS( szNAK ) )
					{
						iState -= 2;			// send last record again
						lngPacketNumber--;		// back one record
						iRetry++;
					}
				}
				else
				{
					iNextState = STATE_UL_NAK;
				}
				
				break;

			// END OF FILE REACHED ; TELL SERVER
			case STATE_UL_SEND_EOF:
				FORMAT_PACKET1( "%c%03dEOF%c", g_Configuration.iHHTNumber );
				SEND_PACKET
				break;

			// CHECK SERVERS RESPONSE TO EOF
			case STATE_UL_CHECK_EOF:
				RECEIVE_PACKET
				if( Error  == SERIAL_SUCCESS )
				{
					PID_STRING_SETUP

					if( PID_STRING_IS( szACK ) )
					{
						bEndOfProcess = true;
						iState = STATE_UL_LAST;	// bail out
					}
					else if( PID_STRING_IS( szNAK ) )
					{
						iState -= 2;	// repeat last step ( -2 coz iState++ down below )
						iRetry++;
					}
				}
				else
				{
					iNextState = STATE_UL_NAK;
				}
				
				break;

			// SEND ACK SO SERVER WILL SEND NEXT PACKET
			case STATE_UL_ACK:
				FORMAT_PACKET1( "%c%03dACK%c", g_Configuration.iHHTNumber );
				SEND_PACKET
				break;

			// SEND NAK TO SERVER!! BAD PACKET RECEIVED
			case STATE_UL_NAK:
				FORMAT_PACKET1( "%c%03dNAK%c", g_Configuration.iHHTNumber );
				SEND_PACKET
				break;
		}

		CLEAR_ERROR
/*		if( serErrLineErr == Error )
		{
			CLEAR_ERROR
		}
		else if( Error )
		{
			DisplayCommError( Error );
			goto Exit;
		}
*/

		// back iState so we can return to it; but only if its not nak/ack
		if( iState < STATE_UL_RESPONSE )
			iLastState = iState;

		if( iRetry >= MAX_SERIAL_ERRORS_RETRY ) 
			Error  = serLineErrorSWOverrun;

		if( Error  == SERIAL_SUCCESS )
		{
			// if ACK, restore iLastState
			if( iState==STATE_UL_ACK ) iState = iLastState;

			iState++;
		}
		else
		{
			ReturnError = Error;
			goto Exit;
		}

		// jumping to cts, ack or nak iState
		if( iNextState > 0 )
		{
			iState = iNextState;
			iNextState = 0;
		}
	}

Exit:
	if( Error )
		ReturnError = Error;
	Error = FileClose( hUploadFile );
	DISPLAY_IF_ERROR( Error )
	Error = SerSendFlush( s_SLRefNum );
	DISPLAY_IF_ERROR( Error )
	SerReceiveFlush( s_SLRefNum, TIMEOUT_RECEIVE );
	Error = SerialClose( );
	DISPLAY_IF_ERROR( Error )

	EvtResetAutoOffTimer( );
	
	return( ReturnError  );
}

int DownloadFile( const char * p_szFileName )
{
	Err	Error = 0;
	Err	ReturnError = SERIAL_SUCCESS;

	Boolean				bEndOfProcess 	= false;
	
	int					iState 			= STATE_DL_OPEN;
	int					iNextState 		= STATE_DL_OPEN;
	int					iLastState 		= STATE_DL_OPEN;
	int					iRetry 			= 0;
	int					iTimerEvent		= 0;
	
	long					lngFilePosition	= 0L;
	long					lngPacketSize	 	= 0L;
	unsigned long			lngBytes 			= 0L;
	DWord				dwAutoOffTime	= 0L;
	FileHand 				* hDownloadFile	= NULL;

	char					szPacketData		[ DEFAULT_BUFFER_SIZE ];
	char					szPIDString		[ 10 ];

	if( g_Configuration.iPacketSize != DEFAULT_PACKET_SIZE )
	{
		g_Configuration.iPacketSize = DEFAULT_PACKET_SIZE;
	}
		
	dwAutoOffTime = PrefGetPreference( prefAutoOffDuration );
	
	while( iState < STATE_DL_LAST && !bEndOfProcess )
	{
		if( iTimerEvent++ > ( dwAutoOffTime - 10 ) )
		{	// dont let the pilot turn off while we are in communications
			iTimerEvent = 0;
			EvtResetAutoOffTimer( );
		}

		ZEROOUT( szPacketData );
		switch( iState )
		{
			// OPEN FILE TO WRITE TO, VERIFY SERIAL ROUTINES EXIST and OPEN COM PORT
			case STATE_DL_OPEN:
			
				Error = FileDelete( 0, (char*)p_szFileName );
				if( Error != fileErrNotFound )
					EXIT_AND_DISPLAY_IF_ERROR( Error )

				hDownloadFile = FileOpen( 0, (char*)p_szFileName, 0, 0, 
										fileModeAppend, //fileModeAppend, fileModeReadWrite|fileModeAnyTypeCreator
										&Error );
				EXIT_AND_DISPLAY_IF_ERROR( Error )

				Error  = SerialOpen( );
				EXIT_AND_DISPLAY_IF_ERROR( Error )

				break;

			// SEND ENQ TO LOGON TO SYSTEM AND REQUEST DAT
			case STATE_DL_SEND_ENQ:
				FORMAT_PACKET2( "%c%03dENQC%04d%c", g_Configuration.iHHTNumber, g_Configuration.iPacketSize );
				SEND_PACKET
				break;

			// CHECK ENQ AND USE DATE/TIME RETRIEVED
			case STATE_DL_CHECK_ENQ:
				RECEIVE_PACKET
				if( Error  == SERIAL_SUCCESS )
				{
					PID_STRING_SETUP
				
					if( PID_STRING_IS( szACK ) )
					{
						iNextState = STATE_DL_REQUEST_FILE;
						iRetry = 0;
					}
					else if( PID_STRING_IS( szNAK ) )
					{
						iState -= 2;	// repeat last step ( -2 coz iState++ down below )
						iRetry++;
					}
				}
				else
				{
					iNextState = STATE_DL_NAK;
				}
				
				break;

			// SEND REQUEST FOR FILENAME
			case STATE_DL_REQUEST_FILE:
				FORMAT_PACKET2( "%c%03dRTS%s%c", g_Configuration.iHHTNumber, p_szFileName );
				SEND_PACKET
				break;

			// GET ANSWER FROM SERVER
			case STATE_DL_CHECK_REQUEST_FILE:
				RECEIVE_PACKET
				if( Error  == SERIAL_SUCCESS )
				{
					PID_STRING_SETUP

					if( PID_STRING_IS( szACK ) )
					{
						iNextState = STATE_DL_CTS;	// send clear to send signal
						iRetry = 0;
					}
					else if( PID_STRING_IS( szNAK ) )
					{
						iState -= 2;	// repeat last step ( -2 coz iState++ down below )
						iRetry++;
					}
				}
				else
				{
					iNextState = STATE_DL_NAK;
				}
				
				break;

			case STATE_DL_GET_FILE_DATA:
				RECEIVE_PACKET
				if( Error  == SERIAL_SUCCESS )
				{
					PID_STRING_SETUP

					if( PID_STRING_IS( szDAT ) )
					{
						iNextState = STATE_DL_CTS;	// send clear to send
						iState--;		// want to repeat this step ( will increment down below )

						lngPacketSize	 = (long)( lngBytes - SIZE_OF_HEADER );
						
						FileWrite( hDownloadFile,
								szPacketData + START_OF_DATA,
								lngPacketSize,
								1, &Error );
						EXIT_AND_DISPLAY_IF_ERROR( Error )
						lngFilePosition += lngPacketSize;
						FileFlush( hDownloadFile );

						Error = FileSeek( hDownloadFile, lngFilePosition, fileOriginBeginning ); 
						EXIT_AND_DISPLAY_IF_ERROR( Error )
					}
					else if( PID_STRING_IS( szNAK ) )
					{
						iNextState = STATE_DL_NAK;	// send NAK
						iState--;					// want to repeat this step ( will increment down below )
						iRetry++;
					}
					else if( PID_STRING_IS( szEOF ) )
					{
						iNextState = STATE_DL_ACK;
						bEndOfProcess = true;
					}
				}
				else
				{
					iNextState = STATE_DL_NAK;		// NAK!
				}
				
				break;

			// SEND CTS ( CLEAR TO SEND ) so server will send first packet
			case STATE_DL_CTS:
				FORMAT_PACKET1( "%c%03dCTS%c", g_Configuration.iHHTNumber );
				SEND_PACKET
				break;

			// SEND ACK SO SERVER WILL SEND NEXT PACKET
			case STATE_DL_ACK:
				FORMAT_PACKET1( "%c%03dACK%c", g_Configuration.iHHTNumber );
				SEND_PACKET
				break;

			// SEND NAK TO SERVER!! BAD PACKET RECEIVED
			case STATE_DL_NAK:
				FORMAT_PACKET1( "%c%03dNAK%c", g_Configuration.iHHTNumber );
				SEND_PACKET
				break;
		}

		CLEAR_ERROR
/*		if( serErrLineErr == Error )
		{
			CLEAR_ERROR
		}
		else if( Error )
		{
			DisplayCommError( Error );
			goto Exit;
		}
*/
		// back iState so we can return to it; but only if its not nak/ack
		if( iState < STATE_DL_RESPONSE )
		{
			iLastState = iState;
		}

		if( iRetry >= MAX_SERIAL_ERRORS_RETRY )
		{
			Error  = serLineErrorSWOverrun;
		}

		if( Error == SERIAL_SUCCESS )
		{
			// if CTS or ACK, restore iLastState
			if( iState == STATE_DL_CTS || iState == STATE_DL_ACK )
				iState = iLastState;

			iState++;
		}
		else
		{
			ReturnError = Error;
			goto Exit;
		}

		// jumping to CTS, ACK or NAK iState
		if( iNextState > STATE_DL_OPEN )
		{
			iState = iNextState;
			iNextState = STATE_DL_OPEN;
		}
	}

Exit:
	if( Error )
		ReturnError = Error;
	Error = FileClose( hDownloadFile );
	DISPLAY_IF_ERROR( Error )
	Error = SerSendFlush( s_SLRefNum );
	DISPLAY_IF_ERROR( Error )
	SerReceiveFlush( s_SLRefNum, TIMEOUT_RECEIVE );
	Error = SerialClose( );
	DISPLAY_IF_ERROR( Error )

	EvtResetAutoOffTimer( );
	
	return( ReturnError  );
}

int SerialReceive( char * p_szData, unsigned long * p_plngLength )
{
	Err	Error = SERIAL_SUCCESS;

	Boolean				bSTXFound 		= false;
	Boolean				bETXFound 		= false;

	int					iRetry 			= 0;
	unsigned long			lngCounter		= 0;
	unsigned long			lngBytes 			= 0;
	unsigned long			lngTotalBytes 		= 0;
	unsigned long			lngTimeStart		= 0;
	ULong				ulBytesReceived	= 0;

	char					szPacketData	[ DEFAULT_BUFFER_SIZE ];

	p_szData[0 ] = '\0';

	lngTimeStart = TimGetSeconds( );

	while( !bETXFound )
	{
		lngBytes = 0;

		CLEAR_ERROR
		
		// check how many bytes in the queue
		Error = SerReceiveCheck( s_SLRefNum, &lngBytes );
		
		if( lngBytes )
		{
			CLEAR_ERROR
			
			if( lngBytes > DEFAULT_PACKET_SIZE )
				lngBytes = DEFAULT_PACKET_SIZE;

			ulBytesReceived = 0;
			ulBytesReceived = SerReceive( s_SLRefNum, szPacketData, lngBytes, TIMEOUT_RECEIVE, &Error );
			//Error = SerReceive10( s_SLRefNum, szPacketData, lngBytes, TIMEOUT_RECEIVE );
			
			if( Error == SERIAL_SUCCESS ) //&& ulBytesReceived != 0 )
			{
				lngCounter = 0;
				lngTotalBytes += lngBytes;
				//lngCounter = lngTotalBytes;
				//lngTotalBytes += ulBytesReceived;
				StrCat( p_szData, szPacketData );
				p_szData[ lngTotalBytes ] = '\0';
				
				for( ; lngCounter<lngTotalBytes; lngCounter++ )
				{
					// search for STX only once
					if( !bSTXFound )
					{
						if( p_szData[ lngCounter ] == SOM ) 
							bSTXFound = true;
					}

					// search for ETX only once
					if( p_szData[ lngCounter ] == EOM )
					{
						if( bSTXFound )
						{
							bETXFound = true;
							*p_plngLength = lngTotalBytes;
							goto Exit;
						}
					}
				}
			}
			else
			{ //receive error, f#%@!!!
			}
		}
		else
		{
			// only check every 100 loops
			if( iRetry++ > 100 )
			{
				// after 10 seconds abort
				if( TimGetSeconds( ) - lngTimeStart > RECEIVE_TIMEOUT_SECONDS )
				{
					Error = SERIAL_TIMEOUT;
					goto Exit;
				}
			}
		}
	}

	*p_plngLength = lngTotalBytes;

Exit:
	return( Error );
}

int SerialSend( char * p_szData, const int p_iLength )
{
	Err	Error = SERIAL_SUCCESS;
	ULong		ulBytesSent = 0;
	
	CLEAR_ERROR

	ulBytesSent = SerSend( s_SLRefNum, p_szData, p_iLength, &Error );
//	Error = SerSend10( s_SLRefNum, p_szData, p_iLength );
	EXIT_AND_DISPLAY_IF_ERROR( Error )
	CLEAR_ERROR

	Error = SerSendWait( s_SLRefNum, TIMEOUT_RESERVED );	// wait till transmit buffer is clear
	EXIT_AND_DISPLAY_IF_ERROR( Error )

Exit:
	return( Error );
}

int GetNumberOfRecords( FileHand * p_hFile, const int p_iRecordLength )
{
	Err Error = 0;
	int iRecordCount = 0;
	long lngFileLength;
	
	if( p_iRecordLength < 1 )
	{
		DisplayFileError( fileErrInvalidParam );
		return ( fileErrInvalidParam );
	}
	
	Error = FileSeek( p_hFile, 0L, fileOriginEnd );	// move to eof
	EXIT_AND_DISPLAY_IF_ERROR( Error )
	
	Error = 0;
	lngFileLength = FileTell( p_hFile, NULL, &Error );
	iRecordCount = (int) ( lngFileLength / (long)p_iRecordLength ) ;	// calc number of records
	EXIT_AND_DISPLAY_IF_ERROR( Error )
	
Exit:
	return( iRecordCount );	// return number of records
}

int GetNumberOfRecords2( const char * p_szFileName, const int p_iRecordLength )
{
	Err Error = 0;
	int iRecordCount = 0;
	long lngFileLength;
	FileHand 	* hFile = NULL;
	
	if( p_iRecordLength < 1 )
	{
		DisplayFileError( fileErrInvalidParam );
		return ( fileErrInvalidParam );
	}
	
	hFile = FileOpen( 0, (char*)p_szFileName, 0, 0, fileModeReadOnly, &Error );
	if( Error == fileErrNotFound )
		return( FAILIURE );
	RETURN_BOOL_AND_DISPLAY_IF_FILE_ERROR( Error )
	
	Error = FileSeek( hFile, 0L, fileOriginEnd );	// move to eof
	EXIT_AND_DISPLAY_IF_ERROR( Error )
	
	Error = 0;
	lngFileLength = FileTell( hFile, NULL, &Error );
	iRecordCount = (int) ( lngFileLength / (long)p_iRecordLength ) ;	// calc number of records

	EXIT_AND_DISPLAY_IF_ERROR( Error )
	
Exit:
	Error = FileClose( hFile );
	DISPLAY_IF_ERROR( Error )
	
	return( iRecordCount );	// return number of records
}

int SerialOpen( void )
{
	SerSettingsType		ComSettings;
	Err	Error = SERIAL_SUCCESS;

	Error = SysLibFind( (char*)szSERIAL_LIBRARY, &s_SLRefNum );
	EXIT_AND_DISPLAY_IF_ERROR( Error )

	CLEAR_ERROR

	Error = SerGetSettings( s_SLRefNum, &ComSettings );
	EXIT_AND_DISPLAY_IF_ERROR( Error )

	//SerGetSettings function is a piece of sh*t and does not return correct value, f$%#!!!
	ComSettings.baudRate = BAUD;
	ComSettings.flags = 	serSettingsFlagBitsPerChar8 |
						serSettingsFlagBitsPerCharM |
						serSettingsFlagStopBits1 |
						serSettingsFlagRTSAutoM |
						serSettingsFlagCTSAutoM ;
	ComSettings.ctsTimeout = 20;

//serSettingsFlagParityEvenM	
//serSettingsFlagXonXoffM not implemented, damn!
//serSettingsFlagParityOnM
//serSettingsFlagStopBitsM
	
	Error = SerSetSettings( s_SLRefNum, &ComSettings );
	EXIT_AND_DISPLAY_IF_ERROR( Error )

	Error = SerOpen( s_SLRefNum, COM1, ComSettings.baudRate );
	EXIT_AND_DISPLAY_IF_ERROR( Error )
	
//	Error = SerSetReceiveBuffer( s_SLRefNum, s_szReceiveBuffer, DEFAULT_RECEIVE_BUFFER_SIZE );
//	EXIT_AND_DISPLAY_IF_ERROR( Error )
	
	CLEAR_ERROR

Exit:
	return( Error );
}

int SerialClose( void )
{
	Err	Error = SERIAL_SUCCESS;

	CLEAR_ERROR

//	Error = SerSetReceiveBuffer( s_SLRefNum, NULL, 0L );
//	EXIT_AND_DISPLAY_IF_ERROR( Error )

	Error = SerClose( s_SLRefNum );
	EXIT_AND_DISPLAY_IF_ERROR( Error )

Exit:
	return( Error );
}

void DisplayCommError( Err Error )
{
	SndPlaySystemSound( (SndSysBeepType)sndError );
	switch( Error )
	{
	case SERIAL_TIMEOUT:
		MsgBoxError( "Comm Error: Receive TimeOut Error in SerialReceive" );
		break;
	case sysErrLibNotFound:
		MsgBoxError( "Library Error: library not found" );
		break;
	case serErrBadParam:
		MsgBoxError( "Comm Error: bad parameter" );
		break;
	case serErrBadPort:
		MsgBoxError( "Comm Error: bad port" );
		break;
	case serErrNoMem:
		MsgBoxError( "Comm Error: insufficient memory" );
		break;
	case serErrBadConnID:
		MsgBoxError( "Comm Error: bad connection id" );
		break;
	case serErrTimeOut:
		MsgBoxError( "Comm Error: timeout Error" );
		break;
	case serErrLineErr:
		MsgBoxError( "Comm Error: line Error" );
		break;
	case serErrAlreadyOpen:
		MsgBoxError( "Comm Error: port already open" );
		break;
	case serErrStillOpen:
		MsgBoxError( "Comm Error: port still open" );
		break;
	case serErrNotOpen:
		MsgBoxError( "Comm Error: port not open" );
		break;
	case serErrNotSupported:
		MsgBoxError( "Comm Error: unsupported feature" );
		break;
	case serLineErrorParity:
		MsgBoxError( "Comm Error: parity Error" );
		break;
	case serLineErrorHWOverrun:
		MsgBoxError( "Comm Error: hardware overrun" );
		break;
	case serLineErrorFraming:
		MsgBoxError( "Comm Error: framing Error" );
		break;
	case serLineErrorBreak:
		MsgBoxError( "Comm Error: break Error" );
		break;
	case serLineErrorHShake:
		MsgBoxError( "Comm Error: hardware handshake Error" );
		break;
	case serLineErrorSWOverrun	:
		MsgBoxError( "Comm Error: software overrun" );
		break;
	case fileErrMemError:
		MsgBoxError( "Out of memory" );
		break;
	case fileErrInvalidParam:
		MsgBoxError( "Invalid parameter passed." );
		break;
	case fileErrCorruptFile:
		MsgBoxError( "File is corrupted/invalid/not a stream file." );
		break;
	case fileErrNotFound:
		MsgBoxError( "File not found." );
		break;
	case fileErrTypeCreatorMismatch:
		MsgBoxError( "File type/creator does not match expected." );
		break;
	case fileErrReplaceError:
		MsgBoxError( "Unable to replace existing szData." );
		break;
	case fileErrCreateError:
		MsgBoxError( "Couldn't create a new file." );
		break;
	case fileErrOpenError:
		MsgBoxError( "Unable to open file." );
		break;
	case fileErrInUse:
		MsgBoxError( "Unable to open/delete file, already in use." );
		break;
	case fileErrReadOnly:
		MsgBoxError( "Couldn't open in write mode, already in read-only mode." );
		break;
	case fileErrInvalidDescriptor:
		MsgBoxError( "Invalid file descriptor." );
		break;
	case fileErrCloseError:
		MsgBoxError( "Error closing file." );
		break;
	case fileErrOutOfBounds:
		MsgBoxError( "Out of bounds on file." );
		break;
	case fileErrPermissionDenied:
		MsgBoxError( "File is open for read-only access." );
		break;
	case fileErrIOError:
		MsgBoxError( "General I/O Error." );
		break;
	case fileErrEOF:
		MsgBoxError( "End of file found." );
		break;
	case fileErrNotStream:
		MsgBoxError( "File is not a stream." );
		break;
	default:
		MsgBoxError( "Unknown Error" );
		break;
	}
}

void DisplayFileError( Err Error )
{
	SndPlaySystemSound( (SndSysBeepType)sndError );
	switch( Error )
	{
	case fileErrMemError:
		MsgBoxError( "Out of memory" );
		break;
	case fileErrInvalidParam:
		MsgBoxError( "Invalid parameter passed." );
		break;
	case fileErrCorruptFile:
		MsgBoxError( "File is corrupted/invalid/not a stream file." );
		break;
	case fileErrNotFound:
		MsgBoxError( "File not found." );
		break;
	case fileErrTypeCreatorMismatch:
		MsgBoxError( "File type/creator does not match expected." );
		break;
	case fileErrReplaceError:
		MsgBoxError( "Unable to replace existing szData." );
		break;
	case fileErrCreateError:
		MsgBoxError( "Couldn't create a new file." );
		break;
	case fileErrOpenError:
		MsgBoxError( "Unable to open file." );
		break;
	case fileErrInUse:
		MsgBoxError( "Unable to open/delete file, already in use." );
		break;
	case fileErrReadOnly:
		MsgBoxError( "Couldn't open in write mode, already in read-only mode." );
		break;
	case fileErrInvalidDescriptor:
		MsgBoxError( "Invalid file descriptor." );
		break;
	case fileErrCloseError:
		MsgBoxError( "Error closing file." );
		break;
	case fileErrOutOfBounds:
		MsgBoxError( "Out of bounds on file." );
		break;
	case fileErrPermissionDenied:
		MsgBoxError( "File is open for read-only access." );
		break;
	case fileErrIOError:
		MsgBoxError( "General I/O Error." );
		break;
	case fileErrEOF:
		MsgBoxError( "End of file found." );
		break;
	case fileErrNotStream:
		MsgBoxError( "File is not a stream." );
		break;
	default:
		MsgBoxError( "Unknown Error" );
		break;
	}
}


// copy a string with limit and zero fill
//num = total # of bytes in destination, including '\0' terminator
char *stzcpy(char *dst, char *src, int num)
{
	int i;
	char *orgdst;

	orgdst = dst;
	for (i=0 ; i < num-1 && *src != '\0' ; i++)
	{
		*dst++=*src++;
	}
	for ( ; i < num ; i++)
	{
		*dst++='\0';
	}
	return(orgdst);
}

