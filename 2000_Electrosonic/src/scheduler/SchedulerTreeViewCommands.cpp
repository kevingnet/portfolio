// SchedulerTreeView.cpp : implementation file
//

#include "stdafx.h"
#include "Scheduler.h"
#include "SchedulerTreeView.h"

#include "SchedulesView.h"
#include "SequencesView.h"
#include "TriggersView.h"
#include "EMailGroupsView.h"
#include "InputBoxDlg.h"
#include "DefaultsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////
//////			INLINE FUNCTIONS
//////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

bool CSchedulerTreeView::CancelIfCannotTransferObject( CCmdUI* pCmdUI, UINT id )
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return false;}
	if(!m_pDoc){RemoveMenu(pCmdUI,id);return false;}
	if(!m_pDoc->IsRemoteInstalled()){RemoveMenu(pCmdUI,id);return false;}
	if(m_pDoc->GetDestinationSiteLinxDeviceName()==""){pCmdUI->Enable(FALSE);return false;}
	if(!m_pDoc->IsRemoteEnabled()){pCmdUI->Enable(FALSE);return false;}
	if(!m_pDoc->IsFileConnected()){pCmdUI->Enable(FALSE);return false;}
	return true;
}

void CSchedulerTreeView::RemoveMenu( CCmdUI* pCmdUI, UINT id )
{
	if(!pCmdUI)return;
	pCmdUI->Enable(FALSE);
	if( pCmdUI->m_pMenu )
	{
		pCmdUI->m_pMenu->RemoveMenu( id, MF_BYCOMMAND );
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////
//////			APP COMMANDS
//////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void CSchedulerTreeView::OnSchedulerSettings() 
{
	CDefaultsDlg DefaultsDialog;
	DefaultsDialog.DoModal();
}

void CSchedulerTreeView::OnUpdateWindowCloseAll(CCmdUI* pCmdUI) 
{
	if( !gpApp->HasView() )
		pCmdUI->Enable(FALSE);
}

void CSchedulerTreeView::OnWindowClose() 
{
	gpApp->CloseView();
}

void CSchedulerTreeView::OnUpdateWindowClose(CCmdUI* pCmdUI) 
{
	if( !gpApp->HasView() )
		pCmdUI->Enable(FALSE);
}

void CSchedulerTreeView::OnWindowCloseAll() 
{
	gpApp->CloseAllViews();
}

void CSchedulerTreeView::OnUpdateFileClose(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	if( !gpApp->HasDoc() )
		pCmdUI->Enable(FALSE);
}

void CSchedulerTreeView::OnFileClose() 
{
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc )return;
	m_pDoc->OnCloseDocument2();
}

void CSchedulerTreeView::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	if( !gpApp->HasDoc() )
		pCmdUI->Enable(FALSE);
}

void CSchedulerTreeView::OnFileSave() 
{
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc )return;
	m_pDoc->OnSaveDocument( m_pDoc->GetPathName() );
}

void CSchedulerTreeView::OnUpdateFileSaveAs(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	if( !gpApp->HasDoc() )
		pCmdUI->Enable(FALSE);
}

void CSchedulerTreeView::OnFileSaveAs() 
{
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc )return;
	static char BASED_CODE szFilter[] = 
		"Schedules (*.ess)|*.ess|";
 	CFileDialog fdSaveAs( FALSE,
						  "ess",
						  m_pDoc->GetPathName(),
						  OFN_HIDEREADONLY|
						  OFN_NOCHANGEDIR|
						  OFN_OVERWRITEPROMPT|
						  OFN_PATHMUSTEXIST,
						  szFilter,
						  this );
	fdSaveAs.DoModal();
	m_pDoc->OnSaveDocument( fdSaveAs.GetPathName() );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////
//////			VIEW CREATION AND ACTIVATION, COMMANDS
//////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void CSchedulerTreeView::OnUpdateViewDevice(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
//	if( m_szItemName == g_szSchedulerName )
//		pCmdUI->Enable(FALSE);
}

void CSchedulerTreeView::OnViewDeviceProperties() 
{
	if(!m_htCurrentItem)return;
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc )return;
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	m_pDoc->ViewDeviceProperties( m_szItemName );
}

void CSchedulerTreeView::OnViewVariablesGroup() 
{
	if(!m_htCurrentItem)return;
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	ShowView( ITEM_SCHED_CHILD_VARIABLESGROUPS, m_szItemName );
}

void CSchedulerTreeView::OnViewEmailgroup() 
{
	if(!m_htCurrentItem)return;
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	ShowView( ITEM_SCHED_CHILD_EMAILGROUPS, m_szItemName );
}

void CSchedulerTreeView::OnViewSchedule() 
{
	if(!m_htCurrentItem)return;
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	ShowView( ITEM_SCHED_CHILD_SCHEDULES, m_szItemName );
}

void CSchedulerTreeView::OnViewSequence() 
{
	if(!m_htCurrentItem)return;
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	ShowView( ITEM_SCHED_CHILD_SEQUENCES, m_szItemName );
}

void CSchedulerTreeView::OnViewTriggerGroup() 
{
	if(!m_htCurrentItem)return;
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	ShowView( ITEM_SCHED_CHILD_TRIGGERS, m_szItemName );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////
//////			EDITING
//////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


void CSchedulerTreeView::OnUpdateAddDevice(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_DEVICES &&
		m_wItemType != ITEM_SCHED_DEVICES )
		pCmdUI->Enable(FALSE);
}

void CSchedulerTreeView::OnAddDevice() 
{
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc )return;
	m_pDoc->AddDevice();
}

void CSchedulerTreeView::OnUpdateDeleteDevice(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_DEVICES )
		pCmdUI->Enable(FALSE);
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
//	if( m_szItemName == g_szSchedulerName )
//		pCmdUI->Enable(FALSE);
	//if( m_pDoc->IsDeviceConnected( m_szItemName ) ){pCmdUI->Enable(FALSE);return;}
	if(m_pDoc->IsActive()){pCmdUI->Enable(FALSE);return;}

}

void CSchedulerTreeView::OnDeleteDevice() 
{
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc )return;
	if(AfxMessageBox( IDS_CHECK_DELETE_DEVICE, MB_OKCANCEL|MB_ICONEXCLAMATION) != IDOK )
		return;
	CString szNextItem = GetNextSelectItemText();
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	m_pDoc->DeleteDevice( m_szItemName );
	SelectItem( m_pDoc->GetDocName(), szNextItem, m_wItemType );
}

void CSchedulerTreeView::OnUpdateAddSchedule(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_SCHEDULES &&
		m_wItemType != ITEM_SCHED_SCHEDULES )
		pCmdUI->Enable(FALSE);
}

void CSchedulerTreeView::OnAddSchedule() 
{
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc )return;
	LPCTSTR lpName = m_pDoc->AddSchedule();
	if( lpName )
	{
		SelectItem( m_pDoc->GetDocName(), lpName, m_wItemType );
		if ( gpApp->m_Defaults.bDisplayViewOnCreate == true )
			ShowViewForSelectedItem();
	}
}


void CSchedulerTreeView::OnUpdateDeleteSchedule(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_SCHEDULES )
		pCmdUI->Enable(FALSE);
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	if( !m_pDoc )return;
	if( m_pDoc->IsSequenceRunning( m_szItemName ) ) 
	{
		pCmdUI->Enable(false);		
	} 
	else 
	{
		pCmdUI->Enable(true);
	}
}

void CSchedulerTreeView::OnDeleteSchedule() 
{
	if(AfxMessageBox( IDS_CHECK_DELETE_SCHEDULE, MB_OKCANCEL|MB_ICONEXCLAMATION) != IDOK )
		return;
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_SCHEDULES )
		return;
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	CString szNextItem = GetNextSelectItemText();
	m_pDoc->DeleteSchedule( m_szItemName );
	SelectItem( m_pDoc->GetDocName(), szNextItem, m_wItemType );
}

void CSchedulerTreeView::OnUpdateAddSequence(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_SEQUENCES &&
		m_wItemType != ITEM_SCHED_SEQUENCES )
		pCmdUI->Enable(FALSE);
}

void CSchedulerTreeView::OnAddSequence() 
{
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc )return;
	LPCTSTR lpName = m_pDoc->AddSequence();
	if( lpName )
	{
		SelectItem( m_pDoc->GetDocName(), lpName, m_wItemType );
		if ( gpApp->m_Defaults.bDisplayViewOnCreate == true )
			ShowViewForSelectedItem();
	}
}

void CSchedulerTreeView::OnUpdateCopySchedule(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_SCHEDULES )
		pCmdUI->Enable(FALSE);
}

void CSchedulerTreeView::OnCopySchedule() 
{
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc )return;
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	m_pDoc->CopySchedule( m_szItemName );
}

void CSchedulerTreeView::OnUpdateCopySequence(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_SEQUENCES )
		pCmdUI->Enable(FALSE);
}

void CSchedulerTreeView::OnCopySequence() 
{
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc )return;
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	m_pDoc->CopySequence( m_szItemName );
}

void CSchedulerTreeView::OnUpdateRename(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_EMAILGROUPS ) {
		m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
		if( m_szItemName == CTriggerGroupsCollection::sm_HaltedSequences )
			pCmdUI->Enable(FALSE);
	}
	if( !m_pDoc )return;
	if( m_pDoc->IsActive() ){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
}

void CSchedulerTreeView::OnRename() 
{
	switch( m_wItemType )
	{
		case ITEM_SCHED_CHILD_DEVICES:
		case ITEM_SCHED_CHILD_SEQUENCES:
		case ITEM_SCHED_CHILD_TRIGGERS:
		case ITEM_SCHED_CHILD_EMAILGROUPS:
		case ITEM_SCHED_CHILD_VARIABLESGROUPS:
			GetTreeCtrl().ModifyStyle( 0, TVS_EDITLABELS );
			GetTreeCtrl().EditLabel( m_htCurrentItem );
			break;
		case ITEM_SCHED_CHILD_SCHEDULES:
		case ITEM_SCHEDULER:
		case ITEM_SCHED_DEVICES:
		case ITEM_SCHED_SCHEDULES:
		case ITEM_SCHED_SEQUENCES:
		case ITEM_SCHED_TRIGGERS:
		case ITEM_SCHED_EMAILGROUPS:
		case ITEM_SCHED_VARIABLESGROUPS:
		default:
			break;
	}
}

void CSchedulerTreeView::OnUpdateDeleteSequence(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_SEQUENCES )
		pCmdUI->Enable(FALSE);
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	if( !m_pDoc )return;
	if( m_pDoc->IsSequenceRunning( m_szItemName ) ) 
	{
		pCmdUI->Enable(false);		
	} 
	else 
	{
		pCmdUI->Enable(true);
	}
}

void CSchedulerTreeView::OnDeleteSequence() 
{
	if(AfxMessageBox( IDS_CHECK_DELETE_SEQUENCE, MB_OKCANCEL|MB_ICONEXCLAMATION) != IDOK )
		return;
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_SEQUENCES )
		return;
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	CString szNextItem = GetNextSelectItemText();
	m_pDoc->DeleteSequence( m_szItemName );
	SelectItem( m_pDoc->GetDocName(), szNextItem, m_wItemType );
}

void CSchedulerTreeView::OnUpdateAddTriggerGroup(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_TRIGGERS &&
		m_wItemType != ITEM_SCHED_TRIGGERS )
		pCmdUI->Enable(FALSE);
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc ){pCmdUI->Enable(FALSE);return;}
//	if( !m_pDoc->GetDeviceManager().HasTriggeringDevices() )
//		{pCmdUI->Enable(FALSE);return;}
}

void CSchedulerTreeView::OnAddTriggerGroup() 
{
	LPCTSTR lpName = m_pDoc->AddTriggerGroup();
	if( lpName && *lpName )
	{
		SelectItem( m_pDoc->GetDocName(), lpName, m_wItemType );
		if ( gpApp->m_Defaults.bDisplayViewOnCreate == true )
			ShowViewForSelectedItem();
	}
}

void CSchedulerTreeView::OnUpdateDeleteTriggerGroup(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	if( m_szItemName == CTriggerGroupsCollection::sm_HaltedSequences )
		pCmdUI->Enable(FALSE);
	if( m_wItemType != ITEM_SCHED_CHILD_TRIGGERS )
		pCmdUI->Enable(FALSE);
}

void CSchedulerTreeView::OnDeleteTriggerGroup() 
{
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc )return;
	if(AfxMessageBox( IDS_CHECK_DELETE_TRIGGERGROUP, MB_OKCANCEL|MB_ICONEXCLAMATION) != IDOK )
		return;
	CString szNextItem = GetNextSelectItemText();
	m_pDoc->DeleteTriggerGroup( m_szItemName );
	SelectItem( m_pDoc->GetDocName(), szNextItem, m_wItemType );
}

void CSchedulerTreeView::OnUpdateAddEmailgroup(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_EMAILGROUPS &&
		m_wItemType != ITEM_SCHED_EMAILGROUPS )
		pCmdUI->Enable(FALSE);
}

void CSchedulerTreeView::OnAddEmailgroup() 
{
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc )return;
	LPCTSTR lpName = m_pDoc->AddEmailgroup();
	if( lpName && *lpName )
	{
		SelectItem( m_pDoc->GetDocName(), lpName, m_wItemType );
		if ( gpApp->m_Defaults.bDisplayViewOnCreate == true )
			ShowViewForSelectedItem();
	}
}

void CSchedulerTreeView::OnUpdateDeleteEmailgroup(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_EMAILGROUPS )
		pCmdUI->Enable(FALSE);
}

void CSchedulerTreeView::OnDeleteEmailgroup() 
{
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc )return;
	if(AfxMessageBox( IDS_CHECK_DELETE_EMAILGROUP, MB_OKCANCEL|MB_ICONEXCLAMATION) != IDOK )
		return;
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	CString szNextItem = GetNextSelectItemText();
	m_pDoc->DeleteEmailgroup( m_szItemName );
	SelectItem( m_pDoc->GetDocName(), szNextItem, m_wItemType );
}

void CSchedulerTreeView::OnUpdateAddVariablesGroup(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_VARIABLESGROUPS ) 
		pCmdUI->Enable(FALSE);
}

void CSchedulerTreeView::OnAddVariablesGroup() 
{
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc )return;
	LPCTSTR lpName = m_pDoc->AddVariablesGroup();
	if( lpName && *lpName )
	{
		SelectItem( m_pDoc->GetDocName(), lpName, m_wItemType );
		if ( gpApp->m_Defaults.bDisplayViewOnCreate == true )
			ShowViewForSelectedItem();
	}
}

void CSchedulerTreeView::OnUpdateDeleteVariablesGroup(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_VARIABLESGROUPS )
		pCmdUI->Enable(FALSE);
}

void CSchedulerTreeView::OnDeleteVariablesGroup() 
{
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc )return;
	if(AfxMessageBox( IDS_CHECK_DELETE_VARIABLESGROUP, MB_OKCANCEL|MB_ICONEXCLAMATION) != IDOK )
		return;
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	CString szNextItem = GetNextSelectItemText();
	m_pDoc->DeleteVariablesGroup( m_szItemName );
	SelectItem( m_pDoc->GetDocName(), szNextItem, m_wItemType );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////
//////			SCHEDULER COMMANDS
//////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

//DEVICE COMMANDS

void CSchedulerTreeView::OnUpdateConnectDevice(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc ){pCmdUI->Enable(FALSE);return;}
	if(!m_pDoc->IsActive()){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_DEVICES )
	{
		pCmdUI->Enable(FALSE);
	}
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
//	if( m_szItemName == g_szSchedulerName ){pCmdUI->Enable(FALSE);return;}
	if(m_pDoc->IsConnectedToRemote())return;
	if( m_pDoc->IsDeviceConnected( m_szItemName ) )
		pCmdUI->Enable(false);
	if( !m_pDoc->GetDeviceManager().AllDeviceConnectionsResolved() )
		pCmdUI->Enable(false);
}

void CSchedulerTreeView::OnConnectDevice() 
{
	m_pDoc->ConnectDevice( m_szItemName );
}

void CSchedulerTreeView::OnUpdateConnectDevices(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc ){pCmdUI->Enable(FALSE);return;}
	if(!m_pDoc->IsActive()){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
//	if( m_wItemType != ITEM_SCHED_CHILD_DEVICES &&
//		m_wItemType != ITEM_SCHED_DEVICES )
//	{
//		pCmdUI->Enable(FALSE);
//	}
	if(m_pDoc->IsConnectedToRemote())return;
	if( !m_pDoc->GetDeviceManager().AllDeviceConnectionsResolved() )
		{pCmdUI->Enable(FALSE);return;}
	//gpApp->EndWaitCursor();

}

void CSchedulerTreeView::OnConnectDevices() 
{
	m_pDoc->ConnectDevice();
}

void CSchedulerTreeView::OnUpdateDisconnectDevice(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc ){pCmdUI->Enable(FALSE);return;}
	if(!m_pDoc->IsActive()){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_DEVICES )
	{
		pCmdUI->Enable(FALSE);
	}
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
//	if( m_szItemName == g_szSchedulerName ){pCmdUI->Enable(FALSE);return;}
	if(m_pDoc->IsConnectedToRemote())return;
	if( !m_pDoc->IsDeviceConnected( m_szItemName ) )
		pCmdUI->Enable(false);
	if( !m_pDoc->GetDeviceManager().AllDeviceConnectionsResolved() )
		pCmdUI->Enable(false);
}

void CSchedulerTreeView::OnDisconnectDevice() 
{
	m_pDoc->DisconnectDevice( m_szItemName );
}

void CSchedulerTreeView::OnUpdateDisconnectDevices(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc ){pCmdUI->Enable(FALSE);return;}
	if(!m_pDoc->IsActive()){pCmdUI->Enable(FALSE);return;}
//	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
//	if( m_wItemType != ITEM_SCHED_CHILD_DEVICES &&
//		m_wItemType != ITEM_SCHED_DEVICES )
//	{
//		pCmdUI->Enable(FALSE);
//	}
	if(m_pDoc->IsConnectedToRemote())return;
	if( !m_pDoc->GetDeviceManager().AllDeviceConnectionsResolved() )
		pCmdUI->Enable(false);
}

void CSchedulerTreeView::OnDisconnectDevices() 
{
	m_pDoc->DisconnectDevice();
}

//SCHEDULE COMMANDS

void CSchedulerTreeView::OnUpdateRunSchedule(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc ){pCmdUI->Enable(FALSE);return;}
	if(!m_pDoc->IsActive()){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_SCHEDULES )
	{
		pCmdUI->Enable(FALSE);
		return;
	}
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	if(m_pDoc->IsConnectedToRemote())return;
	if( m_pDoc->IsSequenceRunning( m_szItemName ) ) {
		pCmdUI->Enable(false);		
	} else {
		pCmdUI->Enable(true);
	}
//	if( m_pDoc->IsSequenceSuspended( m_szItemName ) ) {
}

void CSchedulerTreeView::OnRunSchedule() 
{
	m_pDoc->RunSchedule( m_szItemName );
}

void CSchedulerTreeView::OnUpdateStopSchedule(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc ){pCmdUI->Enable(FALSE);return;}
	if(!m_pDoc->IsActive()){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_SCHEDULES )
	{
		pCmdUI->Enable(FALSE);
		return;
	}
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	if(m_pDoc->IsConnectedToRemote())return;
	if( m_pDoc->IsSequenceRunning( m_szItemName ) ) {
		pCmdUI->Enable(true);		
	} else {
		pCmdUI->Enable(false);
	}
}

void CSchedulerTreeView::OnUpdateSuspendSchedule(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc ){pCmdUI->Enable(FALSE);return;}
	if(!m_pDoc->IsActive()){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_SCHEDULES )
	{
		pCmdUI->Enable(FALSE);
		return;
	}
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	if(m_pDoc->IsConnectedToRemote())return;
	if( m_pDoc->IsSequenceRunning( m_szItemName ) ) {
		pCmdUI->Enable(true);		
	} else {
		pCmdUI->Enable(false);
	}
	if( m_pDoc->IsSequenceSuspended( m_szItemName ) ) 
	{
		pCmdUI->SetText( "Wakeup Schedule" );
	}
	else
	{
		pCmdUI->SetText( "Suspend Schedule" );
	}
}

void CSchedulerTreeView::OnSuspendSchedule() 
{
	m_pDoc->SuspendSchedule( m_szItemName );
}

void CSchedulerTreeView::OnStopSchedule() 
{
	m_pDoc->StopSchedule( m_szItemName );
}

void CSchedulerTreeView::OnUpdateScheduleReload(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc ){pCmdUI->Enable(FALSE);return;}
	if(!m_pDoc->IsActive()){pCmdUI->Enable(FALSE);return;}
}

void CSchedulerTreeView::OnScheduleReload() 
{
	m_pDoc->ScheduleReload();
}

//SEQUENCE COMMANDS

void CSchedulerTreeView::OnUpdateRunSequence(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc ){pCmdUI->Enable(FALSE);return;}
	if(!m_pDoc->IsActive()){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_SEQUENCES )
	{
		pCmdUI->Enable(FALSE);
		return;
	}
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	if(m_pDoc->IsConnectedToRemote())return;
	if( m_pDoc->IsSequenceRunning( m_szItemName ) )
		pCmdUI->Enable(false);
}

void CSchedulerTreeView::OnRunSequence() 
{
	m_pDoc->RunSequence( m_szItemName );
}

void CSchedulerTreeView::OnUpdateStopSequence(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc ){pCmdUI->Enable(FALSE);return;}
	if(!m_pDoc->IsActive()){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_SEQUENCES )
	{
		pCmdUI->Enable(FALSE);
		return;
	}
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	if(m_pDoc->IsConnectedToRemote())return;
	if( !m_pDoc->IsSequenceRunning( m_szItemName ) )
		pCmdUI->Enable(false);
}

void CSchedulerTreeView::OnStopSequence() 
{
	m_pDoc->StopSequence( m_szItemName );
}

void CSchedulerTreeView::OnUpdateSuspendSequence(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc ){pCmdUI->Enable(FALSE);return;}
	if(!m_pDoc->IsActive()){pCmdUI->Enable(FALSE);return;}
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_SEQUENCES )
	{
		pCmdUI->Enable(FALSE);
		return;
	}
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	if(m_pDoc->IsConnectedToRemote())return;
	if( !m_pDoc->IsSequenceRunning( m_szItemName ) )
		pCmdUI->Enable(false);
	if( m_pDoc->IsSequenceSuspended( m_szItemName ) ) 
	{
		pCmdUI->SetText( "Wakeup Sequence" );
	}
	else
	{
		pCmdUI->SetText( "Suspend Sequence" );
	}
}

void CSchedulerTreeView::OnSuspendSequence() 
{
	m_pDoc->SuspendSequence( m_szItemName );
}

//ACTIVE STATE

void CSchedulerTreeView::OnUpdateSchedulerSetActive(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc ){pCmdUI->Enable(FALSE);return;}
	if( m_pDoc->IsActive() )
		pCmdUI->Enable(FALSE);
}

void CSchedulerTreeView::OnSchedulerSetActive() 
{
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc )return;
	SchedulerSetActive( m_pDoc->GetDocName() );
}

void CSchedulerTreeView::SchedulerSetActive( LPCTSTR lpName ) 
{
	if( lpName )
	{
		CSchedulerDoc * pDoc = GetDocument( GetDocItem( lpName ) );
		if( pDoc )
		{
			gpApp->BeginWaitCursor();
			if( pDoc->IsFileSupposedlyNotConnected() )
			{
				//deactivate any locally active docs
				if(!::IsWindow(((CWnd*)&(GetTreeCtrl()))->GetSafeHwnd())) 
					return;
				HTREEITEM hTempItem = GetTreeCtrl().GetRootItem();
				while( hTempItem )
				{
					CString szTemp = GetTreeCtrl().GetItemText( hTempItem );
					CSchedulerDoc * pTempDoc = gpApp->GetDocument( szTemp );
					if( pTempDoc )
					{
						if( pTempDoc->IsFileSupposedlyNotConnected() &&
							pTempDoc->IsActive() )
							pTempDoc->SetActiveState( false );
					}
					hTempItem = GetTreeCtrl().GetNextSiblingItem( hTempItem );
				}
			}
			pDoc->SetActiveState( true );
			gpApp->EndWaitCursor();
		}
	}
}

void CSchedulerTreeView::OnUpdateSchedulerSetInactive(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc ){pCmdUI->Enable(FALSE);return;}
	if( !m_pDoc->IsActive() )
		pCmdUI->Enable(FALSE);
}

void CSchedulerTreeView::OnSchedulerSetInactive() 
{
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc )return;
	SchedulerSetInactive( m_pDoc->GetDocName() );
}

void CSchedulerTreeView::SchedulerSetInactive( LPCTSTR lpName ) 
{
	if( lpName )
	{
		CString m_szItemName = lpName;
		if(!::IsWindow(((CWnd*)&(GetTreeCtrl()))->GetSafeHwnd())) 
			return;
		HTREEITEM hTempItem = GetTreeCtrl().GetRootItem();
		while( hTempItem )
		{
			CString szTemp = GetTreeCtrl().GetItemText( hTempItem );
			CSchedulerDoc * pDoc = gpApp->GetDocument( szTemp );
			if( pDoc )
			{
				if( m_szItemName == pDoc->GetDocName() )
				{
					gpApp->BeginWaitCursor();
					pDoc->SetActiveState( false );
					gpApp->EndWaitCursor();
				}
			}
			hTempItem = GetTreeCtrl().GetNextSiblingItem( hTempItem );
		}
	}
}

void CSchedulerTreeView::OnUpdateToggleActiveMode(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if( !m_pDoc ){pCmdUI->Enable(FALSE);return;}
	pCmdUI->SetCheck(m_pDoc->IsActive());
}

void CSchedulerTreeView::OnToggleActiveMode() 
{
	bool bNewActiveState = !m_pDoc->IsActive();
	if( bNewActiveState == true )
		SchedulerSetActive( m_pDoc->GetDocName() );
	else
		SchedulerSetInactive( m_pDoc->GetDocName() );
}

void CSchedulerTreeView::OnUpdateFileSetsitelinxdevice(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	m_pDoc=GetDocument(GetParentItem(m_htCurrentItem));
	if(!m_pDoc){RemoveMenu(pCmdUI,ID_FILE_SETSITELINXDEVICE);return;}
	if(!m_pDoc->IsRemoteInstalled()){RemoveMenu(pCmdUI,ID_FILE_SETSITELINXDEVICE);return;}
}

void CSchedulerTreeView::OnFileSetsitelinxdevice() 
{
	CInputBoxDlg dlgInput;
	dlgInput.SetTitle( "Enter Remote Scheduler Node Name" );
	dlgInput.SetHelp( "The Remote Scheduler must have a SiteLinx device, we use that name as a Node Name" );
	dlgInput.m_szInput = m_pDoc->GetDestinationSiteLinxDeviceName();
	if ( dlgInput.DoModal() == IDCANCEL )
		return;
	m_pDoc->SetDestinationSiteLinxDeviceName( dlgInput.GetInput() );
}

void CSchedulerTreeView::OnUpdateFileConnect(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	if(!m_pDoc){RemoveMenu(pCmdUI,ID_FILE_CONNECT);return;}
	if(!m_pDoc->IsRemoteInstalled()){RemoveMenu(pCmdUI,ID_FILE_CONNECT);return;}
	if(m_pDoc->GetDestinationSiteLinxDeviceName()==""){pCmdUI->Enable(FALSE);return;}
	if(!m_pDoc->IsRemoteEnabled()){pCmdUI->Enable(FALSE);return;}
	if( m_pDoc->IsFileConnected() )
		pCmdUI->Enable(FALSE);
}

void CSchedulerTreeView::OnFileConnect() 
{
	m_pDoc->ConnectFile();
}

void CSchedulerTreeView::OnUpdateFileDisconnect(CCmdUI* pCmdUI) 
{
	if(!m_htCurrentItem){pCmdUI->Enable(FALSE);return;}
	if(!m_pDoc){RemoveMenu(pCmdUI,ID_FILE_DISCONNECT);return;}
	if(!m_pDoc->IsRemoteInstalled()){RemoveMenu(pCmdUI,ID_FILE_DISCONNECT);return;}
	if(m_pDoc->IsRemoteEnabled() && !m_pDoc->IsFileConnected())
	{
		pCmdUI->Enable(FALSE);
		return;
	}
	if(m_pDoc->IsFileSupposedlyNotConnected())
	{
		pCmdUI->Enable(FALSE);
		return;
	}
}

void CSchedulerTreeView::OnFileDisconnect() 
{
	m_pDoc->DisconnectFile();
}

void CSchedulerTreeView::OnUpdateFileGetremote(CCmdUI* pCmdUI) 
{
	CancelIfCannotTransferObject(pCmdUI,ID_FILE_GETREMOTE);
}

void CSchedulerTreeView::OnFileGetremote() 
{
	m_pDoc->RemoteGetDocument();
}

void CSchedulerTreeView::OnUpdateFileSendremote(CCmdUI* pCmdUI) 
{
	CancelIfCannotTransferObject(pCmdUI,ID_FILE_SENDREMOTE);
}

void CSchedulerTreeView::OnFileSendremote() 
{
	m_pDoc->RemoteSendDocument();
}

void CSchedulerTreeView::OnUpdateGetremoteDevices(CCmdUI* pCmdUI) 
{
	CancelIfCannotTransferObject(pCmdUI,ID_GETREMOTE_DEVICES);
}

void CSchedulerTreeView::OnGetremoteDevices() 
{
	m_pDoc->RemoteGetObjects( ITEM_SCHED_DEVICES );
}

void CSchedulerTreeView::OnUpdateGetremoteSchedules(CCmdUI* pCmdUI) 
{
	CancelIfCannotTransferObject(pCmdUI,ID_GETREMOTE_SCHEDULES);
}

void CSchedulerTreeView::OnGetremoteSchedules() 
{
	m_pDoc->RemoteGetObjects( ITEM_SCHED_SCHEDULES );
}

void CSchedulerTreeView::OnUpdateGetremoteSequences(CCmdUI* pCmdUI) 
{
	CancelIfCannotTransferObject(pCmdUI,ID_GETREMOTE_SEQUENCES);
}

void CSchedulerTreeView::OnGetremoteSequences() 
{
	m_pDoc->RemoteGetObjects( ITEM_SCHED_SEQUENCES );
}

void CSchedulerTreeView::OnUpdateGetremoteTriggergroups(CCmdUI* pCmdUI) 
{
	CancelIfCannotTransferObject(pCmdUI,ID_GETREMOTE_TRIGGERGROUPS);
}

void CSchedulerTreeView::OnGetremoteTriggergroups() 
{
	m_pDoc->RemoteGetObjects( ITEM_SCHED_TRIGGERS );
}

void CSchedulerTreeView::OnUpdateGetremoteEmailgroups(CCmdUI* pCmdUI) 
{
	CancelIfCannotTransferObject(pCmdUI,ID_GETREMOTE_EMAILGROUPS);
}

void CSchedulerTreeView::OnGetremoteEmailgroups() 
{
	m_pDoc->RemoteGetObjects( ITEM_SCHED_EMAILGROUPS );
}

void CSchedulerTreeView::OnUpdateGetremoteVariablesGroups(CCmdUI* pCmdUI) 
{
	CancelIfCannotTransferObject(pCmdUI,ID_GETREMOTE_VARIABLESGROUPS);
}

void CSchedulerTreeView::OnGetremoteVariablesGroups() 
{
	m_pDoc->RemoteGetObjects( ITEM_SCHED_VARIABLESGROUPS );
}

void CSchedulerTreeView::OnUpdateSendremoteDevice(CCmdUI* pCmdUI) 
{
	if(!CancelIfCannotTransferObject(pCmdUI,ID_SENDREMOTE_DEVICE))return;
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_DEVICES )
	{
		pCmdUI->Enable(FALSE);
	}
}

void CSchedulerTreeView::OnSendremoteDevice() 
{
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	m_pDoc->RemoteSendObjects( ITEM_SCHED_DEVICES, m_szItemName );
}

void CSchedulerTreeView::OnUpdateSendremoteDevices(CCmdUI* pCmdUI) 
{
	CancelIfCannotTransferObject(pCmdUI,ID_SENDREMOTE_DEVICES);
}

void CSchedulerTreeView::OnSendremoteDevices() 
{
	m_pDoc->RemoteSendObjects( ITEM_SCHED_DEVICES );
}

void CSchedulerTreeView::OnUpdateSendremoteSchedule(CCmdUI* pCmdUI) 
{
	if(!CancelIfCannotTransferObject(pCmdUI,ID_SENDREMOTE_SCHEDULE))return;
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_SCHEDULES )
	{
		pCmdUI->Enable(FALSE);
	}
}

void CSchedulerTreeView::OnSendremoteSchedule() 
{
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	m_pDoc->RemoteSendObjects( ITEM_SCHED_SCHEDULES, m_szItemName );
}

void CSchedulerTreeView::OnUpdateSendremoteSchedules(CCmdUI* pCmdUI) 
{
	CancelIfCannotTransferObject(pCmdUI,ID_SENDREMOTE_SCHEDULES);
}

void CSchedulerTreeView::OnSendremoteSchedules() 
{
	m_pDoc->RemoteSendObjects( ITEM_SCHED_SCHEDULES );
}

void CSchedulerTreeView::OnUpdateSendremoteSequence(CCmdUI* pCmdUI) 
{
	if(!CancelIfCannotTransferObject(pCmdUI,ID_SENDREMOTE_SEQUENCE))return;
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_SEQUENCES )
	{
		pCmdUI->Enable(FALSE);
	}
}

void CSchedulerTreeView::OnSendremoteSequence() 
{
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	m_pDoc->RemoteSendObjects( ITEM_SCHED_SEQUENCES, m_szItemName );
}

void CSchedulerTreeView::OnUpdateSendremoteSequences(CCmdUI* pCmdUI) 
{
	CancelIfCannotTransferObject(pCmdUI,ID_SENDREMOTE_SEQUENCES);
}

void CSchedulerTreeView::OnSendremoteSequences() 
{
	m_pDoc->RemoteSendObjects( ITEM_SCHED_SEQUENCES );
}

void CSchedulerTreeView::OnUpdateSendremoteTriggergroup(CCmdUI* pCmdUI) 
{
	if(!CancelIfCannotTransferObject(pCmdUI,ID_SENDREMOTE_TRIGGERGROUP))return;
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_TRIGGERS )
	{
		pCmdUI->Enable(FALSE);
	}
}

void CSchedulerTreeView::OnSendremoteTriggergroup() 
{
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	m_pDoc->RemoteSendObjects( ITEM_SCHED_TRIGGERS, m_szItemName );
}

void CSchedulerTreeView::OnUpdateSendremoteTriggergroups(CCmdUI* pCmdUI) 
{
	CancelIfCannotTransferObject(pCmdUI,ID_SENDREMOTE_TRIGGERGROUPS);
}

void CSchedulerTreeView::OnSendremoteTriggergroups() 
{
	m_pDoc->RemoteSendObjects( ITEM_SCHED_TRIGGERS );
}

void CSchedulerTreeView::OnUpdateSendremoteEmailgroup(CCmdUI* pCmdUI) 
{
	if(!CancelIfCannotTransferObject(pCmdUI,ID_SENDREMOTE_EMAILGROUP))return;
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_EMAILGROUPS )
	{
		pCmdUI->Enable(FALSE);
	}
}

void CSchedulerTreeView::OnSendremoteEmailgroup() 
{
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	m_pDoc->RemoteSendObjects( ITEM_SCHED_EMAILGROUPS, m_szItemName );
}

void CSchedulerTreeView::OnUpdateSendremoteEmailgroups(CCmdUI* pCmdUI) 
{
	CancelIfCannotTransferObject(pCmdUI,ID_SENDREMOTE_EMAILGROUPS);
}

void CSchedulerTreeView::OnSendremoteEmailgroups() 
{
	m_pDoc->RemoteSendObjects( ITEM_SCHED_EMAILGROUPS );
}

void CSchedulerTreeView::OnUpdateSendremoteVariablesGroup(CCmdUI* pCmdUI) 
{
	if(!CancelIfCannotTransferObject(pCmdUI,ID_SENDREMOTE_VARIABLESGROUP))return;
	m_wItemType = LOWORD(GetTreeCtrl().GetItemData( m_htCurrentItem ));
	if( m_wItemType != ITEM_SCHED_CHILD_VARIABLESGROUPS )
	{
		pCmdUI->Enable(FALSE);
	}
}

void CSchedulerTreeView::OnSendremoteVariablesGroup() 
{
	m_szItemName=GetTreeCtrl().GetItemText(m_htCurrentItem);
	m_pDoc->RemoteSendObjects( ITEM_SCHED_VARIABLESGROUPS, m_szItemName );
}

void CSchedulerTreeView::OnUpdateSendremoteVariablesGroups(CCmdUI* pCmdUI) 
{
	CancelIfCannotTransferObject(pCmdUI,ID_SENDREMOTE_VARIABLESGROUPS);
}

void CSchedulerTreeView::OnSendremoteVariablesGroups() 
{
	m_pDoc->RemoteSendObjects( ITEM_SCHED_VARIABLESGROUPS );
}

void CSchedulerTreeView::OnUpdateGetremoteAll(CCmdUI* pCmdUI) 
{
	CancelIfCannotTransferObject(pCmdUI,ID_GETREMOTE_ALL);
}

void CSchedulerTreeView::OnGetremoteAll() 
{
	m_pDoc->RemoteGetObjects( ITEM_SCHED_DEVICES );
	m_pDoc->RemoteGetObjects( ITEM_SCHED_SCHEDULES );
	m_pDoc->RemoteGetObjects( ITEM_SCHED_SEQUENCES );
	m_pDoc->RemoteGetObjects( ITEM_SCHED_TRIGGERS );
	m_pDoc->RemoteGetObjects( ITEM_SCHED_EMAILGROUPS );
	m_pDoc->RemoteGetObjects( ITEM_SCHED_VARIABLESGROUPS );
}

void CSchedulerTreeView::OnUpdateSendremoteAll(CCmdUI* pCmdUI) 
{
	CancelIfCannotTransferObject(pCmdUI,ID_SENDREMOTE_ALL);
}

void CSchedulerTreeView::OnSendremoteAll() 
{
	m_pDoc->RemoteSendObjects( ITEM_SCHED_DEVICES );
	m_pDoc->RemoteSendObjects( ITEM_SCHED_SCHEDULES );
	m_pDoc->RemoteSendObjects( ITEM_SCHED_SEQUENCES );
	m_pDoc->RemoteSendObjects( ITEM_SCHED_TRIGGERS );
	m_pDoc->RemoteSendObjects( ITEM_SCHED_EMAILGROUPS );
	m_pDoc->RemoteSendObjects( ITEM_SCHED_VARIABLESGROUPS );
}

void CSchedulerTreeView::OnUpdateUpdateIcons(CCmdUI* pCmdUI) 
{
	CancelIfCannotTransferObject(pCmdUI,ID_UPDATE_ICONS);
	if( m_pDoc->IsConnectedFromRemote() )
		pCmdUI->Enable(FALSE);
}

void CSchedulerTreeView::OnUpdateIcons() 
{
	m_pDoc->RemoteGetAllStatus();
}
