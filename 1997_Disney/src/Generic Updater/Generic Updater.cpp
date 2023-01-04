

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//			 ____________________________________________________________________
//			|							Includes							     |
//			 ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//

#include "Generic Updater.h"

SERVER_INFO	* aServerInfo = 0 ;
INT	iLocalServer = 0 ;
INT	iBackupServer = 1 ;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//			 ____________________________________________________________________
//			|							FUNCTIONS							     |
//			 ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//


//------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: WinMain
// PURPOSE:	
//			
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{

	SECURITY_ATTRIBUTES			sa				;
	HANDLE						hMutex			;
	DWORD						dwError			;
	DString						szMessage		;

	sa.lpSecurityDescriptor			=		NULL		;
	sa.bInheritHandle				=		FALSE		;
	sa.nLength						=		sizeof(sa)	;

	DString::SetModuleInstance( hInstance ) ;

	hMutex = CreateMutex( & sa, FALSE, "FILMISUPDATER" ) ; 
	dwError = GetLastError() ;
	if ( dwError == ERROR_ALREADY_EXISTS )
	{
		szMessage.MessageBox ( IDS_ALREADY_RUNNING, IDS_ONLY_ONE_INSTANCE, MB_OK | MB_ICONEXCLAMATION ) ;
		return ( TRUE ) ;
	}

	if ( _stricmp( lpCmdLine, "dr" ) == 0 || _stricmp( lpCmdLine, "/dr" ) == 0 ||
		 _stricmp( lpCmdLine, "d" ) == 0 || _stricmp( lpCmdLine, "/d" ) == 0 )
	{
		DeleteRegistry() ;
		return ( FALSE ) ;
	}

	ghInst = hInstance ;

	if ( GetINISettings() == FALSE ) return ( FALSE ) ;

	if ( GetRegistry() == FALSE )
	{
		if ( DialogBox ( ghInst, 
						 MAKEINTRESOURCE(ID_SERVER_DIALOG), 
						 NULL, 
						 (DLGPROC)ServerDlgProc ) == TRUE )
		{
			DialogBox ( ghInst, MAKEINTRESOURCE(IDD_UPDATER), NULL, (DLGPROC)UpdaterDlgProc ) ;
		}
	}
	else
	{
		DialogBox ( ghInst, MAKEINTRESOURCE(IDD_UPDATER), NULL, (DLGPROC)UpdaterDlgProc ) ;
	}

	return ( FALSE ) ;
	UNREFERENCED_PARAMETER(nCmdShow);
	UNREFERENCED_PARAMETER(hPrevInstance);
}


//------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: UpdaterDlgProc
// PURPOSE:	
//			
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK UpdaterDlgProc ( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	ghDlg = hDlg ;
	DString::SetWindowHandle( hDlg ) ;

	switch ( uMsg )
	{
	case WM_INITDIALOG :
		ProcessUpdate () ;
		//MessageBox(NULL, "Updating...", "Updater", MB_OK);
		EndDialog ( hDlg, TRUE ) ;
		break ;
	case WM_DESTROY :
		break ;
	default :
		return ( FALSE ) ;
	}
	return ( TRUE ) ;
	UNREFERENCED_PARAMETER(lParam);
	UNREFERENCED_PARAMETER(wParam);
}

//------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: ServerDlgProc
// PURPOSE:	
//			
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK ServerDlgProc ( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	UINT					i									;
	UINT					iServerCount						;
	CHAR					szBuffer			[STRING_BUFFER]	;
	DString					szMessage							;
	DString					szSection							;
	DString					szTemp							;
	DStringArray			saServers							;
	DStringArray			saServerSections					;
	DINIFile				INIFileReader						;
	DString::SetWindowHandle( hDlg ) ;

	switch ( uMsg )
	{
	case WM_INITDIALOG :
		hDlg = hDlg ;
		szSection.LoadString( IDS_SECTION_SERVERS ) ;
		saServers = INIFileReader.GetAllSectionItems(szSection);
		iServerCount = saServers.GetSize() ;
		aServerInfo = new SERVER_INFO [ iServerCount + 1 ] ;
		for ( i=0; i<=iServerCount; i++ )
		{
			saServerSections.Tokenize(INIFileReader.GetString(saServers[i], szSection), ",");
			aServerInfo[i].szTitle = saServerSections[0];
			aServerInfo[i].szFullPath = saServerSections[1];
			aServerInfo[i].szTree = saServerSections[2];
			aServerInfo[i].szContext = saServerSections[3];
			aServerInfo[i].szUser = saServerSections[4];
			aServerInfo[i].szPassword = saServerSections[5];
			aServerInfo[i].lOptions = atol( saServerSections[6] ) ;
			szMessage = "( " ;
			szMessage += aServerInfo[i].szTitle ;
			szMessage +=  " ) - " ;
			szMessage += aServerInfo[i].szFullPath ;
			SendDlgItemMessage ( hDlg, ID_LOCAL_SERVER,	LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)szMessage ) ; 
			SendDlgItemMessage ( hDlg, ID_BACKUP_SERVER,LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)szMessage ) ;  
		}
		SendDlgItemMessage ( hDlg, ID_LOCAL_SERVER,		LB_SETCURSEL, 0, 0 ) ; 
		SendDlgItemMessage ( hDlg, ID_BACKUP_SERVER,	LB_SETCURSEL, 1, 0 ) ; 
		iLocalServer = 0 ;
		iBackupServer = 1 ;
		SetDlgItemText ( hDlg, ID_LOCAL_SERVER_NAME,	(LPCTSTR) aServerInfo[iLocalServer].szUser			) ; 
		SetDlgItemText ( hDlg, ID_LOCAL_SERVER_PWD,		(LPCTSTR) aServerInfo[iLocalServer].szPassword		) ; 
		SetDlgItemText ( hDlg, ID_BACKUP_SERVER_NAME,	(LPCTSTR) aServerInfo[iBackupServer].szUser			) ; 
		SetDlgItemText ( hDlg, ID_BACKUP_SERVER_PWD,	(LPCTSTR) aServerInfo[iBackupServer].szPassword		) ; 
		SendDlgItemMessage ( hDlg, ID_LOCAL_SERVER,		LB_SETHORIZONTALEXTENT, LISTBOX_LENGTH, 0 ) ; 
		SendDlgItemMessage ( hDlg, ID_BACKUP_SERVER,	LB_SETHORIZONTALEXTENT, LISTBOX_LENGTH, 0 ) ; 
		ShowWindow ( hDlg, SW_SHOW ) ;
		SetForegroundWindow ( hDlg ) ;
		break ;

	case WM_DESTROY :
		delete [] aServerInfo ;
		break ;

	case WM_COMMAND :
		switch ( LOWORD(wParam) )
		{
		case IDOK :
			SendDlgItemMessage ( hDlg, ID_LOCAL_SERVER_NAME, WM_GETTEXT, 60, (LPARAM)((LPSTR)szBuffer) ) ;
			aServerInfo[iLocalServer].szUser = szBuffer ;
			SendDlgItemMessage ( hDlg, ID_LOCAL_SERVER_PWD, WM_GETTEXT, 60, (LPARAM)((LPSTR)szBuffer) ) ;
			aServerInfo[iLocalServer].szPassword = szBuffer ;
			SendDlgItemMessage ( hDlg, ID_BACKUP_SERVER_NAME, WM_GETTEXT, 60, (LPARAM)((LPSTR)szBuffer) ) ;
			aServerInfo[iLocalServer].szUser = szBuffer ;
			SendDlgItemMessage ( hDlg, ID_BACKUP_SERVER_PWD, WM_GETTEXT, 60, (LPARAM)((LPSTR)szBuffer) ) ;
			aServerInfo[iLocalServer].szPassword = szBuffer ;
  
			gLocalServer.szTitle		=		aServerInfo[iLocalServer].szTitle		;
			gLocalServer.szTree			=		aServerInfo[iLocalServer].szTree		;
			gLocalServer.szContext		=		aServerInfo[iLocalServer].szContext		;
			gLocalServer.szFullPath		=		aServerInfo[iLocalServer].szFullPath	;
			gLocalServer.szUser			=		aServerInfo[iLocalServer].szUser		;
			gLocalServer.szPassword		=		aServerInfo[iLocalServer].szPassword	;
			gLocalServer.lOptions		=		aServerInfo[iLocalServer].lOptions		;

			gBackupServer.szTitle		=		aServerInfo[iBackupServer].szTitle		;
			gBackupServer.szTree		=		aServerInfo[iBackupServer].szTree		;
			gBackupServer.szContext		=		aServerInfo[iBackupServer].szContext	;
			gBackupServer.szFullPath	=		aServerInfo[iBackupServer].szFullPath	;
			gBackupServer.szUser		=		aServerInfo[iBackupServer].szUser		;
			gBackupServer.szPassword	=		aServerInfo[iBackupServer].szPassword	;
			gBackupServer.lOptions		=		aServerInfo[iBackupServer].lOptions		;

			if ( SetRegistry() == FALSE )
			{
				szMessage.MessageBox ( IDS_ERROR_REGISTRY_WRITE, "ERROR", MB_OK | MB_ICONEXCLAMATION ) ;
				EndDialog ( hDlg, FALSE ) ;
				return ( FALSE ) ;
			}
			else
			{
				EndDialog ( hDlg, TRUE ) ;
				return ( TRUE ) ;
			}
			break ;

		case IDCANCEL :
			szMessage.LoadString(IDS_EXIT_UPDATER_CONFIRM) ;
			szMessage += gINI.szRegistrySection ;
			if ( szMessage.MessageBox ( szMessage, IDS_EXIT_UPDATER, MB_OK | MB_ICONWARNING | MB_YESNO ) == IDYES )
			{
				EndDialog ( hDlg, FALSE ) ;
				return ( FALSE ) ;
			}
			break ;

		case ID_LOCAL_SERVER :
			switch ( HIWORD(wParam) )
			{
			case LBN_SELCHANGE :
				iLocalServer = (INT)SendDlgItemMessage ( hDlg, ID_LOCAL_SERVER, LB_GETCURSEL, 0, 0 ) ;
				SetDlgItemText( hDlg, ID_LOCAL_SERVER_NAME, (LPCTSTR)aServerInfo[iLocalServer].szUser ) ; 
				SetDlgItemText( hDlg, ID_LOCAL_SERVER_PWD, (LPCTSTR)aServerInfo[iLocalServer].szPassword ) ; 
				break ;
			}
			break ;

		case ID_BACKUP_SERVER :
			switch ( HIWORD(wParam) )
			{
			case LBN_SELCHANGE :
				iBackupServer = (INT)SendDlgItemMessage ( hDlg, ID_BACKUP_SERVER, LB_GETCURSEL, 0, 0 ) ; 
				SetDlgItemText( hDlg, ID_BACKUP_SERVER_NAME, (LPCTSTR) aServerInfo[iBackupServer].szUser ) ; 
				SetDlgItemText( hDlg, ID_BACKUP_SERVER_PWD, (LPCTSTR) aServerInfo[iBackupServer].szPassword ) ; 
				break ;
			}
			break ;
		
		default :
			break ;
		}
		break ;
	default:
		return ( FALSE ) ;
	}
	return ( FALSE ) ;
	UNREFERENCED_PARAMETER(lParam);
}


//------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: ReadINISettings
// PURPOSE:	
//			
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL GetINISettings()
{

	DString szSection ;
	DString szKey ;
	DString szTemp ;
	DString	szMessage ;
	DINIFile INIFileReader ;

	szSection.LoadString( IDS_SECTION_APPLICATION ) ;

	gINI.szLocalPath			= 
		INIFileReader.GetString(szKey.LoadString(IDS_LOCAL_PATH), szSection);

	gszEXEPath = INIFileReader.GetINIFileName () ;
	gszEXEPath.SetMid ( 0, gszEXEPath.ReverseFind('\\') ) ;
	if ( gINI.szLocalPath == "" )
	{
		gINI.szLocalPath = gszEXEPath ;
	}
	CreateDirectory ( gINI.szLocalPath, NULL ) ;

	gINI.szINIFilename = INIFileReader.GetINIFileName () ;
	gINI.szLaunchApp			=
		INIFileReader.GetString(szKey.LoadString(IDS_LAUNCH_APP), szSection);
	EXIT_IF_EMPTY(gINI.szLaunchApp)

	gINI.szApplicationName		=
		INIFileReader.GetString(szKey.LoadString(IDS_APPLICATION_NAME), szSection);
	EXIT_IF_EMPTY(gINI.szApplicationName)

	gINI.szRegistrySection		=
		INIFileReader.GetString(szKey.LoadString(IDS_REGISTRY_SECTION), szSection);
	if ( gINI.szLocalPath == "" )
	{
		gINI.szRegistrySection = gINI.szApplicationName ;
	}

	gINI.szVersionFile			=
		INIFileReader.GetString(szKey.LoadString(IDS_VERSION_FILE), szSection);
	if ( gINI.szLocalPath == "" )
	{
		gINI.szVersionFile = "version" ;
	}

	gINI.bAskToUpdate			=
		INIFileReader.GetBool(szKey.LoadString(IDS_ASK_TO_UPDATE), szSection);
	gINI.bRunWithoutUpdate		=
		INIFileReader.GetBool(szKey.LoadString(IDS_RUN_WITHOUT_UPDATE), szSection);
	gINI.bCopyOnlyChangedFiles	=
		INIFileReader.GetBool(szKey.LoadString(IDS_COPY_ONLY_CHANGED_FILES), szSection);
	gINI.bDeleteBackupFiles		=
		INIFileReader.GetBool(szKey.LoadString(IDS_DELETE_BACKUP_FILES), szSection);
	gINI.bDebug					=
		INIFileReader.GetBool(szKey.LoadString(IDS_DEBUG), szSection);
	gINI.iNDSCredentialsChange	=
		INIFileReader.GetInt(szKey.LoadString(IDS_NDS_CREDENTIALS_CHANGE), szSection);
	gINI.iAppWaitUptoDate		=
		INIFileReader.GetInt("AppWaitUptoDate", szSection);
	gINI.iAppWaitUpdated		=
		INIFileReader.GetInt("AppWaitUpdated", szSection);
	gINI.iProgressBarMaxVal		=
		INIFileReader.GetInt("ProgressBarMaxVal", szSection);
	gINI.iTextWait				=
		INIFileReader.GetInt("TextWait", szSection);

	gLocalServer.lNDSCredentialsChange		=	gINI.iNDSCredentialsChange	;
	gBackupServer.lNDSCredentialsChange		=	gINI.iNDSCredentialsChange	;

	return ( TRUE ) ;
}


//------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: DeleteRegistry
// PURPOSE:	
//			
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL DeleteRegistry()
{
	DString szRegistrySection ;
	DString szSection ;
	DString szKey ;
	DString szMessage ;
	DINIFile INIFileReader ;
	DRegistry RegistryDeleter ;

	szSection.LoadString( IDS_SECTION_APPLICATION ) ;

	szRegistrySection = INIFileReader.GetString(szKey.LoadString(IDS_REGISTRY_SECTION), szSection) ;
	if ( szRegistrySection == "" ) return ( FALSE ) ;

	szMessage.LoadString(IDS_REG_DEL_CONFIRM) ;
	szMessage += szRegistrySection ;
	if ( IDNO == szMessage.MessageBox ( szMessage, IDS_REG_DEL, MB_YESNO | MB_ICONEXCLAMATION ) ) return ( FALSE ) ;

	szKey.LoadString(IDS_REGISTRY_KEY) ;
	szKey += szRegistrySection ;
	RegistryDeleter = szKey ;
	if ( RegistryDeleter.DeleteCurrentKey() == TRUE )
	{
		szRegistrySection.LoadString(IDS_APPNAME) ;
		szMessage.LoadString(IDS_DELETED_REGISTRY) ;
		szMessage += szKey ;
		szKey.LoadString(IDS_DELETED_REGISTRY_COMPLETED) ;
		szMessage += szKey ;
		szMessage.MessageBox ( szMessage, szRegistrySection, MB_OK | MB_ICONEXCLAMATION) ;	
		return ( TRUE ) ;
	}
	else
	{
		szRegistrySection.LoadString(IDS_APPNAME) ;
		szMessage.LoadString(IDS_ERROR_REGISTRY) ;
		szMessage += szKey ;
		szKey.LoadString(IDS_ERROR_REGISTRY_END) ;
		szMessage += szKey ;
		szMessage += "\r\n\t\n\tSystem returned:\r\n" ;
		szMessage += RegistryDeleter.GetError() ;
		szMessage.MessageBox ( szMessage, szRegistrySection, MB_OK | MB_ICONERROR) ;	
		return ( FALSE ) ;
	}
}

//------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: GetRegistry
// PURPOSE:	
//			
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL GetRegistry ()
{
	DString szKey ;
	DRegistry RegistryReader ;

	szKey.LoadString(IDS_REGISTRY_KEY) ;
	szKey += gINI.szRegistrySection ;
	RegistryReader = szKey ;

	gLocalServer.szTitle = RegistryReader.GetString(szKey.LoadString(IDS_CS_TITLE)) ;
	if ( RegistryReader.IsInError() )return ( FALSE ) ;
	gLocalServer.szFullPath = RegistryReader.GetString(szKey.LoadString(IDS_CS_NAME)) ;
	if ( RegistryReader.IsInError() )return ( FALSE ) ;
	if ( gLocalServer.szFullPath[1] != '\\' ) gLocalServer.szFullPath = "\\" + gLocalServer.szFullPath ;
	if ( gLocalServer.szFullPath[1] != '\\' ) gLocalServer.szFullPath = "\\" + gLocalServer.szFullPath ;
	gLocalServer.szTree = RegistryReader.GetString(szKey.LoadString(IDS_CS_NWTREE)) ;
	if ( RegistryReader.IsInError() )return ( FALSE ) ;
	gLocalServer.szContext = RegistryReader.GetString(szKey.LoadString(IDS_CS_NWCONTEXT)) ;
	if ( RegistryReader.IsInError() )return ( FALSE ) ;
	gLocalServer.szUser = RegistryReader.GetString(szKey.LoadString(IDS_CS_USER)) ;
	if ( RegistryReader.IsInError() )return ( FALSE ) ;
	gLocalServer.szPassword = RegistryReader.GetString(szKey.LoadString(IDS_CS_PWD)) ;
	if ( RegistryReader.IsInError() )return ( FALSE ) ;
	gLocalServer.lOptions = RegistryReader.GetLongInt(szKey.LoadString(IDS_CS_OPTIONS)) ;
	if ( RegistryReader.IsInError() )return ( FALSE ) ;

	gBackupServer.szTitle = RegistryReader.GetString(szKey.LoadString(IDS_BS_TITLE)) ;
	if ( RegistryReader.IsInError() )return ( FALSE ) ;
	gBackupServer.szFullPath = RegistryReader.GetString(szKey.LoadString(IDS_BS_NAME)) ;
	if ( RegistryReader.IsInError() )return ( FALSE ) ;
	if ( gLocalServer.szFullPath[1] != '\\' ) gLocalServer.szFullPath = "\\" + gLocalServer.szFullPath ;
	if ( gLocalServer.szFullPath[1] != '\\' ) gLocalServer.szFullPath = "\\" + gLocalServer.szFullPath ;
	gBackupServer.szTree = RegistryReader.GetString(szKey.LoadString(IDS_BS_NWTREE)) ;
	if ( RegistryReader.IsInError() )return ( FALSE ) ;
	gBackupServer.szContext = RegistryReader.GetString(szKey.LoadString(IDS_BS_NWCONTEXT)) ;
	if ( RegistryReader.IsInError() )return ( FALSE ) ;
	gBackupServer.szUser = RegistryReader.GetString(szKey.LoadString(IDS_BS_USER)) ;
	if ( RegistryReader.IsInError() )return ( FALSE ) ;
	gBackupServer.szPassword = RegistryReader.GetString(szKey.LoadString(IDS_BS_PWD)) ;
	if ( RegistryReader.IsInError() )return ( FALSE ) ;
	gBackupServer.lOptions = RegistryReader.GetLongInt(szKey.LoadString(IDS_BS_OPTIONS)) ;
	if ( RegistryReader.IsInError() )return ( FALSE ) ;

	return ( TRUE ) ;
}

//------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: SetRegistry
// PURPOSE:	
//			
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL SetRegistry()
{
	DString szKey ;
	DRegistry RegistryReader ;

	szKey.LoadString(IDS_REGISTRY_KEY) ;
	szKey += gINI.szRegistrySection ;
	RegistryReader = szKey ;

	if ( RegistryReader.SetString(gLocalServer.szTitle, szKey.LoadString(IDS_CS_TITLE)) == FALSE ) return ( FALSE ) ;
	if ( RegistryReader.SetString(gLocalServer.szFullPath, szKey.LoadString(IDS_CS_NAME)) == FALSE ) return ( FALSE ) ;
	if ( RegistryReader.SetString(gLocalServer.szTree, szKey.LoadString(IDS_CS_NWTREE)) == FALSE ) return ( FALSE ) ;
	if ( RegistryReader.SetString(gLocalServer.szContext, szKey.LoadString(IDS_CS_NWCONTEXT)) == FALSE ) return ( FALSE ) ;
	if ( RegistryReader.SetString(gLocalServer.szUser, szKey.LoadString(IDS_CS_USER)) == FALSE ) return ( FALSE ) ;
	if ( RegistryReader.SetString(gLocalServer.szPassword, szKey.LoadString(IDS_CS_PWD)) == FALSE ) return ( FALSE ) ;
	if ( RegistryReader.SetLongInt(gLocalServer.lOptions, szKey.LoadString(IDS_CS_OPTIONS)) == FALSE ) return ( FALSE ) ;

	if ( RegistryReader.SetString(gBackupServer.szTitle, szKey.LoadString(IDS_BS_TITLE)) == FALSE ) return ( FALSE ) ;
	if ( RegistryReader.SetString(gBackupServer.szFullPath, szKey.LoadString(IDS_BS_NAME)) == FALSE ) return ( FALSE ) ;
	if ( RegistryReader.SetString(gBackupServer.szTree, szKey.LoadString(IDS_BS_NWTREE)) == FALSE ) return ( FALSE ) ;
	if ( RegistryReader.SetString(gBackupServer.szContext, szKey.LoadString(IDS_BS_NWCONTEXT)) == FALSE ) return ( FALSE ) ;
	if ( RegistryReader.SetString(gBackupServer.szUser, szKey.LoadString(IDS_BS_USER)) == FALSE ) return ( FALSE ) ;
	if ( RegistryReader.SetString(gBackupServer.szPassword, szKey.LoadString(IDS_BS_PWD)) == FALSE ) return ( FALSE ) ;
	if ( RegistryReader.SetLongInt(gBackupServer.lOptions, szKey.LoadString(IDS_BS_OPTIONS)) == FALSE ) return ( FALSE ) ;

	return ( TRUE ) ;
}

	