#include "stdafx.h"
#include "DBLogger.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CDBLogger CDBLogger
// PURPOSE: Constructor
/////////////////////////////////////////////////////////////////////////////
CDBLogger::CDBLogger()
{
	if( !this ) return;
	m_pdb = NULL;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CDBLogger ~CDBLogger
// PURPOSE: Destructor
/////////////////////////////////////////////////////////////////////////////
CDBLogger::~CDBLogger()
{
	if( !this ) return;
	if ( m_pdb )
	{
		m_pdb->Close();
		delete m_pdb;
	}
	m_pdb = NULL;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CDBLogger Initialize
// PURPOSE: Initialization
/////////////////////////////////////////////////////////////////////////////
bool CDBLogger::Initialize( LPCTSTR lpDBPath )
{
	if( !this ) return false;
    try
	{
		CString szTemp;
		if( lpDBPath )
		{
			szTemp = lpDBPath;
		}
		else
		{
			szTemp = GetCommandLine();
			szTemp = szTemp.Mid( 1, szTemp.Find( "\"", 1 ) - 4 ) + "mdb";
		}

		WIN32_FIND_DATA fd;
		HANDLE hFile = FindFirstFile( szTemp, &fd );
		if ( hFile == INVALID_HANDLE_VALUE )
		{
			TRACE( "Could not find database file" );
			FindClose( hFile );
			return false;
		}
		FindClose( hFile );

		//create db object
		if ( m_pdb == NULL )
		{
			m_pdb = new CDaoDatabase;
		}
		//open database
		m_pdb->Open( (LPCTSTR)szTemp );
		return true;
	}    
	catch( CDaoException* e )    
	{
        TRACE( e->m_pErrorInfo->m_strDescription );
		e->Delete();    
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CDBLogger AddLog
// PURPOSE:
/////////////////////////////////////////////////////////////////////////////
bool CDBLogger::AddLog( LPDBLOG pDBLog )
{
//declarations
	CDaoRecordset * prs;
	COleVariant		OLEVar;
	COleDateTime	OLEDateTime;

//code
	if ( !this ) return false;
	if ( !pDBLog ) return false;
	if ( !m_pdb ) return false;

	prs = new CDaoRecordset( m_pdb );
	if ( prs == NULL ) return false;

    try
	{
		prs->Open( dbOpenTable, 
				   "tblLog", 
				   dbForwardOnly );

		prs->AddNew();

		OLEDateTime.SetDateTime( 
						pDBLog->cTime.GetYear(), 
						pDBLog->cTime.GetMonth(), 
						pDBLog->cTime.GetDay(), 
						pDBLog->cTime.GetHour(), 
						pDBLog->cTime.GetMinute(), 
						pDBLog->cTime.GetSecond() );
		prs->SetFieldValue( FLD_LOG_TIME, OLEDateTime );

		OLEVar = pDBLog->lngScenario;
		prs->SetFieldValue( FLD_LOG_SCENARIO, OLEVar );

		OLEVar = pDBLog->lngVoteID;
		prs->SetFieldValue( FLD_LOG_VOTEID, OLEVar );

		OLEVar = pDBLog->lngRes1;
		prs->SetFieldValue( FLD_LOG_RES1, OLEVar );

		OLEVar = pDBLog->lngRes2;
		prs->SetFieldValue( FLD_LOG_RES2, OLEVar );

		OLEVar = pDBLog->lngRes3;
		prs->SetFieldValue( FLD_LOG_RES3, OLEVar );

		OLEVar = pDBLog->lngRes4;
		prs->SetFieldValue( FLD_LOG_RES4, OLEVar );

		OLEVar = pDBLog->lngRes5;
		prs->SetFieldValue( FLD_LOG_RES5, OLEVar );

		prs->Update();
		prs->Close();
	}    
	catch( CDaoException* e )    
	{
        TRACE( e->m_pErrorInfo->m_strDescription );
		AfxMessageBox( e->m_pErrorInfo->m_strDescription );
		Beep( 3000, 500 );
		prs->Close();
		e->Delete();    
		return false;
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////

