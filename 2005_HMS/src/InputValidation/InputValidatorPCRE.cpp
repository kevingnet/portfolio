#include "InputValidatorPCRE.h"

namespace InputValidator {

string InputValidatorPCRE::sm_PCREVersion;

const int PCRE_OPTION_COUNT = 9;

DTPCRECompilationOptions InputValidatorPCRE::sm_PCRECompilationOptions[] = {
0,	PCRE_UNGREEDY,		"UNGREEDY",
1,	PCRE_DOTALL,			"DOTALL",
2,	PCRE_NO_AUTO_CAPTURE,	"NO_AUTO_CAPTURE",
3,	PCRE_UTF8,			"UTF8",
4,	PCRE_NO_UTF8_CHECK,	"NO_UTF8_CHECK",
5,	PCRE_EXTENDED,		"EXTENDED",
6,	PCRE_ANCHORED,		"ANCHORED",
7,	PCRE_PARTIAL,			"PARTIAL",
8,	PCRE_NOTEMPTY,		"NOTEMPTY"
};

int InputValidatorPCRE::GetPCRECompileOptions( const string& options )
{
	int PCRECompileOptions = 0;
	if ( options.size() == 0) return 0;
	for( int i=0; i<PCRE_OPTION_COUNT; i++ ) {
		if(options.find(sm_PCRECompilationOptions[i].option) != string::npos)
			PCRECompileOptions |= sm_PCRECompilationOptions[i].PCREOption;
	}
	return PCRECompileOptions;
}

int InputValidatorPCRE::Validate(const char * val, int size) const
{
	if ( m_HasPCREErrors == true ) {
#ifdef DEBUG
iv_log_msg( LOG_ERR, "InputValidatorPCRE: HasPCREErrors == true - skipping PCRE check" );
#endif
#ifdef ProcessStats
		StatsValidationsErrors_PCREData++;
#endif
		m_LastValidationFailure = VF_Errors_PCREData;
		return IV_VALIDATION_ERROR;
	}

	#define NUMBER_OF_VECTORS 40
	int PCREResult;
	int ovector[NUMBER_OF_VECTORS];
	PCREResult = pcre_exec(
		static_cast<pcre*>(m_PCRECompiledData),	
		static_cast<pcre_extra*>(m_PCRECompiledExtraData),	
		val,	
		size,	
		0, // start at offset 0
		m_PCREExecOptions,
		ovector,// vector for substring information
		NUMBER_OF_VECTORS
	);
	
	//it passed with multiple matches!
	if( PCREResult > 2 ) PCREResult = 2;
	switch (PCREResult)
	{
	case 0:			//match
	case 1:			//match
#ifdef DEBUG
iv_log_msg( LOG_DEBUG, "InputValidatorValidateItem: PCRE Validation *succeeded*: PCRE =  %s", m_PCRE.c_str() );
#endif
		break;			// i know, i know
	case 2:
#ifdef DEBUG
iv_log_msg( LOG_DEBUG, "InputValidatorValidateItem: PCRE Validation *succeeded* @@@, BUT!!, there were several matches ???: PCRE =  %s", m_PCRE.c_str() );
#endif
		break;			// i know, i know
	case PCRE_ERROR_NOMATCH:
#ifdef DEBUG
iv_log_msg( LOG_DEBUG, "InputValidatorValidateItem: PCRE Validation ***FAILED***, string did not match: PCRE =  %s", m_PCRE.c_str() );
//sm_ErrorHandler.PCREError( PCREResult );
#endif
#ifdef ProcessStats
		StatsValidationsFailed_PCRENoMatch++;
#endif
		m_LastValidationFailure = VF_PCRENoMatch;
		return IV_VALIDATION_FAILED;
		break;
	case PCRE_ERROR_NULL:
	case PCRE_ERROR_BADOPTION:
	case PCRE_ERROR_BADMAGIC:
	case PCRE_ERROR_UNKNOWN_NODE:
	case PCRE_ERROR_NOMEMORY:
	case PCRE_ERROR_NOSUBSTRING:
	case PCRE_ERROR_MATCHLIMIT:
	case PCRE_ERROR_CALLOUT:
	case PCRE_ERROR_BADUTF8:
	case PCRE_ERROR_BADUTF8_OFFSET:
	case PCRE_ERROR_PARTIAL:
	case PCRE_ERROR_BADPARTIAL:
	case PCRE_ERROR_INTERNAL:
	case PCRE_ERROR_BADCOUNT:
		sm_ErrorHandler.PCREError( PCREResult );
#ifdef ProcessStats
		StatsValidationsErrors_PCRE++;
#endif
		m_LastValidationFailure = VF_Errors_PCRE;
		return IV_VALIDATION_ERROR;
		break;
	default:
		sm_ErrorHandler.PCREError( PCREResult );
#ifdef ProcessStats
		StatsValidationsErrors_PCRE++;
#endif
		m_LastValidationFailure = VF_Errors_PCRE;
		return IV_VALIDATION_ERROR;
		break;
	}
	return IV_VALIDATION_PASSED;
}

void InputValidatorPCRE::GetPCREOptions()
{
	strncpy(const_cast<char*>(sm_buffer), const_cast<char*>(m_PCRE.c_str()), m_PCRE.length());
	char * ps = sm_buffer;
	ps++;
	char * po = ps;
	while(*ps++ != 0)
		if( *ps == '\\' ) po = ps;

	if( ps == po ) { //error, but, let's be a little resilient
		//back track and assign PCRE
		ps = sm_buffer;
		ps++;
		m_CleanPCRE = ps;
	} else {
		//we found the terminating \ so zero it out
 		*po++ = 0;
		ps = sm_buffer;
		//skip first
		ps++;
		ps++;
		//we got a clean pcre string now
		m_CleanPCRE = ps;
		//get options
		while (*po != 0)
		{
			switch (*po++)
			{
			case 'f': m_PCRECompileOptions |= PCRE_FIRSTLINE; break;
			case 'i': m_PCRECompileOptions |= PCRE_CASELESS; break;
			case 'm': m_PCRECompileOptions |= PCRE_MULTILINE; break;
			case 's': m_PCRECompileOptions |= PCRE_DOTALL; break;
			case 'x': m_PCRECompileOptions |= PCRE_EXTENDED; break;
			
			case 'A': m_PCRECompileOptions |= PCRE_ANCHORED; break;
			case 'C': m_PCRECompileOptions |= PCRE_AUTO_CALLOUT;
				break;
			case 'E': m_PCRECompileOptions |= PCRE_DOLLAR_ENDONLY; break;
			case 'N': m_PCRECompileOptions |= PCRE_NO_AUTO_CAPTURE; break;
			case 'U': m_PCRECompileOptions |= PCRE_UNGREEDY; break;
			case 'X': m_PCRECompileOptions |= PCRE_EXTRA; break;
			case '8': m_PCRECompileOptions |= PCRE_UTF8; 
				//use_utf8 = 1; 
				break;
			case '?': m_PCRECompileOptions |= PCRE_NO_UTF8_CHECK; break;
			
			case 'L':
			ps = po;
			/* The '\r' test here is so that it works on Windows */
			while (*ps != '\n' && *ps != '\r' && *ps != ' ') ps++;
			*ps = 0;
			if (setlocale(LC_CTYPE, static_cast<const char*>(po)) == NULL)
				{
				//fprintf(outfile, "** Failed to set locale \"%s\"\n", pp);
				}
			m_PCRETables = pcre_maketables();
			po = ps;
			break;
			
			case '\r':                      /* So that it works in Windows */
			case '\n':
			case ' ':
			break;
			
			default:
			//fprintf(outfile, "** Unknown option '%c'\n", pp[-1]);
			break;
			}
		}
	}
}

void InputValidatorPCRE::CalcPCREOptions()
{
	m_PCREExecOptions = 0;
	//get global options
	m_PCRECompileOptions = InputValidatorPCRE::GetPCRECompileOptions( m_PCRECompilationOptions );

	if( m_PCRE[0] == '\\' ) {
		GetPCREOptions();
	} else {
		m_CleanPCRE = m_PCRE;
	}

	if( m_PCRECompileOptions & PCRE_ANCHORED )
		m_PCREExecOptions |= PCRE_ANCHORED;

	if ( m_IsCaseless == true ) {
		m_PCRECompileOptions |= PCRE_CASELESS;
	}
}

void InputValidatorPCRE::Init()
{
	CalcPCREOptions();

	m_HasPCREErrors = true;

	switch( m_PCREStatus ) {
	PCRESatus_Compiled:
	PCRESatus_FromFile:
	PCRESatus_FromObject:
		m_HasPCREErrors = false;
		//we're cool
		break;
	PCRESatus_Invalid:
	PCRESatus_BadFromFile_Compiled:
	PCRESatus_BadFromObject_Compiled:
	PCRESatus_InError: //try again
	default:
		if( CompilePCRE() == true ) {
			m_HasPCREErrors = false;
			m_PCREStatus = PCRESatus_Compiled;
		}else{
			m_PCREStatus = PCRESatus_InError;
		}	
		break;
	}
}

#ifdef ProcessStats
void InputValidatorPCRE::SaveStats(ofstream& ofs) const
{
	ofs << "6:" << StatsValidationsFailed_PCRENoMatch << " 7:"
	<< StatsValidationsErrors_PCREData << " 8:"
	<< StatsValidationsErrors_PCRE << " ";
	ResetStats();
}
#endif

InputValidatorPCRE::~InputValidatorPCRE()
{
#ifdef FNCDEBUG
LOGFNC( "InputValidatorPCRE ~~~DTOR [%p]" )
#endif
	Delete();
}

InputValidatorPCRE::InputValidatorPCRE(
	const string& PCRE,
	const string& PCRECompilationOptions,
	bool IsCaseless,
	bool SavePCRECompiledData
)
:InputValidator(),
m_CleanPCRE(),
m_PCREStatus(PCRESatus_Invalid),
m_PCRECompileOptions(),
m_PCRE(PCRE),
m_PCRECompilationOptions(PCRECompilationOptions),
m_PCREExecOptions(0),
m_PCRETables(0),
m_HasPCREErrors(true),
m_IsCaseless(IsCaseless),
m_SavePCRECompiledData(SavePCRECompiledData),
m_PCRECompiledData(0),
m_PCRECompiledDataLength(0),
m_PCRECompiledExtraData(0),
m_PCRECompiledExtraDataLength(0)
{
#ifdef FNCDEBUG
LOGFNC( "InputValidatorPCRE CTOR [%p]" )
#endif
/*	Delete();
	if( pData && Length ) {
		SetPCRECompiledData( reinterpret_cast<pcre*>(const_cast<char*>(pData)), Length );
		if( pExtraData && ExtraLength ) {
			SetPCRECompiledExtraData( reinterpret_cast<pcre_extra*>(const_cast<char*>(pExtraData)), ExtraLength );
		}
		m_PCREStatus = PCRESatus_FromObject;
	}*/
}

void InputValidatorPCRE::Load( ifstream& ifs )
{
#ifdef FNCDEBUG
LOGFNC( "InputValidatorPCRE Load [%p]" )
#endif
	Delete();
	m_PCRE = ReadString( ifs );
	m_PCRECompilationOptions = ReadString( ifs );
	m_IsCaseless = static_cast<bool>(ReadInt( ifs ));
	m_SavePCRECompiledData = static_cast<bool>(ReadInt( ifs ));
	if( m_SavePCRECompiledData == true ) {
#ifdef DEBUG
iv_log_msg( LOG_DEBUG, "InputValidatorPCRE: Loading PCRE data" );
#endif
		int size = ReadInt( ifs );
		ReadBuffer( ifs, size );
		SetPCRECompiledData( reinterpret_cast<pcre*>(const_cast<char*>(sm_buffer)), size );
		size = ReadInt( ifs );
		ReadBuffer( ifs, size );
		SetPCRECompiledExtraData( reinterpret_cast<pcre_extra*>(const_cast<char*>(sm_buffer)), size );
		m_PCREStatus = PCRESatus_FromFile;
	}
}

void InputValidatorPCRE::ReadBuffer( ifstream& ifs, int iSize )
{
	sm_buffer[0] = 0;
	if( iSize > MAX_BUF_SIZE || iSize < 0 )
		throw (ios::failure ("read error, possible file corruption"));

	if (iSize > 0)
	{
		if( iSize > MAX_BUF_SIZE )
			throw (ios::failure ("too much data to read"));

		ifs.read (static_cast<char*>(sm_buffer), iSize);
		if ( ifs.fail() != 0 )
			throw (ios::failure ("read error"));
		sm_buffer[iSize] = 0;
	}
}

void InputValidatorPCRE::SetPCRECompiledData (pcre *pData, 	int length)
{
#ifdef FNCDEBUG
	iv_log_msg( LOG_DEBUG, "InputValidatorPCRE SetPCRECompiledData length %d",length );
#endif
	if (!pData || !length)
		return;
	m_PCRECompiledDataLength = length;
	m_PCRECompiledData = reinterpret_cast<pcre*>(new char[length]);
	void * pVoid = memcpy (static_cast<void*>(m_PCRECompiledData), pData, sizeof (char) * length);
	if ( pVoid != static_cast<void*>(m_PCRECompiledData) ) {
		sm_ErrorHandler.Error( IVERROR_MEMCPY );
	}
}

void InputValidatorPCRE::SetPCRECompiledExtraData (pcre_extra *pData, int length)
{
#ifdef FNCDEBUG
	iv_log_msg( LOG_DEBUG, "InputValidatorPCRE SetPCRECompiledExtraData length %d",length );
#endif
	if (!pData || !length)
		return;
	m_PCRECompiledExtraDataLength = length;
	m_PCRECompiledExtraData = reinterpret_cast<pcre_extra*>(new char[length]);
	void * pVoid = memcpy (static_cast<void*>(m_PCRECompiledExtraData), pData, sizeof (char) * length);
	if ( pVoid != static_cast<void*>(m_PCRECompiledExtraData) ) {
		sm_ErrorHandler.Error( IVERROR_MEMCPY );
	}
}

void InputValidatorPCRE::Delete()
{
#ifdef FNCDEBUG
	iv_log_msg( LOG_DEBUG, "InputValidatorPCRE Delete [%p]",(void*)this );
#endif

	if (m_PCRECompiledDataLength > 0)
	{
		if (m_PCRECompiledData != 0)
		{
			delete m_PCRECompiledData;
			m_PCRECompiledData = 0;
		}
		m_PCRECompiledDataLength = 0;
	}
	if (m_PCRECompiledExtraDataLength > 0)
	{
		if (m_PCRECompiledExtraData != 0)
		{
			delete m_PCRECompiledExtraData;
			m_PCRECompiledExtraData = 0;
		}
		m_PCRECompiledExtraDataLength = 0;
	}
	m_PCREStatus = PCRESatus_Invalid;
}

#ifdef DEBUG
bool InputValidatorPCRE::IsEqual( InputValidatorPCRE & item )
{
	if( m_SavePCRECompiledData != item.m_SavePCRECompiledData )
		return false;
	if( m_PCRECompilationOptions != item.m_PCRECompilationOptions )
		return false;
	if( m_PCRE != item.m_PCRE )
		return false;
	if( m_IsCaseless != item.m_IsCaseless )
		return false;
	return true;
}

void InputValidatorPCRE::Dump()
{
	cout << "### InputValidatorPCRE::Dump()\n";
	cout << "m_PCRE " << m_PCRE << "\n";
	cout << "m_IsCaseless " << m_IsCaseless << "\n";
}
#endif

bool InputValidatorPCRE::CompilePCRE()
{
#ifdef FNCDEBUG
LOGFNC( "InputValidatorPCRE Compile() [%p]" )
#endif
	int       erroroffset;
	const char *error;
	int       error_code;
	int PCREResult;

	pcre * PCRECompiledData;
	int PCRECompiledDataLength;
	pcre_extra * PCRECompiledExtraData;
	int PCRECompiledExtraDataLength;

	sm_PCREVersion = pcre_version();

	PCRECompiledData = pcre_compile2 ( m_PCRE.c_str(), m_PCRECompileOptions, &error_code, &error, &erroroffset, NULL );

	if ( error_code == 0 && PCRECompiledData != NULL )
	{
		PCREResult = pcre_fullinfo ( PCRECompiledData, NULL, PCRE_INFO_SIZE, &PCRECompiledDataLength );
		if ( PCREResult < 0 )
		{
			sm_ErrorHandler.Error( IVERROR_PCRE, "Unable to get size info about PCRE" );
			return false;
		}
	
		SetPCRECompiledData(PCRECompiledData, PCRECompiledDataLength);

		PCRECompiledExtraData = pcre_study ( PCRECompiledData, 0, &error );
		if ( PCRECompiledExtraData != NULL )
		{
			PCREResult = pcre_fullinfo ( PCRECompiledData, PCRECompiledExtraData, PCRE_INFO_SIZE,
					&PCRECompiledExtraDataLength );
			if ( PCREResult >= 0 )
			{
				if ( PCRECompiledExtraDataLength == PCRECompiledDataLength )
				{
					//		cerr << "study of PCRE pattern did not return any valuable information because the length of data and extra data were the same.\n";
				}
				else if ( PCRECompiledExtraDataLength < PCRECompiledDataLength )
				{
					sm_ErrorHandler.Error( IVERROR_PCRE, "study of PCRE pattern caused an ODD result, the data length before the study was greater than after" );
					return false;
				}
				SetPCRECompiledExtraData (PCRECompiledExtraData, PCRECompiledExtraDataLength);
			}
			else
			{
				sm_ErrorHandler.Error( IVERROR_PCRE, "Unable to get size info about PCRE" );
				return false;
			}
		}
	}
	else
	{
		sm_ErrorHandler.PCREError( error_code );
		return false;
	}
	return true;
}

InputValidatorPCRE::InputValidatorPCRE()
:InputValidator(),
m_CleanPCRE(),
m_PCREStatus(PCRESatus_Invalid),
m_PCRECompileOptions(0),
m_PCRE(),
m_PCRECompilationOptions(),
m_PCREExecOptions(0),
m_PCRETables(0),
m_HasPCREErrors(true),
m_IsCaseless(true),
m_SavePCRECompiledData(false),
m_PCRECompiledData(0),
m_PCRECompiledDataLength(0),
m_PCRECompiledExtraData(0),
m_PCRECompiledExtraDataLength(0)
{
#ifdef FNCDEBUG
LOGFNC( "InputValidatorPCRE ***DEFAULT*** CTOR [%p]" )
#endif
}

#ifdef PCREBinGen
InputValidatorPCRE::InputValidatorPCRE( const InputValidatorPCRE& item )
{
#ifdef FNCDEBUG
LOGFNC( "InputValidatorPCRE Copy CTOR [%p]" )
#endif
	*this = item;
}

InputValidatorPCRE & InputValidatorPCRE::operator= (const InputValidatorPCRE & item)
{
#ifdef FNCDEBUG
LOGFNC( "InputValidatorPCRE operator= [%p]" )
#endif
	Delete();
	m_PCRE = item.m_PCRE;
	m_PCRECompilationOptions = item.m_PCRECompilationOptions;
	m_IsCaseless = item.m_IsCaseless;
	m_SavePCRECompiledData = item.m_SavePCRECompiledData;
	if( item.m_PCRECompiledData && item.m_PCRECompiledDataLength ) {
		SetPCRECompiledData( item.m_PCRECompiledData, item.m_PCRECompiledDataLength );
		if( item.m_PCRECompiledExtraData && item.m_PCRECompiledExtraDataLength ) {
			SetPCRECompiledExtraData( item.m_PCRECompiledExtraData, item.m_PCRECompiledExtraDataLength );
		}
		m_PCREStatus = PCRESatus_FromObject;
	}
	return * this;
}

void InputValidatorPCRE::Save( ofstream& ofs ) const
{
#ifdef FNCDEBUG
LOGFNC( "InputValidatorPCRE Save [%p]" )
#endif
	WriteString( ofs, m_PCRE );
	WriteString( ofs, m_PCRECompilationOptions );
	WriteInt( ofs, static_cast<int>(m_IsCaseless) );
	WriteInt( ofs, static_cast<int>(m_SavePCRECompiledData) );
	if( m_HasPCREErrors == false && m_SavePCRECompiledData == true ) 
	{
		WriteBuffer( ofs, static_cast<const char*>(m_PCRECompiledData), m_PCRECompiledDataLength );
		WriteBuffer( ofs, static_cast<const char*>(m_PCRECompiledExtraData), m_PCRECompiledExtraDataLength );
	}
}

void InputValidatorPCRE::WriteBuffer( ofstream& ofs, const char * buff, int iSize ) const
{
	ofs.write (reinterpret_cast<char*>(&iSize), sizeof (int));
	if ( ofs.fail() != 0 )
		throw (ios::failure ("write error"));

	if (iSize > 0)
	{
		if( iSize > MAX_BUF_SIZE )
			throw (ios::failure ("too much data to write"));

		ofs.write (buff, iSize);
		if ( ofs.fail() != 0 )
			throw (ios::failure ("write error"));
	}
}

#endif

}
