#include "stdafx.h"
#include "Sequence.h"
#include "FileVersion.h"
#include "SchedulerDoc.h"
#include "DeviceScheduler.h"
#include "ByteArrayHelpers.h"

CSequence::CSequence()
{
	m_dwState = 0;
	m_pSchedulerDoc		= NULL;
	m_pDeviceColl		= NULL;
	m_pHaltedSequencesTriggerGroup = NULL;
}

CSequence::~CSequence()
{
	RemoveAll();
}

const CSequence& CSequence::operator=( const CSequence* pSeq )
{
	if( pSeq )
	{
		int nEvents = pSeq->GetSize();
		for( int i = 0; i < nEvents; i++ )
		{
			CEvent* pSource = (*pSeq)[i];
			CEvent* pE = new CEvent( pSource );
			Add( pE );
		}
	}
	return *this;
}

bool CSequence::Copy( const CSequence* pSeq )
{
	if( pSeq )
	{
		int nEvents = pSeq->GetSize();
		for( int i = 0; i < nEvents; i++ )
		{
			CEvent* pSource = (*pSeq)[i];
			CEvent* pE = new CEvent( pSource );
			Add( pE );
		}
		return true;
	}
	return false;
}

void CSequence::SetSchedulerDoc( CSchedulerDoc* pDoc )
{ 
	m_pSchedulerDoc = pDoc;
	m_pHaltedSequencesTriggerGroup = m_pSchedulerDoc->
		GetActiveSequenceManager()->
			GetGenericTriggers()->
				GetGroup( CTriggerGroupsCollection::sm_HaltedSequences );
}

CEvent * CSequence::GetEvent( int index )
{
	if( !this ) return NULL;
	if( GetSize() && 
		index>=0 && 
		index<GetSize() )
		return (*this)[index];;
	return NULL;
}

void CSequence::ChangeCueTimes( ESTime time, bool bAdd, int iCueStart, int iCueCount, int iCueTimeTypes )
{
	int iNumberOfCues=GetSize();

	int iFirstCue = iCueStart;
	if( iFirstCue > iNumberOfCues ) iFirstCue = iNumberOfCues;

	int iLastCue = iFirstCue + iCueCount;
	if( iCueCount == -1 ) iLastCue = iNumberOfCues;
	if( iLastCue > iNumberOfCues ) iLastCue = iNumberOfCues;

	bool bChange;
	for( int i=iFirstCue; i<iLastCue; i++ )
	{
		CEvent* pE = GetEvent(i);
		bChange = false;
		if( pE ) {
			switch( iCueTimeTypes )
			{
			case TIME_TYPE_AT:
				if( pE->IsAtTime() )
					bChange = true;
				break;
			case TIME_TYPE_WAIT:
				if( pE->IsWaitTime() )
					bChange = true;
				break;
			case TIME_TYPE_HALT:
				if( pE->IsHaltTime() )
					bChange = true;
				break;
			case TIME_TYPE_HALT_UNTIL:
				if( pE->IsHaltUntilTime() )
					bChange = true;
				break;
			default:
				bChange = true;
				break;
			}
			if( bChange )
			{
				if( bAdd )
					pE->GetTimeRef() += time;
				else
					pE->GetTimeRef() -= time;
				if( pE->GetTime() < 0 )
					pE->SetTime(0);
				if( pE->GetTime() > 86399 )
					pE->SetTime(86399);
			}
		}
	}
}

void CSequence::SetChanged( bool bVal )
{
	if( bVal ) {
		m_dwState |= SEQ_ForceCalc;
		m_dwState |= SEQ_Changed;
	} else {
		m_dwState &= (~SEQ_Changed);
	}
}

void CSequence::SetSuspended( bool bVal )
{
	if( bVal )
		m_dwState |= SEQ_Suspended;
	else
		m_dwState &= (~SEQ_Suspended);
}

void CSequence::SetHalted( bool bVal )
{
	//mutually exclusive
	m_dwState &= (~SEQ_HaltedUntilTrigger);
	if( bVal ) {
		m_dwState |= SEQ_Halted;
	} else {
		m_dwState &= (~SEQ_Halted);
	}
}

void CSequence::SetHaltedUntilTrigger( bool bVal )
{
	//mutually exclusive
	m_dwState &= (~SEQ_Halted);
	if( bVal ) {
		m_dwState |= SEQ_HaltedUntilTrigger;
	} else {
		m_dwState &= (~SEQ_HaltedUntilTrigger);
	}
}

void CSequence::SetRunning( bool bVal )
{
	if( bVal ) {
		m_dwState |= SEQ_Running;
	} else {
		m_dwState &= (~SEQ_Running);
		// if stopped cannot be suspended so take flag off as well
		m_dwState &= (~SEQ_Suspended);
		m_dwState &= (~SEQ_Halted);
		m_dwState &= (~SEQ_HaltedUntilTrigger);
	}
}

void CSequence::SetDeleted( bool bVal )
{
	if( bVal ) {
		m_dwState |= SEQ_Delete;
	} else {
		m_dwState &= (~SEQ_Delete);
	}
}

void CSequence::SetSchedule( bool bVal )
{
	if( bVal )
		m_dwState |= SEQ_Schedule;
	else
		m_dwState &= (~SEQ_Schedule);
}

void CSequence::Serialize( CArchive& ar )
{
	// serialize a sequence out to an archive.
	enum {
		VERSION_BASE,
		VERSION_CURRENT = VERSION_BASE,
	};
	CVersion V( ar, VERSION_CURRENT );

	if( ar.IsStoring() ) {
		long mask = m_dwState & SEQ_Schedule;

		// mask out the states we are not interested in
		ar << mask;
		
		int nEvents = GetSize();

		ar << m_szName;
		ar << nEvents;

		for( int i = 0; i < nEvents; i++ ) {
			CEvent* pE = GetEvent(i);
			if( pE )
				pE->Serialize( ar );
		}
	} else {
		int	nEvents;

		ar >> m_dwState;

		ar >> m_szName;
		ar >> nEvents;

		for( int i = 0; i < nEvents; i++ ) {
			CEvent* pE = new CEvent();
			pE->Serialize( ar );
			Add( pE );
		}
	}
}

bool CSequence::InsertEvent( int iAtEvent, int TimeType, ESTime tEventTime, LPCTSTR pDeviceName, CByteArray& baData, ESTime tRepeatTime, ESTime tRepeatFor )
{
	ESTime tLastTime = tEventTime + tRepeatFor;	//get end time

	// check if repeat time fits within a day
	if( tRepeatTime <= 0 || tRepeatTime > (24*60*60 - 1) ) {
		// ensure termination of the loop below.
		tRepeatTime = 24*60*60;
	}

	CEvent* pEv;
	int iAt = iAtEvent;
	pEv = new CEvent( tEventTime, pDeviceName, baData, TimeType );
	InsertEvent( iAt++, pEv );
	tEventTime += tRepeatTime;
	while( tEventTime <= tLastTime )
	{
		if( TimeType & ESTIME_TYPE_WAIT )
			pEv = new CEvent( tRepeatTime, pDeviceName, baData, TimeType );
		else
			pEv = new CEvent( tEventTime, pDeviceName, baData, TimeType );
		InsertEvent( iAt++, pEv );
		tEventTime += tRepeatTime;
	}

	SetChanged( true );
	return true;
}

bool CSequence::InsertEvent( int iAtEvent, CEvent * pEvent )
{
	if(!pEvent)return false;
	if( pEvent->IsWaitTime() )
	{
		InsertAt( iAtEvent, pEvent );
	}
	else
	{
		ESTime CuesTime = 0;
		for( int i=0; i<GetSize(); i++ )
		{
			CEvent * pTempEvent = GetEvent(i);
			if( pTempEvent ) {
				if( pTempEvent->IsWaitTime() )
					CuesTime += pTempEvent->GetTime();
				else
					CuesTime = pTempEvent->GetTime();
				if( CuesTime > pEvent->GetTime())
				{
					InsertAt( i, pEvent );
					SetChanged( true );
					return true;
				}
			}
		}
		InsertAt( i, pEvent );
	}
	SetChanged( true );
	return true;
}

bool CSequence::DeleteEvent( int iID, bool bDeleteIt )
{
	// deletes an event from the list by index.

	bool Res = false;

	if( iID < GetSize() ) {
		if( bDeleteIt )
			delete ElementAt( iID );
		RemoveAt( iID );
		Res = true;
	}

	SetChanged( true );
	return Res;
}

bool CSequence::DeleteEvent( CEvent* pEvent, bool bDeleteIt )
{
	if(!pEvent)return false;

	int iCount = GetSize();
	for( int i = 0; i < iCount; i++ ) {
		if( ElementAt(i) == pEvent ) {
			if( bDeleteIt )
				delete ElementAt(i);
			RemoveAt(i);
			SetChanged( true );
			return true;
		}
	}
	SetChanged( true );
	return false;
}

void CSequence::RemoveAll( void )
{
	int iCount = GetSize();
	for( int i = 0; i < iCount; i++ ) {
		delete ElementAt(i);
	}
	CEventList::RemoveAll();
	SetChanged( true );
}

CActiveSequence::CActiveSequence( CSequence* pSeq, int CueNo )
{
	m_pSequence		= pSeq;
	m_bSuspended	= false;
	m_bLooping		= false;
	if( m_pSequence )
	{
		bool IsNotFirstCue = true;
		if( CueNo == 0 )
			IsNotFirstCue = false;
		if( CueNo == 0 )
			m_pSequence->GetRunStatus()->LoadCue( m_pSequence, CueNo, false );

//		m_pSequence->GetRunStatus()->LoadCue( m_pSequence, CueNo, true );
//		if( CueNo > 0 )
//			m_pSequence->GetRunStatus()->SetRunTime( m_pSequence, m_pSequence->GetRunStatus()->GetRunTime() );
		m_pSequence->SetRunning( true );
	}
}

CActiveSequence::~CActiveSequence()
{
	if( m_pSequence )
		m_pSequence->SetRunning( false );
}

void CActiveSequence::Restart( void )
{
	if( m_pSequence )
	{
		m_pSequence->GetRunStatus()->LoadCue( m_pSequence, 0 );
		m_pSequence->SetRunning( true );
	}
}

void CActiveSequence::Suspend( bool bSuspend )
{
	m_bSuspended = bSuspend;
	m_pSequence->SetSuspended( bSuspend );
}

ESTime CSequence::GetTotalRunTime()
{
	CRunStatus	RunStatus;
	ESTime TotalTime = RunStatus.GetSequenceTotalTime( this );
	return TotalTime;
}

CSequence::ResultCode CSequence::RC_TimeTick( ESTime tTimeLapse, bool Execute )
{
	CEvent * pEv = NULL;
	ResultCode Res = CAS_NoChange;
	if(!m_pDeviceColl) return Res;
	m_RunStatus.RC_AddTime( tTimeLapse );
	pEv = GetEvent( m_RunStatus.GetCurrentCue()+1 );
	if( pEv ) {
		if( pEv->GetTimeType() == ESTIME_TYPE_HALT ) {
			FireDynamicTrigger( pEv->GetDeviceName(), pEv->GetCue().GetAt(0) );
			SetHalted( true );
		}
	}
	while( m_RunStatus.CanExecuteCue( this ) )
	{
		if( Execute && !IsSuspended() && !IsHaltedUntilTrigger() )
		{
			pEv = GetEvent( m_RunStatus.GetCurrentCue() );
			if( pEv && pEv->IsEnabled() )
			{
				switch( pEv->GetTimeType() ){
				case ESTIME_TYPE_HALT:
					FireDynamicTrigger( pEv->GetDeviceName(), pEv->GetCue().GetAt(0) );
					SetHalted( true );
					break;
				case ESTIME_TYPE_HALT_UNTIL:
					FireDynamicTrigger( pEv->GetDeviceName(), pEv->GetCue().GetAt(0) );
					SetHaltedUntilTrigger( true );
					break;
				default:
					FireEvent( pEv->GetDeviceName(), pEv->GetCue() );
					break;
				}
			}
		}
		m_RunStatus.IncrementCurrentCue();
		m_RunStatus.LoadNextCue( this );
		Res = CAS_CurrentMoved;
		pEv = GetEvent( m_RunStatus.GetCurrentCue() );
		if( pEv ) {
			if( pEv->GetTimeType() == ESTIME_TYPE_HALT ) {
				FireDynamicTrigger( pEv->GetDeviceName(), pEv->GetCue().GetAt(0) );
			}
		}
	}
	if( m_RunStatus.GetCurrentCue() >= GetSize() )
	{
		Res = CAS_EndOfList;
		m_RunStatus.Reset();
	}
	return Res;
}

bool CSequence::ExecuteCue( int iCueNo )
{
	if(!m_pDeviceColl) return false;
	CEvent * pEv = GetEvent( iCueNo );
	if( pEv && pEv->IsEnabled() )
	{
		if( pEv->GetTimeType() == ESTIME_TYPE_HALT ||
			pEv->GetTimeType() == ESTIME_TYPE_HALT_UNTIL ) {
			return true;
		} else {
			return FireEvent( pEv->GetDeviceName(), pEv->GetCue() );
		}
	}
	else
	{
		return true;
	}
	return false;
}

bool CSequence::FireEvent( LPCTSTR lpDeviceName, CByteArray& baCue )
{
	if( !*lpDeviceName ) return false;
	if( !baCue.GetSize() ) return false;
	char cType = LEVEL_ERROR;
	CBaseDevice * pDev = NULL;
	CString szError = lpDeviceName;
	if( !m_pDeviceColl->Lookup( lpDeviceName, pDev ) )
		pDev = NULL;
	if( pDev ) 
	{
		if( pDev->IsSendEnabled() ) 
		{
			return pDev->FireEvent( baCue );
		}
		else
		{
			cType = LEVEL_DEVICE_OFFLINE;
			CString szCue;
			pDev->GetCueText( baCue, szCue );
			szError += " Device Trasnmit is Disabled. Ignored Cue: ";
			szError += szCue;
		}
	} 
	else 
	{
		szError += " Device does not exist. Failed to execute cue!";
	}
	CLog::Write( m_pSchedulerDoc->GetDocName(), szError, cType );
	return false;
}

bool CSequence::FireDynamicTrigger( LPCTSTR lpDeviceName, int Trigger )
{
	if( m_pHaltedSequencesTriggerGroup ) {
		LPTRIGGER pT = NULL;
		pT = new CTrigger();
		if( pT ) {
			CByteArray Cue;
			Cue.Add( SchedulerCmdEnableHaltedSequence );
			AppendData( Cue, GetName() );
			pT->m_iTrigger			= Trigger;
			pT->m_szDevice			= lpDeviceName;
			pT->m_szTriggeredDevice	= g_szSchedulerName;
			pT->m_baCue.Copy( Cue );
			m_pHaltedSequencesTriggerGroup->Add( pT );
			m_pSchedulerDoc->UpdateAllViews( NULL, HINT_TRIGGER_LIST_CHANGE );
			return true;
		}
	}
	return false;
}
