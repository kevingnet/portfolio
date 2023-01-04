#include "stdafx.h"
#include "resource.h"
#include "DeviceVideoServer.h"
#include "ppgIPAddress.h"
#include "dlgCueVideoServer.h"
#include "ByteArrayHelpers.h"
#include "Log.h"
#include "SchedulerTreeView.h"
#include "DeviceVideoServerPlayListView.h"
#include "ppgTriggersVideoServer.h"

#include "FileVersion.h"


CPlayListCollection::CPlayListCollection()
{
}

CPlayListCollection::~CPlayListCollection()
{
	DeleteAllPlayLists();
}


CPlayList* CPlayListCollection::CopyPlayList( LPCTSTR pstrCopyTo, LPCTSTR pstrCopyFrom )
{
	CPlayList *pFrom	= GetPlayList( pstrCopyFrom );
	CPlayList* pTo		= NULL;

	if( pFrom ) {
		pTo = CreatePlayList( pstrCopyTo );

		if( pTo ) {
			for( int i = 0; i < pFrom->GetSize(); i++ ) {
				pTo->Add( pFrom->ElementAt(i) );
			}
		}
	}
	return pTo;
}

CPlayList* CPlayListCollection::CreatePlayList( LPCTSTR pstrName )
{
	// creates a new playlist to the collection.
	// Note. returns NULL if playlist already exists.
	CPlayList *pPlay = NULL;
	
	if( GetPlayList( pstrName ) == NULL ) {
		pPlay = new CPlayList();
		SetAt( pstrName, pPlay );
	}

	return pPlay;
}

CPlayList* CPlayListCollection::GetPlayList( LPCTSTR pstrName )
{
	CPlayList *pPlay;
	
	if( Lookup( pstrName, pPlay )) {
		return pPlay;
	}

	return NULL;		
}

void CPlayListCollection::DeletePlayList( LPCTSTR pstrName )
{
	CPlayList *pPlay;
	
	if( Lookup( pstrName, pPlay )) {
		RemoveKey( pstrName );
		delete pPlay;
	}
}

void CPlayListCollection::DeleteAllPlayLists()
{
	POSITION Pos = GetStartPosition();

	while( Pos ) {
		CPlayList	*pPlay;
		CString		Name;

		GetNextAssoc( Pos, Name, pPlay );

		if( pPlay ) {
			delete pPlay;
		}
	}

	RemoveAll();
}

void CPlayListCollection::Serialize( CArchive& ar )
{
	if( ar.IsStoring() ) {
		POSITION	Pos = GetStartPosition();
		CPlayList	*pPlay;
		CString		Name;

		while( Pos ) {
			GetNextAssoc( Pos, Name, pPlay );

			ar << Name;
			pPlay->Serialize( ar );
		}

		// write terminator.
		Name.Empty();
		ar << Name;
	} else {
		CPlayList	*pPlay;
		CString		Name;

		ar >> Name;
		while( Name.GetLength() ) {
			pPlay = new CPlayList;
			pPlay->Serialize( ar );
			SetAt( Name, pPlay );
			ar >> Name;
		}
	}
}





PCHAR CDeviceVideoServer::m_DeviceType = "ES VideoServer";

// Implements the video server device.
// this class requires the telnet parser to be active on
// the remote side.
//
// The byte list simply contains the text with spaces to
// separate the fields. This means the text command is the
// same as the cue.

CDeviceVideoServer::CDeviceVideoServer()
{
	m_iLast	= 0;
	InitSubItemIcons();
}

CDeviceVideoServer::~CDeviceVideoServer()
{
}

void CDeviceVideoServer::InitSubItemIcons(void)
{
	m_iIconArray[0] = gpTreeView->GetImageList()->Add( gpApp->LoadIcon(IDI_PLAYLIST));
}

int	CDeviceVideoServer::GetFirstSubItem(void)
{
	return 1;
}

static char szSubItemName[] = "Play Lists";

VideoServerTriggerNames vsTriggerNames[] = {
	TRIGGER_PLAYLIST_COMPLETED, "PlayList Completed",
};

int CDeviceVideoServer::GetNextSubItem( int& iPos, CString& szName )
{
	switch( iPos )
	{
	case 1:
		szName = szSubItemName;
		iPos++;
		return m_iIconArray[0];
	default:
		szName = "";
		iPos = 0;
		return -1;
	}
}


CWnd * CDeviceVideoServer::CreateSubItemView( int /*iType*/, int iId, CWnd* pWndParent )
{
	CWnd * pWnd= NULL;

//	if( strcmp( szName, szSubItemName ) == 0 ) {
	CRect rect(0,0,10,10);

	pWnd = new CDeviceVideoServerPlayListView( this, iId );
	pWnd->Create(  NULL, "Play Lists", WS_CHILD|WS_VISIBLE, rect, pWndParent, 2 );
//	}

	return pWnd;
}

void CDeviceVideoServer::DeleteSubItemView( CWnd* /*pWnd*/ )
{
	// Parent View has destroyed the window. 
	// All we need to do is delete is make sure we are not using the window pointer in
	// the device.
}

void CDeviceVideoServer::GetDeviceType( CString& str )
{
	str = m_DeviceType;
}


void CDeviceVideoServer::Serialize( CArchive& ar )
{
	CBaseDevice::Serialize( ar );

	enum {
		VERSION_BASE,
		VERSION_PLAYLIST,
		VERSION_TRIGGERS,
		VERSION_CURRENT = VERSION_TRIGGERS,
	};
	CVersion V( ar, VERSION_CURRENT );

	if( ar.IsStoring() ) {

		GetPlayListCollection()->Serialize( ar );

		// archive the trigger table
		ar << m_Triggers.GetSize();
		for( int i = 0; i < m_Triggers.GetSize(); i++ ) {
			ar << m_Triggers[i].m_Trigger;
			ar << m_Triggers[i].m_Channel;
		}

	} else {
		switch( V ) {
		case VERSION_PLAYLIST:
			GetPlayListCollection()->Serialize( ar );
			break;
		case VERSION_TRIGGERS:
		{
			GetPlayListCollection()->Serialize( ar );
			int count=0;
			ar >> count;
			for( int i=0; i<count; i++ ) {
				CVideoServerTrigger T;
				ar >> T.m_Trigger;
				ar >> T.m_Channel;
				m_Triggers.Add( T );
			}
		}
			break;
		default:
			break;
		}
	}
}

int CDeviceVideoServer::GetFirstTrigger( void )
{
	if( m_Triggers.GetSize() )
		return 1;
	return 0;
}

void CDeviceVideoServer::GetTrigger( int& Pos, CString& Name )
{
	if( Pos >= 1 && Pos <= m_Triggers.GetSize() ) {
		Name.Format("%s Channel %i", vsTriggerNames[m_Triggers[Pos-1].m_Trigger].Name, m_Triggers[Pos-1].m_Channel );
		Pos++;
	} else {
		Name.Empty();
	}

	if( Pos < 1 || Pos > m_Triggers.GetSize() ) {
		Pos = 0;
	}
}

void CDeviceVideoServer::AddPropertyPages( CPropertySheet& Sheet )
{
	m_ppgTriggers = new CppgTriggersVideoServer( &m_Triggers );
	Sheet.AddPage( m_ppgTriggers );
}

void CDeviceVideoServer::DeletePropertyPages( bool /*bUpdate*/ )
{
	if( m_ppgTriggers ) {
		delete m_ppgTriggers;
		m_ppgTriggers = NULL;
	}
}

bool CDeviceVideoServer::EditCue( CByteArray& Cue )
{
	CdlgCueVideoServer	dlg;
	dlg.SetDevice( this );

	if( Cue.GetSize() ) {
		CopyData( dlg.m_szCue, Cue );
	}

	if( dlg.DoModal() == IDOK ) {
		CopyData( Cue, dlg.m_szCue );
		return true;
	}
	return false;
}

bool CDeviceVideoServer::GetCueText( CByteArray& Cue, CString& text )
{
	CopyData( text, Cue );
	return true;
}

void CDeviceVideoServer::SendPlayList( LPCTSTR pName, int Channel )
{
	CPlayList	*pPlay;
	CString		strLog;

	strLog.Format( "Loading PlayList %s", pName );
	SendToLog( strLog, LEVEL_DEVICE_SEND );
	
	if( GetPlayListCollection()->Lookup( pName, pPlay )) {
		CString str;
		str.Format( "clear %d\n\r", Channel );
		SendPacket( (PBYTE)(LPCTSTR)str, str.GetLength() );

		for( int i = 0; i < pPlay->GetSize(); i++ ) {
			str.Format( "add %d \"%s\"\n\r", Channel, pPlay->ElementAt(i) );
			SendPacket( (PBYTE)(LPCTSTR)str, str.GetLength() );
		}
	} else {
		strLog.Format( "Could Not Find PlayList %s", pName );
		SendToLog( strLog, LEVEL_ERROR );
	}
}

bool CDeviceVideoServer::FireEvent( CByteArray& Cue, LPCTSTR lpTriggeringDevice )
{
	int iType = LEVEL_DEVICE_SEND;
	CString str;
	if( Cue.GetSize() ) {
		CString szMsg;

		CopyData( szMsg, Cue );
		if( !IsOpen() )
		{
			iType = LEVEL_DEVICE_OFFLINE;
			str = "Off line, could not execute ";
			str += szMsg;
		}
		else
		{
			str = szMsg;
		}

		if( strncmp( szMsg, "SendPlayList", 8 ) == 0 ) {
			// send playlist command.
			char	Command[30];
			char	Channels[80];
			int		Channel;
			char	Name[50];
			sscanf( szMsg, "%s %s %s", Command, Channels, Name );

			szMsg = Channels;
			szMsg.MakeLower();
			if( szMsg.Find("all") != -1 ) {
				for( int i=1; i<=MAX_CHANNELS; i++ ) {
					SendPlayList( Name, i );
				}
			} else {
				CStringArray saChannels;
				StringToArray( szMsg, saChannels, "," );
				for( int i=0; i<saChannels.GetSize(); i++ ) {
					Channel = atoi(saChannels.GetAt(i));
					SendPlayList( Name, Channel );
				}
			}
		} else {
			str = szMsg;
			szMsg += "\n\r";
			SendPacket( (const PBYTE)(LPCTSTR)szMsg, szMsg.GetLength() );
		}
	}
	if( lpTriggeringDevice && *lpTriggeringDevice )
	{
		iType = LEVEL_EXTERNAL_TRIGGER;
		str += " by Device: ";
		str += lpTriggeringDevice;
	}
	SendToLog( str, iType );
	if( iType == LEVEL_DEVICE_SEND )
		return true;
	return false;
}

void CDeviceVideoServer::ProcessResponse( PBYTE pData )
{
	if( !pData || !*pData ) return;
	if( IsReceiveEnabled() )
	{
		CString szTrigger = vsTriggerNames[0].Name; //only one trigger
		CString szMessage = pData;
		CStringArray saMessages;
		StringToArray( szMessage, saMessages, "\n" );
		char msg[40];
		int channel=0;
		for( int i=0; i<saMessages.GetSize(); i++ ) {
			szMessage = saMessages.GetAt(i);
			sscanf( szMessage, "%s", msg );
			if( -1 != szMessage.Find( msg ) ) {
				int idx = szMessage.Find("Completed", 7 );
				if( -1 != idx ) {
					idx += 10; //sizeof "Completed" + space
					szMessage = szMessage.Mid( idx );
					channel = atoi(szMessage);
					channel++; //it's zero based from vs, so inc
					for( int j=0; j<m_Triggers.GetSize(); j++ ) {
						if( m_Triggers[j].m_Channel == channel ) {
							FireTrigger( j+1 );
							break;
						}
					}
				}
			}
		}

	}
}

void CDeviceVideoServer::OnReceive( int /*nErrorCode*/ )
{
	const int MIN_SIZE = 20;
	BYTE	Data[255];
	BOOL	More = TRUE;
	while( More ) 
	{
		int Count = Receive(Data,sizeof(Data));
		if( Count >= MIN_SIZE ) {
			Data[Count] = '\0';
			ProcessResponse( Data );
		} else {
			More = FALSE;
		}
	}
}

