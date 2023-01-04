#include <Pilot.h>
#include <FileStream.h>

#include "PDIFile.h"

#define  		MAX_REC_LEN 			255
#define 		BLOCKSIZE 				1024

static const Char szSUCCESS[] 					= "success";
static const Char szERROR_NOTFOUND[] 			= "Record Not found";
static const Char szERROR_INVALIDPARAM[] 		="Invalid Parameter";
static const Char szERROR_FAILIURE[] 			= "Failiure";
static const Char szfileErrMemError[] 				= "Out of memory";
static const Char szfileErrInvalidParam[] 			= "Invalid parameter passed.";
static const Char szfileErrCorruptFile[] 				= "File is corrupted/invalid/not a stream file.";
static const Char szfileErrNotFound[] 				= "File not found.";
static const Char szfileErrTypeCreatorMismatch[] 	= "File type/creator does not match expected.";
static const Char szfileErrReplaceError[] 			= "Unable to replace existing szData.";
static const Char szfileErrCreateError[] 				= "Couldn't create a new file.";
static const Char szfileErrOpenError[] 				= "Unable to open file.";
static const Char szfileErrInUse[] 					= "Unable to open/delete file, already in use.";
static const Char szfileErrReadOnly[] 				= "Couldn't open in write mode, already in read-only mode.";
static const Char szfileErrInvalidDescriptor[] 		= "Invalid file descriptor.";
static const Char szfileErrCloseError[] 				= "Error closing file.";
static const Char szfileErrOutOfBounds[] 			= "Out of bounds on file.";
static const Char szfileErrPermissionDenied[] 		= "File is open for read-only access.";
static const Char szfileErrIOError[] 				= "General I/O error.";
static const Char szfileErrEOF[] 					= "End of file found.";
static const Char szfileErrNotStream[] 				= "File is not a stream.";
static const Char szUnknownError[] 				= "Unknown Error";

/*====================================================================
	GetNumberOfRecords
======================================================================

Passed:		szFileName 		- name of disk file, optional drive identifier
			iRecordLength 	- record length for that file

Returned:	Number of records in file, or -1 if file does not exist.

====================================================================*/
long GetNumberOfRecords( const char * szFileName, long lngRecordLength )
{
	Err Error = ERROR_FAILIURE;
	long lngRecordCount = ERROR_FAILIURE;
	FileHand * 		hFile = NULL;
	
	hFile = FileOpen( 0, (char*)szFileName, 0, 0, fileModeReadOnly, &Error );
	if( Error == SUCCESS )
	{
		lngRecordCount = GetRecordCount( hFile, lngRecordLength );
	}
	else
	{
		return Error;
	}
	Error = FileClose( hFile );
	
	return( lngRecordCount );
}

long GetRecordCount( FileHand * hFile, long lngRecordLength )
{
	Err Error = ERROR_FAILIURE;
	long lngRecordCount = ERROR_FAILIURE;
	long lngFileLength = 0;
	
	Error = FileSeek( hFile, 0L, fileOriginEnd );
	if( Error == SUCCESS )
	{
		lngFileLength = FileTell( hFile, NULL, &Error );
		if( Error == SUCCESS )
		{
			lngRecordCount = ( lngFileLength / lngRecordLength );
		}
		else
		{
			lngRecordCount = Error;
		}
	}
	else
	{
		lngRecordCount = Error;
	}
	return( lngRecordCount );
}

/*====================================================================
	ReadRecord
======================================================================

Passed:		szFileName 		- name of disk file, optional drive identifier
			lngRecordLength 	- record length for that file
			szData			- pointer to area at which szData read will be placed
			lRecordPosition	- record number from which to read record

Returned:	Number of records in file, or -1 if lRecordPosition was invalid or the file could not be opened for any reason.

				Reads a record from a fixed record length file.

====================================================================*/
int ReadRecord( FileHand * hFile, long lngRecordLength, char * szData, long lngRecordNumber )
{
	Err Error = ERROR_FAILIURE;
	long lngFilePosition = 0L;	
	long lngRecordCount = 0;
	
	szData[0] = 0;
	
	lngRecordCount = GetRecordCount( hFile, lngRecordLength );
	if( lngRecordCount <= 0 )
		return ERROR_FAILIURE;
	
	if( 	( lngRecordNumber < 0 ) ||
		( lngRecordNumber + 1 > lngRecordCount  ) )
	{
		return ERROR_INVALIDPARAM;
	}

	lngFilePosition = lngRecordLength * lngRecordNumber;

	Error = FileSeek( hFile, lngFilePosition, fileOriginBeginning );
	if( Error == SUCCESS )
	{
		FileRead( hFile, szData, lngRecordLength, 1, &Error );
	}
	return( Error );
}	

/*====================================================================
	UpdateRecord
======================================================================

Passed:		szFileName 		- name of disk file, optional drive identifier
			lngRecordLength 	- record length for that file
			szData			- pointer to area at which holds szData
			iRecordPosition	- record number to update

Returned:	Number of records in file, or -1 if iRecordPosition was invalid or the file could not be opened for any reason.

				Updates a record in a fixed record length file.

====================================================================*/
int UpdateRecord( FileHand * hFile, long lngRecordLength, char *szData, long lngRecordNumber )
{
	Err Error = ERROR_FAILIURE;
	long lngFilePosition = 0L;	
	long lngRecordCount = 0;	

	lngRecordCount = GetRecordCount( hFile, lngRecordLength );
	if( lngRecordCount <= 0 )
		return ERROR_FAILIURE;
	
	if( 	( lngRecordNumber < 0 ) ||
		( lngRecordNumber + 1 > lngRecordCount  ) )
	{
		return ERROR_INVALIDPARAM;
	}
	
	lngFilePosition = lngRecordLength * lngRecordNumber;
	
	Error = FileSeek( hFile, lngFilePosition, fileOriginBeginning );
	if( Error == SUCCESS )
	{
		FileWrite( hFile, szData, lngRecordLength, 1, &Error );
	}
	return( Error );
}


/*====================================================================
	AppendRecord
======================================================================

Passed:		szFileName 		- name of disk file, optional drive identifier
			lngRecordLength 	- record length for that file
			szData			- szData to be appended to file

Returned:	Number of records in file, or -1 if file could not be opened for any reason.

				Appends a record to a fixed record length file.

====================================================================*/
int AppendRecord( FileHand * hFile, long lngRecordLength, char * szData )
{
	Err Error = ERROR_FAILIURE;
	Error = FileSeek( hFile, 0L, fileOriginEnd );
	if( Error == SUCCESS )
	{
		FileWrite( hFile, szData, lngRecordLength, 1, &Error );
	}
	return( Error );
}	

/*====================================================================
	DeleteRecord
======================================================================

Passed:		szFileName 		- name of disk file, optional drive identifier
			lngRecordLength 	- record length for that file
			lngRecordNumber	- number of record to be deleted

Returned:	Number of records in file, or -1 if lngRecordNumber was invalid or the file could not be opened for any reason.

				Inserts a record to a fixed record length file.

====================================================================*/
int DeleteRecord( FileHand * hFile, long lngRecordLength, long lngRecordNumber )
{
	Err Error = ERROR_FAILIURE;
	
	long 	lngRecordCount		= 0;			
	long 	lngFilePostion			= 0;
	long 	lngBlockStart			= 0;			// block start position
	long 	lngBytesToMove		= 0;			// total number of bytes to move
	long 	lngBlocksToMove		= 0;			// block start position
	long 	lngPartialBlockSize		= 0;			// size of any parial block
	long		lngFileLength			= 0;			// count of bytes in file
	char 	cBuffer				[ BLOCKSIZE+10 ];

	Error = FileSeek( hFile, 0L, fileOriginEnd );
	RETURN_IF_ERROR
	
	lngFileLength = FileTell( hFile, NULL, &Error );
	RETURN_IF_ERROR
	
	lngRecordCount = ( lngFileLength / lngRecordLength );
	
	if( lngRecordCount <= 0 )
		return ERROR_FAILIURE;
	
	if( 	( lngRecordNumber < 0 ) ||
		( lngRecordNumber + 1 > lngRecordCount  ) )
	{
		return ERROR_INVALIDPARAM;
	}
	
	lngBlockStart		= ( lngRecordNumber + 1 ) * lngRecordLength;	// start of block to move
	lngBytesToMove	= lngFileLength - lngBlockStart;			// calc number of bytes to move
	lngBlocksToMove	= lngBytesToMove / BLOCKSIZE; 			// number of complete blocks to move
	lngPartialBlockSize	= lngBytesToMove % BLOCKSIZE;			// size of any incomplete block

	while( lngBytesToMove && lngBlocksToMove )								// while more blocks to move
	{
		Error = FileSeek( hFile, lngBlockStart, fileOriginBeginning );
		FileRead( hFile, cBuffer, BLOCKSIZE, 1, &Error ); // read partial block
		lngFilePostion = lngBlockStart - lngRecordLength;
		Error = FileSeek( hFile, lngFilePostion, fileOriginBeginning );// move to new start of block
		FileWrite( hFile, cBuffer, BLOCKSIZE, 1, &Error );	// write partial block
		lngBlockStart += BLOCKSIZE;									// move up a block
		lngBlocksToMove--;										// decrement blocks left
	}

	Error = FileSeek( hFile, lngBlockStart, fileOriginBeginning );				// move to start of block
	FileRead( hFile, cBuffer, lngPartialBlockSize, 1, &Error ); // read partial block
	lngFilePostion = lngBlockStart - lngRecordLength;
	Error = FileSeek( hFile, lngFilePostion, fileOriginBeginning );// move to new start of block
	FileWrite( hFile, cBuffer, lngPartialBlockSize, 1, &Error );	// write partial block

	Error = FileTruncate( hFile, lngFileLength - lngRecordLength );
	
	return( Error );
}

/*====================================================================
	SearchRecord
======================================================================

Passed:		szFileName 		- name of disk file, optional drive identifier
			lngRecordLength 	- record length for that file
			szData			- szData to be searched for
			iKeyStart			- start of search key within record (starting at 0)
			iKeyLength		- length of search key
			iRecordStart		- record within file at which to commence search (starting at 1)
			iRecordEnd		- record within file at which to terminate search, if unsuccessful

Returned:	-1 if file could not be opened for any reason,
				0 if a match was not found within the specified range,
				or the record number at which the match was found.

Searches all or part of a fixed record length file for all or part of a szData pattern.
====================================================================*/
long SearchRecord( FileHand * hFile, long lngRecordLength, char * szData, long lngDataStart, 
					long lngDataLength, long lngRecordStart, long lngRecordEnd )
{
	Err Error = ERROR_FAILIURE;
	long 	lngRecordNumber 		= 0;
	long 	lngFilePosition 		= 0L;	
	char 	cBuffer				[ BLOCKSIZE+1 ];

	long lngRecordCount = GetRecordCount( hFile, lngRecordLength );
	if( lngRecordCount <= 0 )
		return ERROR_FAILIURE;
	
	lngFilePosition = lngRecordLength * lngRecordStart;
	Error = FileSeek( hFile, lngFilePosition, fileOriginBeginning );
	RETURN_IF_ERROR
	
	for( lngRecordNumber=lngRecordStart; lngRecordNumber<lngRecordEnd; lngRecordNumber++ ) 	// within the specified range
	{
		ZEROOUT ( cBuffer );
		FileRead( hFile, cBuffer, lngRecordLength, 1, &Error );
		RETURN_IF_ERROR
		if( StrNCompare( cBuffer + lngDataStart, szData, (DWord)lngDataLength ) == 0 )
		{
			return( lngRecordNumber );
		}
	}	
	return( ERROR_NOTFOUND );
}

/*=============================================================================
   BinarySearchRecord
===============================================================================

Passed:
   char  * szFileName  	-     pointer to string containing szFileName;
   long   lngRecordLength     	-     record length in the file (MAX 255);
   long   lngDataLength      		-     length of key to search on (<= lngRecordLength);
   void  *szData       	-     pointer to szData on which to search (any type);

Returned:
   long               -     1..X      -  record number (key found);
                          -(1..X)     -  position where key record would reside
                                          if present.

   This routine searches the file to see if a record of exists with the key
   present.  It is a binary search, so naturally it assumes that the file has
   been presorted using the key passed to this function.

=============================================================================*/
long BinarySearchRecord( FileHand * hFile, long lngRecordLength, long lngDataLength, char * szData )
{
	Err Error = ERROR_FAILIURE;
	
	long		lngUpperBound	= 0L;			// Upper range for binary chop
	long		lngLowerBound	= 0L;			// Lower range for binary chop
	long		lngCurrentRecord	= 0L;			// Current record number

	lngLowerBound = 0L;		// Set up low/high ranges for chop
	lngUpperBound = GetRecordCount( hFile, lngRecordLength );
	if( lngUpperBound <= 0L  )
		return ERROR_FAILIURE;
	
	// HOME IN ON THE EXACT OR CLOSEST MATCH
	while( lngLowerBound < lngUpperBound )
	{
		lngCurrentRecord = ( ( lngLowerBound + lngUpperBound ) / 2L );
		if( CompareRecord( hFile, lngRecordLength, szData, 0, lngDataLength, lngCurrentRecord ) < 0 ) 
			lngLowerBound = lngCurrentRecord + 1;					// searching too low
		else
			lngUpperBound = lngCurrentRecord;						// searching too high
	}
	
	if( CompareRecord( hFile, lngRecordLength, szData, 0, lngDataLength, lngUpperBound ) == 0 ) 
		return( lngUpperBound );
	else
		return( ERROR_NOTFOUND );
}

/*=============================================================================
   BinarySearchRecordOffset
===============================================================================

Passed:
   char  * szFileName   	-     pointer to string containing szFileName;
   long   lngRecordLength	-     record length in the file (MAX 255);
   long   lngDataStart    		-     offset in the record where the key starts (0-based)
   long   lngDataLength     	-     length of key to search on (<= lngRecordLength);
   void  *szData      		-     pointer to szData on which to search (any type);

Returned:
   long               -     1..X      -  record number (key found);
                          -(1..X)     -  position where key record would reside
                                          if present.

   This routine searches the file to see if a record of exists with the key
   present.  It is a binary search, so naturally it assumes that the file has
   been presorted using the key passed to this function.

=============================================================================*/
long BinarySearchRecordOffset( FileHand * hFile, long lngRecordLength, long lngDataStart, long lngDataLength, char * szData )
{
	Err Error = ERROR_FAILIURE;

	long		lngUpperBound	= 0L;			// Upper range for binary chop
	long		lngLowerBound	= 0L;			// Lower range for binary chop
	long		lngCurrentRecord	= 0L;			// Current record number

	lngLowerBound = 0L;		// Set up low/high ranges for chop
	lngUpperBound = GetRecordCount( hFile, lngRecordLength );
	if( lngUpperBound <= 0L  )
		return ERROR_FAILIURE;

	// HOME IN ON THE EXACT OR CLOSEST MATCH
	while( lngLowerBound < lngUpperBound )
	{
		lngCurrentRecord = ( ( lngLowerBound + lngUpperBound ) / 2L );
		if( CompareRecord( hFile, lngRecordLength, szData, lngDataStart, lngDataLength, lngCurrentRecord ) < 0 ) 
			lngLowerBound = lngCurrentRecord + 1L;		// searching too low
		else
			lngUpperBound = lngCurrentRecord;			// searching too high
	}
	
	if( CompareRecord( hFile, lngRecordLength, szData, lngDataStart, lngDataLength, lngUpperBound ) == 0 ) 
		return( lngUpperBound );
	else
		return( ERROR_NOTFOUND );
}


/*=============================================================================
   CompareRecord
===============================================================================

Passed:
   FILE  *hFile	-     pointer into file being searched;
   long   rec_len		-     length of each record in the file;
   void  *key		-     pointer to key to be compared;
   long   key_start	-     offset of which key value starts within record (0-based)
   long   lngDataLength	-     length of valid szData in bytes;
   long  rec_num	-     record number to compare.

Returns:
   long            -     < 0	-     key too high;
                        =	-     key found;
                        > 0	-     key too low.

   This function simply reads the specified record and then compares the 
   key field of that record with the key passed...

=============================================================================*/
long CompareRecord( FileHand * hFile, long lngRecordLength, void * pKey, long lngDataStart, long lngDataLength, long iRecordNumber )
{
	Err Error = ERROR_FAILIURE;
	long lngFilePosition = 0L;	
	char cRecord[ MAX_REC_LEN ];
	cRecord[0] = 0;
	
	lngFilePosition = ( ( iRecordNumber * lngRecordLength ) + lngDataStart );
	Error = FileSeek( hFile, lngFilePosition, fileOriginBeginning );
	if( Error == SUCCESS )
	{
		ZEROOUT( cRecord );
		FileRead( hFile, cRecord, 1, lngDataLength, &Error );
	}
	return ( StrNCompare( cRecord, pKey, (DWord)lngDataLength ) );
}

void DisplayFileError( Err Error )
{
	SndPlaySystemSound( sndError );
	switch( Error )
	{
		case ERROR_NOTFOUND:
			MsgBoxError( szERROR_NOTFOUND );
			break;
		case ERROR_INVALIDPARAM:
			MsgBoxError( szERROR_INVALIDPARAM );
			break;
		case ERROR_FAILIURE:
			MsgBoxError( szERROR_FAILIURE );
			break;
		case fileErrMemError:
			MsgBoxError( szfileErrMemError );
			break;
		case fileErrInvalidParam:
			MsgBoxError( szfileErrInvalidParam );
			break;
		case fileErrCorruptFile:
			MsgBoxError( szfileErrCorruptFile );
			break;
		case fileErrNotFound:
			MsgBoxError( szfileErrNotFound );
			break;
		case fileErrTypeCreatorMismatch:
			MsgBoxError( szfileErrTypeCreatorMismatch );
			break;
		case fileErrReplaceError:
			MsgBoxError( szfileErrReplaceError );
			break;
		case fileErrCreateError:
			MsgBoxError( szfileErrCreateError );
			break;
		case fileErrOpenError:
			MsgBoxError( szfileErrOpenError );
			break;
		case fileErrInUse:
			MsgBoxError( szfileErrInUse );
			break;
		case fileErrReadOnly:
			MsgBoxError( szfileErrReadOnly );
			break;
		case fileErrInvalidDescriptor:
			MsgBoxError( szfileErrInvalidDescriptor );
			break;
		case fileErrCloseError:
			MsgBoxError( szfileErrCloseError );
			break;
		case fileErrOutOfBounds:
			MsgBoxError( szfileErrOutOfBounds );
			break;
		case fileErrPermissionDenied:
			MsgBoxError( szfileErrPermissionDenied );
			break;
		case fileErrIOError:
			MsgBoxError( szfileErrIOError );
			break;
		case fileErrEOF:
			MsgBoxError( szfileErrEOF );
			break;
		case fileErrNotStream:
			MsgBoxError( szfileErrNotStream );
			break;
		default:
			MsgBoxError( szUnknownError );
			break;
	}
}

const char * GetFileError( Err Error )
{
	switch( Error )
	{
		case SUCCESS:
			return szSUCCESS;
		case ERROR_NOTFOUND:
			return  szERROR_NOTFOUND;
			break;
		case ERROR_INVALIDPARAM:
			return  szERROR_INVALIDPARAM;
			break;
		case ERROR_FAILIURE:
			return  szERROR_FAILIURE;
			break;
		case fileErrMemError:
			return  szfileErrMemError;
			break;
		case fileErrInvalidParam:
			return  szfileErrInvalidParam;
			break;
		case fileErrCorruptFile:
			return  szfileErrCorruptFile;
			break;
		case fileErrNotFound:
			return  szfileErrNotFound;
			break;
		case fileErrTypeCreatorMismatch:
			return  szfileErrTypeCreatorMismatch;
			break;
		case fileErrReplaceError:
			return  szfileErrReplaceError;
			break;
		case fileErrCreateError:
			return  szfileErrCreateError;
			break;
		case fileErrOpenError:
			return  szfileErrOpenError;
			break;
		case fileErrInUse:
			return  szfileErrInUse;
			break;
		case fileErrReadOnly:
			return  szfileErrReadOnly;
			break;
		case fileErrInvalidDescriptor:
			return  szfileErrInvalidDescriptor;
			break;
		case fileErrCloseError:
			return  szfileErrCloseError;
			break;
		case fileErrOutOfBounds:
			return  szfileErrOutOfBounds;
			break;
		case fileErrPermissionDenied:
			return  szfileErrPermissionDenied;
			break;
		case fileErrIOError:
			return  szfileErrIOError;
			break;
		case fileErrEOF:
			return  szfileErrEOF;
			break;
		case fileErrNotStream:
			return  szfileErrNotStream;
			break;
		default:
			return  szUnknownError;
			break;
	}
}

