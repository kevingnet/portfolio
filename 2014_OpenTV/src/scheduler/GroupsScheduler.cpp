/////////////////////////////////////////////////////////////////////////////////
//
// GroupsScheduler  Version 1.0
//
// OpenTV
// KevinG
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GroupsScheduler.h"
#include <UpdateStatus.h>

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// CGroupsSetScheduler
//////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------
CGroupsSetScheduler::CGroupsSetScheduler
    (MODE               p_mode,
    DayScheduledMap *   p_pDayScheduledMap,
    CUnplacedList&      p_upl,
    CUnplacedList&      p_add,
    CUnplacedList&      p_del,
    CUnplacedList&      p_dup,
    ULONGSet&           p_crossed)
        : m_UnplacedList    (p_upl)
        , m_AddList         (p_add)
        , m_DeleteList      (p_del)
        , m_DuplicatesList  (p_dup)
{
    try
    {
        InitStatus("Initializing Scheduling Core2", 6);

        CBLLService::SetMode(p_mode); StepProgress();
        CBLLService::SetDayScheduledMap(p_pDayScheduledMap); StepProgress();

        BreakItemAdditions::GetRef().SetCUnplacedList(&m_AddList); StepProgress();
        BreakItemDeletions::GetRef().SetCUnplacedList(&m_DeleteList); StepProgress();
        BreakItemDuplicates::GetRef().SetCUnplacedList(&m_DuplicatesList); StepProgress();
        CCrossedNetworks::GetRef().SetCrossedNetworks(&crossed); StepProgress();
    }
    EXCEPTION_HANDLER("CGroupsSetScheduler::CGroupsSetScheduler")
}

//--------------------------------------------------------------------
CGroupsSetScheduler::~CGroupsSetScheduler()
{
    Destroy();
}

//--------------------------------------------------------------------
void CGroupsSetScheduler::Destroy()
{
    InitStatus("Cleaning up CGroupsSetScheduler Core2", 5);

	//these are only wrappers around the actual data
    CCrossedNetworks::Destroy(); StepProgress();
	//this should cause the 'unused' New records to be discarded
    BreakItemAdditions::Destroy(); StepProgress();
    BreakItemDeletions::Destroy(); StepProgress();
    BreakItemDuplicates::Destroy(); StepProgress();
	//destroy actual cache in CBLLService but not actual app cache
    CBLLService::Destroy(); StepProgress();
    //at this point we've released memory, placed break items in the right containers, etc...
}

#define DEFINE_PROGRESS int count = 0; int inc = 0; int skip = 0; bool step = true;
#define INIT_PROGRESS( TXT ) count = m_Queue.size(); step = true; skip = 0; if (count < 3) { step = false; InitStatus(TXT); } else { InitStatus(TXT, count); }
#define STEP_PROGRESS if (step) { if (inc) { skip += inc; } if (skip == 0) StepProgress(); else skip--; }

#define CANMOVEACROSSNETWORKS true
#define PHASE1 false
#define PHASE2 true
void CGroupsSetScheduler::Run()
{
    try
    {
        //Change of tactics, groups won't 'move' they simply change the state...easier to manage..
        int groupCount = BuildGroups(); //we should have no duplicates at this point (dupes are checked while group is building...

		if (groupCount < 1)
		{
			LOG(CAM_WARNING, FormatString("CGroupsSetScheduler::Run - No groups to schedule!"));
			return;
		}

        CGroupSet orbits;
        DEFINE_PROGRESS

        m_Queue += m_GroupsPhase1; //[ScheduleUnplacedSpots - main loop]
        INIT_PROGRESS( "Scheduling groups, ScheduleUnplacedSpots loop" )
        while (!m_Queue.empty())
        {
            CGroup* pg = m_Queue.poptop();
            pg->Schedule(); //groups: non-orbits, orbits with all retail headnets active
            inc = QueueBumpedGroups(PHASE1);
            STEP_PROGRESS
        }
        m_Queue += m_GroupsPhase2; //[MoveSpotsAcrossNetworks - main loop]
        INIT_PROGRESS( "Scheduling groups, MoveSpotsAcrossNetworks loop" )
        while (!m_Queue.empty())
        {
            CGroup* pg = m_Queue.poptop();
            pg->Schedule(CANMOVEACROSSNETWORKS); //groups: non-orbits, rotating networks orbits
            inc = QueueBumpedGroups(PHASE2);
            STEP_PROGRESS
        }
    }
    EXCEPTION_HANDLER("CGroupsSetScheduler::Run()")
}

//--------------------------------------------------------------------
int CGroupsSetScheduler::QueueBumpedGroups() //[ScheduleBreakItemGroup - CommitUnplacedStack]
{
    try
    {
        CUnplacedList upl;
        CommitUnplacedStack(upl);
        BreakItemsVector bumpedbreakitems;
        bumpedbreakitems.reserve(upl.GetCount());
        LoadBreakItems(bumpedbreakitems, upl, 0);
        sort(bumpedbreakitems.begin(), bumpedbreakitems.end(), ltbi());

        int bumpedgroups = 0;
		TGroupSet* pgs = 0;
		if (IsPhase1)
		{
			//this will change the group state and its ability to be processed in the queue
			bumpedgroups = SignalGroupsBump(m_GroupsPhase1, bumpedbreakitems);
			pgs = &m_GroupsPhase1.GetBumpedGroups();
		}
		else
		{
			bumpedgroups = SignalGroupsBump(m_GroupsPhase2, bumpedbreakitems);
			pgs = &m_GroupsPhase2.GetBumpedGroups();
		}
		for (GroupSetIterator it = pgs->begin(); it != pgs->end(); ++it)
		{
            CGroup * pg = *it;
            ASSERT(pg);
            if (pg->IsDisabled())
                continue;
			if (IsPhase1)
			{
				if (pg->IsOrbit())
					m_GroupsPhase2 += pg;
				else
					m_Queue += pg;
			}
			else
			{
				if (pg->CanSchedulePhase2())
					m_Queue += pg;
			}
		}

        return bumpedgroups;
    }
    EXCEPTION_HANDLER("CGroupsSetScheduler::QueueBumpedGroups()")
    return 0;
}

//--------------------------------------------------------------------
int CGroupsSetScheduler::BuildGroups() //[ScheduleUnplacedSpots - main loop, builds groups] NOTE: it builds from all placed and unplaced, so items don't have to 'move'
{
    BreakItemsVector allbreakitems;
    allbreakitems.reserve(m_UnplacedList.GetCount());
    LoadBreakItems(allbreakitems, m_UnplacedList, "Loading unplaced break items");
    //CUnplacedList PlacedList;
    //CBLLService::GetScheduled(PlacedList);
    //LoadBreakItems(allbreakitems, PlacedList, "Loading scheduled break items");
    sort(allbreakitems.begin(), allbreakitems.end(), ltbi());
    return CreateGroups(m_GroupsPhase1, allbreakitems);
}

//--------------------------------------------------------------------
int CGroupsSetScheduler::CreateGroups(CGroupSet& p_GroupSet, BreakItemsVector& BIV)
{
    try
    {
        int count = BIV.size();
		if (count == 0)
			return 0;
        InitStatus("Buidling groups", count);
        CBreakItem * pbi = 0;
        p_GroupSet.ResetBuffers();
        for (IteratorBreakItemsVector it = BIV.begin(); it != BIV.end(); ++it)
        {
            pbi = (*it);
            ASSERT(pbi);
			if (pbi)
				p_GroupSet.GroupsFactoryAddItem(pbi);
            StepProgress();
        }
		it = BIV.begin();
        pbi = *it;
		if (pbi)
		{
			p_GroupSet.GroupsFactoryAddItem(pbi); //let's fool the loop, this is so the last group will load...
		}
		count = p_GroupSet.Count();
#ifdef _DEBUG
		p_GroupSet.Dump();
#endif
		p_GroupSet.ResetBuffers();
        return count;
    }
    EXCEPTION_HANDLER("CGroupsSetScheduler::CreateGroups()")
    return 0;
}

//--------------------------------------------------------------------
int CGroupsSetScheduler::SignalGroupsBump(CGroupSet& p_GroupSet, BreakItemsVector& BIV)
{
    try
    {
        p_GroupSet.ResetBuffers();
        int count = BIV.size();
        CBreakItem * pbi = 0;
        for (IteratorBreakItemsVector it = BIV.begin(); it != BIV.end(); ++it)
        {
            pbi = (*it);
            ASSERT(pbi);
			if (pbi)
	            p_GroupSet.GroupsBumpedAddItem(pbi);
        }
		it = BIV.begin();
        pbi = *it;
		if (pbi)
		{
			p_GroupSet.GroupsBumpedAddItem(pbi); //let's fool the loop, this is so the last group will load...
		}
		count = p_GroupSet.Count();
		p_GroupSet.ResetBuffers();
        return count;
    }
    EXCEPTION_HANDLER("CGroupsSetScheduler::SignalGroupsBump()")
    return 0;
}

//--------------------------------------------------------------------
void CGroupsSetScheduler::LoadBreakItems(BreakItemsVector& BIV, CUnplacedList& pUPL, char * p_pMessage)
{
   try
    {
        int count = pUPL.GetCount();
        bool step = true;
        if (p_pMessage == 0)
        {
            step = false;
        }
        else
        {
            if (count < 10)
            {
                step = false;
                InitStatus(p_pMessage);
            }
            else
            {
                InitStatus(p_pMessage, count);
            }
        }
        int idx = 0;
        while (idx < count)
        {
            CBreakItem * pbi = pUPL[idx++];
            ASSERT(pbi);
            BIV.push_back(pbi);
            if (step)
                StepProgress();
        }
    }
    EXCEPTION_HANDLER("CGroupsSetScheduler::LoadBreakItems()")
}

