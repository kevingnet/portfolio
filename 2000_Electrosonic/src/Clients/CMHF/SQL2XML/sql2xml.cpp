// SQL2XML.cpp : Implementation of CSQL2XMLApp and DLL registration.

#include "stdafx.h"
#include "SQL2XML.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CSQL2XMLApp NEAR theApp;

const GUID CDECL BASED_CODE _tlid =
		{ 0x703ef6e1, 0x2ce, 0x11d5, { 0xb9, 0x5d, 0, 0x10, 0x4b, 0x6d, 0x42, 0xee } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;


////////////////////////////////////////////////////////////////////////////
// CSQL2XMLApp::InitInstance - DLL initialization

BOOL CSQL2XMLApp::InitInstance()
{
	BOOL bInit = COleControlModule::InitInstance();

	if (bInit)
	{
		// TODO: Add your own module initialization code here.
	}

	return bInit;
}


////////////////////////////////////////////////////////////////////////////
// CSQL2XMLApp::ExitInstance - DLL termination

int CSQL2XMLApp::ExitInstance()
{
	// TODO: Add your own module termination code here.

	return COleControlModule::ExitInstance();
}


/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
		return ResultFromScode(SELFREG_E_CLASS);

	return NOERROR;
}


/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))
		return ResultFromScode(SELFREG_E_CLASS);

	return NOERROR;
}


/////////////////////////////////////////////////////////////////////////////
// _DSQL2XML properties

CString _DSQL2XML::GetDsn()
{
	CString result;
	GetProperty(0x1, VT_BSTR, (void*)&result);
	return result;
}

void _DSQL2XML::SetDsn(LPCTSTR propVal)
{
	SetProperty(0x1, VT_BSTR, propVal);
}

CString _DSQL2XML::GetSQLString()
{
	CString result;
	GetProperty(0x2, VT_BSTR, (void*)&result);
	return result;
}

void _DSQL2XML::SetSQLString(LPCTSTR propVal)
{
	SetProperty(0x2, VT_BSTR, propVal);
}

CString _DSQL2XML::GetXMLFile()
{
	CString result;
	GetProperty(0x3, VT_BSTR, (void*)&result);
	return result;
}

void _DSQL2XML::SetXMLFile(LPCTSTR propVal)
{
	SetProperty(0x3, VT_BSTR, propVal);
}

CString _DSQL2XML::GetCaption()
{
	CString result;
	GetProperty(DISPID_CAPTION, VT_BSTR, (void*)&result);
	return result;
}

void _DSQL2XML::SetCaption(LPCTSTR propVal)
{
	SetProperty(DISPID_CAPTION, VT_BSTR, propVal);
}

CString _DSQL2XML::Get_Caption()
{
	CString result;
	GetProperty(0x0, VT_BSTR, (void*)&result);
	return result;
}

void _DSQL2XML::Set_Caption(LPCTSTR propVal)
{
	SetProperty(0x0, VT_BSTR, propVal);
}

long _DSQL2XML::GetControlPointer()
{
	long result;
	GetProperty(0x4, VT_I4, (void*)&result);
	return result;
}

void _DSQL2XML::SetControlPointer(long propVal)
{
	SetProperty(0x4, VT_I4, propVal);
}

/////////////////////////////////////////////////////////////////////////////
// _DSQL2XML operations

CString _DSQL2XML::BuildDSN(LPCTSTR pszDSN)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x5, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		pszDSN);
	return result;
}

void _DSQL2XML::Execute(LPCTSTR pszDSN, LPCTSTR pszSQLString, LPCTSTR pszXMLFile)
{
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR VTS_BSTR;
	InvokeHelper(0x6, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 pszDSN, pszSQLString, pszXMLFile);
}

CString _DSQL2XML::GetLastErrorString()
{
	CString result;
	InvokeHelper(0x7, DISPATCH_METHOD, VT_BSTR, (void*)&result, NULL);
	return result;
}

long _DSQL2XML::GetLastError()
{
	long result;
	InvokeHelper(0x8, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

void _DSQL2XML::DoClick()
{
	InvokeHelper(DISPID_DOCLICK, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void _DSQL2XML::AboutBox()
{
	InvokeHelper(0xfffffdd8, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}
