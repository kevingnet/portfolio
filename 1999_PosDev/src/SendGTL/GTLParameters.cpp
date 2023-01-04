// GTLParameters.cpp : implementation file
//

#include "stdafx.h"
#include "SendGTL.h"
#include "GTLParameters.h"
#include "StringFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGTLParameters

void CGTLParameters::GetFormattedParametersString( CString& szString )
{
	if( !this ) return;
	const char * pString = NULL;
	szString.Empty();

	pString = GetFormattedParameter( PARAMETERPOSITION_LOADFILENAME );
	if( !pString || !*pString ) return;
	szString = pString;
	pString = GetFormattedParameter( PARAMETERPOSITION_PORT );
	if( pString && *pString )
	{
		szString += " ";
		szString += pString;
	}
	pString = GetFormattedParameter( PARAMETERPOSITION_BPS );
	if( pString && *pString )
	{
		szString += " ";
		szString += pString;
	}
	pString = GetFormattedParameter( PARAMETERPOSITION_DATABITS );
	if( pString && *pString )
	{
		szString += " ";
		szString += pString;
	}
	pString = GetFormattedParameter( PARAMETERPOSITION_STOPBITS );
	if( pString && *pString )
	{
		szString += " ";
		szString += pString;
	}
	pString = GetFormattedParameter( PARAMETERPOSITION_PARITY );
	if( pString && *pString )
	{
		szString += " ";
		szString += pString;
	}
	pString = GetFormattedParameter( PARAMETERPOSITION_FLOWCONTROL );
	if( pString && *pString )
	{
		szString += " ";
		szString += pString;
	}
	pString = GetFormattedParameter( PARAMETERPOSITION_EXTRAAPPLICATIONPARAMETERS );
	if( pString && *pString )
	{
		szString += " ";
		szString += pString;
	}
}

const char * CGTLParameters::GetFormattedParameter( int iParameterPosition )
{
	if( !this ) return NULL;
	int iParam = INVALID_PARAMETER;
	const char * pString = NULL;

	iParam = GetParameterInteger( iParameterPosition );
	switch( iParameterPosition )
	{
	case PARAMETERPOSITION_LOADFILENAME:
		if( m_szFormatLoadFileName.IsEmpty() )
			pString = (LPCTSTR)m_szLoadFileName;
		else
			m_szTemp.Format( m_szFormatLoadFileName, m_szLoadFileName );
		pString = (LPCTSTR)m_szTemp;
		break;
	case PARAMETERPOSITION_PORT:
		if( !m_szFormatPort.IsEmpty() )
		{
			m_szTemp.Format( m_szFormatPort, iParam + COM_ZERO_BASE );
			pString = (LPCTSTR)m_szTemp;
		}
		break;
	case PARAMETERPOSITION_BPS:
		if( !m_szFormatBPS.IsEmpty() )
		{
			m_szTemp.Format( m_szFormatBPS, iParam );
			pString = (LPCTSTR)m_szTemp;
		}
		break;
	case PARAMETERPOSITION_DATABITS:
		if( !m_szFormatDataBits.IsEmpty() )
		{
			m_szTemp.Format( m_szFormatDataBits, iParam );
			pString = (LPCTSTR)m_szTemp;
		}
		break;
	case PARAMETERPOSITION_STOPBITS:
		if( !m_szFormatStopBits.IsEmpty() )
		{
			m_szTemp.Format( m_szFormatStopBits, iParam );
			pString = (LPCTSTR)m_szTemp;
		}
		break;
	case PARAMETERPOSITION_PARITY:
		if( !m_szFormatParity.IsEmpty() )
		{
			m_szTemp.Format( m_szFormatParity, g_aParity[ iParam ] );
			pString = (LPCTSTR)m_szTemp;
		}
		break;
	case PARAMETERPOSITION_FLOWCONTROL:
		if( !m_szFormatFlowControl.IsEmpty() )
		{
			m_szTemp.Format( m_szFormatFlowControl, g_aFlowControl[ iParam ] );
			pString = (LPCTSTR)m_szTemp;
		}
		break;
	case PARAMETERPOSITION_EXTRAAPPLICATIONPARAMETERS:
		if( !m_szExtraApplicationParameters.IsEmpty() )
		{
			pString = (LPCTSTR)m_szExtraApplicationParameters;
		}
		break;
	}
	return pString;
}

CGTLParameters::CGTLParameters()
{
	m_bCommParametersGiven = false;
	m_iError		= PARAMETER_LOAD_ERROR_SUCCESS;
	m_iPort			= DEFAULT_PORT;
	m_iBPS			= DEFAULT_BPS;
	m_iDataBits		= DEFAULT_DATABITS;
	m_iParity		= DEFAULT_PARITY;
	m_iStopBits		= DEFAULT_STOPBITS;
	m_iFlowControl	= DEFAULT_FLOWCONTROL;
}

CGTLParameters::~CGTLParameters()
{
}

CGTLParameters::CGTLParameters( CString& szParametersString )
{
	ParseString( szParametersString );
}

bool CGTLParameters::CalculateAndVerifyFileNamesAndPaths( CString& szPath, CString& szName )
{
#define PATH_SIZE 1024
	DWORD dwResult;
	char path_buffer	[PATH_SIZE];
   	char drive			[_MAX_DRIVE];
	char dir			[PATH_SIZE];
	char fname			[_MAX_FNAME];
	char ext			[_MAX_EXT];

	dwResult = GetShortPathName( szPath, path_buffer, PATH_SIZE );
	if( dwResult == 0 )
	{
		LPVOID lpMsgBuf;
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0,
			NULL );
		szName.Format( "%s %s", lpMsgBuf, szPath );
		LocalFree( lpMsgBuf );
	}
	else
	{
		_splitpath( path_buffer, drive, dir, fname, ext );

		_makepath( path_buffer, drive, dir, NULL, NULL );
   		szPath = path_buffer;

		_makepath( path_buffer, NULL, NULL, fname, ext );
   		szName = path_buffer;
		return true;
	}
	return false;
}

BOOL CGTLParameters::ParseString( CString& szParametersString )
{
	if( !this ) return FALSE;

	BOOL bReturn;

//required parameters
	bReturn = AfxExtractSubString(	m_szManufacturer,			
								szParametersString, 
								PARAMETERPOSITION_MANUFACTURER, '|' );
	if( bReturn == FALSE ) return FALSE;
	bReturn = AfxExtractSubString(	m_szModel,				
								szParametersString, 
								PARAMETERPOSITION_MODEL, '|' );
	if( bReturn == FALSE ) return FALSE;
	bReturn = AfxExtractSubString(	m_szLoadFileDescription,				
								szParametersString, 
								PARAMETERPOSITION_LOADFILEDESCRIPTION, '|' );
	if( bReturn == FALSE ) return FALSE;

	
	bReturn = AfxExtractSubString(	m_szLoadFilePath,			
								szParametersString, 
								PARAMETERPOSITION_LOADFILENAME, '|' );
	if( bReturn == FALSE ) return FALSE;
	if( false == CalculateAndVerifyFileNamesAndPaths( m_szLoadFilePath, m_szLoadFileName ) )
		m_iError = PARAMETER_LOAD_ERROR_BADFILE;

	bReturn = AfxExtractSubString(	m_szExecutableFilePath,			
								szParametersString, 
								PARAMETERPOSITION_EXECUTABLEFILENAME, '|' );
	if( bReturn == FALSE ) return FALSE;
	if( false == CalculateAndVerifyFileNamesAndPaths( m_szExecutableFilePath, m_szExecutableFileName ) )
		m_iError = PARAMETER_LOAD_ERROR_BADLOADER;

//optional parameters Comm
	bReturn = AfxExtractSubString(	m_szFormatLoadFileName, szParametersString, 
								PARAMETERPOSITION_LOADFILEFORMAT, '|' );

	bReturn = AfxExtractSubString(	m_szTemp, szParametersString, 
								PARAMETERPOSITION_PORT, '|' );
	AfxExtractSubString( m_szFormatPort, m_szTemp, 
						PARAMETERS_POSITIONS_FORMAT_FORMAT, ',' );
	AfxExtractSubString( m_szValuePort, m_szTemp, 
						PARAMETERS_POSITIONS_FORMAT_DEFAULT, ',' );
	SetParameter( m_szValuePort, PARAMETERPOSITION_PORT );

	bReturn = AfxExtractSubString(	m_szTemp, szParametersString, 
								PARAMETERPOSITION_BPS, '|' );
	AfxExtractSubString( m_szFormatBPS, m_szTemp, 
						PARAMETERS_POSITIONS_FORMAT_FORMAT, ',' );
	AfxExtractSubString( m_szValueBPS, m_szTemp, 
						PARAMETERS_POSITIONS_FORMAT_DEFAULT, ',' );
	SetParameter( m_szValueBPS, PARAMETERPOSITION_BPS );

	if( !m_szValuePort.IsEmpty() && !m_szValueBPS.IsEmpty() )
		m_bCommParametersGiven = true;

	bReturn = AfxExtractSubString(	m_szTemp, szParametersString, 
								PARAMETERPOSITION_DATABITS, '|' );
	AfxExtractSubString( m_szFormatDataBits, m_szTemp, 
						PARAMETERS_POSITIONS_FORMAT_FORMAT, ',' );
	AfxExtractSubString( m_szValueDataBits, m_szTemp, 
						PARAMETERS_POSITIONS_FORMAT_DEFAULT, ',' );
	SetParameter( m_szValueDataBits, PARAMETERPOSITION_DATABITS );

	bReturn = AfxExtractSubString(	m_szTemp, szParametersString, 
								PARAMETERPOSITION_PARITY, '|' );
	AfxExtractSubString( m_szFormatParity, m_szTemp, 
						PARAMETERS_POSITIONS_FORMAT_FORMAT, ',' );
	AfxExtractSubString( m_szValueParity, m_szTemp, 
						PARAMETERS_POSITIONS_FORMAT_DEFAULT, ',' );
	SetParameter( m_szValueParity, PARAMETERPOSITION_PARITY );

	bReturn = AfxExtractSubString(	m_szTemp, szParametersString, 
								PARAMETERPOSITION_STOPBITS, '|' );
	AfxExtractSubString( m_szFormatStopBits, m_szTemp, 
						PARAMETERS_POSITIONS_FORMAT_FORMAT, ',' );
	AfxExtractSubString( m_szValueStopBits, m_szTemp, 
						PARAMETERS_POSITIONS_FORMAT_DEFAULT, ',' );
	SetParameter( m_szValueStopBits, PARAMETERPOSITION_STOPBITS );

	bReturn = AfxExtractSubString(	m_szTemp, szParametersString, 
								PARAMETERPOSITION_FLOWCONTROL, '|' );
	AfxExtractSubString( m_szFormatFlowControl, m_szTemp, 
						PARAMETERS_POSITIONS_FORMAT_FORMAT, ',' );
	AfxExtractSubString( m_szValueFlowControl, m_szTemp, 
						PARAMETERS_POSITIONS_FORMAT_DEFAULT, ',' );
	SetParameter( m_szValueFlowControl, PARAMETERPOSITION_FLOWCONTROL );

//optional parameters
	bReturn = AfxExtractSubString(	m_szExtraApplicationParameters,		
								szParametersString, 
								PARAMETERPOSITION_EXTRAAPPLICATIONPARAMETERS, '|' );

	bReturn = AfxExtractSubString(	m_szLoadFileVersion,				
								szParametersString, 
								PARAMETERPOSITION_LOADFILEVERSION, '|' );

	bReturn = AfxExtractSubString(	m_szExtraInstructions,				
								szParametersString, 
								PARAMETERPOSITION_EXTRAINSTRUCTIONS, '|' );

	bReturn = AfxExtractSubString(	m_szDialogCommunicationsBMP,	
								szParametersString, 
								PARAMETERPOSITION_DIALOGCOMMUNICATIONSBMP, '|' );

	bReturn = AfxExtractSubString(	m_szDialogStartBMP,		
								szParametersString, 
								PARAMETERPOSITION_DIALOGSTARTBMP, '|' );
	
	return TRUE;
}

void CGTLParameters::GetParameter( CString& szParameter, int iParameterPosition )
{
	szParameter = GetParameter( iParameterPosition );
}

const char * CGTLParameters::GetParameter( int iParameterPosition )
{
	if( !this ) return NULL;
	const char * pString = NULL;
	int iParam = INVALID_PARAMETER;

	if( iParameterPosition < START_INTEGER_PARAMETER ||
		iParameterPosition > LAST_INTEGER_PARAMETER )
	{
		pString = GetParameterString( iParameterPosition );
	}
	else
	{
		iParam = GetParameterInteger( iParameterPosition );
		switch( iParameterPosition )
		{
		case PARAMETERPOSITION_PORT:
			pString = g_aPort[ iParam ];
			break;
		case PARAMETERPOSITION_PARITY:
			pString = g_aParity[ iParam ];
			break;
		case PARAMETERPOSITION_FLOWCONTROL:
			pString = g_aFlowControl[ iParam ];
			break;
		case PARAMETERPOSITION_BPS:
			m_szValueBPS.Format( "%i", iParam );
			pString = (LPCTSTR)m_szValueBPS;
			break;
		case PARAMETERPOSITION_DATABITS:
			m_szValueDataBits.Format( "%i", iParam );
			pString = (LPCTSTR)m_szValueDataBits;
			break;
		case PARAMETERPOSITION_STOPBITS:
			m_szValueStopBits.Format( "%i", iParam );
			pString = (LPCTSTR)m_szValueStopBits;
			break;
		}
	}
	return pString;
}

void CGTLParameters::SetParameter( const char * szParameter, int iParameterPosition )
{
	if( !this ) return;
	const char * pString = NULL;

	if( !szParameter || !*szParameter )
		return;

	if( iParameterPosition < START_INTEGER_PARAMETER ||
		iParameterPosition > LAST_INTEGER_PARAMETER )
	{
		SetParameterString( szParameter, iParameterPosition );
	}
	else
	{
		switch( iParameterPosition )
		{
		case PARAMETERPOSITION_PORT:
			m_iPort = GetFirstDigitValue( szParameter ) - COM_ZERO_BASE;
			break;
		case PARAMETERPOSITION_PARITY:
			switch( szParameter[0] )
			{
			case 1:
			case '1':
			case 'e':
			case 'E':
				m_iParity = GTLPARITY_EVEN;
				break;
			case 2:
			case '2':
			case 'o':
			case 'O':
				m_iParity = GTLPARITY_ODD;
				break;
			case 3:
			case '3':
			case 'm':
			case 'M':
				m_iParity = GTLPARITY_MARK;
				break;
			case 4:
			case '4':
			case 's':
			case 'S':
				m_iParity = GTLPARITY_SPACE;
				break;
			case '0':
			case 'N':
			case 'n':
			default:
				m_iParity = GTLPARITY_NONE;
				break;
			}
			break;
		case PARAMETERPOSITION_FLOWCONTROL:
			switch( szParameter[0] )
			{
			case 1:
			case '1':
			case 'x':
			case 'X':
				m_iFlowControl = FLOWCONTROL_XONXOFF;
				break;
			case 2:
			case '2':
			case 'c':
			case 'C':
				m_iFlowControl = FLOWCONTROL_CTSRTS;
				break;
			case 3:
			case '3':
			case 'h':
			case 'H':
				m_iFlowControl = FLOWCONTROL_HARDWARE;
				break;
			case '0':
			case 'N':
			case 'n':
			default:
				m_iFlowControl = FLOWCONTROL_NONE;
				break;
			}
			break;
		case PARAMETERPOSITION_BPS:
			m_iBPS = atoi( szParameter );
			break;
		case PARAMETERPOSITION_DATABITS:
			m_iDataBits = atoi( szParameter );
			break;
		case PARAMETERPOSITION_STOPBITS:
			m_iStopBits = atoi( szParameter );
			break;
		}
	}
}

int CGTLParameters::GetFirstDigitValue( const char * szString )
{
	if( !szString || !*szString )
		return INVALID_PARAMETER;

	for( int i=0; i<(int)strlen(szString); i++ )
	{
		if( isdigit( szString[ i ] ) )
			return (int)( szString[ i ] - ASCII_ZERO );
	}
	return INVALID_PARAMETER;
}

void CGTLParameters::SetParameterString( const char * szParameter, int iParameterPosition )
{
	if( !szParameter || !*szParameter )
		return;

	if( iParameterPosition >= START_INTEGER_PARAMETER &&
		iParameterPosition <= LAST_INTEGER_PARAMETER )
		return;

	switch( iParameterPosition )
	{
	case PARAMETERPOSITION_MANUFACTURER:
		m_szManufacturer = szParameter;
		break;
	case PARAMETERPOSITION_MODEL:
		m_szModel = szParameter;
		break;
	case PARAMETERPOSITION_LOADFILEDESCRIPTION:
		m_szLoadFileDescription = szParameter;
		break;
	case PARAMETERPOSITION_LOADFILENAME:
		m_szLoadFileName = szParameter;
		break;
	case PARAMETERPOSITION_EXECUTABLEFILENAME:
		m_szExecutableFileName = szParameter;
		break;
	case PARAMETERPOSITION_EXTRAINSTRUCTIONS:
		m_szExtraInstructions = szParameter;
		break;
	case PARAMETERPOSITION_LOADFILEVERSION:
		m_szLoadFileVersion = szParameter;
		break;
	case PARAMETERPOSITION_DIALOGCOMMUNICATIONSBMP:
		m_szDialogCommunicationsBMP = szParameter;
		break;
	case PARAMETERPOSITION_DIALOGSTARTBMP:
		m_szDialogStartBMP = szParameter;
		break;
	case PARAMETERPOSITION_EXTRAAPPLICATIONPARAMETERS:
		m_szExtraApplicationParameters = szParameter;
		break;
	}
}

void CGTLParameters::SetParameterInteger( int iParameter, int iParameterPosition )
{
	if( iParameterPosition < START_INTEGER_PARAMETER ||
		iParameterPosition > LAST_INTEGER_PARAMETER )
		return;

	switch( iParameterPosition )
	{
	case PARAMETERPOSITION_PORT:
		m_iPort = iParameter;
		break;
	case PARAMETERPOSITION_BPS:
		m_iBPS = iParameter;
		break;
	case PARAMETERPOSITION_DATABITS:
		m_iDataBits = iParameter;
		break;
	case PARAMETERPOSITION_PARITY:
		m_iParity = iParameter;
		break;
	case PARAMETERPOSITION_STOPBITS:
		m_iStopBits = iParameter;
		break;
	case PARAMETERPOSITION_FLOWCONTROL:
		m_iFlowControl = iParameter;
		break;
	}
}

const char * CGTLParameters::GetParameterString( int iParameterPosition )
{
	if( !this ) return NULL;

	if( iParameterPosition >= START_INTEGER_PARAMETER &&
		iParameterPosition <= LAST_INTEGER_PARAMETER )
		return NULL;

	switch( iParameterPosition )
	{
	case PARAMETERPOSITION_MANUFACTURER:
		return (LPCTSTR)m_szManufacturer;
		break;
	case PARAMETERPOSITION_MODEL:
		return (LPCTSTR)m_szModel;
		break;
	case PARAMETERPOSITION_LOADFILEDESCRIPTION:
		return (LPCTSTR)m_szLoadFileDescription;
		break;
	case PARAMETERPOSITION_LOADFILENAME:
		return (LPCTSTR)m_szLoadFileName;
		break;
	case PARAMETERPOSITION_EXECUTABLEFILENAME:
		return (LPCTSTR)m_szExecutableFileName;
		break;
	case PARAMETERPOSITION_EXTRAINSTRUCTIONS:
		return (LPCTSTR)m_szExtraInstructions;
		break;
	case PARAMETERPOSITION_LOADFILEVERSION:
		return (LPCTSTR)m_szLoadFileVersion;
		break;
	case PARAMETERPOSITION_DIALOGCOMMUNICATIONSBMP:
		return (LPCTSTR)m_szDialogCommunicationsBMP;
		break;
	case PARAMETERPOSITION_DIALOGSTARTBMP:
		return (LPCTSTR)m_szDialogStartBMP;
		break;
	case PARAMETERPOSITION_EXTRAAPPLICATIONPARAMETERS:
		return (LPCTSTR)m_szExtraApplicationParameters;
		break;
	case PARAMETERPOSITION_LOADFILEPATH:
		return (LPCTSTR)m_szLoadFilePath;
		break;
	case PARAMETERPOSITION_EXECUTABLEFILEPATH:
		return (LPCTSTR)m_szExecutableFilePath;
		break;
	default:
		return NULL;
		break;
	}
}

int	CGTLParameters::GetParameterInteger( int iParameterPosition )
{
	if( iParameterPosition < START_INTEGER_PARAMETER ||
		iParameterPosition > LAST_INTEGER_PARAMETER )
		return INVALID_PARAMETER;

	switch( iParameterPosition )
	{
	case PARAMETERPOSITION_PORT:
		return m_iPort;
		break;
	case PARAMETERPOSITION_BPS:
		return m_iBPS;
		break;
	case PARAMETERPOSITION_DATABITS:
		return m_iDataBits;
		break;
	case PARAMETERPOSITION_PARITY:
		return m_iParity;
		break;
	case PARAMETERPOSITION_STOPBITS:
		return m_iStopBits;
		break;
	case PARAMETERPOSITION_FLOWCONTROL:
		return m_iFlowControl;
		break;
	default:
		return INVALID_PARAMETER;
		break;
	}
}

CGTLParametersArray::CGTLParametersArray()
{
}

CGTLParametersArray::~CGTLParametersArray()
{
	CGTLParametersArray::CleanUp();
}

void CGTLParametersArray::CleanUp()
{
	PGTLPARAMETERS pParams = NULL;
	for( int i=0; i<GetSize(); i++ )
	{
		pParams = (PGTLPARAMETERS)GetAt( i );
		if( pParams != NULL )
			delete pParams;
	}
	RemoveAll();
}

void CGTLParametersArray::LoadManufacturersComboBox( CComboBox& cb )
{
	cb.SetCurSel( CB_ERR );
	cb.ResetContent();
	PGTLPARAMETERS pParams = NULL;
	CMapStringToPtr	map;

	for( int i=0; i<GetSize(); i++ )
	{
		pParams = (PGTLPARAMETERS)GetAt( i );
		if( pParams != NULL )
		{
			LPCTSTR pManufacturer = pParams->GetParameterString( PARAMETERPOSITION_MANUFACTURER );
			if( pManufacturer )
				map.SetAt( pManufacturer, NULL );
		}
	}

	POSITION pos;
	CString key;
	void * ptr;
   	for( pos = map.GetStartPosition(); pos != NULL; )
	{
		map.GetNextAssoc( pos, key, (void*&)ptr );
		cb.AddString( key );
	}
}

void CGTLParametersArray::LoadModelComboBox( CComboBox& cb, LPCTSTR szManufacturer )
{
	cb.SetCurSel( CB_ERR );
	cb.ResetContent();
	PGTLPARAMETERS pParams = NULL;
	CMapStringToPtr	map;

	for( int i=0; i<GetSize(); i++ )
	{
		pParams = (PGTLPARAMETERS)GetAt( i );
		if( pParams != NULL )
		{
			LPCTSTR pManufacturer = pParams->GetParameterString( PARAMETERPOSITION_MANUFACTURER );
			if( pManufacturer &&
				strcmp( szManufacturer, pManufacturer )
				== 0 )
			{
				LPCTSTR pModel = pParams->GetParameterString( PARAMETERPOSITION_MODEL );
				if( pModel )
					map.SetAt( pModel, NULL );
			}
		}
	}

	POSITION pos;
	CString key;
	void * ptr;
   	for( pos = map.GetStartPosition(); pos != NULL; )
	{
		map.GetNextAssoc( pos, key, (void*&)ptr );
		cb.AddString( key );
	}
}

void CGTLParametersArray::LoadFileDescriptionComboBox( CComboBox& cb, LPCTSTR szManufacturer, LPCTSTR szModel )
{
	cb.SetCurSel( CB_ERR );
	cb.ResetContent();
	PGTLPARAMETERS pParams = NULL;
	CMapStringToPtr	map;

	for( int i=0; i<GetSize(); i++ )
	{
		pParams = (PGTLPARAMETERS)GetAt( i );
		if( pParams != NULL )
		{
			LPCTSTR pManufacturer = pParams->GetParameterString( PARAMETERPOSITION_MANUFACTURER );
			LPCTSTR pModel = pParams->GetParameterString( PARAMETERPOSITION_MODEL );
			if(	pManufacturer &&
				pModel &&
				strcmp( szManufacturer, pManufacturer )
				== 0 &&
				strcmp( szModel, pModel )
				== 0 )
			{
				LPCTSTR pFileDescription = pParams->GetParameterString( PARAMETERPOSITION_LOADFILEDESCRIPTION );
				if( pFileDescription )
					map.SetAt( pFileDescription, NULL );
			}
		}
	}

	POSITION pos;
	CString key;
	void * ptr;
   	for( pos = map.GetStartPosition(); pos != NULL; )
	{
		map.GetNextAssoc( pos, key, (void*&)ptr );
		cb.AddString( key );
	}
}

PGTLPARAMETERS CGTLParametersArray::GetMatchingParameters( LPCTSTR szManufacturer, LPCTSTR szModel, LPCTSTR szFileDescription )
{
	PGTLPARAMETERS pParams = NULL;
	for( int i=0; i<GetSize(); i++ )
	{
		pParams = (PGTLPARAMETERS)GetAt( i );
		if( pParams != NULL )
		{
			LPCTSTR pManufacturer = pParams->GetParameterString( PARAMETERPOSITION_MANUFACTURER );
			LPCTSTR pModel = pParams->GetParameterString( PARAMETERPOSITION_MODEL );
			LPCTSTR pFileDescription = pParams->GetParameterString( PARAMETERPOSITION_LOADFILEDESCRIPTION );

			if(	pManufacturer &&
				pModel &&
				pFileDescription &&
				strcmp( szManufacturer, pManufacturer )
				== 0 &&
				strcmp( szModel, pModel )
				== 0 &&
				strcmp( szFileDescription, pFileDescription )
				== 0 )
			{
				return pParams;
			}
		}
	}
	return pParams;
}

BOOL CGTLParametersArray::LoadGTLFile()
{
	BOOL bResult = FALSE;

	CGTLParametersArray::CleanUp();

	SetSize( PARAMETERS_ARRAY_SIZE, PARAMETERS_ARRAY_GROW );

	int i = 0;
	PGTLPARAMETERS pParams = NULL;
	CStringFile 	sfText;
	CString			szLine;

	szLine = g_pApp->m_pszExeName;
	szLine += ".txt";

	bResult = sfText.Open( szLine );
	if( bResult == TRUE ) 
	{
		while( sfText.GetNextLine(szLine )!=0 )
		{
			pParams = new CGTLParameters();
			if( pParams && TRUE == pParams->ParseString( szLine ) )
			{
				SetAtGrow( i++, pParams );
			}
			else
			{
				delete pParams;
			}
		}
		sfText.Close();
	}
	else
	{
		sfText.GetErrorText( szLine );
		MessageBox( NULL, szLine, "GTL Send Error", MB_ICONSTOP );
	}
	return bResult;
}

