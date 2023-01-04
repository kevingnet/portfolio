#include "InputValidatorValidateItemString.h"

namespace InputValidator {

int InputValidatorValidateItemString::Validate() const
{
#ifdef DEBUG
	iv_log_msg( LOG_DEBUG, "InputValidatorValidateItemString Validate: %s", m_Name.c_str() );
#endif
	SetLastValidationFailure(VF_SUCCESS);
	int res = InputValidatorValidateItem::Validate();
	if( res != IV_VALIDATION_PASSED )
		return res;
	
	m_bufRepeats[0] = 0;
	m_RepeatsIndex = 0;

	res = (*this.*m_pValidateItemFunction)();

	if ( res == IV_VALIDATION_PASSED ) {
#ifdef DEBUG
iv_log_msg( LOG_DEBUG, "InputValidatorValidateItem: Validation Passed" );
	} 
	else 
	{
iv_log_msg( LOG_DEBUG, "InputValidatorValidateItem: Validation ***FAILED!!!***" );
#endif
	}
	return res;
}

//std C function
//#define IFISALNUM( c ) if(isalnum(c))

//slightly faster...
#define IFISALNUM( c ) if(((c>='0')&&(c<='9'))||((c>='a')&&(c<='z'))||((c>='A')&&(c<='Z')))

//only alpha numeric, no space or multiline :(
int InputValidatorValidateItemString::ValidateItemNoVec() const
{
	for( m_i=0; m_i<m_size; m_i++ ) 
	{
		m_c = m_value[m_i];
		if( m_c == 0 ) break;
		m_FoundExtraCharacter = true;
		IFISALNUM( m_c ) m_FoundExtraCharacter = false;
		if( m_FoundExtraCharacter == true ) 
		{
#ifdef ProcessStats
			StatsValidationsFailed_InvalidChar++;
#endif
			m_LastValidationFailure = VF_InvalidChar;
			return IV_VALIDATION_FAILED;
		}
	}
	return IV_VALIDATION_PASSED;
}

static inline size_t strchar2(const char * s, char c, unsigned int len )
{
int d0;
register int __res;
__asm__ __volatile__(
"inc %%cx\n\t"	//increment counter CX, cause' it's zero based, duh!
"cld\n\t"		//clear direction flag
"repne\n\t"		//repeat until found
"scasb\n\t"		//scan ...
:"=c" (__res), "=&D" (d0)	//output: in result, from counter CX
:"1" (s),"a" (c), "c"(len));	//input: string to ES:DI char to AX and len to CX
return __res;
}

//alphanumeric, plus extra chars, but no repeating chars, probably no space or multiline
int InputValidatorValidateItemString::ValidateItemVecNoReps() const
{
	for( m_i=0; m_i<m_size; m_i++ ) 
	{
		m_c = m_value[m_i];
		if( m_c == 0 ) break;
		m_FoundExtraCharacter = true;
		IFISALNUM( m_c ) m_FoundExtraCharacter = false;
		if( m_FoundExtraCharacter == true ) 
		{
			if( strchar2(static_cast<const char*>(m_pValidExtraChars), m_c, m_ValidExtraCharsLength) != 0 )
			{
				if( m_c == m_value[m_i-1] )
				{
#ifdef ProcessStats
					StatsValidationsFailed_InvalidChar++;
#endif
					m_LastValidationFailure = VF_InvalidChar;
					return IV_VALIDATION_FAILED;
				}
			}
			else
			{
#ifdef ProcessStats
				StatsValidationsFailed_InvalidChar++;
#endif
				m_LastValidationFailure = VF_InvalidChar;
				return IV_VALIDATION_FAILED;
			}
		} 
	}
	return IV_VALIDATION_PASSED;
}

//alphanumeric, plus extra chars, with repeating chars, definitively space and/or multiline
int InputValidatorValidateItemString::ValidateItemVecReps() const
{
	for( m_i=0; m_i<m_size; m_i++ ) 
	{
		m_c = m_value[m_i];
		if( m_c == 0 ) break;
		m_FoundExtraCharacter = true;
		IFISALNUM( m_c ) m_FoundExtraCharacter = false;
		if( m_FoundExtraCharacter == true ) 
		{
			if( strchar2(static_cast<const char*>(m_pValidExtraChars), m_c, m_ValidExtraCharsLength) != 0 )
			{
				if( m_c == m_value[m_i-1] )
				{
					m_bufRepeats[m_RepeatsIndex++] = m_c;
					m_bufRepeats[m_RepeatsIndex] = 0;
				}
			} 
			else 
			{
#ifdef ProcessStats
				StatsValidationsFailed_InvalidChar++;
#endif
				return IV_VALIDATION_FAILED;
				m_LastValidationFailure = VF_InvalidChar;
			}
		} 
	}
	//check allowed repeating characters
	for( m_i=0; m_i<m_RepeatsIndex; m_i++ ) 
	{
		if( m_bufRepeats[m_i] == 0 ) break;
		if( strchar2(static_cast<const char*>(m_pAllowedRepeatingChars), m_bufRepeats[m_i], m_AllowedRepeatingCharsLength) == 0 )
		{
#ifdef ProcessStats
			StatsValidationsFailed_InvalidRepeatChar++;
#endif
			m_LastValidationFailure = VF_InvalidRepeatChar;
			return IV_VALIDATION_FAILED;
		}
	}
	return IV_VALIDATION_PASSED;
}

void InputValidatorValidateItemString::Init()
{
	InputValidatorValidateItem::Init();
	string sTmp;
	if( m_IsMultiline == true ) {
		sTmp += "\n\r";
	}
	if( m_AllowSpaces == true ) {
		sTmp = " \t";
	}
	m_ValidExtraChars += sTmp;
	m_AllowedRepeatingChars += sTmp;

	m_pValidateItemFunction = &InputValidatorValidateItemString::ValidateItemNoVec;

	m_pValidExtraChars = 0;
	m_ValidExtraCharsLength = 0;
	m_pAllowedRepeatingChars = 0;
	m_AllowedRepeatingCharsLength = 0;

	if( m_ValidExtraChars.size() != 0 ) {
		m_pValidExtraChars = m_ValidExtraChars.c_str();
		m_ValidExtraCharsLength = static_cast<unsigned int>(m_ValidExtraChars.size());
		if( m_AllowedRepeatingChars.size() == 0 ) {
			m_pValidateItemFunction = &InputValidatorValidateItemString::ValidateItemVecNoReps;
		} else {
			m_pValidateItemFunction = &InputValidatorValidateItemString::ValidateItemVecReps;
			m_pAllowedRepeatingChars = m_AllowedRepeatingChars.c_str();
			m_AllowedRepeatingCharsLength = static_cast<unsigned int>(m_AllowedRepeatingChars.size());
		}
	}
}

#ifdef ProcessStats
void InputValidatorValidateItemString::SaveStats(ofstream& ofs) const
{
	InputValidatorValidateItem::SaveStats(ofs);
	ofs << "2:"<< StatsValidationsFailed_InvalidChar << "3: "
	<< StatsValidationsFailed_InvalidRepeatChar << " ";
	ResetStats();
}
#endif

InputValidatorValidateItemString::~InputValidatorValidateItemString()
{
#ifdef FNCDEBUG
LOGFNC( "InputValidatorValidateItemString ~~~DTOR [%p]" )
#endif
}

InputValidatorValidateItemString::InputValidatorValidateItemString (
	uint ID,
	const string& Name, 
	const string& Type,
	const string& ValidExtraChars,
	const string& AllowedRepeatingChars,
	const string& ValidSequences,
	uint MinSize, 
	uint MaxSize, 
	bool AllowSpaces,
	bool IsMultiline
)
:InputValidatorValidateItem(ID,Name,Type,MinSize,MaxSize),
m_ValidExtraChars(ValidExtraChars),
m_AllowedRepeatingChars(AllowedRepeatingChars),
m_ValidSequences(ValidSequences),
m_AllowSpaces(AllowSpaces),
m_IsMultiline(IsMultiline),
m_pValidateItemFunction(0),
m_pValidExtraChars(0),
m_ValidExtraCharsLength(0),
m_pAllowedRepeatingChars(0),
m_AllowedRepeatingCharsLength(0)
{
#ifdef FNCDEBUG
LOGFNCWNAME( "InputValidatorValidateItemString CTOR for %s [%p]" )
#endif
	SetClassType( CT_VI_STR );
}

void InputValidatorValidateItemString::Load( ifstream& ifs )
{
#ifdef FNCDEBUG
LOGFNC( "InputValidatorValidateItemString Load [%p]" )
#endif
	InputValidatorValidateItem::Load(ifs);
	m_ValidExtraChars = ReadString( ifs );
	m_AllowedRepeatingChars = ReadString( ifs );
	m_ValidSequences = ReadString( ifs );
	m_AllowSpaces = static_cast<bool>(ReadInt( ifs ));
	m_IsMultiline = static_cast<bool>(ReadInt( ifs ));
}

InputValidatorValidateItemString::InputValidatorValidateItemString()
:InputValidatorValidateItem(),
m_ValidExtraChars(),
m_AllowedRepeatingChars(),
m_ValidSequences(),
m_AllowSpaces(false),
m_IsMultiline(false),
m_pValidateItemFunction(0),
m_pValidExtraChars(0),
m_ValidExtraCharsLength(0),
m_pAllowedRepeatingChars(0),
m_AllowedRepeatingCharsLength(0)
{
#ifdef FNCDEBUG
LOGFNC( "InputValidatorValidateItemString ***DEFAULT*** CTOR [%p]" )
#endif
	SetClassType( CT_VI_STR );
}

#ifdef PCREBinGen
InputValidatorValidateItemString::InputValidatorValidateItemString ( const InputValidatorValidateItemString& item )
{
#ifdef FNCDEBUG
LOGFNC( "InputValidatorValidateItemString Copy CTOR [%p]" )
#endif
	*this = item;
}

InputValidatorValidateItemString & InputValidatorValidateItemString::operator= (const InputValidatorValidateItemString & item)
{
#ifdef FNCDEBUG
LOGFNC( "InputValidatorValidateItemString operator= [%p]" )
#endif
	InputValidatorValidateItem::operator=(dynamic_cast<const InputValidatorValidateItem&>(item));
	m_ValidExtraChars = item.m_ValidExtraChars;
	m_ValidSequences = item.m_ValidSequences;
	m_AllowedRepeatingChars = item.m_AllowedRepeatingChars;
	m_AllowSpaces = item.m_AllowSpaces;
	m_IsMultiline = item.m_IsMultiline;
	SetClassType( CT_VI_STR );
	return * this;
}

void InputValidatorValidateItemString::Save( ofstream& ofs ) const
{
#ifdef FNCDEBUG
LOGFNCWNAME2( "InputValidatorValidateItemString Save for %s [%p]" )
#endif
	InputValidatorValidateItem::Save(ofs);
	WriteString( ofs, m_ValidExtraChars );
	WriteString( ofs, m_AllowedRepeatingChars );
	WriteString( ofs, m_ValidSequences );
	WriteInt( ofs, (int)m_AllowSpaces );
	WriteInt( ofs, (int)m_IsMultiline );
}
#endif

#ifdef DEBUG
bool InputValidatorValidateItemString::IsEqual( InputValidatorValidateItemString & item )
{
	if( InputValidatorValidateItem::IsEqual(item) == false )
		return false;
	if( m_AllowSpaces != item.m_AllowSpaces )
		return false;
	if( m_IsMultiline != item.m_IsMultiline )
		return false;
	if( m_ValidExtraChars != item.m_ValidExtraChars )
		return false;
	if( m_AllowedRepeatingChars != item.m_AllowedRepeatingChars )
		return false;
	if( m_ValidSequences != item.m_ValidSequences )
		return false;
	return true;
}

void InputValidatorValidateItemString::Dump()
{
	cout << "### InputValidatorValidateItemString::Dump()\n";
	cout << "m_ValidExtraChars " << m_ValidExtraChars << "\n";
	cout << "m_AllowedRepeatingChars " << m_AllowedRepeatingChars << "\n";
	cout << "m_ValidSequences " << m_ValidSequences << "\n";
	cout << "m_AllowSpaces " << m_AllowSpaces << "\n";
	cout << "m_IsMultiline " << m_IsMultiline << "\n";
	InputValidatorValidateItem::Dump();
}
#endif

}



