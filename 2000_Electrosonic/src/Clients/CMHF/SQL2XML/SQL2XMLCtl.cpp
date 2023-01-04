// SQL2XMLCtl.cpp : Implementation of the CSQL2XMLCtrl ActiveX Control class.

#include "stdafx.h"
#include "SQL2XML.h"
#include "SQL2XMLCtl.h"
#include "SQL2XMLPpg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CSQL2XMLCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CSQL2XMLCtrl, COleControl)
	ON_MESSAGE(WM_USER_SQL2XML_QUERYCOMPLETED, OnSQL2XMLQueryCompleted)
	//{{AFX_MSG_MAP(CSQL2XMLCtrl)
	// NOTE - ClassWizard will add and remove message map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
	ON_MESSAGE(OCM_COMMAND, OnOcmCommand)
	ON_OLEVERB(AFX_IDS_VERB_EDIT, OnEdit)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CSQL2XMLCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CSQL2XMLCtrl)
	DISP_PROPERTY_NOTIFY(CSQL2XMLCtrl, "DSN", m_szDSN, OnDSNChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CSQL2XMLCtrl, "SQLString", m_szSQLString, OnSQLStringChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CSQL2XMLCtrl, "XMLFile", m_szXMLFile, OnXMLFileChanged, VT_BSTR)
	DISP_PROPERTY_EX(CSQL2XMLCtrl, "ControlPointer", GetControlPointer, SetControlPointer, VT_I4)
	DISP_FUNCTION(CSQL2XMLCtrl, "BuildDSN", BuildDSN, VT_BSTR, VTS_BSTR)
	DISP_FUNCTION(CSQL2XMLCtrl, "Execute", Execute, VT_EMPTY, VTS_BSTR VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CSQL2XMLCtrl, "GetLastErrorString", GetLastErrorString, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CSQL2XMLCtrl, "GetLastError", GetLastError, VT_I4, VTS_NONE)
	DISP_DEFVALUE(CSQL2XMLCtrl, "Caption")
	DISP_STOCKFUNC_DOCLICK()
	DISP_STOCKPROP_CAPTION()
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CSQL2XMLCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CSQL2XMLCtrl, COleControl)
	//{{AFX_EVENT_MAP(CSQL2XMLCtrl)
	EVENT_CUSTOM("QueryComplete", FireQueryComplete, VTS_NONE)
	EVENT_STOCK_ERROREVENT()
	EVENT_STOCK_CLICK()
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CSQL2XMLCtrl, 1)
	PROPPAGEID(CSQL2XMLPropPage::guid)
END_PROPPAGEIDS(CSQL2XMLCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CSQL2XMLCtrl, "SQL2XML.SQL2XMLCtrl.1",
	0x3d113c39, 0xfe52, 0x11d4, 0xb9, 0x5a, 0xff, 0xff, 0xff, 0, 0, 0)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CSQL2XMLCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DSQL2XML =
		{ 0x703ef6e2, 0x2ce, 0x11d5, { 0xb9, 0x5d, 0, 0x10, 0x4b, 0x6d, 0x42, 0xee } };
const IID BASED_CODE IID_DSQL2XMLEvents =
		{ 0x703ef6e3, 0x2ce, 0x11d5, { 0xb9, 0x5d, 0, 0x10, 0x4b, 0x6d, 0x42, 0xee } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwSQL2XMLOleMisc =
	OLEMISC_SIMPLEFRAME |
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CSQL2XMLCtrl, IDS_SQL2XML, _dwSQL2XMLOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CSQL2XMLCtrl::CSQL2XMLCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CSQL2XMLCtrl

BOOL CSQL2XMLCtrl::CSQL2XMLCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegInsertable | afxRegApartmentThreading to afxRegInsertable.

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_SQL2XML,
			IDB_SQL2XML,
			afxRegInsertable | afxRegApartmentThreading,
			_dwSQL2XMLOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CSQL2XMLCtrl::CSQL2XMLCtrl - Constructor

CSQL2XMLCtrl::CSQL2XMLCtrl()
{
	InitializeIIDs(&IID_DSQL2XML, &IID_DSQL2XMLEvents);

	EnableSimpleFrame();

	// TODO: Initialize your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// CSQL2XMLCtrl::~CSQL2XMLCtrl - Destructor

CSQL2XMLCtrl::~CSQL2XMLCtrl()
{
	// TODO: Cleanup your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// CSQL2XMLCtrl::OnDraw - Drawing function

void CSQL2XMLCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	DoSuperclassPaint(pdc, rcBounds);
}


/////////////////////////////////////////////////////////////////////////////
// CSQL2XMLCtrl::DoPropExchange - Persistence support

void CSQL2XMLCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CSQL2XMLCtrl::OnResetState - Reset control to default state

void CSQL2XMLCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CSQL2XMLCtrl::AboutBox - Display an "About" box to the user

void CSQL2XMLCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_SQL2XML);
	dlgAbout.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// CSQL2XMLCtrl::PreCreateWindow - Modify parameters for CreateWindowEx

BOOL CSQL2XMLCtrl::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.lpszClass = _T("BUTTON");
	return COleControl::PreCreateWindow(cs);
}


/////////////////////////////////////////////////////////////////////////////
// CSQL2XMLCtrl::IsSubclassedControl - This is a subclassed control

BOOL CSQL2XMLCtrl::IsSubclassedControl()
{
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CSQL2XMLCtrl::OnOcmCommand - Handle command messages

LRESULT CSQL2XMLCtrl::OnOcmCommand(WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN32
	WORD wNotifyCode = HIWORD(wParam);
#else
	WORD wNotifyCode = HIWORD(lParam);
#endif
//	switch (wNotifyCode)
//	{
//	case BN_CLICKED:
//		// Fire click event when button is clicked
//		FireClick();
//		break;
//	}
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
// CSQL2XMLCtrl message handlers

LRESULT CSQL2XMLCtrl::OnSQL2XMLQueryCompleted(WPARAM, LPARAM)
{
	m_WorkerThreadInfo.bThreadIsWorking = false;
	switch( m_WorkerThreadInfo.hrError )
	{
	case 0:
		FireQueryComplete();
		break;
	default:
		FireError( m_WorkerThreadInfo.hrError, (LPCTSTR)*m_WorkerThreadInfo.pErrorCString );
		break;
	}
	return 0;
}

void CSQL2XMLCtrl::OnDSNChanged() 
{
	// TODO: Add notification handler code

	SetModifiedFlag();
}

void CSQL2XMLCtrl::OnSQLStringChanged() 
{
	// TODO: Add notification handler code

	SetModifiedFlag();
}

void CSQL2XMLCtrl::OnXMLFileChanged() 
{
	// TODO: Add notification handler code

	SetModifiedFlag();
}

BSTR CSQL2XMLCtrl::GetLastErrorString() 
{
	return m_szError.AllocSysString();
}

long CSQL2XMLCtrl::GetLastError() 
{
	return m_hrError;
}

void CSQL2XMLCtrl::Execute(LPCTSTR pszDSN, LPCTSTR pszSQL, LPCTSTR pszFile) 
{
	m_hrError = 0;

	if( m_WorkerThreadInfo.bThreadIsWorking == true )
	{
		m_hrError = AFX_IDP_E_DEVICEUNAVAILABLE;
		m_szError = "Unable to execute command. The control thread is currently executing. Please retry later.";
		FireError( CTL_E_DEVICEUNAVAILABLE, m_szError );
		return;
	}

	if( !pszDSN || !*pszDSN )
	{
		//check if we already have a DSN
		if( !m_szDSN.IsEmpty() )
		{
			pszDSN = (LPCTSTR)m_szDSN;
		}
		else
		{
			m_hrError = AFX_IDP_E_ILLEGALFUNCTIONCALL;
			m_szError = "Invalid DSN Name";
			FireError( CTL_E_ILLEGALFUNCTIONCALL, m_szError );
			return;
		}
	}

	if( !pszSQL || !*pszSQL )
	{
		//check if we already have a SQL String
		if( !m_szSQLString.IsEmpty() )
		{
			pszSQL = (LPCTSTR)m_szSQLString;
		}
		else
		{
			m_hrError = AFX_IDP_E_ILLEGALFUNCTIONCALL;
			m_szError = "Invalid SQL Name";
			FireError( CTL_E_ILLEGALFUNCTIONCALL, m_szError );
			return;
		}
	}

	//accept no file name in case the SQL does not return data
	if( !pszFile || !*pszFile )
	{
		//check if we already have a XML File name
		if( !m_szXMLFile.IsEmpty() )
		{
			pszFile = (LPCTSTR)m_szXMLFile;
		}
		else
		{
			//null out the pointer in case it was valid but
			//pointed to nothing
			pszFile = NULL;
		}
	}

	m_WorkerThreadInfo.bThreadIsWorking = true;
	m_WorkerThreadInfo.hrError = 0;
	m_WorkerThreadInfo.m_hwndNotifyDone = m_hWnd;
	m_WorkerThreadInfo.pszDSN = pszDSN;
	m_WorkerThreadInfo.pszSQL = pszSQL;
	m_WorkerThreadInfo.pszFile = pszFile;
	m_WorkerThreadInfo.pErrorCString = &m_szError;

	m_pWorkerThread = AfxBeginThread( SQL2XMLThreadProc, &m_WorkerThreadInfo );
}

UINT SQL2XMLThreadProc( LPVOID pParam )
{
	CSQL2XMLThreadInfo * pSQL2XMLInfo = (CSQL2XMLThreadInfo*)pParam;

	ADODB::_ConnectionPtr pConnection;
	HRESULT hr;

	try
	{
		hr = CoInitialize( NULL );
		if(FAILED(hr)) _com_issue_error(hr);

		ADODB::_RecordsetPtr	pRS;

		hr = pConnection.CreateInstance(__uuidof(ADODB::Connection));
		if(FAILED(hr)) _com_issue_error(hr);

		hr = pRS.CreateInstance(__uuidof(ADODB::Recordset));
		if(FAILED(hr)) _com_issue_error(hr);
 
		hr = pConnection->Open(
				_bstr_t(pSQL2XMLInfo->pszDSN),
				_bstr_t(L""),
				_bstr_t(L""),
				ADODB::adConnectUnspecified
				);
		if(FAILED(hr)) _com_issue_error(hr);

		hr = pRS->Open(
				_bstr_t(pSQL2XMLInfo->pszSQL),
				pConnection.GetInterfacePtr(),
				ADODB::adOpenForwardOnly,
				ADODB::adLockReadOnly,
				ADODB::adCmdText
				);
		if(FAILED(hr)) _com_issue_error(hr);

		//did user want to save to a file?
		if( pSQL2XMLInfo->pszFile && *pSQL2XMLInfo->pszFile )
		{
			CString szFile	= pSQL2XMLInfo->pszFile;
			int iFound = szFile.ReverseFind('.');
			if( iFound > 0 )
				szFile = szFile.Mid( 0, iFound+1) + "xml";
			else
				szFile = szFile + ".xml";

			DeleteFile( szFile );

			switch( pRS->GetState() ) 
			{
			case ADODB::adStateClosed:
				//case when the SQL string does not return data
			default:
				break;
			case ADODB::adStateOpen:
			case ADODB::adStateConnecting:
			case ADODB::adStateExecuting:
			case ADODB::adStateFetching:
				_variant_t  file(szFile);
				hr = pRS->Save( file, ADODB::adPersistXML );
				if(FAILED(hr)) _com_issue_error(hr);
				hr = pRS->Close();
				if(FAILED(hr)) _com_issue_error(hr);
				break;
			}
		}

		hr = pConnection->Close();
		if(FAILED(hr)) _com_issue_error(hr);

		CoUninitialize();

		pSQL2XMLInfo->hrError = 0;

		::PostMessage( pSQL2XMLInfo->m_hwndNotifyDone,
			WM_USER_SQL2XML_QUERYCOMPLETED, 0, 0);
		return 0;
	}

	catch(const _com_error& e)
	{
		pSQL2XMLInfo->hrError = e.Error();
		*(pSQL2XMLInfo->pErrorCString) = GetProviderError( pConnection );
		if(pSQL2XMLInfo->pErrorCString->IsEmpty())
			*(pSQL2XMLInfo->pErrorCString) = GetComError(e);
	}
	catch(COleException* e)
	{
		if(e)
		{
			TCHAR szMsg[255];
			e->GetErrorMessage(szMsg, 255);
			*(pSQL2XMLInfo->pErrorCString) = szMsg;
			pSQL2XMLInfo->hrError = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,e->m_sc);
			e->Delete();
		}
		else
		{
			pSQL2XMLInfo->hrError = E_POINTER;
			*(pSQL2XMLInfo->pErrorCString) = "COleException. No pointer available for error code.";
		}
	}
	catch(COleDispatchException* e)
	{
		if(e)
		{
			pSQL2XMLInfo->hrError = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,e->m_wCode);
			*(pSQL2XMLInfo->pErrorCString) = e->m_strDescription;
			e->Delete();
		}
		else
		{
			pSQL2XMLInfo->hrError = E_POINTER;
			*(pSQL2XMLInfo->pErrorCString) = "COleDispatchException. No pointer available for error code.";
		}
	}
	catch(CMemoryException* e)
	{
		if(e)
		{
			pSQL2XMLInfo->hrError = AFX_IDP_E_OUTOFMEMORY;
			TCHAR szMsg[255];
			e->GetErrorMessage(szMsg, 255);
			*(pSQL2XMLInfo->pErrorCString) = szMsg;
			e->Delete();
		}
		else
		{
			pSQL2XMLInfo->hrError = E_POINTER;
			*(pSQL2XMLInfo->pErrorCString) = "CMemoryException. No pointer available for error code.";
		}
	}
	catch(CFileException* e)
	{
		if(e)
		{
			pSQL2XMLInfo->hrError = HRESULT_FROM_WIN32(e->m_lOsError);
			TCHAR szMsg[255];
			e->GetErrorMessage(szMsg, 255);
			*(pSQL2XMLInfo->pErrorCString) = szMsg;
			e->Delete();
		}
		else
		{
			pSQL2XMLInfo->hrError = E_POINTER;
			*(pSQL2XMLInfo->pErrorCString) = "CFileException. No pointer available for error code.";
		}
	}
	catch(CArchiveException* e)
	{
		if(e)
		{
			pSQL2XMLInfo->hrError = HRESULT_FROM_WIN32(e->m_cause);
			TCHAR szMsg[255];
			e->GetErrorMessage(szMsg, 255);
			*(pSQL2XMLInfo->pErrorCString) = szMsg;
			e->Delete();
		}
		else
		{
			pSQL2XMLInfo->hrError = E_POINTER;
			*(pSQL2XMLInfo->pErrorCString) = "CArchiveException. No pointer available for error code.";
		}
	}
	catch(CException* e)
	{
		if(e)
		{
			pSQL2XMLInfo->hrError = E_UNEXPECTED;
			TCHAR szMsg[255];
			e->GetErrorMessage(szMsg, 255);
			*(pSQL2XMLInfo->pErrorCString) = szMsg;
			e->Delete();
		}
		else
		{
			pSQL2XMLInfo->hrError = E_POINTER;
			*(pSQL2XMLInfo->pErrorCString) = "CException. No pointer available for error code.";
		}
	}
	catch(...)
	{	
		pSQL2XMLInfo->hrError = E_UNEXPECTED;
		*(pSQL2XMLInfo->pErrorCString) = "Errors occurred.";
	}

	CoUninitialize();

	::PostMessage( pSQL2XMLInfo->m_hwndNotifyDone,
		WM_USER_SQL2XML_QUERYCOMPLETED, 0, 0);

	return 1;
}

BSTR CSQL2XMLCtrl::BuildDSN( LPCTSTR pszDSN )
{
	ADODB::_ConnectionPtr ptrConnection;
	HRESULT hr = S_OK;
	short bSuccess = 0;
	CString szError;
	CString szDSN;

	CWaitCursor wait;

	USES_CONVERSION;
	try
	{
		//create ADO dialog box instance
		MSDASC::IDataSourceLocatorPtr ptrDataSourceWnd = NULL;
		hr = ptrDataSourceWnd.CreateInstance(__uuidof(MSDASC::DataLinks));
		if(FAILED(hr)) _com_issue_error(hr);
    
		//put window handle
		hr = ptrDataSourceWnd->put_hWnd((long)this->m_hWnd);
		if(FAILED(hr)) _com_issue_error(hr);

		//initial string is null, so start fresh
		if( !pszDSN || !*pszDSN )
		{
			IDispatchPtr ptrDispatch = NULL;
			hr = ptrDataSourceWnd->PromptNew(&ptrDispatch);
			if(FAILED(hr)) _com_issue_error(hr);

			//finished with dialog box
			if(ptrDispatch != NULL)
			{
				ptrConnection = ptrDispatch;
				if( ((LPCTSTR)ptrConnection->ConnectionString) &&
					*((LPCTSTR)ptrConnection->ConnectionString) )
				{
					//copy string
					BSTR bstrConnectString = ptrConnection->ConnectionString;
					szDSN = W2CT(bstrConnectString);
				}
			}
		}
		else
		{
			//we were passed a connection string
			ptrConnection.CreateInstance(__uuidof(ADODB::Connection));
			COleVariant dsn(pszDSN);
			ptrConnection->ConnectionString = dsn.bstrVal;

			//check validity
			IDispatch* pDispatch = NULL;
			hr = ptrConnection.QueryInterface(IID_IDispatch, &pDispatch);
			if(FAILED(hr)) _com_issue_error(hr);

			//edit with ADO dialog box
			hr = ptrDataSourceWnd->PromptEdit(&pDispatch, &bSuccess);
			if(FAILED(hr))
			{
				//failed
				szError = "Invalid DSN. Pass NULL to create a new one";
				MessageBox( szError );
			}

			if(!bSuccess) // User clicked on <Cancel>
			{
				//just leave the old value
				szDSN = pszDSN;
			}
			else
			{
				//user clicked <OK> copy new value
				if( ((LPCTSTR)ptrConnection->ConnectionString) &&
					*((LPCTSTR)ptrConnection->ConnectionString) )
				{
					//copy string
					BSTR bstrConnectString = ptrConnection ->ConnectionString;
					szDSN = W2CT(bstrConnectString);
				}
			}

			if(pDispatch != NULL)
			{
				pDispatch->Release();
				pDispatch = NULL;
			}
		}

		if(ptrDataSourceWnd != NULL)
		{
			ptrDataSourceWnd.Release();
			ptrDataSourceWnd = NULL;
		}
	}

	catch(const _com_error& e)
	{
		hr = e.Error();
		szError = GetProviderError( ptrConnection );
		if(szError.IsEmpty())
			szError = GetComError(e);
	}
	catch(COleException* e)
	{
		if(e)
		{
			TCHAR szMsg[255];
			e->GetErrorMessage(szMsg, 255);
			szError = szMsg;
			hr = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,e->m_sc);
			e->Delete();
		}
		else
		{
			hr = E_POINTER;
			szError = "COleException. No pointer available for error code.";
		}
	}
	catch(COleDispatchException* e)
	{
		if(e)
		{
			hr = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,e->m_wCode);
			szError = e->m_strDescription;
			e->Delete();
		}
		else
		{
			hr = E_POINTER;
			szError = "COleDispatchException. No pointer available for error code.";
		}
	}
	catch(CMemoryException* e)
	{
		if(e)
		{
			hr = AFX_IDP_E_OUTOFMEMORY;
			TCHAR szMsg[255];
			e->GetErrorMessage(szMsg, 255);
			szError = szMsg;
			e->Delete();
		}
		else
		{
			hr = E_POINTER;
			szError = "CMemoryException. No pointer available for error code.";
		}
	}
	catch(CException* e)
	{
		if(e)
		{
			hr = E_UNEXPECTED;
			TCHAR szMsg[255];
			e->GetErrorMessage(szMsg, 255);
			szError = szMsg;
			e->Delete();
		}
		else
		{
			hr = E_POINTER;
			szError = "CException. No pointer available for error code.";
		}
	}
	catch(...)
	{	
		hr = E_UNEXPECTED;
		szError = "Errors occurred.";
	}
	if( !szError.IsEmpty() )
		MessageBox( szError );

	return szDSN.AllocSysString();
}

CString GetProviderError( ADODB::_ConnectionPtr ptrConnection )
{
	CString sErrors;
	if( ptrConnection != NULL )
	{
		ADODB::ErrorsPtr ptrErrors = ptrConnection->Errors;
		long lCount = ptrErrors->Count;
		ADODB::ErrorPtr ptrError = NULL;
		CString sError;
		for(long n = 0; n < lCount; n++)
		{
			ptrError = ptrErrors->GetItem(n);
			sError.Format(
				_T("%s\nState: %s, Native: %d, Source: %s"),
				(LPCTSTR)ptrError->Description,
				(LPCTSTR)ptrError->SQLState,
				ptrError->NativeError,
				(LPCTSTR)ptrError->Source
				);
			sErrors += sError + "\n\n";
		}
	}

	if(!sErrors.IsEmpty())
		sErrors = sErrors.Left(sErrors.GetLength()-2);

	return sErrors;
}

CString GetComError(const _com_error& e)
{
	CString sMsg;
	sMsg.Format(
		_T("HRESULT: 0x%08lx; Error: %s\n"),
		e.Error(),
		e.ErrorMessage()
		);
	
	if(e.ErrorInfo())
	{
		sMsg += "Source: " + CString((LPCTSTR)e.Source()) +
			"; Description: " + CString((LPCTSTR)e.Description());
	}
	else
	{
		sMsg += e.Description();
	}

	return sMsg;
}


long CSQL2XMLCtrl::GetControlPointer() 
{
	return reinterpret_cast<long>(this);
}

void CSQL2XMLCtrl::SetControlPointer(long nNewValue) 
{
	SetNotSupported();
}
