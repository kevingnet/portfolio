#include "stdafx.h"
#include "resource.h"
#include "DeviceScheduler.h"
#include "ppgIPAddress.h"
#include "Log.h"
#include "DlgCueSchedule.h"
#include "ByteArrayHelpers.h"

#include "FileVersion.h"

PCHAR CDeviceScheduler::m_DeviceType = (LPSTR)(LPCTSTR)g_szSchedulerName;

static PCHAR CmdText[] =  {
	"Run Sequence: ",
	"Stop Sequence: ",
	"Disable Device Tx: ",
	"Enable Device Tx: ",
	"Disable Day Schedule",
	"Enable Day Schedule",
	"Disable Device Rx: ",
	"Enable Device Rx: ",
	"Reconnect Devices: ",
	"Loop Sequence: ",
	"NoOp: ",
	"Suspend Sequence: ",
	"Wakeup Sequence: ",
	"Enable Halted Sequence: ",
};

#ifndef NumberOf
#define	NumberOf(x)	(sizeof(CmdText)/sizeof(CmdText[0]))
#endif

CDeviceScheduler::CDeviceScheduler( CDeviceManager* pDevMgr, CSequenceTable* pSeqMgr )
{
	ASSERT( pDevMgr );
	ASSERT( pSeqMgr );

	m_pSeqMgr	= pSeqMgr;
	m_pDevMgr	= pDevMgr;
}

CDeviceScheduler::~CDeviceScheduler()
{
}

void CDeviceScheduler::GetDeviceType( CString& str )
{
	str = m_DeviceType;
}

void CDeviceScheduler::Serialize( CArchive& /*ar*/ )
{
	// never gets serialized
}

BOOL CDeviceScheduler::IsOpen( void )
{
	return true;
}

bool CDeviceScheduler::Configure( void )
{
	return false;
}

TCPCommPort::SocketState CDeviceScheduler::GetState( void )
{
	return TCPCommPort::STATE_CONNECTED;
}

void CDeviceScheduler::OnStateChanging( TCPCommPort::SocketState State )
{
	CBaseDevice::OnStateChanging( State );
}

bool CDeviceScheduler::GoOnline( bool /*bOnline*/ )
{
	// scheduler is always online
	return true;
}

bool CDeviceScheduler::EditCue( CByteArray& Cue )
{
	bool			Res = false;
	CDlgCueSchedule	dlg( &(m_pDevMgr->GetDeviceCollection()), m_pSeqMgr );

	if( Cue.GetSize() ) {
		CString str;
		dlg.m_Command = (ScheduleCommands)Cue[0];
		
		if( Cue.GetSize() > 1 ) {
			dlg.m_Action = Cue[0];
			CopyData( dlg.m_Action, 1, Cue );
			CopyData( dlg.m_szComment, 1, Cue );
		}
	}

	if( dlg.DoModal() == IDOK ) {
		switch( dlg.m_Command ) {
			case SchedulerCmdNoOP:
				CopyData( Cue, 1, dlg.m_szComment );
				Cue[0] = (BYTE)dlg.m_Command;
				break;

			case SchedulerCmdRunSequence:
			case SchedulerCmdStopSequence:
			case SchedulerCmdSuspendSequence:
			case SchedulerCmdWakeupSequence:
			case SchedulerCmdEnableHaltedSequence:
			case SchedulerCmdLoopSequence:
			case SchedulerCmdDisableDeviceTx:
			case SchedulerCmdEnableDeviceTx:
			case SchedulerCmdDisableDeviceRx:
			case SchedulerCmdEnableDeviceRx:
				CopyData( Cue, 1, dlg.m_Action );
				Cue[0] = (BYTE)dlg.m_Command;
				break;
			case SchedulerCmdDisableDaySchedule:
			case SchedulerCmdEnableDaySchedule:
			case SchedulerCmdReconnectDevices:
				Cue.SetSize(1);
				Cue[0] = (BYTE)dlg.m_Command;
				break;
		}
		Res = true;
	}
	return Res;
}

void CDeviceScheduler::OnRenameDevice( CByteArray& baData, LPCTSTR lpOldName, LPCTSTR lpNewName )
{
	if( lpOldName && *lpOldName && 
		lpNewName && *lpNewName &&
		baData.GetSize() )
	{
		switch( baData[0] ) 
		{
		case SchedulerCmdDisableDeviceTx:
		case SchedulerCmdEnableDeviceTx:
		case SchedulerCmdDisableDeviceRx:
		case SchedulerCmdEnableDeviceRx:
		{
			CString szDeviceName;
			CopyData( szDeviceName, 1, baData );
			if( szDeviceName == lpOldName )
			{
				szDeviceName = lpNewName;
				baData.SetSize(1);
				CopyData( baData, 1, szDeviceName );
			}
		}
			break;

		case SchedulerCmdRunSequence:
		case SchedulerCmdStopSequence:
		case SchedulerCmdSuspendSequence:
		case SchedulerCmdWakeupSequence:
		case SchedulerCmdLoopSequence:
		case SchedulerCmdEnableHaltedSequence:
		
		case SchedulerCmdDisableDaySchedule:
		case SchedulerCmdEnableDaySchedule:
		case SchedulerCmdReconnectDevices:
		case SchedulerCmdNoOP:
			break;
		}
	}
}

void CDeviceScheduler::OnRenameSequence( CByteArray& baData, LPCTSTR lpOldName, LPCTSTR lpNewName )
{
	if( lpOldName && *lpOldName && 
		lpNewName && *lpNewName &&
		baData.GetSize() )
	{
		switch( baData[0] ) 
		{
		case SchedulerCmdRunSequence:
		case SchedulerCmdStopSequence:
		case SchedulerCmdSuspendSequence:
		case SchedulerCmdWakeupSequence:
		case SchedulerCmdEnableHaltedSequence:
		case SchedulerCmdLoopSequence:
		{
			CString szSequenceName;
			CopyData( szSequenceName, 1, baData );
			if( szSequenceName == lpOldName )
			{
				szSequenceName = lpNewName;
				baData.SetSize(1);
				CopyData( baData, 1, szSequenceName );
			}
		}
			break;
		
		case SchedulerCmdDisableDeviceTx:
		case SchedulerCmdEnableDeviceTx:
		case SchedulerCmdDisableDeviceRx:
		case SchedulerCmdEnableDeviceRx:

		case SchedulerCmdDisableDaySchedule:
		case SchedulerCmdEnableDaySchedule:
		case SchedulerCmdReconnectDevices:
		case SchedulerCmdNoOP:
			break;
		}
	}
}

bool CDeviceScheduler::GetCueText( CByteArray& Cue, CString& text )
{
	if( Cue.GetSize() )  {
		switch( Cue[0] ) {
			case SchedulerCmdNoOP:
			case SchedulerCmdRunSequence:
			case SchedulerCmdStopSequence:
			case SchedulerCmdSuspendSequence:
			case SchedulerCmdWakeupSequence:
			case SchedulerCmdEnableHaltedSequence:
			case SchedulerCmdLoopSequence:
			case SchedulerCmdDisableDeviceTx:
			case SchedulerCmdEnableDeviceTx:
			case SchedulerCmdDisableDeviceRx:
			case SchedulerCmdEnableDeviceRx:
			{
				text = CmdText[ Cue[0] ];
				CString str;
				CopyData( str, 1, Cue );
				text += str;
				break;
			}
			case SchedulerCmdDisableDaySchedule:
			case SchedulerCmdEnableDaySchedule:
			case SchedulerCmdReconnectDevices:
			{
				text = CmdText[ Cue[0] ];
				break;
			}
		}
	} else {
		text = "[Select Action]";
		return false;
	}
	return true;
}

bool CDeviceScheduler::FireEvent( CByteArray& Cue, LPCTSTR lpTriggeringDevice )
{
// the scheduler talks to the control system via a 
// windows notification message. This message is a byte
// array with the first byte being a command
	bool Res = false;
	if( Cue.GetSize() )
	{
		CSchedulerNotification* pN = new CSchedulerNotification;
		switch( Cue[0] ) {
			case SchedulerCmdNoOP:
			case SchedulerCmdRunSequence:
			case SchedulerCmdStopSequence:
			case SchedulerCmdSuspendSequence:
			case SchedulerCmdWakeupSequence:
			case SchedulerCmdEnableHaltedSequence:
			case SchedulerCmdLoopSequence:
			case SchedulerCmdDisableDeviceTx:
			case SchedulerCmdEnableDeviceTx:
			case SchedulerCmdDisableDeviceRx:
			case SchedulerCmdEnableDeviceRx:
			{
				pN->m_Cmd = (ScheduleCommands)Cue[0];
				CopyData( pN->m_Data, 1, Cue );
				break;
			}
			case SchedulerCmdDisableDaySchedule:
			case SchedulerCmdEnableDaySchedule:
			case SchedulerCmdReconnectDevices:
			{
				pN->m_Cmd = (ScheduleCommands)Cue[0];
				break;
			}
		}
		SendNotification( DEVNOTIFY_SCHEDULE_COMMAND, pN );
		if( lpTriggeringDevice && *lpTriggeringDevice )
		{
			CString str;
			GetCueText( Cue, str );
			str += " by Device: ";
			str += lpTriggeringDevice;
			SendToLog( str, LEVEL_EXTERNAL_TRIGGER );
		}
		Res = true;
	} else {
		TRACE(" Bad Command for Scheduler\n\r" );
	}
	return Res;
}

