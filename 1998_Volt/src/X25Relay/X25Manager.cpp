// X25Manager.cpp : implementation file
//

#include "stdafx.h"
#include "X25Relay.h"
#include "X25Manager.h"
#include "X25Line.h"
#include "X25PVC.h"
#include "ListenSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define szCONNECTION_STRING "Provider=MSDASQL.1;Persist Security Info=False;Data Source=X25Relay"
#define szTABLE_DEFAULTS "tblDefaults"
#define szTABLE_SERVICES "qryServices"

CTrace g_TraceForManager(_T("X.25 Manager"));
CTrace g_TraceForX25(_T("X.25 Line"));
CTrace g_TraceForPVCSocket(_T("X.25 PVC"));
CTrace g_TraceForAdminListenSocket(_T("Admin Listen Socket"));
CTrace g_TraceForAdminSocket(_T("Admin Socket"));
CTrace g_TraceForPVCListenSocket(_T("PVC Listen Socket"));

#define CATCH_ERRORS \
	catch(const _com_error& e) \
	{ \
		CalcProviderError(); \
		if(m_szError.IsEmpty()) \
			CalcComError(e); \
		OWS_0trace( (*m_pCTrace), LEVEL_ERROR, m_szError ); \
		m_Result = INVALID_VALUE; \
	} \
	catch( CException * e ) \
	{ \
		CalcExceptionError( e ); \
		OWS_0trace( (*m_pCTrace), LEVEL_ERROR, m_szError ); \
		m_Result = INVALID_VALUE; \
	}

/////////////////////////////////////////////////////////////////////////////
// CX25Manager

CX25Manager::CX25Manager()
{
	m_pCTrace		= NULL;
	m_bInitialized	= false;
	m_bStarted		= false;
}

CX25Manager::~CX25Manager()
{
	Shutdown();
	Deinitialize();
}

/////////////////////////////////////////////////////////////////////////////
// CX25Manager member functions

LPX25PVC CX25Manager::GetPVC( int idx )
{
	if( idx < 0 || idx >= m_PVCs.GetSize() )
		return NULL;
	return (LPX25PVC)m_PVCs.GetAt( idx );
}

void CX25Manager::GetDefaults( ADODB::_RecordsetPtr prs )
{
	try
	{
		HRESULT hr = prs->Open(
				_bstr_t((LPCTSTR)szTABLE_DEFAULTS),
				m_pConn.GetInterfacePtr(),
				ADODB::adOpenForwardOnly,
				ADODB::adLockReadOnly,
				ADODB::adCmdTable
				);
		if( FAILED(hr) )
			_com_issue_error(hr);

		_variant_t IntVal;
		IntVal.ChangeType( VT_I4 );
		_variant_t BoolVal;
		BoolVal.ChangeType( VT_BOOL );

		if( !prs->adoEOF )
		{
			OWS_0trace( (*m_pCTrace), LEVEL_INFO, "Reading defaults..." );

			m_Defaults.szConfigurationFile = (LPCTSTR)(_bstr_t)prs->Fields->
				GetItem(_variant_t((long)FIELD_CONFIGFILE))->Value;

			m_Defaults.szConfigurationPath = (LPCTSTR)(_bstr_t)prs->Fields->
				GetItem(_variant_t((long)FIELD_CONFIGPATH))->Value;

			IntVal = prs->Fields->
				GetItem(_variant_t((long)FIELD_BASEPORT))->Value;
			m_Defaults.iBasePort = IntVal.lVal;

			CX25PVC::sm_iBasePort = m_Defaults.iBasePort+1;

			CAdminSocket::sm_iBasePort = m_Defaults.iBasePort;

			BoolVal = prs->Fields->
				GetItem(_variant_t((long)FIELD_AUTOSTARTUP))->Value;
			m_Defaults.m_bAutoStartup = (BoolVal.boolVal==0?false:true);

			IntVal = prs->Fields->
				GetItem(_variant_t((long)FIELD_POLLINGRATE))->Value;
			m_Defaults.iPollingRate = IntVal.lVal;

			IntVal = prs->Fields->
				GetItem(_variant_t((long)FIELD_CONTROLSREFRESHRATE))->Value;
			m_Defaults.iControlsRefreshRate = IntVal.lVal;

			BoolVal = prs->Fields->
				GetItem(_variant_t((long)FIELD_REFRESHWHENINACTIVE))->Value;
			m_Defaults.bRefreshWhenInactive = (BoolVal.boolVal==0?false:true);

		}

		hr = prs->Close();
		if(FAILED(hr))
			_com_issue_error(hr);

		if( m_Defaults.szConfigurationPath.Right(1) != '\\' )
			m_Defaults.szConfigurationPath += '\\';
	}
	CATCH_ERRORS
}

Result CX25Manager::Initialize()
{
	if( true == m_bInitialized )
		return -1;

	try
	{
		m_pCTrace = &g_TraceForManager;

		m_AdminListenSocket.SetTrace( &g_TraceForAdminListenSocket );
		m_AdminListenSocket.m_AdminSocket.SetTrace( &g_TraceForAdminSocket );

		LPADMINSOCKET pAdminSocket = NULL;
		pAdminSocket = m_AdminListenSocket.GetAdminSocket();
		pAdminSocket->m_pListenAdminSocket = &m_AdminListenSocket;
		pAdminSocket->m_pX25Manager = this;

		m_Result = SUCCESS;
		HRESULT hr;

		OWS_0trace( (*m_pCTrace), LEVEL_FUNCTIONCALLS, "Initializing..." );

		m_szConnection = gpApp->GetProfileString( "Settings", "ConnectionString" );

		if( m_szConnection == "" )
		{
			m_szConnection = szCONNECTION_STRING;
			gpApp->WriteProfileString( "Settings", "ConnectionString", m_szConnection );
		}

		hr = m_pConn.CreateInstance(__uuidof(ADODB::Connection));
		if(FAILED(hr))
			_com_issue_error(hr);

		hr = m_pConn->Open(
				_bstr_t((LPCTSTR)m_szConnection),
				_bstr_t(L""),
				_bstr_t(L""),
				ADODB::adConnectUnspecified
				);
		if(FAILED(hr))
			_com_issue_error(hr);

		ADODB::_RecordsetPtr	prs;

		hr = prs.CreateInstance(__uuidof(ADODB::Recordset));
		if(FAILED(hr))
			_com_issue_error(hr);

		GetDefaults( prs );

		hr = m_pConn->Close();
		if(FAILED(hr))
			_com_issue_error(hr);

		m_bInitialized = true;

		if( m_Defaults.m_bAutoStartup == true )
			Startup( RECALC_DEFAULTS_FALSE );
	}
	CATCH_ERRORS

	return ( m_Result );
}

void CX25Manager::Deinitialize()
{
	m_bInitialized	= false;
	m_bStarted		= false;
}

Result CX25Manager::Startup( bool bRecalcDefaults )
{
	if( false == m_bInitialized )
		return -1;

	if( true == m_bStarted )
		return -1;

	try
	{
		m_Result = SUCCESS;
		HRESULT hr;

		OWS_0trace( (*m_pCTrace), LEVEL_FUNCTIONCALLS, "Starting..." );

		_variant_t IntVal;
		IntVal.ChangeType( VT_I4 );
		_variant_t BoolVal;
		BoolVal.ChangeType( VT_BOOL );

		hr = m_pConn->Open(
				_bstr_t((LPCTSTR)m_szConnection),
				_bstr_t(L""),
				_bstr_t(L""),
				ADODB::adConnectUnspecified
				);
		if(FAILED(hr))
			_com_issue_error(hr);

		ADODB::_RecordsetPtr	prs;

		hr = prs.CreateInstance(__uuidof(ADODB::Recordset));
		if(FAILED(hr))
			_com_issue_error(hr);

		if( bRecalcDefaults == true )
		{
			GetDefaults( prs );
		}

		OWS_0trace( (*m_pCTrace), LEVEL_INFO, "Initializing Protocols..." );

		m_Result = lgo_ConfigureStack( (LPSTR)(LPCTSTR)m_Defaults.szConfigurationFile );
		ReportError( "Fatal error: lgo_ConfigureStack", m_Result );

		m_Result = lgo_InitializeStack( INTERRUPT_DRIVEN, TIMER_MULTIPLIER );
		ReportError( "Fatal error: lgo_InitializeStack", m_Result );

		m_Result = lgo_EnableStack();
		ReportError( "Fatal error: lgo_EnableStack", m_Result );

		hr = prs->Open(
				_bstr_t((LPCTSTR)szTABLE_SERVICES),
				m_pConn.GetInterfacePtr(),
				ADODB::adOpenForwardOnly,
				ADODB::adLockReadOnly,
				ADODB::adCmdUnspecified
				);
		if( FAILED(hr) )
			_com_issue_error(hr);

		OWS_0trace( (*m_pCTrace), LEVEL_INFO, "Initializing Lines and PVC's..." );

		tblProtocolService	ps;
		MinorId				minorId			= 0;
		char 				bufPhysical	[PROTO_BUF_SIZE];
		char 				bufLink		[PROTO_BUF_SIZE];
		char 				bufPacket	[PROTO_BUF_SIZE];

		CString				szLastPhysicalConfigFile;
		CString				szLastLinkConfigFile;
		CString				szLastPacketConfigFile;

		CString				szConfigurationFile;

		//used for getting packet size
		PktConfigStruct *	pPacketStruct	= NULL;

		LPLINE				pLine			= NULL;
		LPLISTENSOCKET		pListenSocket	= NULL;
		LPX25PVC			pPVCSocket		= NULL;

		int					iLine			= 0;
		int					iPVC			= 0;

			ps.majorId			= 0;
			ps.iPacketSize		= 0;
			ps.iPVCCount		= 0;

		while( !prs->adoEOF )
		{
			//zero out...
			ps.szPhysical.Empty();
			ps.szLink.Empty();
			ps.szPacket.Empty();
			ps.iConnectionType	= CONNECTION_TYPE_DISABLED;
			ps.physical			= lgo_INVALID_CID;
			ps.link				= lgo_INVALID_CID;
			ps.packet			= lgo_INVALID_CID;
			ps.pvc				= lgo_INVALID_CID;

			//get line parameters

			IntVal = prs->Fields->
				GetItem(_variant_t((long)SERVICES_FIELD_MAJORDEVICE))->Value;
			ps.majorId = (MajorId)IntVal.lVal;

			IntVal = prs->Fields->
				GetItem(_variant_t((long)SERVICES_FIELD_PROTOPHYSICAL))->Value;
			ps.physical = (Protocol)IntVal.lVal;

			IntVal = prs->Fields->
				GetItem(_variant_t((long)SERVICES_FIELD_PROTOLINK))->Value;
			ps.link = (Protocol)IntVal.lVal;

			IntVal = prs->Fields->
				GetItem(_variant_t((long)SERVICES_FIELD_PROTOPACKET))->Value;
			ps.packet = (Protocol)IntVal.lVal;

			ps.szPhysical = (LPCTSTR)(_bstr_t)prs->Fields->
				GetItem(_variant_t((long)SERVICES_FIELD_CONFIGPHYSICAL))->Value;

			ps.szLink = (LPCTSTR)(_bstr_t)prs->Fields->
				GetItem(_variant_t((long)SERVICES_FIELD_CONFIGLINK))->Value;

			ps.szPacket = (LPCTSTR)(_bstr_t)prs->Fields->
				GetItem(_variant_t((long)SERVICES_FIELD_CONFIGPACKET))->Value;

			IntVal = prs->Fields->
				GetItem(_variant_t((long)SERVICES_FIELD_PROTOPVC))->Value;
			ps.pvc = (Protocol)IntVal.lVal;

			IntVal = prs->Fields->
				GetItem(_variant_t((long)SERVICES_FIELD_MAXXOFFBUFFER))->Value;
			ps.iMaxXOffBuffer = IntVal.lVal;

			IntVal = prs->Fields->
				GetItem(_variant_t((long)SERVICES_FIELD_CONNECTIONTYPE))->Value;
			ps.iConnectionType = IntVal.lVal;

			//create line
			pLine						= new CX25Line;
			pLine->SetTrace( &g_TraceForX25 );

			pLine->m_iLineNumber		= iLine++;
			pLine->m_iPVCCount			= ps.iPVCCount;
			pLine->m_MajorId			= ps.majorId;
			pLine->m_ConnectionType		= ps.iConnectionType;

			m_Lines.Add( pLine );

			if( pLine->m_ConnectionType != CONNECTION_TYPE_DISABLED )
			{
				//setup lines

				if( 0 != szLastPhysicalConfigFile.CompareNoCase( ps.szPhysical ) ) //if not equal
				{
					//load configuration
					szConfigurationFile = m_Defaults.szConfigurationPath + ps.szPhysical;
					szLastPhysicalConfigFile = ps.szPhysical;
					m_Result = lgo_LoadConfiguration(	(char*)(LPCTSTR)szConfigurationFile, 
														ps.physical, 
														bufPhysical, 
														PROTO_BUF_SIZE );
					ReportError( "lgo_LoadConfiguration", m_Result );
				}

				if( 0 != szLastLinkConfigFile.CompareNoCase( ps.szLink ) ) //if not equal
				{
					//load configuration
					szConfigurationFile = m_Defaults.szConfigurationPath + ps.szLink;
					szLastLinkConfigFile = ps.szLink;
					m_Result = lgo_LoadConfiguration(	(char*)(LPCTSTR)szConfigurationFile, 
														ps.link, 
														bufLink, 
														PROTO_BUF_SIZE );
					ReportError( "lgo_LoadConfiguration", m_Result );
				}

				if( 0 != szLastPacketConfigFile.CompareNoCase( ps.szPacket ) ) //if not equal
				{
					//load configuration
					szConfigurationFile = m_Defaults.szConfigurationPath + ps.szPacket;
					szLastPacketConfigFile = ps.szPacket;
					m_Result = lgo_LoadConfiguration(	(char*)(LPCTSTR)szConfigurationFile, 
														ps.packet, 
														bufPacket, 
														PROTO_BUF_SIZE );
					ReportError( "lgo_LoadConfiguration", m_Result );
					
					pPacketStruct	= (PktConfigStruct*)bufPacket;
					ps.iPVCCount	= pPacketStruct->pvcCount;
					ps.iPacketSize	= pPacketStruct->pvcConfig.packetSize;
				}

				//allocate buffer memory
				pLine->m_iMaxBufferSize = ps.iPacketSize;
				if( NULL == pLine->m_Buffer )
					pLine->m_Buffer = new BYTE[pLine->m_iMaxBufferSize+HEADER_BYTE_COUNT];

				pLine->m_CIDPhysical = lgo_OpenProtocol( ps.physical, ps.majorId, minorId );
				ReportError( "lgo_OpenProtocol Physical Layer", pLine->m_CIDPhysical );
				m_Result = lgo_SetConfiguration( pLine->m_CIDPhysical, bufPhysical, *(BufferSize *)bufPhysical );
				ReportError( "lgo_SetConfiguration Physical Layer", m_Result );

				pLine->m_CIDLink = lgo_OpenProtocol( ps.link, ps.majorId, minorId );
				ReportError( "lgo_OpenProtocol Link Layer", pLine->m_CIDLink );
				m_Result = lgo_SetConfiguration( pLine->m_CIDLink, bufLink, *(BufferSize *)bufLink );
				ReportError( "lgo_SetConfiguration Link Layer", m_Result );

				pLine->m_CIDPacket = lgo_OpenProtocol( ps.packet, ps.majorId, minorId );
				ReportError( "lgo_OpenProtocol Packet Layer", pLine->m_CIDPacket );
				m_Result = lgo_SetConfiguration( pLine->m_CIDPacket, bufPacket, *(BufferSize *)bufPacket );
				ReportError( "lgo_SetConfiguration Packet Layer", m_Result );

				m_Result = lgo_Push( pLine->m_CIDPhysical, pLine->m_CIDLink );
				ReportError( "lgo_Push Link Layer -> Physical Layer", m_Result );

				m_Result = lgo_Push( pLine->m_CIDLink, pLine->m_CIDPacket );
				ReportError( "lgo_Push Packet Layer -> Link Layer", m_Result );

				pLine->m_LineState = CX25Line::LINE_STATE_OPEN;

			}

			//setup PVC's for line
			for( int i=0; i<ps.iPVCCount; i++ )
			{
				pListenSocket = new CListenSocket;
				pListenSocket->SetTrace( &g_TraceForPVCListenSocket );

				pPVCSocket						= pListenSocket->GetCommSocket();
				pPVCSocket->SetTrace( &g_TraceForPVCSocket );

				pPVCSocket->m_ConnectionType	= ps.iConnectionType;
				pPVCSocket->m_pListenSocket		= pListenSocket;
				pPVCSocket->m_pLine				= pLine;
				pPVCSocket->m_iPVCNumber		= iPVC;
				pPVCSocket->m_iMaxXOffBuffer	= ps.iMaxXOffBuffer;

				if( pPVCSocket->m_ConnectionType != CONNECTION_TYPE_DISABLED )
				{
					pListenSocket->OpenListenPort( CX25PVC::sm_iBasePort+iPVC );

					pPVCSocket->m_CIDPVC = lgo_OpenProtocol( ps.pvc, ps.majorId, (MinorId)i+1 );
					ReportError( "lgo_OpenProtocol PVC", pPVCSocket->m_CIDPVC );
				}

				iPVC++;
				m_PVCs.Add( pPVCSocket );
			}

			prs->MoveNext();
		}

		hr = prs->Close();
		if(FAILED(hr))
			_com_issue_error(hr);

		hr = m_pConn->Close();
		if(FAILED(hr))
			_com_issue_error(hr);

		m_bStarted = true;
	}
	CATCH_ERRORS
	return ( m_Result );
}

void CX25Manager::Shutdown()
{
	if( false == m_bInitialized )
		return;

	if( false == m_bStarted )
		return;

	try
	{
		OWS_0trace( (*m_pCTrace), LEVEL_FUNCTIONCALLS, "Shutting down..." );

		LPX25PVC pPVC	= NULL;
		LPLINE pLine	= NULL;

		for( int i=0; i<m_PVCs.GetSize(); i++ )
		{
			pPVC = (LPX25PVC)m_PVCs.GetAt( i );
			if( pPVC )
			{
				pPVC->m_pListenSocket->ShutDown();
				pPVC->m_pListenSocket->Close();
				pPVC->ShutDown();
				pPVC->Close();

				if( pPVC->m_ConnectionType != CONNECTION_TYPE_DISABLED )
				{
					m_Result = lgo_DisconnectRequest( pPVC->m_CIDPVC, NULL, 0 );
					ReportError( "lgo_DisconnectRequest PVC", m_Result );
					m_Result = lgo_Close( pPVC->m_CIDPVC );
					ReportError( "lgo_Close PVC", m_Result );
				}
			}
		}
		
		for( i=0; i<m_Lines.GetSize(); i++ )
		{
			pLine = (LPLINE)m_Lines.GetAt(i);

			if( pLine && pLine->m_ConnectionType != CONNECTION_TYPE_DISABLED )
			{
				// Disconnect the m_CIDPhysical, m_CIDLink and X.25 major devices
				m_Result = lgo_DisconnectRequest( pLine->m_CIDPacket, NULL, 0 );
				ReportError( "lgo_DisconnectRequest Packet Layer", m_Result );
				m_Result = lgo_DisconnectRequest( pLine->m_CIDLink, NULL, 0 );
				ReportError( "lgo_DisconnectRequest Link Layer", m_Result );
				m_Result = lgo_DisconnectRequest( pLine->m_CIDPhysical, NULL, 0 );
				ReportError( "lgo_DisconnectRequest Physical Layer", m_Result );

				// remove m_CIDPacket layer from top of device and close it
				m_Result = lgo_Pop( pLine->m_CIDPacket );
				ReportError( "lgo_Pop Packet Layer", m_Result );
				m_Result = lgo_Close( pLine->m_CIDPacket );
				ReportError( "lgo_Close Packet Layer", m_Result );

				// remove m_CIDLink layer from new top of device and close it
				m_Result = lgo_Pop( pLine->m_CIDLink );
				ReportError( "lgo_Pop Link Layer", m_Result );
				m_Result = lgo_Close( pLine->m_CIDLink );
				ReportError( "lgo_Close Link Layer", m_Result );

				// m_CIDPhysical layer is always the bottom, is never pushed or popped
				m_Result = lgo_Close( pLine->m_CIDPhysical );
				ReportError( "lgo_Close Physical Layer", m_Result );

				//dealocate buffer memory
			}
		}

		for( i=0; i<m_PVCs.GetSize(); i++ )
		{
			pPVC = (LPX25PVC)m_PVCs.GetAt( i );
			if( pPVC && pPVC->m_pListenSocket )
				delete pPVC->m_pListenSocket; //this calls it's destructor, tricky, I know...
		}
		
		for( i=0; i<m_Lines.GetSize(); i++ )
		{
			pLine = (LPLINE)m_Lines.GetAt(i);
			if( pLine )
			{
				delete pLine;
			}
		}

		m_Result = lgo_DisableStack();
		ReportError( "Fatal error: lgo_DisableStack", m_Result );

		// protocol stack cleanup at program termination
		m_Result = lgo_UninitializeStack();
		ReportError( "Error: lgo_UninitializeStack", m_Result );

		m_PVCs.RemoveAll();
		m_Lines.RemoveAll();

		m_bStarted = false;
	}
	CATCH_ERRORS
}

bool CX25Manager::RestartLine( int iLineNumber )
{
	if( false == m_bInitialized )
		return false;

	if( false == m_bStarted )
		return false;

	try
	{
		m_Result = SUCCESS;
		HRESULT hr;

		OWS_1trace( (*m_pCTrace), LEVEL_FUNCTIONCALLS, "Restarting line #i...", iLineNumber );

		_variant_t IntVal;
		IntVal.ChangeType( VT_I4 );
		_variant_t BoolVal;
		BoolVal.ChangeType( VT_BOOL );

		hr = m_pConn->Open(
				_bstr_t((LPCTSTR)m_szConnection),
				_bstr_t(L""),
				_bstr_t(L""),
				ADODB::adConnectUnspecified
				);
		if(FAILED(hr))
			_com_issue_error(hr);

		ADODB::_RecordsetPtr	prs;

		hr = prs.CreateInstance(__uuidof(ADODB::Recordset));
		if(FAILED(hr))
			_com_issue_error(hr);

		OWS_0trace( (*m_pCTrace), LEVEL_INFO, "Initializing Protocol..." );

		hr = prs->Open(
				_bstr_t((LPCTSTR)szTABLE_SERVICES),
				m_pConn.GetInterfacePtr(),
				ADODB::adOpenForwardOnly,
				ADODB::adLockReadOnly,
				ADODB::adCmdUnspecified
				);
		if( FAILED(hr) )
			_com_issue_error(hr);

		OWS_0trace( (*m_pCTrace), LEVEL_INFO, "Initializing Line and PVC's..." );

		tblProtocolService	ps;
		MinorId				minorId			= 0;
		char 				bufPhysical	[PROTO_BUF_SIZE];
		char 				bufLink		[PROTO_BUF_SIZE];
		char 				bufPacket	[PROTO_BUF_SIZE];

		CString				szLastPhysicalConfigFile;
		CString				szLastLinkConfigFile;
		CString				szLastPacketConfigFile;

		CString				szConfigurationFile;

		//used for getting packet size
		PktConfigStruct *	pPacketStruct	= NULL;

		LPLINE				pLine			= NULL;
		LPLISTENSOCKET		pListenSocket	= NULL;
		LPX25PVC			pPVC			= NULL;

		int					iLine			= 0;
		int					iPVC			= 0;

		while( !prs->adoEOF )
		{
			IntVal = prs->Fields->
				GetItem(_variant_t((long)SERVICES_FIELD_MAJORDEVICE))->Value;
			
			if( iLineNumber == IntVal.lVal )
				break;

			prs->MoveNext();
		}

		//zero out...
		ps.szPhysical.Empty();
		ps.szLink.Empty();
		ps.szPacket.Empty();
		ps.iConnectionType	= CONNECTION_TYPE_DISABLED;
		ps.physical			= lgo_INVALID_CID;
		ps.link				= lgo_INVALID_CID;
		ps.packet			= lgo_INVALID_CID;
		ps.pvc				= lgo_INVALID_CID;
		ps.majorId			= 0;
		ps.iPacketSize		= 0;
		ps.iPVCCount		= 0;

		pLine = (LPLINE)m_Lines.GetAt( iLineNumber );

		//get line parameters

		IntVal = prs->Fields->
			GetItem(_variant_t((long)SERVICES_FIELD_MAJORDEVICE))->Value;
		ps.majorId = (MajorId)IntVal.lVal;

		IntVal = prs->Fields->
			GetItem(_variant_t((long)SERVICES_FIELD_PROTOPHYSICAL))->Value;
		ps.physical = (Protocol)IntVal.lVal;

		IntVal = prs->Fields->
			GetItem(_variant_t((long)SERVICES_FIELD_PROTOLINK))->Value;
		ps.link = (Protocol)IntVal.lVal;

		IntVal = prs->Fields->
			GetItem(_variant_t((long)SERVICES_FIELD_PROTOPACKET))->Value;
		ps.packet = (Protocol)IntVal.lVal;

		ps.szPhysical = (LPCTSTR)(_bstr_t)prs->Fields->
			GetItem(_variant_t((long)SERVICES_FIELD_CONFIGPHYSICAL))->Value;

		ps.szLink = (LPCTSTR)(_bstr_t)prs->Fields->
			GetItem(_variant_t((long)SERVICES_FIELD_CONFIGLINK))->Value;

		ps.szPacket = (LPCTSTR)(_bstr_t)prs->Fields->
			GetItem(_variant_t((long)SERVICES_FIELD_CONFIGPACKET))->Value;

		IntVal = prs->Fields->
			GetItem(_variant_t((long)SERVICES_FIELD_PROTOPVC))->Value;
		ps.pvc = (Protocol)IntVal.lVal;

		IntVal = prs->Fields->
			GetItem(_variant_t((long)SERVICES_FIELD_MAXXOFFBUFFER))->Value;
		ps.iMaxXOffBuffer = IntVal.lVal;

		IntVal = prs->Fields->
			GetItem(_variant_t((long)SERVICES_FIELD_CONNECTIONTYPE))->Value;
		ps.iConnectionType = IntVal.lVal;

		pLine->m_ConnectionType		= ps.iConnectionType;

		if( pLine->m_ConnectionType != CONNECTION_TYPE_DISABLED )
		{

			//setup line

			szConfigurationFile = m_Defaults.szConfigurationPath + ps.szPhysical;
			szLastPhysicalConfigFile = ps.szPhysical;
			m_Result = lgo_LoadConfiguration(	(char*)(LPCTSTR)szConfigurationFile, 
												ps.physical, 
												bufPhysical, 
												PROTO_BUF_SIZE );
			ReportError( "lgo_LoadConfiguration", m_Result );

			szConfigurationFile = m_Defaults.szConfigurationPath + ps.szLink;
			szLastLinkConfigFile = ps.szLink;
			m_Result = lgo_LoadConfiguration(	(char*)(LPCTSTR)szConfigurationFile, 
												ps.link, 
												bufLink, 
												PROTO_BUF_SIZE );
			ReportError( "lgo_LoadConfiguration", m_Result );

			szConfigurationFile = m_Defaults.szConfigurationPath + ps.szPacket;
			szLastPacketConfigFile = ps.szPacket;
			m_Result = lgo_LoadConfiguration(	(char*)(LPCTSTR)szConfigurationFile, 
												ps.packet, 
												bufPacket, 
												PROTO_BUF_SIZE );
			ReportError( "lgo_LoadConfiguration", m_Result );
			
			pPacketStruct	= (PktConfigStruct*)bufPacket;
			ps.iPVCCount	= pPacketStruct->pvcCount;
			ps.iPacketSize	= pPacketStruct->pvcConfig.packetSize;

			if( ps.iPVCCount != pLine->m_iPVCCount )
			{
				//we cannot restart the line when the number of PVCs change
				//it may cause other PVC's to be offset
				return false;
			}

			// Disconnect the m_CIDPhysical, m_CIDLink and X.25 major devices
			m_Result = lgo_DisconnectRequest( pLine->m_CIDPacket, NULL, 0 );
			ReportError( "lgo_DisconnectRequest Packet Layer", m_Result );
			m_Result = lgo_DisconnectRequest( pLine->m_CIDLink, NULL, 0 );
			ReportError( "lgo_DisconnectRequest Link Layer", m_Result );
			m_Result = lgo_DisconnectRequest( pLine->m_CIDPhysical, NULL, 0 );
			ReportError( "lgo_DisconnectRequest Physical Layer", m_Result );

			// remove m_CIDPacket layer from top of device and close it
			m_Result = lgo_Pop( pLine->m_CIDPacket );
			ReportError( "lgo_Pop Packet Layer", m_Result );
			m_Result = lgo_Close( pLine->m_CIDPacket );
			ReportError( "lgo_Close Packet Layer", m_Result );

			// remove m_CIDLink layer from new top of device and close it
			m_Result = lgo_Pop( pLine->m_CIDLink );
			ReportError( "lgo_Pop Link Layer", m_Result );
			m_Result = lgo_Close( pLine->m_CIDLink );
			ReportError( "lgo_Close Link Layer", m_Result );

			// m_CIDPhysical layer is always the bottom, is never pushed or popped
			m_Result = lgo_Close( pLine->m_CIDPhysical );
			ReportError( "lgo_Close Physical Layer", m_Result );

			//allocate buffer memory
			pLine->m_iMaxBufferSize = ps.iPacketSize;
			if( NULL != pLine->m_Buffer )
				delete pLine->m_Buffer;
			pLine->m_Buffer = new BYTE[pLine->m_iMaxBufferSize+HEADER_BYTE_COUNT];

			pLine->m_CIDPhysical = lgo_OpenProtocol( ps.physical, ps.majorId, minorId );
			ReportError( "lgo_OpenProtocol Physical Layer", pLine->m_CIDPhysical );
			m_Result = lgo_SetConfiguration( pLine->m_CIDPhysical, bufPhysical, *(BufferSize *)bufPhysical );
			ReportError( "lgo_SetConfiguration Physical Layer", m_Result );

			pLine->m_CIDLink = lgo_OpenProtocol( ps.link, ps.majorId, minorId );
			ReportError( "lgo_OpenProtocol Link Layer", pLine->m_CIDLink );
			m_Result = lgo_SetConfiguration( pLine->m_CIDLink, bufLink, *(BufferSize *)bufLink );
			ReportError( "lgo_SetConfiguration Link Layer", m_Result );

			pLine->m_CIDPacket = lgo_OpenProtocol( ps.packet, ps.majorId, minorId );
			ReportError( "lgo_OpenProtocol Packet Layer", pLine->m_CIDPacket );
			m_Result = lgo_SetConfiguration( pLine->m_CIDPacket, bufPacket, *(BufferSize *)bufPacket );
			ReportError( "lgo_SetConfiguration Packet Layer", m_Result );

			m_Result = lgo_Push( pLine->m_CIDPhysical, pLine->m_CIDLink );
			ReportError( "lgo_Push Link Layer -> Physical Layer", m_Result );

			m_Result = lgo_Push( pLine->m_CIDLink, pLine->m_CIDPacket );
			ReportError( "lgo_Push Packet Layer -> Link Layer", m_Result );

			pLine->m_LineState = CX25Line::LINE_STATE_OPEN;

		}

		for( int i=0; i<m_PVCs.GetSize(); i++ )
		{
			pPVC = (LPX25PVC)m_PVCs.GetAt( i );
			if( pPVC && pPVC->m_pLine == pLine )
			{
				pPVC->m_ConnectionType		= ps.iConnectionType;
				pPVC->m_iMaxXOffBuffer		= ps.iMaxXOffBuffer;

				m_Result = lgo_DisconnectRequest( pPVC->m_CIDPVC, NULL, 0 );
				ReportError( "lgo_DisconnectRequest PVC", m_Result );
				
				m_Result = lgo_Close( pPVC->m_CIDPVC );
				ReportError( "lgo_Close PVC", m_Result );
			
				pPVC->m_CIDPVC = lgo_OpenProtocol( ps.pvc, ps.majorId, (MinorId)i+1 );
				ReportError( "lgo_OpenProtocol PVC", pPVC->m_CIDPVC );
			}
		}
		
		hr = prs->Close();
		if(FAILED(hr))
			_com_issue_error(hr);

		hr = m_pConn->Close();
		if(FAILED(hr))
			_com_issue_error(hr);

		m_Result = 0;
	}
	CATCH_ERRORS

	return( m_Result==0?true:false );
}

void CX25Manager::DoEvents()
{
	if( m_AdminListenSocket.IsOpen() == FALSE )
	{
		if( TCPSocket::IP_STATE_LISTENING != m_AdminListenSocket.GetState() )
		{
			if( m_AdminListenSocket.m_AdminSocket.IsOpen() == FALSE )
			{
				m_AdminListenSocket.OpenListenPort( m_Defaults.iBasePort );
			}
		}
	}

	if( IsStarted() )
	{
		LPLINE pLine = NULL;
		//for( int i=0; i<1; i++ )
		for( int i=0; i<m_Lines.GetSize(); i++ )
		{
			pLine = (LPLINE)m_Lines.GetAt(i);
			if( pLine )
				pLine->DoEvents();
		}
		LPX25PVC pPVC = NULL;
		//for( i=0; i<1; i++ )
		for( i=0; i<m_PVCs.GetSize(); i++ )
		{
			pPVC = (LPX25PVC)m_PVCs.GetAt( i );
			if( pPVC )
				pPVC->DoEvents();
		}
	}
}

void CX25Manager::ReportError( LPCTSTR pStr, Result Error )
{
	if( Error < 0 )
	{
		OWS_2trace( (*m_pCTrace), LEVEL_ERROR, "%s : %s", pStr, lgo_ErrorMessage(Error) );
	}
}

void CX25Manager::AskForConnectionStringDialog()
{
	try
	{
		OWS_0trace( (*m_pCTrace), LEVEL_OPERATIONS, "Getting connection string..." );

		HRESULT hr;

		MSDASC::IDataSourceLocatorPtr ptrDataSourceWnd = NULL;
		hr = ptrDataSourceWnd.CreateInstance(__uuidof(MSDASC::DataLinks));
		if(FAILED(hr))
			_com_issue_error(hr);

		hr = ptrDataSourceWnd->put_hWnd((long)gpApp->m_pMainWnd->m_hWnd);
		if(FAILED(hr))
			_com_issue_error(hr);

		IDispatchPtr ptrDispatch = NULL;
		hr = ptrDataSourceWnd->PromptNew(&ptrDispatch);
		if(FAILED(hr))
			_com_issue_error(hr);
		if(ptrDispatch != NULL)
		{
			m_pConn = ptrDispatch;
			BSTR bstrConnectString = m_pConn->ConnectionString;
			USES_CONVERSION;
			m_szConnection = W2CT(bstrConnectString);
			gpApp->WriteProfileString( "Settings", "ConnectionString", m_szConnection );
		}

	}
	CATCH_ERRORS
}

void CX25Manager::CalcProviderError()
{
	m_szError.Empty();
	if(m_pConn != NULL)
	{
		ADODB::ErrorsPtr ptrErrors = m_pConn->Errors;
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
			m_szError += sError + "\n\n";
		}
	}

	if(!m_szError.IsEmpty())
		m_szError = m_szError.Left(m_szError.GetLength()-2);
}

void CX25Manager::CalcComError(const _com_error& e)
{
	m_szError.Empty();
	m_szError.Format(
		_T("HRESULT: 0x%08lx; Error: %s"),
		e.Error(),
		e.ErrorMessage()
		);
	
	if(e.ErrorInfo())
	{
		m_szError += "\nSource: " + CString((LPCTSTR)e.Source()) +
			"; Description: " + CString((LPCTSTR)e.Description());
	}
}

void CX25Manager::CalcExceptionError( CException * e )
{
	m_szError.Empty();
	TCHAR   szCause[255];
	if( 0 == e->GetErrorMessage( szCause, 255 ) )
		m_szError = "An error occurred";
	else
		m_szError = szCause;
}

