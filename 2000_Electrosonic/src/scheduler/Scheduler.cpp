// Scheduler.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Scheduler.h"
#include "Utility.h"
#include "HelpKeys.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "EmailFrameWnd.h"
#include "SchedulesFrameWnd.h"
#include "SequencesFrameWnd.h"
#include "TriggersFrameWnd.h"
#include "VariablesFrameWnd.h"

#include "SchedulerDoc.h"
#include "SchedulerTreeView.h"
#include "DummyView.h"
#include "DummyFrame.h"

#include "SubItemView.h"
#include "SubItemFrame.h"

#include "SchedulesView.h"
#include "SequencesView.h"
#include "TriggersView.h"
#include "EMailGroupsView.h"
#include "VariablesView.h"

#include "aboutbox.h"
#include "Splash.h"
#include "Registry.h"
#include "ExecImageVersion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static TCHAR szFormat[] = _T("%u,%u,%d,%d,%d,%d,%d,%d,%d,%d");
LPCTSTR g_pClassName = "ESCANSchedulerApplicationClass";
//LPCTSTR g_pClassName = NULL;
CSchedulerApp * gpApp;

/////////////////////////////////////////////////////////////////////////////
// CSchedulerApp

BEGIN_MESSAGE_MAP(CSchedulerApp, CWinApp)
	//{{AFX_MSG_MAP(CSchedulerApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_APP_HELP_KEYS, OnAppHelpKeys)
	ON_COMMAND_EX_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16, OnOpenRecentFile)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSchedulerApp construction

CSchedulerApp::CSchedulerApp()
{
	m_pToolBar = NULL;
	m_pMainFrame = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSchedulerApp object

CSchedulerApp theApp;

//__declspec(allocate(".SharedData")) 
//#pragma data_seg(".SharedData") 
//static LONG nUsageCount = -1; 
//#pragma data_seg() 

/////////////////////////////////////////////////////////////////////////////
// CSchedulerApp initialization

BOOL CSchedulerApp::InitApplication() 
{
	return CWinApp::InitApplication();
}

BOOL CSchedulerApp::InitInstance()
{

#ifndef _DEBUG
	if( (HIBYTE(GetAsyncKeyState(VK_SHIFT))) == 0 ) {

		//if( nUsageCount >= 0 )
		//	return FALSE;
		//InterlockedIncrement ( &nUsageCount );

		BOOL bFound = FALSE;
		CString szClassName = "ESCAN:SchedulerApp";
		HANDLE hMutexOneInstance = CreateMutex(NULL, TRUE, _T(szClassName) );

		if( GetLastError() == ERROR_ALREADY_EXISTS )
		{
			bFound = TRUE;
			CWnd *PrevCWnd, *ChildCWnd;
    
			// Determine if another window with our class name exists...
			PrevCWnd = CWnd::FindWindow(g_pClassName, NULL);
			if (PrevCWnd != NULL)
			{ 
				// if so, does it have any popups?
				ChildCWnd=PrevCWnd->GetLastActivePopup();
				// Bring the main window to the top
				PrevCWnd->BringWindowToTop();
				// If iconic, restore the main window
				if (PrevCWnd->IsIconic())
					PrevCWnd->ShowWindow(SW_RESTORE);
				 // If there are popups, bring them along too!
				 if (PrevCWnd != ChildCWnd)
					 ChildCWnd->BringWindowToTop();
			}
		}
    
		if( hMutexOneInstance )
			ReleaseMutex(hMutexOneInstance);
		if( bFound == TRUE )
			return FALSE;
	}
#endif

	BeginWaitCursor();

	m_OpSysVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&m_OpSysVersion);

	if (!AfxOleInit())
	{
		//AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CString szRegEntry;
	CString szRegMRU;
	CString szRegLUF;

	m_szRegName.LoadString( IDS_REGISTRY_ENTRY );
	szRegMRU.LoadString( IDS_REGISTRY_MRU );
	szRegLUF.LoadString( IDS_REGISTRY_FILE );

	CRegistry cRegistry;
	CString szVersion;
	szVersion.LoadString( IDS_REGISTRY_TVF );
	szVersion = szVersion.Mid(0,szVersion.ReverseFind('\\'));
	cRegistry.SetKeyName( szVersion );
	
	szVersion = cRegistry.GetString( "Version" );

	CExecImageVersion ver;
	m_strFileVer = ver.GetFileVersion();

	if( szVersion != m_strFileVer )
	{
		szVersion.LoadString( IDS_REGISTRY_TVF );
		szVersion = szVersion.Mid(0,szVersion.ReverseFind('\\'));
		cRegistry.SetKeyName( szVersion );
		cRegistry.DeleteCurrentKey();
		cRegistry.SetKeyName( szVersion );
		cRegistry.SetString( m_strFileVer, "Version" );

		m_strFileVer.LoadString( IDS_REGISTRY_TVF );
		cRegistry.SetKeyName( m_strFileVer );

		cRegistry.SetLongInt( 40, "Column 0 Width" );
		cRegistry.SetLongInt( 110, "Column 1 Width" );
		cRegistry.SetLongInt( 90, "Column 2 Width" );
		cRegistry.SetLongInt( 300, "Column 3 Width" );
	}

	gpApp = (CSchedulerApp*)AfxGetApp();

	if (!AfxSocketInit())
	{
		AfxMessageBox("AfxSocketInit failed!");
		return FALSE;
	}

	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	SetRegistryKey(_T(m_szRegName));
	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	m_Defaults.iSiteLinxConnectionRetry =
	GetProfileInt( m_szRegName, "SiteLinx Connect Retry", 10 );

	m_Defaults.bAutoSave = ( GetProfileInt( m_szRegName, "AutoSave", 1 )
							 ? true:false );

	m_Defaults.bEditMode = ( GetProfileInt( m_szRegName, "EditMode", 1 )
							 ? true:false );

	m_Defaults.bSound = ( GetProfileInt( m_szRegName, "Sound", 1 )
							 ? true:false );
	m_Defaults.bEditSounds = ( GetProfileInt( m_szRegName, "EditSounds", 1 )
							 ? true:false );

	m_Defaults.bLEDLook = ( GetProfileInt( m_szRegName, "LEDLook", 1 )
							 ? true:false );

	m_Defaults.bDisplayViewOnCreate = ( GetProfileInt( m_szRegName, "DisplayViewOnCreate", 1 )
							 ? true:false );

	m_Defaults.bPerItemViews = ( GetProfileInt( m_szRegName, "PerItemViews", 1 )
							 ? true:false );

	m_Defaults.bColumnAutoResize = ( GetProfileInt( m_szRegName, "ColumnAutoResize", 1 )
							 ? true:false );

	m_Defaults.bShowToolBar = ( GetProfileInt( m_szRegName, "ShowToolBar", 1 )
							 ? true:false );
	m_Defaults.bShowStatusBar = ( GetProfileInt( m_szRegName, "ShowStatusBar", 1 )
							 ? true:false );
	m_Defaults.bShowChildToolBar = ( GetProfileInt( m_szRegName, "ShowChildToolBar", 1 )
							 ? true:false );
	m_Defaults.bShowChildTimeCounters = ( GetProfileInt( m_szRegName, "ShowChildTimeCounters", 1 )
							 ? true:false );
	m_Defaults.bShowListHeader = ( GetProfileInt( m_szRegName, "ShowListHeader", 1 )
							 ? true:false );
	m_Defaults.bGrid = ( GetProfileInt( m_szRegName, "ShowGrid", 1 )
							 ? true:false );

	m_Defaults.szStatusBarText = ver.GetFileDescription();

	szRegEntry.LoadString( IDS_WINDOW_FRAME_POS );
	CString szWindowPos = GetProfileString( m_szRegName, 
		szRegEntry );

	m_wpFrame.length = sizeof m_wpFrame;
	if( szWindowPos == "" )
	{
		m_wpFrame.flags						= 0;
		m_wpFrame.showCmd					= SW_SHOW;
		m_wpFrame.ptMinPosition.x			= -1;
		m_wpFrame.ptMinPosition.y			= -1;
		m_wpFrame.ptMaxPosition.x			= -1;
		m_wpFrame.ptMaxPosition.y			= -1;
		m_wpFrame.rcNormalPosition.left		= 0;
		m_wpFrame.rcNormalPosition.top		= 0;
		m_wpFrame.rcNormalPosition.right	= 640;
		m_wpFrame.rcNormalPosition.bottom	= 480;

	}
	else
	{
		_stscanf(szWindowPos, szFormat,
		&m_wpFrame.flags, &m_wpFrame.showCmd,
		&m_wpFrame.ptMinPosition.x, &m_wpFrame.ptMinPosition.y,
		&m_wpFrame.ptMaxPosition.x, &m_wpFrame.ptMaxPosition.y,
		&m_wpFrame.rcNormalPosition.left, &m_wpFrame.rcNormalPosition.top,
		&m_wpFrame.rcNormalPosition.right, &m_wpFrame.rcNormalPosition.bottom);
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

#ifndef _DEBUG
	if( (HIBYTE(GetAsyncKeyState(VK_SHIFT))) == 0 ) {
		CSplashWnd::EnableSplashScreen(TRUE);
		CSplashWnd::ShowSplashScreen(GetMainWnd());
	}
#endif

	CMultiDocTemplate * m_pDocTemplate = new CMultiDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CSchedulerDoc),
		RUNTIME_CLASS(CDummyFrame), // Dummy
		RUNTIME_CLASS(CDummyView) );
	AddDocTemplate(m_pDocTemplate);

	m_pSubItemsDocTemplate = new CMultiDocTemplate(
		IDR_SUBITEMS,
		RUNTIME_CLASS(CSchedulerDoc),
		RUNTIME_CLASS(CSubItemFrame), // custom MDI child frame
		RUNTIME_CLASS(CSubItemView));
	AddDocTemplate(m_pSubItemsDocTemplate);

	m_pSchedulesDocTemplate = new CMultiDocTemplate(
		IDR_SCHEDULES,
		RUNTIME_CLASS(CSchedulerDoc),
		RUNTIME_CLASS(CSchedulesFrameWnd), // custom MDI child frame
		RUNTIME_CLASS(CSchedulesView));
	AddDocTemplate(m_pSchedulesDocTemplate);

	m_pSequencesDocTemplate = new CMultiDocTemplate(
		IDR_SEQUENCES,
		RUNTIME_CLASS(CSchedulerDoc),
		RUNTIME_CLASS(CSequencesFrameWnd), // custom MDI child frame
		RUNTIME_CLASS(CSequencesView));
	AddDocTemplate(m_pSequencesDocTemplate);

	m_pTriggersDocTemplate = new CMultiDocTemplate(
		IDR_TRIGGERS,
		RUNTIME_CLASS(CSchedulerDoc),
		RUNTIME_CLASS(CTriggersFrameWnd), // custom MDI child frame
		RUNTIME_CLASS(CTriggersView));
	AddDocTemplate(m_pTriggersDocTemplate);

	m_pEMailGroupsDocTemplate = new CMultiDocTemplate(
		IDR_EMAILGROUPS,
		RUNTIME_CLASS(CSchedulerDoc),
		RUNTIME_CLASS(CEmailFrameWnd), // custom MDI child frame
		RUNTIME_CLASS(CEMailGroupsView));
	AddDocTemplate(m_pEMailGroupsDocTemplate);

	m_pVariablesGroupsDocTemplate = new CMultiDocTemplate(
		IDR_VARIABLES,
		RUNTIME_CLASS(CSchedulerDoc),
		RUNTIME_CLASS(CVariablesFrameWnd), // custom MDI child frame
		RUNTIME_CLASS(CVariablesView));
	AddDocTemplate(m_pVariablesGroupsDocTemplate);

	CSchedulerDoc * pDoc = NULL;

	CCreateContext context;
	context.m_pCurrentDoc=pDoc;
	context.m_pNewViewClass=NULL;
	context.m_pNewDocTemplate=NULL;
	context.m_pLastView=NULL;
	context.m_pCurrentFrame=NULL;
	//context.m_pNewViewClass->m_lpszClassName = g_pClassName;
	//context.m_pNewViewClass->CreateObject();

	// create main MDI Frame window
	m_pMainFrame = new CMainFrame;
//	m_pMainFrame->Create( g_pClassName, g_pClassName, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
//		CFrameWnd::rectDefault, NULL, NULL, 0, &context );
	if (!m_pMainFrame->LoadFrame(
								IDR_MAINFRAME,
								WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
								NULL,
								&context
								))
		return FALSE;
	m_pMainWnd = m_pMainFrame;
	m_pMainFrame->ShowWindow(SW_HIDE);

#ifndef _DEBUG 

	if( (HIBYTE(GetAsyncKeyState(VK_SHIFT))) == 0 )
	{

		// Initialize the cool menus...
		m_pMainFrame->InitializeMenu( m_pDocTemplate,
		IDR_MAINFRAME, IDR_TOOLBAR_FOR_MENU, IDR_MAINFRAME);

		m_pMainFrame->InitializeMenu( m_pSubItemsDocTemplate,
		IDR_MAINFRAME, IDR_TOOLBAR_FOR_MENU );

		m_pMainFrame->InitializeMenu( m_pSchedulesDocTemplate,
		IDR_SCHEDULES, IDR_TOOLBAR_FOR_MENU);
		m_pMainFrame->InitializeMenu( m_pSequencesDocTemplate,
		IDR_SEQUENCES, IDR_TOOLBAR_FOR_MENU);
		m_pMainFrame->InitializeMenu( m_pTriggersDocTemplate,
		IDR_TRIGGERS, IDR_TOOLBAR_FOR_MENU);
		m_pMainFrame->InitializeMenu( m_pEMailGroupsDocTemplate,
		IDR_EMAILGROUPS, IDR_TOOLBAR_FOR_MENU);
		m_pMainFrame->InitializeMenu( m_pVariablesGroupsDocTemplate,
		IDR_VARIABLES, IDR_TOOLBAR_FOR_MENU);
	}
#endif


#define __VERSION_	1

	CString szTemp;
	CString szRegTVF;
	CString szInfo;

	szRegTVF.LoadString( IDS_REGISTRY_TVF );

	CFileStatus cFileStatus;
	cRegistry.SetKeyName( szRegTVF );

	CStringArray saFilesInfo;
	CStringArray saTemp;

	szRegTVF = "File";
	for( char i=0; i<20; i++ ) //20 is the maximum number of docs we process
	{
		szTemp = szRegTVF;
		szTemp += char(i + 49); //49 obtains number '1' char
		szInfo = cRegistry.GetString( szTemp );
		if( szInfo == "" )
			break;
		saFilesInfo.Add( szInfo );
		StringToArray( szInfo, saTemp, "\1" );
		if( CFile::GetStatus( saTemp.GetAt(1), cFileStatus ) )
		{
			pDoc = (CSchedulerDoc*)OpenDocumentFile( saTemp.GetAt(1) );
			if( pDoc )
			{
				if( saTemp.GetSize() > 4 )
				{
					int iExpandState = atoi(saTemp.GetAt(4));
					gpTreeView->ExpandDocumentObjects( iExpandState, pDoc );
				}
				pDoc->SetDestinationSiteLinxDeviceName( saTemp.GetAt(3) );
				
				int iRemoteConnectionState = atoi(saTemp.GetAt(2));
				pDoc->SetConnectionState( (char)iRemoteConnectionState );
				if( saTemp.GetAt(0) == "1" )
				{
					gpTreeView->ActivateDocItem( pDoc->GetDocName() );
				}
				gpTreeView->OnUpdate( HINT_CONNECTION_STATE_CHANGE, pDoc );
			}
		}
		szTemp.Empty();
		szTemp = cRegistry.GetString( "ActiveView" );
		if( !szTemp.IsEmpty() ) {
			szInfo = cRegistry.GetString( "ActiveDocument" );
			long Maximized = cRegistry.GetLongInt( "ActiveViewMaximized" );
			gpTreeView->SelectItem( szInfo, szTemp, 0, true );
			if( Maximized ) {
				CMDIChildWnd * pfw = (CMDIChildWnd*)m_pMainFrame->GetActiveFrame();
				if( (CMDIChildWnd*)m_pMainFrame != pfw ) {
					pfw->MDIMaximize();
					//pfw->MDIActivate();
				}
			}

		} else {
			szInfo = cRegistry.GetString( "Parent" );
			szTemp = cRegistry.GetString( "Selected" );
			gpTreeView->SelectItem( szInfo, szTemp, 0 );
		}
	}

	// The main window has been initialized, so show and update it.
	m_pMainFrame->ShowWindow(m_nCmdShow);
	m_pMainFrame->UpdateWindow();

	CloseAllDummyViews();

	gpTreeView->SetActiveWindow();
	gpTreeView->GetParentFrame()->SetActiveWindow();
	gpTreeView->GetParentFrame()->SetActiveView(gpTreeView);

	EndWaitCursor();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSchedulerApp message handlers

// App command to run the dialog
void CSchedulerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


int CSchedulerApp::ExitInstance() 
{
	BeginWaitCursor();

	CString szRegEntry;
	CString szRegMRU;
	CString szRegLUF;

	try
	{
		SaveAllModified();
		CloseDocs();

		WSACleanup();

		szRegMRU.LoadString( IDS_REGISTRY_MRU );
		szRegLUF.LoadString( IDS_REGISTRY_FILE );

		TCHAR szBuffer[sizeof("-32767")*8 + sizeof("65535")*2];

		wsprintf(szBuffer, szFormat,
			m_wpFrame.flags, m_wpFrame.showCmd,
			m_wpFrame.ptMinPosition.x, m_wpFrame.ptMinPosition.y,
			m_wpFrame.ptMaxPosition.x, m_wpFrame.ptMaxPosition.y,
			m_wpFrame.rcNormalPosition.left, m_wpFrame.rcNormalPosition.top,
			m_wpFrame.rcNormalPosition.right, m_wpFrame.rcNormalPosition.bottom);

		szRegEntry.LoadString( IDS_WINDOW_FRAME_POS );
		WriteProfileString( m_szRegName, 
			szRegEntry, szBuffer );

		WriteProfileInt( m_szRegName, 
						 "SiteLinx Connect Retry", 
						 m_Defaults.iSiteLinxConnectionRetry );

		WriteProfileInt( m_szRegName, 
						 "AutoSave", 
						 m_Defaults.bAutoSave );

		WriteProfileInt( m_szRegName, 
						 "EditMode", 
						 m_Defaults.bEditMode );

		WriteProfileInt( m_szRegName, 
						 "Sound", 
						 m_Defaults.bSound );

		WriteProfileInt( m_szRegName, 
						 "EditSounds", 
						 m_Defaults.bEditSounds );

		WriteProfileInt( m_szRegName, 
						 "LEDLook", 
						 m_Defaults.bLEDLook );
		
		WriteProfileInt( m_szRegName, 
						 "DisplayViewOnCreate", 
						 m_Defaults.bDisplayViewOnCreate );
		
		WriteProfileInt( m_szRegName, 
						 "PerItemViews", 
						 m_Defaults.bPerItemViews );
		
		WriteProfileInt( m_szRegName, 
						 "ColumnAutoResize", 
						 m_Defaults.bColumnAutoResize );

		WriteProfileInt( m_szRegName, 
						 "ShowToolBar", 
						 m_Defaults.bShowToolBar );
		WriteProfileInt( m_szRegName, 
						 "ShowStatusBar", 
						 m_Defaults.bShowStatusBar );
		WriteProfileInt( m_szRegName, 
						 "ShowChildToolBar", 
						 m_Defaults.bShowChildToolBar );
		WriteProfileInt( m_szRegName, 
						 "ShowChildTimeCounters", 
						 m_Defaults.bShowChildTimeCounters );
		WriteProfileInt( m_szRegName, 
						 "ShowListHeader", 
						 m_Defaults.bShowListHeader );
		WriteProfileInt( m_szRegName, 
						 "ShowGrid", 
						 m_Defaults.bGrid );
	}
	catch( CException * e )
	{
		e->Delete();
	}

	//InterlockedDecrement( &nUsageCount );

	EndWaitCursor();

	return CWinApp::ExitInstance();
}

void CSchedulerApp::UpdateAllDocumentViews( LPARAM lHint )
{
	CDocTemplate * curTemplate=NULL;
	CSchedulerDoc * pDoc=NULL;
	POSITION curDoc=NULL;
	POSITION curTemplatePos = GetFirstDocTemplatePosition();
	while(curTemplatePos != NULL) {
		curTemplate =
			GetNextDocTemplate(curTemplatePos);
		if( curTemplate ) {
			curDoc = curTemplate->GetFirstDocPosition();
			while(curDoc != NULL) {
				pDoc = (CSchedulerDoc*)curTemplate->GetNextDoc( curDoc );
				if( pDoc ) {
					pDoc->UpdateAllViews( NULL, lHint );
				}
			}
		}
	}
}

bool CSchedulerApp::CanExit()
{
	CDocTemplate * curTemplate=NULL;
	CSchedulerDoc * pDoc=NULL;
	POSITION curDoc=NULL;
	POSITION curTemplatePos = GetFirstDocTemplatePosition();
	while(curTemplatePos != NULL) {
		curTemplate =
			GetNextDocTemplate(curTemplatePos);
		if( curTemplate ) {
			curDoc = curTemplate->GetFirstDocPosition();
			while(curDoc != NULL) {
				pDoc = (CSchedulerDoc*)curTemplate->GetNextDoc( curDoc );
				if( pDoc ) {
					if( !pDoc->GetDeviceManager().AllDeviceConnectionsResolved() )
						return false;
				}
			}
		}
	}
	return true;
}

BOOL CSchedulerApp::OnIdle(LONG lCount) 
{
/*	if( m_bExit && CanExit() )
	{
		//exit
		if( m_pMainFrame )
			m_pMainFrame->SendMessage(WM_SYSCOMMAND, SC_CLOSE);
		return TRUE;
	}*/
	//this will loop through all docs to do OnIdle update
	gpTreeView->OnIdle();
	m_pMainFrame->Idle();
	return CWinApp::OnIdle(lCount);
}

void CSchedulerApp::OnFileNew() 
{
	POSITION curTemplatePos = GetFirstDocTemplatePosition();
	while(curTemplatePos != NULL)
	{
		CDocTemplate* curTemplate =
			GetNextDocTemplate(curTemplatePos);
		CString str;
		if( curTemplate )
			curTemplate->OpenDocumentFile(NULL);
		CloseAllDummyViews();
		return;
	}
	AfxMessageBox("Cannot create new file");
}

void CSchedulerApp::OnFileOpen() 
{
	CWinApp::OnFileOpen();
	CloseAllDummyViews();
}

BOOL CSchedulerApp::OnOpenRecentFile(UINT nID)
{
	CWinApp::OnOpenRecentFile(nID);
	CloseAllDummyViews();
	return TRUE;
}

bool CSchedulerApp::RemoveDoc( CDocument * pDoc ) 
{
	CDocTemplate * curTemplate=NULL;
	CDocument * pTempDoc=NULL;
	POSITION curDoc=NULL;
	POSITION curTemplatePos = GetFirstDocTemplatePosition();
	while(curTemplatePos != NULL) {
		curTemplate =
			GetNextDocTemplate(curTemplatePos);
		if( curTemplate ) {
			//curTemplate->RemoveDocument( pDoc );
			if( curTemplate )
				curDoc = curTemplate->GetFirstDocPosition();
			while(curDoc != NULL) {
				pTempDoc = curTemplate->GetNextDoc( curDoc );
				if( pTempDoc && pTempDoc == pDoc )
					curTemplate->RemoveDocument( pDoc );
			}
		}
	}
	return true;
}

bool CSchedulerApp::HasDoc() 
{
	CDocTemplate * curTemplate=NULL;
	CSchedulerDoc * pDoc=NULL;
	POSITION curDoc=NULL;
	POSITION curTemplatePos = GetFirstDocTemplatePosition();
	while(curTemplatePos != NULL) {
		curTemplate =
			GetNextDocTemplate(curTemplatePos);
		if( curTemplate ) {
			curDoc = curTemplate->GetFirstDocPosition();
			while(curDoc != NULL) {
				pDoc = (CSchedulerDoc*)curTemplate->GetNextDoc( curDoc );
				if( pDoc ) {
					return true;
				}
			}
		}
	}
	return false;
}

bool CSchedulerApp::CloseDocs() 
{
	CDocTemplate * curTemplate=NULL;
	CSchedulerDoc * pDoc=NULL;
	POSITION curDoc=NULL;
	POSITION curTemplatePos = GetFirstDocTemplatePosition();
	while(curTemplatePos != NULL) {
		curTemplate =
			GetNextDocTemplate(curTemplatePos);
		if( curTemplate ) {
			curDoc = curTemplate->GetFirstDocPosition();
			while(curDoc != NULL) {
				pDoc = (CSchedulerDoc*)curTemplate->GetNextDoc( curDoc );
				if( pDoc ) {
					curTemplate->RemoveDocument( pDoc );
					pDoc->OnCloseDocument2(false);
				}
			}
		}
	}
	return true;
}

bool CSchedulerApp::CloseView( bool Activate )
{
	CMDIChildWnd * pChild = 
		(CMDIChildWnd*)m_pMainFrame->GetActiveFrame();

	CView * pView = (CView*)pChild->GetActiveView();
	CFrameWnd * pFrame = pView->GetParentFrame();
	if( pFrame )
		pFrame->SendMessage(WM_SYSCOMMAND, SC_CLOSE);
	if( Activate && gpTreeView )
	gpTreeView->ActivateAndSetFocus();
	return true;
}

bool CSchedulerApp::CloseAllViews()
{
	CDocTemplate * curTemplate=NULL;
	CDocument * pDoc=NULL;
	CView * pView=NULL;
	CFrameWnd * pFrame=NULL;
	POSITION curView=NULL;
	POSITION curDoc=NULL;
	POSITION curTemplatePos = GetFirstDocTemplatePosition();
	while(curTemplatePos != NULL) {
		curTemplate =
			GetNextDocTemplate(curTemplatePos);
		if( curTemplate ) {
			curDoc = curTemplate->GetFirstDocPosition();
			while(curDoc != NULL) {
				pDoc = curTemplate->GetNextDoc( curDoc );
				if( pDoc ) {
					curView = pDoc->GetFirstViewPosition();
					while(curView != NULL) {
						pView = pDoc->GetNextView( curView );
						if( pView ) {
							pDoc->RemoveView( pView );
							pFrame = pView->GetParentFrame();
							if( pFrame ) {
								pFrame->SendMessage(WM_SYSCOMMAND, SC_CLOSE);
								curView = pDoc->GetFirstViewPosition();
							}
						}
					}
				}
			}
		}
	}
	gpTreeView->ActivateAndSetFocus();
	return true;
}

bool CSchedulerApp::CloseAllOrphanSubItemViews( int iViewType )
{
	CDocTemplate * curTemplate=NULL;
	CSchedulerDoc * pDoc=NULL;
	CView * pView=NULL;
	CFrameWnd * pFrame=NULL;
	POSITION curView=NULL;
	POSITION curDoc=NULL;
	POSITION curTemplatePos = GetFirstDocTemplatePosition();
	while(curTemplatePos != NULL) {
		curTemplate =
			GetNextDocTemplate(curTemplatePos);
		if( curTemplate ) {
			curDoc = curTemplate->GetFirstDocPosition();
			while(curDoc != NULL) {
				pDoc = (CSchedulerDoc*)curTemplate->GetNextDoc( curDoc );
				if( pDoc ) {
					curView = pDoc->GetFirstViewPosition();
					while(curView != NULL) {
						pView = pDoc->GetNextView( curView );
						if( pView ) {
							if ( pView->GetRuntimeClass()->m_lpszClassName ==
								 RUNTIME_CLASS( CSubItemView )->m_lpszClassName ) {
								CSubItemView * pSubItemView = (CSubItemView*)pView;
								if( pSubItemView->GetObjectType() == iViewType ) {
									bool bDestroyView = false;
									switch( iViewType ) {
									case ITEM_SCHED_CHILD_DEVICES_SUBITEM:
										if( !pDoc->HasDeviceNamed( (LPCTSTR)pSubItemView->GetObjectName() ) )
											bDestroyView = true;
										break;
									case ITEM_SCHED_CHILD_SCHEDULES_SUBITEM:
									case ITEM_SCHED_CHILD_SEQUENCES_SUBITEM:
									case ITEM_SCHED_CHILD_TRIGGERS_SUBITEM:
									case ITEM_SCHED_CHILD_EMAILGROUPS_SUBITEM:
									case ITEM_SCHED_CHILD_VARIABLESGROUPS_SUBITEM:
										break;
									}
									if( bDestroyView ) {
										pDoc->RemoveView( pView );
										pFrame = pView->GetParentFrame();
										if( pFrame )
											pFrame->SendMessage(WM_SYSCOMMAND, SC_CLOSE);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	gpTreeView->ActivateAndSetFocus();
	return true;
}

bool CSchedulerApp::CloseAllDummyViews()
{
	CDocTemplate * curTemplate=NULL;
	CDocument * pDoc=NULL;
	CView * pView=NULL;
	CFrameWnd * pFrame=NULL;
	POSITION curView=NULL;
	POSITION curDoc=NULL;
	POSITION curTemplatePos = GetFirstDocTemplatePosition();
	while(curTemplatePos != NULL) {
		curTemplate =
			GetNextDocTemplate(curTemplatePos);
		if( curTemplate ) {
			curDoc = curTemplate->GetFirstDocPosition();
			while(curDoc != NULL) {
				pDoc = curTemplate->GetNextDoc( curDoc );
				if( pDoc ) {
					curView = pDoc->GetFirstViewPosition();
					while(curView != NULL) {
						pView = pDoc->GetNextView( curView );
						if( pView ) {
							if ( pView->GetRuntimeClass()->m_lpszClassName ==
								 RUNTIME_CLASS( CDummyView )->m_lpszClassName ) {
								pDoc->RemoveView( pView );
								pFrame = pView->GetParentFrame();
								if( pFrame ) 
									pFrame->SendMessage(WM_SYSCOMMAND, SC_CLOSE);
							}
						}
					}
				}
			}
		}
	}
	gpTreeView->ActivateAndSetFocus();
	return true;
}

bool CSchedulerApp::HasView()
{
	CDocTemplate * curTemplate=NULL;
	CDocument * pDoc=NULL;
	CView * pView=NULL;
	POSITION curView=NULL;
	POSITION curDoc=NULL;
	POSITION curTemplatePos = GetFirstDocTemplatePosition();
	while(curTemplatePos != NULL) {
		curTemplate =
			GetNextDocTemplate(curTemplatePos);
		if( curTemplate ) {
			curDoc = curTemplate->GetFirstDocPosition();
			while(curDoc != NULL) {
				pDoc = curTemplate->GetNextDoc( curDoc );
				if( pDoc ) {
					curView = pDoc->GetFirstViewPosition();
					while(curView != NULL) {
						pView = pDoc->GetNextView( curView ); 
						if( pView ) {
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

CSchedulerDoc * CSchedulerApp::GetDocument( CString& szDocName )
{
	CSchedulerDoc * pDoc=NULL;
	CDocTemplate * curTemplate=NULL;
	POSITION curDoc=NULL;
	POSITION curTemplatePos = GetFirstDocTemplatePosition();
	while(curTemplatePos != NULL) {
		curTemplate =
			GetNextDocTemplate(curTemplatePos);
		if( curTemplate ) {
			curDoc = curTemplate->GetFirstDocPosition();
			while(curDoc != NULL) {
				pDoc = (CSchedulerDoc*)curTemplate->GetNextDoc( curDoc );
				if( pDoc ) {
					if( szDocName == pDoc->GetDocName() )
						return pDoc;
				}
			}
		}
	}
	return NULL;
}

void CSchedulerApp::SetStatusBarText( LPCTSTR lpText )
{
	if(!m_pMainFrame||!lpText||!*lpText)return;
	CStatusBar * pSB = m_pMainFrame->GetStatusBar();
	if(!pSB)return;
	pSB->SetPaneText( 0, lpText );
}

COMCTL32VERSION CSchedulerApp::c_nComCtl32Version = COMCTL32_UNKNOWN;

COMCTL32VERSION CSchedulerApp::ComCtl32Version() {
     // if we don't already know which version, try to find out
     if (c_nComCtl32Version == COMCTL32_UNKNOWN) {
          // have we loaded COMCTL32 yet?
          HMODULE theModule = ::GetModuleHandle("COMCTL32");
          // if so, then we can check for the version
          if (theModule) {
               // InitCommonControlsEx is unique to 4.7 and later
               FARPROC theProc = ::GetProcAddress(theModule, "InitCommonControlsEx");
               if (! theProc) {
                    // not found, must be 4.00
                    c_nComCtl32Version = COMCTL32_400;
               } else {
                    // we could check for any of these - I chose DllInstall
                    FARPROC theProc = ::GetProcAddress(theModule, "DllInstall");
                    if (! theProc) {
                         // not found, must be 4.70
                         c_nComCtl32Version = COMCTL32_470;
                    } else {
                         // found, must be 4.71
                         c_nComCtl32Version = COMCTL32_471;
                    }
               }
          }
     }
     return c_nComCtl32Version;
}

void CSchedulerApp::OnAppHelpKeys() 
{
	CHelpKeys Dlg;
	Dlg.DoModal();
}
