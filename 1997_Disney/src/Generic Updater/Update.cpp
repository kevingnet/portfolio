

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//			 ____________________________________________________________________
//			|							Includes							     |
//			 ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//

#include "Update.h"

#ifndef __DINIFILE_H__
	#include "DConnection.h"
#endif


DWORD dwResult ;

DString szDoubleLine("\r\n______________________________________________________________________________\r\n¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\r\n") ;

DLogFile log ;
SERVER_INFO_OUT sio ;
DConnection	* gConnUpdater ;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//			 ____________________________________________________________________
//			|							FUNCTIONS							     |
//			 ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//

//------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: ProcessUpdate
// PURPOSE:	
//			
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
void ProcessUpdate ()
{
	SYSTEMTIME			stStart											;
	SYSTEMTIME			stEnd											;
	INT iHours, iMinutes, iSeconds, iMilliseconds ;
	DString szTemp ;
	DString szTitle ;

	GetClientRect ( ghDlg, & gWRect ) ;
	gINI.iWindowTitleHeight =  gWRect.bottom - gWRect.top ;

	GetWindowRect ( ghDlg, & gWRect ) ;
	gINI.iWindowMaxHeight = gWRect.bottom - gWRect.top ;
	gINI.iWindowTitleHeight =  gINI.iWindowMaxHeight - gINI.iWindowTitleHeight ;

	MoveWindow ( 
				ghDlg, 
				gWRect.left, 
				gWRect.top, 
				gWRect.right - gWRect.left, 
				gINI.iWindowTitleHeight, 
				FALSE 
				) ;

	ShowWindow ( ghDlg, SW_SHOW ) ;
	SetForegroundWindow ( ghDlg ) ;

	log.Open() ;
	GetLocalTime( & stStart ) ;
	szTemp.SetLength(1024) ;
	sprintf ( szTemp, "%i.%i.%i.%i.%i.%i.%i ", stStart.wYear, stStart.wMonth,
			  stStart.wDay, stStart.wHour, stStart.wMinute, stStart.wSecond, stStart.wMilliseconds ) ;
	log << "STARTING GENERIC UPDATER: " << szTemp << "\r\n" ;
	log << szDoubleLine ;
	log << "UPDATER INI FILE AND REGISTRY VARIABLES\r\n\r\n" ;
	log << "\r\nINI File: '"	<< gINI.szINIFilename ;
	log << "'\r\n\r\nLocalPath: ............... '" << gINI.szLocalPath ;
	log << "'\r\nLaunchApp: ............... '" << gINI.szLaunchApp ;
	log << "'\r\nApplicationName: ......... '" << gINI.szApplicationName ;
	log << "'\r\nRegistrySection: ......... '" << gINI.szRegistrySection ;
	log << "'\r\nVersionFile: ............. '" << gINI.szVersionFile ;
	log << "'\r\nAskToUpdate: ............. " << ( ( gINI.bAskToUpdate == TRUE ) ? "True" : "False" ) ;
	log << "\r\nRunWithoutUpdate: ........ " << ( ( gINI.bRunWithoutUpdate == TRUE ) ? "True" : "False" ) ;
	log << "\r\nCopyOnlyChangedFiles: .... " << ( ( gINI.bCopyOnlyChangedFiles == TRUE ) ? "True" : "False" ) ;
	log << "\r\nDebug: ................... " << ( ( gINI.bDebug == TRUE ) ? "True" : "False" ) ;
	log << "\r\nNDSCredentialsChange: .... " << ( ( gINI.iNDSCredentialsChange == TRUE ) ? "True" : "False" ) ;
	log << "\r\n\r\nRegistry: \\\\HKEY_CURRENT_USER\\" ;
	log << szTemp.LoadString(IDS_REGISTRY_KEY) ;
	log << gINI.szRegistrySection ;

	log << "\r\n\r\nLocalServer: '" << gLocalServer.szTitle 	;
	log << "'\r\nLocalServer Path: '" << gLocalServer.szFullPath ;
	log << "'\r\nLocalServer User: '" << gLocalServer.szUser ;
	log << "'\r\n\r\nBackupServer: '" << gBackupServer.szTitle ;
	log << "'\r\nBackupServer Path: '" << gBackupServer.szFullPath ;
	log << "'\r\nBackupServer User: '" << gBackupServer.szUser ;
	log << "'\r\n" ;
	log << szDoubleLine ;

	DConnection	ConnUpdater ;
	gConnUpdater = & ConnUpdater ;
	gConnUpdater->SetServerInfo ( & sio ) ;
	gConnUpdater->SetLogFileHandle ( (HANDLE)log.GetLogHandle() ) ;
	gConnUpdater->LoadDLL () ;
	//gConnUpdater->ListConnections () ;

	log << szDoubleLine ;
	log << "\r\nCHECKING IF APPLICATION NEEDS UPDATE\r\n" ;

	switch ( CheckUpdate() )
	{
		case UPDATER_CHECK_UPTODATE :
			log << szTemp.LoadString(IDS_APP_UPTODATE) << "\r\n" ;
			SET_WINDOW_TITLE(szTemp)
			UpdaterEffects ( szTemp.LoadString(IDS_UPDATER_UPTODATE), gINI.iAppWaitUptoDate ) ;
			RunApp() ;
			break ;

		case UPDATER_CHECK_NEEDUPDATE :
			if ( gINI.bAskToUpdate == FALSE )
			{
				if ( Update () == TRUE )
				{
					UpdaterEffects ( szTemp.LoadString(IDS_UPDATER_UPDATED), gINI.iAppWaitUpdated ) ;
					RunApp() ;
				}
				else
				{
					szTemp.MessageBox( IDS_UPDATER_HAD_ERRORS, IDS_UPDATER_HAD_ERRORS, MB_OK | MB_ICONERROR ) ;
					UpdaterEffects ( szTemp.LoadString(IDS_UPDATER_FAILED), gINI.iAppWaitUpdated ) ;
					if ( gINI.bRunWithoutUpdate )
					{
						RunApp() ;
					}
				}
				break ;
			}
			if ( gINI.bRunWithoutUpdate )
			{
				szTitle.LoadString(IDS_APP_UPDATE_NEEDUPDATE) ;
				szTitle += gINI.szApplicationName ;
				dwResult = (LONG)szTemp.MessageBox( IDS_APP_UPDATE_INFORM, szTitle, MB_YESNO | MB_ICONEXCLAMATION ) ;
				switch ( dwResult )
				{
					case IDYES :
						if ( Update () == TRUE )
						{
							UpdaterEffects ( szTemp.LoadString(IDS_UPDATER_UPDATED), gINI.iAppWaitUpdated ) ;
							RunApp() ;
						}
						else
						{
							szTemp.MessageBox( IDS_UPDATER_HAD_ERRORS, IDS_UPDATER_HAD_ERRORS, MB_OK | MB_ICONERROR ) ;
							UpdaterEffects ( szTemp.LoadString(IDS_UPDATER_FAILED), gINI.iAppWaitUpdated ) ;
							RunApp() ;
						}
						break ;
					case IDNO :
						RunApp() ;
						break ;
					default :
						break ;
				}
				break ;
			}
			szTitle.LoadString(IDS_APP_UPDATE_NEEDUPDATE) ;
			szTitle += gINI.szApplicationName ;
			dwResult = (LONG)szTemp.MessageBox( IDS_APP_UPDATE_REQUIRED, szTitle, MB_YESNO | MB_ICONEXCLAMATION ) ;
			switch ( dwResult )
			{
				case IDYES :
					if ( Update () == TRUE )
					{
						UpdaterEffects ( szTemp.LoadString(IDS_UPDATER_UPDATED), gINI.iAppWaitUpdated ) ;
						RunApp() ;
					}
					else
					{
						szTemp.MessageBox( IDS_UPDATER_HAD_ERRORS, IDS_UPDATER_HAD_ERRORS, MB_OK | MB_ICONERROR ) ;
						UpdaterEffects ( szTemp.LoadString(IDS_UPDATER_FAILED), gINI.iAppWaitUpdated ) ;
					}

					break ;
				default :
					break ;
			}
			break ;
		case UPDATER_CHECK_FAILED_CONNECTION :
			szTemp.LoadString(IDS_UPDATER_CHECK_CONNECTION) ;
			log << szTemp ;
			szTemp.MessageBox( szTemp, IDS_UPDATER_CHECK_CONNECTION_TITLE, MB_OK | MB_ICONERROR ) ;
			UpdaterEffects ( szTemp.LoadString(IDS_UPDATER_FAILED_CHECK), gINI.iAppWaitUpdated ) ;
			if ( gINI.bRunWithoutUpdate )
			{
				RunApp() ;
			}
			break ;
		case UPDATER_CHECK_FAILED_VERSIONFILE :
			szTemp.LoadString(IDS_UPDATER_CHECK_VERSIONFILE) ;
			log << szTemp ;
			szTemp.MessageBox( szTemp, IDS_UPDATER_CHECK_VERSIONFILE_TITLE, MB_OK | MB_ICONERROR ) ;
			UpdaterEffects ( szTemp.LoadString(IDS_UPDATER_FAILED_CHECK), gINI.iAppWaitUpdated ) ;
			if ( gINI.bRunWithoutUpdate )
			{
				RunApp() ;
			}
			break ;
		default :
			break ;
	}

	if ( gConnUpdater->Disconnect() == FALSE )
	{
		log << szTemp.LoadString(IDS_CONNECTION_LEFT) ;
	}

	log << szDoubleLine ;

	szTemp.SetLength(1024) ;
	GetLocalTime( & stEnd ) ;
	sprintf ( szTemp, "%i.%i.%i.%i.%i.%i.%i ", stEnd.wYear, stEnd.wMonth,
			  stEnd.wDay, stEnd.wHour, stEnd.wMinute, stEnd.wSecond, stEnd.wMilliseconds ) ;
	log << "COMPLETING GENERIC UPDATER RUN: "  << szTemp << "\r\n" ;

	if ( stEnd.wMilliseconds >= stStart.wMilliseconds )
	{
		iMilliseconds = stEnd.wMilliseconds - stStart.wMilliseconds ;
	}
	else
	{
		iMilliseconds = 1000 + stEnd.wMilliseconds - stStart.wMilliseconds ;
		stStart.wSecond++ ;
	}

	if ( stEnd.wSecond >= stStart.wSecond )
	{
		iSeconds = stEnd.wSecond - stStart.wSecond ;
	}
	else
	{
		iSeconds = 60 + stEnd.wSecond - stStart.wSecond ;
		stStart.wMinute++ ;
	}

	if ( stEnd.wMinute >= stStart.wMinute )
	{
		iMinutes = stEnd.wMinute - stStart.wMinute ;
	}
	else
	{
		iMinutes = 60 + stEnd.wMinute - stStart.wMinute ;
		stStart.wHour++ ;
	}

	if ( stEnd.wHour >= stStart.wHour )
	{
		iHours = stEnd.wHour - stStart.wHour ;
	}
	else
	{
		iHours = 24 + stEnd.wHour - stStart.wHour ;
	}

	sprintf ( szTemp, "\r\nProgram Execution Time was: %i:%i:%i:%i ",
			  iHours, iMinutes, iSeconds, iMilliseconds ) ;

	log << szTemp << "\r\n\r\n" ;
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: CheckUpdate
// PURPOSE:	
//			
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
LONG CheckUpdate ()
{
	BOOL			bReturn						;
	HANDLE			hFile						;
	FILETIME		ftVS						;
	FILETIME		ftVL						;
	DString			szVersionServer				;
	DString			szVersionLocal				;
	DString			szTitle				;
	DString			szTemp				;

	szVersionLocal = gINI.szLocalPath + "\\" + gINI.szVersionFile ;

	szTitle.LoadString(IDS_CONNECT_VERIFY) ;
	szTitle += gLocalServer.szTitle ;
	SET_WINDOW_TITLE(szTitle)
	szTitle += "\r\n" ;
	log << szTitle ;

	gpServer = &gLocalServer ;
	*gConnUpdater = gpServer ;
	bReturn = gConnUpdater->Connect() ;

	if ( bReturn == FALSE )
	{
		szTemp.LoadString(IDS_CONNECT_LOCAL_FAILED) ;
		szTitle.LoadString(IDS_CONNECTION_FAILED) ;
		szTitle += gLocalServer.szTitle ;
		szTitle += szTemp ;
		SET_WINDOW_TITLE(szTitle)
		szTitle += "\r\n" ;
		log << szTitle ;

		gpServer = &gBackupServer ;
		*gConnUpdater = gpServer ;
		bReturn = gConnUpdater->Connect() ;
		if ( bReturn == FALSE )
		{
			return ( UPDATER_CHECK_FAILED_CONNECTION ) ;
		}
		else
		{
			gpServer = & gBackupServer ;
		}
	}
	else
	{
		gpServer = & gLocalServer ;
	}

	szTitle.LoadString(IDS_UPDATER_READING_S_VFILE) ;
	szTitle += gLocalServer.szTitle ;
	SET_WINDOW_TITLE(szTitle)
	szTitle += "\r\n" ;
	log << szTitle ;

	szTemp = gpServer->szFullPath ;
	szVersionServer = "\\" + szTemp.Right ( szTemp.GetLength() -1 ) + "\\" + gINI.szVersionFile ;
    hFile = CreateFile (   (LPCTSTR)szVersionServer, 
							GENERIC_READ, 
							FILE_SHARE_WRITE, 
							NULL, 
							OPEN_EXISTING, 
							FILE_ATTRIBUTE_NORMAL, 
							NULL ) ;
	if ( hFile == INVALID_HANDLE_VALUE ) 
	{
		log << "«» 'CheckUpdate OpenFile' " ;
		szTemp.GetErrorString( GetLastError() ) ;
		log << szTemp << "\r\n" << gpServer->szFullPath << "\\" << gINI.szVersionFile << "\r\n\r\n" ;
		
		if ( gpServer == & gBackupServer )
		{
			return ( UPDATER_CHECK_FAILED_VERSIONFILE ) ;
		}
		else
		{
			bReturn = gConnUpdater->Disconnect() ;
			if ( bReturn == FALSE )
			{
				log << "\r\n" << szTemp.LoadString(IDS_CONNECTION_LEFT) << "\r\n" ;
			}

			szTitle.LoadString(IDS_UPDATER_S_VFILE_FAILED) ;
			szTitle += gLocalServer.szTitle ;
			szTitle += szTemp ;
			SET_WINDOW_TITLE(szTitle)
			szTitle += "\r\n" ;
			log << szTitle ;

			gpServer = & gBackupServer ;
			gpServer = &gBackupServer ;
			*gConnUpdater = gpServer ;
			bReturn = gConnUpdater->Connect() ;
			if ( bReturn == FALSE )
			{
				return ( UPDATER_CHECK_FAILED_CONNECTION ) ;
			}

			szTitle.LoadString(IDS_UPDATER_READING_S_VFILE) ;
			szTitle += gLocalServer.szTitle ;
			SET_WINDOW_TITLE(szTitle)
			szTitle += " Backup\r\n" ;
			log << szTitle ;

			szTemp = gpServer->szFullPath ;
			szVersionServer = "\\" + szTemp.Right ( szTemp.GetLength() -1 ) + "\\" + gINI.szVersionFile ;
			hFile = CreateFile (   (LPCTSTR)szVersionServer, 
									GENERIC_READ, 
									FILE_SHARE_WRITE, 
									NULL, 
									OPEN_EXISTING, 
									FILE_ATTRIBUTE_NORMAL, 
									NULL ) ;
			if ( hFile == INVALID_HANDLE_VALUE ) 
			{
				log << "«» 'CheckUpdate Backup OpenFile' " ;
				szTemp.GetErrorString( GetLastError() ) ;
				log << szTemp << "\r\n" << gpServer->szFullPath << "\\" << gINI.szVersionFile << "\r\n\r\n" ;
				szTitle.LoadString(IDS_UPDATER_S_VFILE_B_FAILED) ;
				SET_WINDOW_TITLE(szTitle)
				szTitle += "\r\n" ;
				log << szTitle ;
				return ( UPDATER_CHECK_FAILED_VERSIONFILE ) ;
			}
		}
	}
	if ( GetFileTime ( hFile, NULL, NULL, & ftVS ) == 0 )
	{
		log << "«» 'CheckUpdate GetFileTime'" ;
		szTemp.GetErrorString( GetLastError() ) ;
		log << szTemp << "\r\n" << gpServer->szFullPath << "\\" << gINI.szVersionFile << "\r\n\r\n" ;
	}
	if ( CloseHandle ( hFile ) == 0 )
	{
		log << "«» 'CheckUpdate CloseHandle'" ;
		szTemp.GetErrorString( GetLastError() ) ;
		log << szTemp << "\r\n" << gpServer->szFullPath << "\\" << gINI.szVersionFile << "\r\n\r\n" ;
	}

	szTitle.LoadString(IDS_UPDATER_READING_L_VFILE) ;
	szTitle += gLocalServer.szTitle ;
	SET_WINDOW_TITLE(szTitle)
	szTitle += "\r\n" ;
	log << szTitle ;

	hFile = CreateFile (   (LPCTSTR)szVersionLocal, 
							GENERIC_READ, 
							FILE_SHARE_WRITE, 
							NULL, 
							OPEN_EXISTING, 
							FILE_ATTRIBUTE_NORMAL, 
							NULL ) ;
	if ( hFile == INVALID_HANDLE_VALUE ) 
	{
		log << "«» CheckUpdate OpenFile " ;
		szTemp.GetErrorString( GetLastError() ) ;
		log << szTemp << "\r\n" << gINI.szLocalPath << "\\" << gINI.szVersionFile << "\r\n\r\n" ;
		return ( UPDATER_CHECK_NEEDUPDATE ) ;
	}
	if ( GetFileTime ( hFile, NULL, NULL, & ftVL ) == 0 )
	{
		log << "«» 'CheckUpdate GetFileTime'" ;
		szTemp.GetErrorString( GetLastError() ) ;
		log << szTemp << "\r\n" << gINI.szLocalPath << "\\" << gINI.szVersionFile << "\r\n\r\n" ;
	}
	if ( CloseHandle ( hFile ) == 0 )
	{
		log << "«» 'CheckUpdate CloseHandle'" ;
		szTemp.GetErrorString( GetLastError() ) ;
		log << szTemp << "\r\n" << gINI.szLocalPath << "\\" << gINI.szVersionFile << "\r\n\r\n" ;
	}
	if ( CompareFileTime ( & ftVS, & ftVL ) == 0 )
	{
		return ( UPDATER_CHECK_UPTODATE ) ;
	}
	else
	{
		return ( UPDATER_CHECK_NEEDUPDATE ) ;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: Update
// PURPOSE:	
//			
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL Update ()
{
	HANDLE					hSearch						; 
	BOOL					bSuccess = TRUE				;
	WIN32_FIND_DATAA		FileData					; 
	DString szTemp ;
	DString szTitle ;
	DString szMessage ;

	glFileCount		=	0		;
	glDirCount		=	0		;

	MoveWindow ( 
				ghDlg, 
				gWRect.left, 
				gWRect.top, 
				gWRect.right - gWRect.left, 
				gINI.iWindowMaxHeight, 
				TRUE 
				) ;
	UpdateWindow ( ghDlg ) ;
	SetForegroundWindow ( ghDlg ) ;

	szTemp.LoadString(IDS_PROGRESSBAR_CHAR) ;
	if ( szTemp == "" )
	{
		cPBChar = PROGRESSBAR_CHAR ;
	}
	else
	{
		cPBChar = szTemp [0] ;
	}
	
	gszProgressBar = DString ( cPBChar, gINI.iProgressBarMaxVal ) ;

	SET_WINDOW_TITLE(szTitle.LoadString(IDS_UPDATER_PREPARING))

	log << szDoubleLine ;
	log << szTitle << "\r\n" ;

	gszDestPath = gINI.szLocalPath + "\\" ;
	
	gszBackupPath = gINI.szLocalPath + "\\" + BACKUP_DIR + "\\" ;
	gszSourcePath = gpServer->szFullPath ;
	gszSourcePath += "\\" ;

	ShowWindow ( ghDlg, SW_SHOW ) ;
	UpdateWindow ( ghDlg ) ;
	SetForegroundWindow ( ghDlg ) ;

	//count files in source
	SET_WINDOW_TITLE(szTitle.LoadString(IDS_UPDATER_COUNTING_FILES))
	log << szDoubleLine ;
	log << szTitle << "\r\n\r\n" ;
	bSuccess = CountFiles ( gszSourcePath ) ;

	hSearch = FindFirstFile ( gINI.szLocalPath + "\\" + BACKUP_OLD_DIR, & FileData ) ;
	if ( hSearch != INVALID_HANDLE_VALUE ) 
	{
		//delete old backup directory
		SET_WINDOW_TITLE(szTitle.LoadString(IDS_UPDATER_DELETING_OLD_FILES))
		log << szDoubleLine ;
		log << szTitle << "\r\n\r\n" ;
		szTemp = gINI.szLocalPath + "\\" + BACKUP_OLD_DIR ;
		bSuccess = DeleteTree ( szTemp ) ;
		if ( RemoveDirectory ( gINI.szLocalPath + "\\" + BACKUP_OLD_DIR ) == 0 )
		{
			bSuccess = FALSE ;
			log << "«» 'Update RemoveDirectory' " + gINI.szLocalPath + "\\" + BACKUP_OLD_DIR + " ERROR: " ;
			log << szTemp.GetErrorString(GetLastError()) << "\r\n" ;
		}
		if ( ! FindClose ( hSearch ) ) 
		{ 
			bSuccess = FALSE ;
			log << "Update 'Could not close find handle' \r\n" ;
		}
		hSearch = FindFirstFile ( gINI.szLocalPath + "\\" + BACKUP_OLD_DIR, & FileData ) ;
		if ( hSearch != INVALID_HANDLE_VALUE ) 
		{
			szTemp = gINI.szLocalPath + "\\Backup.ERR" ;
			bSuccess = DeleteTree ( szTemp ) ;
			if ( RemoveDirectory ( gINI.szLocalPath + "\\Backup.ERR" ) == 0 )
			{
				bSuccess = FALSE ;
				log << "«» 'Update RemoveDirectory' " + gINI.szLocalPath + "\\Backup.ERR ERROR: " ;
				log << szTemp.GetErrorString(GetLastError()) << "\r\n" ;
			}
			if ( MoveFile ( gINI.szLocalPath + "\\" + BACKUP_OLD_DIR, gINI.szLocalPath + "\\Backup.ERR" ) == 0 )
			{
				bSuccess = FALSE ;
				log << "«» 'Update MoveFile' " + gINI.szLocalPath + "\\Backup.ERR ERROR: " ;
				log << szTemp.GetErrorString(GetLastError()) << "\r\n" ;
			}
			if ( ! FindClose ( hSearch ) ) 
			{ 
				bSuccess = FALSE ;
				log << "Update 'Could not close find handle' \r\n" ;
			}
		}
	}

	//move backup directory to old
	hSearch = FindFirstFile ( gINI.szLocalPath + "\\" + BACKUP_DIR, & FileData ) ;
	if ( hSearch != INVALID_HANDLE_VALUE ) 
	{
		if ( MoveFile ( gINI.szLocalPath + "\\" + BACKUP_DIR, gINI.szLocalPath + "\\" + BACKUP_OLD_DIR ) == 0 )
		{
			bSuccess = FALSE ;
			log << "«» 'Update MoveFile' " + gINI.szLocalPath + "\\" + BACKUP_DIR + " ERROR: " ;
			log << szTemp.GetErrorString(GetLastError()) << "\r\n" ;
		}
		if ( ! FindClose ( hSearch ) ) 
		{ 
			bSuccess = FALSE ;
			log << "Update 'Could not close find handle' \r\n" ;
		}
	}

	//calculate progress bar values
	gfCurProgress = 0 ;
	if ( glDirCount > 0 )
	{
		gfMaxProgress = (float)( glDirCount + 1 ) ;
		if ( gfMaxProgress >= gINI.iProgressBarMaxVal )
		{
			gfPBBlock = ( gfMaxProgress / gINI.iProgressBarMaxVal ) ;
		}
		else
		{
			gfPBBlock = ( gINI.iProgressBarMaxVal / gfMaxProgress ) ;
		}
	}
	else
	{
		gfMaxProgress		= 1 ;
		gfPBBlock			= 1 ;
	}

	//create directories
	SET_WINDOW_TITLE(szTitle.LoadString(IDS_UPDATER_CREATING_DIR))
	log << szDoubleLine ;
	log << szTitle << "\r\n\r\n" ;
	if ( CreateDirectory ( gINI.szLocalPath + "\\" + BACKUP_DIR, NULL ) == 0 )
	{
		bSuccess = FALSE ;
		log << "«» 'Update CreateDirectory' " + gINI.szLocalPath + "\\" + BACKUP_DIR + " ERROR: " ;
		log << szTemp.GetErrorString(GetLastError()) << "\r\n" ;
	}
	bSuccess = MakeDirs () ;

	UpdateWindow( ghDlg ) ;
	SendDlgItemMessage ( ghDlg, IDC_MAIN_ICON, WM_PAINTICON, 0, 0 ) ; 

	//calculate progress bar values
	gfCurProgress = 0 ;
	if ( glFileCount > 0 )
	{
		gfMaxProgress = (float)( glFileCount + 1 ) ;
		if ( gfMaxProgress >= gINI.iProgressBarMaxVal )
		{
			gfPBBlock = ( gfMaxProgress / gINI.iProgressBarMaxVal ) ;
		}
		else
		{
			gfPBBlock = ( gINI.iProgressBarMaxVal / gfMaxProgress ) ;
		}
	}
	else
	{
		gfMaxProgress		= 1 ;
		gfPBBlock			= 1 ;
	}

	//copy and backup files
	SET_WINDOW_TITLE(szTitle.LoadString(IDS_UPDATER_COPYING))
	log << szDoubleLine ;
	log << szTitle << "\r\n" ;
	bSuccess = XCopyFiles () ;

	//verify files
	SET_WINDOW_TITLE(szTitle.LoadString(IDS_UPDATER_VERIFYING))
	log << szDoubleLine ;
	log << szTitle << "\r\n" ;
	bSuccess = VerifyFiles () ;
	log << szDoubleLine ;

	//check if there is still a version file in source server, if not
	//		this could indicate that a deployment had begun and the versions
	//		of the files we copied may not match
	if ( CopyVersionFile() == FALSE || bSuccess == FALSE )
	{
		SET_WINDOW_TITLE(szTitle.LoadString(IDS_UPDATER_RESTORING))
		log << szDoubleLine ;
		log << szTitle << "\r\n" ;
		RestoreFiles () ;
		bSuccess = FALSE ;
		DeleteFile ( gszDestPath + "version" ) ;
		szTitle.LoadString(IDS_DEPLOYMENT_BEGUN_TITLE) ;
		szMessage.LoadString(IDS_DEPLOYMENT_BEGUN) ;
		log << szDoubleLine ;
		log << szTitle << " " << szMessage << "\r\n" ;
		MessageBox ( ghDlg, szMessage, szTitle, MB_OK | MB_ICONERROR ) ;	
	}

	return ( bSuccess ) ;

}

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: CountFiles
// PURPOSE:	
//			
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CountFiles ( DString & szPath )
{
	HANDLE					hSearch										; 
	DWORD					dwAttrs										; 
	BOOL					bFinished					= FALSE			; 
	BOOL					bErrors						= FALSE			;
	BOOL					bSuccess					= TRUE			;
	WIN32_FIND_DATAA		FileData									; 
	DString					szTemp									;
	DString					szSource									;
	DString					szSourcePath								;

	szSourcePath = szPath ;
	if ( szSourcePath.Right(1) != "\\" )
	{
		szSourcePath += "\\" ;
	}
	szSource = szSourcePath + "*.*" ;

	hSearch = FindFirstFile ( (LPCTSTR)szSource, & FileData ) ;
	if (hSearch == INVALID_HANDLE_VALUE) 
	{
		bFinished = TRUE ;
		bErrors   = TRUE ;
		bSuccess = FALSE ;
		dwResult = GetLastError() ;
		log << "\r\nERROR! 'CountFiles FindFirstFile' " << gszLastError.GetErrorString(dwResult) << "\r\n" ;
	} 
	while ( ! bFinished ) 
	{ 
		//make sure file name doesn't start with a period
		if ( ( DOT_CHAR != FileData.cFileName[0] ) )
		{
			szSource = szSourcePath + FileData.cFileName ;

			dwAttrs = GetFileAttributes ( (LPCTSTR)szSource ) ; 
			if ( dwAttrs == 0xFFFFFFFF )
			{
				bSuccess = FALSE ;
				dwResult = GetLastError() ;
				log << "\r\nERROR! 'CountFiles GetFileAttributes' " << szSource << " " << gszLastError.GetErrorString(dwResult) << "\r\n" ;
			}
			else
			{
				if ( dwAttrs & FILE_ATTRIBUTE_DIRECTORY ) 
				{
					szTemp = szSource.Right ( szSource.GetLength() - gszSourcePath.GetLength() ) ;
					log << "\tFound Directory: " << szTemp << "\r\n" ;
					gaDirectories.Add ( DString ( szTemp ) ) ;
					glDirCount++ ;
					szSource = szSourcePath + FileData.cFileName ;
					CountFiles ( szSource ) ;
				}
				else
				{
					if ( gINI.szVersionFile.CompareNoCase( (LPCTSTR)FileData.cFileName ) != 0  )
					{
						szSource = szSource.Right ( szSource.GetLength() - gszSourcePath.GetLength() ) ;
						log << "\tFound File: " << szSource << "\r\n" ;
						gaFiles.Add ( DString ( szSource ) ) ;
						glFileCount++ ;
					}
				}
			}
		}
		if ( ! FindNextFile ( hSearch, &FileData ) ) 
		{
			if ( GetLastError() == ERROR_NO_MORE_FILES )
			{ 
				bFinished = TRUE; 
			} 
			else 
			{
				bErrors = TRUE ;
				bSuccess = FALSE ;
				dwResult = GetLastError() ;
				log << "\r\nERROR! 'CountFiles FindNextFile' " << gszLastError.GetErrorString(dwResult) << "\r\n" ;
			} 
		}
	} 
	if ( ! FindClose ( hSearch ) ) 
	{ 
		bErrors = TRUE ;
		bSuccess = FALSE ;
		dwResult = GetLastError() ;
		log << "\r\nERROR! 'CountFiles FindClose' " << gszLastError.GetErrorString(dwResult) << "\r\n" ;
	}
	return bSuccess ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: DeleteTree
// PURPOSE:	
//			
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL DeleteTree ( DString & szPath )
{
	HANDLE					hSearch										; 
	DWORD					dwAttrs										; 
	BOOL					bFinished					= FALSE			; 
	BOOL					bErrors						= FALSE			;
	BOOL					bSuccess					= TRUE			;
	WIN32_FIND_DATAA		FileData									; 
	DString					szSource									;
	DString					szSourcePath								;

	szSourcePath = szPath ;
	if ( szSourcePath.Right(1) != "\\" )
	{
		szSourcePath += "\\" ;
	}
	szSource = szSourcePath + "*.*" ;

	hSearch = FindFirstFile ( (LPCTSTR)szSource, & FileData ) ;
	if (hSearch == INVALID_HANDLE_VALUE) 
	{
		bFinished = TRUE ;
		bErrors   = TRUE ;
		bSuccess = FALSE ;
		dwResult = GetLastError() ;
		log << "\r\nERROR! 'DeleteTree FindFirstFile' " << gszLastError.GetErrorString(dwResult) << "\r\n" ;
	} 
	
	while ( ! bFinished ) 
	{ 
		//make sure file name doesn't start with a period
		if ( ( DOT_CHAR != FileData.cFileName[0] ) )
		{
			szSource = szSourcePath + FileData.cFileName ;
			dwAttrs = GetFileAttributes ( (LPCTSTR)szSource ) ;
			if ( dwAttrs == 0xFFFFFFFF )
			{
				bSuccess = FALSE ;
				dwResult = GetLastError() ;
				log << "\r\nERROR! 'DeleteTree GetFileAttributes' " << szSource << " " << gszLastError.GetErrorString(dwResult) << "\r\n" ;
			}
			else
			{
				if ( dwAttrs & FILE_ATTRIBUTE_DIRECTORY ) 
				{
					DeleteTree ( szSource ) ;
					if ( RemoveDirectory ( (LPCTSTR)szSource ) == 0 )
					{
						bSuccess = FALSE ;
						dwResult = GetLastError() ;
						log << "\r\nERROR! 'DeleteTree RemoveDirectory' " << szSource << " " << gszLastError.GetErrorString(dwResult) << "\r\n" ;
					}
					else
					{
						log << "\tRemoved directory: " << szSource << "\r\n" ;
					}
				}
				else
				{
					if ( DeleteFile ( (LPCTSTR)szSource ) == 0 )
					{
						bSuccess = FALSE ;
						dwResult = GetLastError() ;
						log << "\r\nERROR! 'DeleteTree DeleteFile' " << szSource << " " << gszLastError.GetErrorString(dwResult) << "\r\n" ;
					}
					else
					{
						log << "\tDeleted file: " << szSource << "\r\n" ;
					}
				}
			}
		}
		if ( ! FindNextFile ( hSearch, &FileData ) ) 
		{
			if ( GetLastError() == ERROR_NO_MORE_FILES )
			{ 
				bFinished = TRUE; 
			} 
			else 
			{
				bErrors = TRUE ;
				bSuccess = FALSE ;
				dwResult = GetLastError() ;
				log << "\r\nERROR! 'DeleteTree FindNextFile' " << gszLastError.GetErrorString(dwResult) << "\r\n" ;
			} 
		}
	} 
	if ( ! FindClose ( hSearch ) ) 
	{ 
		bErrors = TRUE ;
		bSuccess = FALSE ;
		dwResult = GetLastError() ;
		log << "\r\nERROR! 'DeleteTree FindClose' " << gszLastError.GetErrorString(dwResult) << "\r\n" ;
	}
	return bSuccess ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: MakeDirs
// PURPOSE:	
//			
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL MakeDirs ()
{
	BOOL			bSuccess = TRUE	;
	DString					szTemp									;
	if ( glDirCount > 0 )
	{
		INIT_PROGRESSBAR
		//loop through the directories array
		for ( int i = 0; i <= gaDirectories.GetSize(); i++ )
		{
			SET_LABEL_TOP(gaDirectories[i])
			//make directory
			szTemp = gszDestPath + gaDirectories[i] ;
			STEP_PROGRESSBAR

			if ( CreateDirectory ( (LPCTSTR)szTemp, NULL ) == 0 )
			{
				dwResult = GetLastError() ;
				if ( dwResult != ERROR_ALREADY_EXISTS )
				{
					bSuccess = FALSE ;
					log << "ERROR! 'MakeDirs CreateDirectory' " << szTemp << " " << gszLastError.GetErrorString(dwResult) << "\r\n" ;
				}
			}
			else
			{
				log << "\tCreated directory: " << szTemp << "\r\n" ;
			}
			//make backup directory
			szTemp = gszBackupPath + gaDirectories[i] ;
			if ( CreateDirectory ( (LPCTSTR)szTemp, NULL ) == 0 )
			{
				dwResult = GetLastError() ;
				if ( dwResult != ERROR_ALREADY_EXISTS )
				{
					bSuccess = FALSE ;
					log << "ERROR! 'MakeDirs CreateDirectory' Backup " << szTemp << " " << gszLastError.GetErrorString(dwResult) << "\r\n" ;
				}
			}
			else
			{
				log << "\tCreated directory: " << szTemp << "\r\n" ;
			}
		}
		FILL_PROGRESSBAR
		return bSuccess ;
 	}
	else
	{
		FILL_PROGRESSBAR
		return bSuccess ;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: XCopyFiles
// PURPOSE:	
//			
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL XCopyFiles ()
{
	int						i					;
	BOOL					bSuccess = TRUE		;
	HANDLE					hSearch				; 
	HANDLE					hFileSource			;
	HANDLE					hFileDest			;
	FILETIME				ftSource			;
	FILETIME				ftDest				;
	WIN32_FIND_DATAA		FileData			; 
	DString					szSource			;
	DString					szDest				;
	DString					szBackup			;

	gfCurProgress = 0 ;
	if ( glFileCount > 0 )
	{
		INIT_PROGRESSBAR
		STEP_PROGRESSBAR
		if ( gINI.bCopyOnlyChangedFiles == FALSE )
		{
			//loop through the file array
			for ( i = 0; i <= gaFiles.GetSize(); i++ )
			{
				SET_LABEL_TOP(gaFiles[i])
				STEP_PROGRESSBAR
		
				//calculate file names
				szDest		= gszDestPath	+ gaFiles[i] ;
				szSource	= gszSourcePath + gaFiles[i] ;
				szBackup	= gszBackupPath + gaFiles[i] ;
				if ( MoveFile ( (LPCTSTR)szDest, (LPCTSTR)szBackup ) == 0 )
				{
					dwResult = GetLastError() ;
					if ( dwResult != ERROR_FILE_NOT_FOUND )
					{
						bSuccess = FALSE ;
						log << "WARNING! 'XCopyFiles MoveFile' " << szDest << " To: " << szBackup << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
					}
				}
				else
				{
					log << "\r\n\tBacked up file: " << szDest << "\r\n\tTo: " << szBackup << "\r\n" ;
				}
				if ( CopyFile ( (LPCTSTR)szSource, (LPCTSTR)szDest, FALSE ) ==0 )
				{
					bSuccess = FALSE ;
					dwResult = GetLastError() ;
					log << "ERROR! XCopyFiles CopyFile' " << szSource << " To: " << szDest << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
				}
				else
				{
					log << "\tCopied file: " << szSource << "\r\n\tTo: " << szDest << "\r\n" ;
				}
			}
		}
		else
		{
			//loop through the file array
			for ( i = 0; i <= gaFiles.GetSize(); i++ )
			{
				SET_LABEL_TOP(gaFiles[i])
				STEP_PROGRESSBAR
		
				//calculate file names
				szDest		= gszDestPath	+ gaFiles[i] ;
				szSource	= gszSourcePath + gaFiles[i] ;
				szBackup	= gszBackupPath + gaFiles[i] ;

				hSearch = FindFirstFile ( (LPCTSTR)szDest, & FileData ) ;
				if (hSearch == INVALID_HANDLE_VALUE) 
				{
					if ( MoveFile ( (LPCTSTR)szDest, (LPCTSTR)szBackup ) == 0 )
					{
						dwResult = GetLastError() ;
						if ( dwResult != ERROR_FILE_NOT_FOUND )
						{
							bSuccess = FALSE ;
							log << "WARNING! XCopyFiles MoveFile' " << szDest << " To: " << szBackup << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
						}
					}
					else
					{
						log << "\r\n\tBacked up file: " << szDest << "\r\n\tTo: " << szBackup << "\r\n\r\n" ;
					}
					if ( CopyFile ( (LPCTSTR)szSource, (LPCTSTR)szDest, FALSE ) == 0 )
					{
						bSuccess = FALSE ;
						dwResult = GetLastError() ;
						log << "ERROR! XCopyFiles CopyFile' " << szSource << " To: " << szDest << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
					}
					else
					{
						log << "\tCopied file: " << szSource << "\r\n\tTo: " << szDest << "\r\n\r\n" ;
					}
				}
				else
				{
					hFileDest = CreateFile (   (LPCTSTR)szDest, 
												GENERIC_READ, 
												FILE_SHARE_WRITE, 
												NULL, 
												OPEN_EXISTING, 
												FILE_ATTRIBUTE_NORMAL, 
												NULL ) ;
					if ( hFileDest == INVALID_HANDLE_VALUE ) 
					{
						bSuccess = FALSE ;
						dwResult = GetLastError() ;
						log << "ERROR! XCopyFiles OpenFile' " << szDest << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
					}
					if ( GetFileTime ( hFileDest, NULL, NULL, & ftDest ) == 0 )
					{
						bSuccess = FALSE ;
						dwResult = GetLastError() ;
						log << "ERROR! XCopyFiles GetFileTime' " << szDest << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
					}
					hFileSource = CreateFile (  (LPCTSTR)szSource, 
												GENERIC_READ, 
												FILE_SHARE_WRITE, 
												NULL, 
												OPEN_EXISTING, 
												FILE_ATTRIBUTE_NORMAL, 
												NULL ) ;
					if ( hFileSource == INVALID_HANDLE_VALUE ) 
					{
						bSuccess = FALSE ;
						dwResult = GetLastError() ;
						log << "ERROR! XCopyFiles OpenFile' " << szSource << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
					}
					if ( GetFileTime ( hFileSource, NULL, NULL, & ftSource ) == 0 )
					{
						bSuccess = FALSE ;
						dwResult = GetLastError() ;
						log << "ERROR! XCopyFiles GetFileTime' " << szSource << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
					}

					if ( CompareFileTime ( & ftSource, & ftDest ) == 0 )
					{
						if ( CloseHandle ( hFileSource ) == 0 )
						{
							bSuccess = FALSE ;
							dwResult = GetLastError() ;
							log << "ERROR! XCopyFiles CloseHandle' " << szSource << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
						}
						if ( CloseHandle ( hFileDest ) == 0 )
						{
							bSuccess = FALSE ;
							dwResult = GetLastError() ;
							log << "ERROR! XCopyFiles CloseHandle' " << szDest << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
						}
						log << "\tFile time: " << szSource << "\r\n\tEquals file time: " << szDest << "\r\n\r\n" ;
					}
					else
					{
						if ( CloseHandle ( hFileSource ) == 0 )
						{
							bSuccess = FALSE ;
							dwResult = GetLastError() ;
							log << "ERROR! XCopyFiles CloseHandle' " << szSource << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
						}
						if ( CloseHandle ( hFileDest ) == 0 )
						{
							bSuccess = FALSE ;
							dwResult = GetLastError() ;
							log << "ERROR! XCopyFiles CloseHandle' " << szDest << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
						}
						if ( MoveFile ( (LPCTSTR)szDest, (LPCTSTR)szBackup ) == 0 )
						{
							dwResult = GetLastError() ;
							if ( dwResult != ERROR_FILE_NOT_FOUND )
							{
								bSuccess = FALSE ;
								log << "WARNING! XCopyFiles MoveFile' " << szDest << " To: " << szBackup << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
							}
						}
						else
						{
							log << "\r\n\tBacked up file: " << szDest << "\r\n\tTo: " << szBackup << "\r\n" ;
						}
						if ( CopyFile ( (LPCTSTR)szSource, (LPCTSTR)szDest, FALSE ) == 0 )
						{
							bSuccess = FALSE ;
							dwResult = GetLastError() ;
							log << "ERROR! XCopyFiles CopyFile' " << szSource << " To: " << szDest << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
						}
						else
						{
							log << "\tCopied file: " << szSource << "\r\n\tTo: " << szDest << "\r\n\r\n" ;
						}
					}
				}
			}
		}
		FILL_PROGRESSBAR
		return bSuccess ;
 	}
	else
	{
		FILL_PROGRESSBAR
		return bSuccess ;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: VerifyFiles
// PURPOSE:	
//			
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL VerifyFiles ()
{
	int						i					;
	HANDLE					hFile				;
	BOOL					bSuccess = TRUE		;
	FILETIME				ftDest				;
	FILETIME				ftSource			;
	SYSTEMTIME				stDest				;
	SYSTEMTIME				stSource			;
	DString					szDest				;
	DString					szSource			;
	DString					szMessage									;
	DString					szTemp									;
	szTemp.SetLength(1024) ;

	if ( glFileCount > 0 )
	{
		INIT_PROGRESSBAR
		for ( i = 0; i <= gaFiles.GetSize(); i++ )
		{
			SET_LABEL_TOP(gaFiles[i])
			szDest		= gszDestPath	+ gaFiles[i] ;
			szSource	= gszSourcePath + gaFiles[i] ;
			STEP_PROGRESSBAR

			hFile = CreateFile (   (LPCTSTR)szDest, 
									GENERIC_READ, 
									FILE_SHARE_WRITE, 
									NULL, 
									OPEN_EXISTING, 
									FILE_ATTRIBUTE_NORMAL, 
									NULL ) ;
			if ( hFile == INVALID_HANDLE_VALUE ) 
			{
				bSuccess = FALSE ;
				dwResult = GetLastError() ;
				log << "ERROR! VerifyFiles OpenFile' " << szDest << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
			}
			if ( GetFileTime ( hFile, NULL, NULL, & ftDest ) == 0 )
			{
				bSuccess = FALSE ;
				dwResult = GetLastError() ;
				log << "ERROR! VerifyFiles GetFileTime' " << szDest << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
			}
			if ( CloseHandle ( hFile ) == 0 )
			{
				bSuccess = FALSE ;
				dwResult = GetLastError() ;
				log << "ERROR! VerifyFiles CloseHandle' " << szDest << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
			}
			hFile = CreateFile (   (LPCTSTR)szSource, 
									GENERIC_READ, 
									FILE_SHARE_WRITE, 
									NULL, 
									OPEN_EXISTING, 
									FILE_ATTRIBUTE_NORMAL, 
									NULL ) ;
			if ( hFile == INVALID_HANDLE_VALUE ) 
			{
				bSuccess = FALSE ;
				dwResult = GetLastError() ;
				log << "ERROR! VerifyFiles OpenFile' " << szSource << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
			}
			if ( GetFileTime ( hFile, NULL, NULL, & ftSource ) == 0 )
			{
				bSuccess = FALSE ;
				dwResult = GetLastError() ;
				log << "ERROR! VerifyFiles GetFileTime' " << szSource << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
			}
			if ( CloseHandle ( hFile ) == 0 )
			{
				bSuccess = FALSE ;
				dwResult = GetLastError() ;
				log << "ERROR! VerifyFiles CloseHandle' " << szSource << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
			}

			if ( CompareFileTime ( & ftSource, & ftDest ) == 0 )
			{
				if ( FileTimeToSystemTime ( & ftSource, & stSource ) == 0 )
				{
					bSuccess = FALSE ;
					dwResult = GetLastError() ;
					log << "ERROR! VerifyFiles FileTimeToSystemTime' " << szSource << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
				}
				if ( FileTimeToSystemTime ( & ftDest, & stDest ) == 0 )
				{
					log << "ERROR! VerifyFiles FileTimeToSystemTime' " << szDest << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
				}
				sprintf(szTemp,
						"\r\n\t%u/%u/%u %u:%u:%u", 
						stSource.wYear,
						stSource.wMonth,
						stSource.wDay,
						stSource.wHour,
						stSource.wMinute,
						stSource.wSecond ) ;
				log << szTemp << " <- " << szSource ;

				sprintf(szTemp,
						"\r\n\t%u/%u/%u %u:%u:%u", 
						stDest.wYear,
						stDest.wMonth,
						stDest.wDay,
						stDest.wHour,
						stDest.wMinute,
						stDest.wSecond ) ;
				log << szTemp << " <- " << szDest << "\r\n\t\t\t\t· OK File times are equal.\r\n" ;
			}
			else
			{
				bSuccess = FALSE ;
				dwResult = GetLastError() ;
				log << "ERROR! VerifyFiles CompareFileTime' " << szSource << " Files times are not the same ERROR: " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;

				if ( FileTimeToSystemTime ( & ftSource, & stSource ) == 0 )
				{
					log << "ERROR! VerifyFiles FileTimeToSystemTime' " << szSource << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
				}
				if ( FileTimeToSystemTime ( & ftDest, & stDest ) == 0 )
				{
					log << "ERROR! VerifyFiles FileTimeToSystemTime' " << szDest << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
				}
				szMessage = "\r\nERROR! File times were not equal. Posibly copy operation failed.\r\n" ;
				sprintf(szTemp,
						"\r\n\t%u/%u/%u %u:%u:%u", 
						stSource.wYear,
						stSource.wMonth,
						stSource.wDay,
						stSource.wHour,
						stSource.wMinute,
						stSource.wSecond ) ;
				log << szTemp << " <- " << szSource ;

				sprintf(szTemp,
						"\r\n\t%u/%u/%u %u:%u:%u", 
						stDest.wYear,
						stDest.wMonth,
						stDest.wDay,
						stDest.wHour,
						stDest.wMinute,
						stDest.wSecond ) ;
				log << szTemp << " <- " << szDest << "\r\n" ;
			}
		}
	}
	FILL_PROGRESSBAR
	return bSuccess ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: RestoreFiles
// PURPOSE:	
//			
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL RestoreFiles ()
{
	int						i					;
	BOOL					bSuccess = TRUE		;
	DString					szSource			;
	DString					szDest				;

	if ( glFileCount > 0 )
	{
		for ( i = 0; i <= gaFiles.GetSize(); i++ )
		{
			SET_LABEL_TOP(gaFiles[i])
			szDest		= gszDestPath	+ gaFiles[i] ;
			szSource	= gszBackupPath + gaFiles[i] ;
			if ( DeleteFile ( (LPCTSTR)szDest ) == 0 )
			{
				bSuccess = FALSE ;
				dwResult = GetLastError() ;
				log << "ERROR! RestoreFiles DeleteFile' " << szDest << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
			}
			else
			{
				if ( MoveFile ( (LPCTSTR)szSource, (LPCTSTR)szDest ) == 0 )
				{
					bSuccess = FALSE ;
					dwResult = GetLastError() ;
					log << "WARNING! RestoreFiles MoveFile' " << szSource << " To: " << szDest << " " << gszLastError.GetErrorString(dwResult) << "\r\n\r\n" ;
				}
				else
				{
					log << "\tRestored file: " << szSource << "\r\n\tTo: " << szDest << "\r\n" ;
				}
			}
		}
	}
	return bSuccess ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: CopyVersionFile
// PURPOSE:	
//			
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CopyVersionFile ()
{
	DString	szVersionSource ;
	DString	szVersionDest ;
	DString	szTemp ;
	
	szTemp = gpServer->szFullPath ;
	szVersionSource = "\\" + szTemp.Right ( szTemp.GetLength() -1 ) + "\\" + gINI.szVersionFile ;
	szVersionDest = gINI.szLocalPath + "\\" + gINI.szVersionFile ;

	if ( CopyFile ( (LPCTSTR)szVersionSource, (LPCTSTR)szVersionDest, FALSE ) == 0 )
	{
		log << "\r\nERROR! CopyFile " + szVersionSource + " To: " + szVersionDest + " " ;
		log << szTemp.GetErrorString(GetLastError()) << "\r\n" ;
		return FALSE ;
	}
	else
	{
		return TRUE ;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: RunApp
// PURPOSE:	
//			
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL RunApp ()
{
	DString					szTemp									;
	DString					szMessage								;
	ShellExecute( NULL, "open", gINI.szLocalPath + "\\" + gINI.szLaunchApp, NULL, gINI.szLocalPath, SW_SHOW ) ;
	DWORD dwError = GetLastError() ;
	if ( ERROR_SUCCESS != dwError )
	{
		szTemp.GetErrorString(dwError) ;
		szTemp += "\r\n" + gINI.szLocalPath + "\\" + gINI.szLaunchApp ;
		szMessage.LoadString( IDS_FAILED_TO_RUN_APP ) ;
		szMessage += gINI.szApplicationName ;
		szTemp.MessageBox ( szTemp, szMessage, MB_OK | MB_ICONERROR ) ;
		return ( FALSE ) ;
	}
	return ( TRUE ) ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: UpdaterEffects
// PURPOSE:	
//			
// AUTHOR: Kevin Guerra
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////////////////////////
void UpdaterEffects ( LPCTSTR szTitle, int iWait )
{
	UINT i, iLen ;
	DString szTemp = " " ;

	MoveWindow ( 
				ghDlg, 
				gWRect.left, 
				gWRect.top, 
				gWRect.right - gWRect.left, 
				gINI.iWindowTitleHeight, 
				TRUE 
				) ;

 	SetForegroundWindow ( ghDlg ) ;

	iLen = strlen(szTitle) ;
	if ( iLen > 100 ) iLen = 100 ;
	for ( i=0; i<=iLen; i++ )
	{
		szTemp += szTitle[i] ;
		SetWindowText ( ghDlg, szTemp ) ;
		UpdateWindow( ghDlg ) ;
		Sleep ( gINI.iTextWait ) ;
	}
	Sleep ( iWait * 1000 ) ;
}
