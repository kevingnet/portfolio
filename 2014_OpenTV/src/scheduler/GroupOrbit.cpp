/////////////////////////////////////////////////////////////////////////////////
//
// COrbitGroup  Version 1.0
//
// OpenTV
// KevinG
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OrbitGroup.h"
#include "BLLService.h"

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// COrbitGroup
//////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------
COrbitGroup::COrbitGroup(TBreakItemsVector& BIV)
    : CGroup(BIV)
{
	if (CanSchedulePhase1() == false && CanSchedulePhase2() == false)
	{
        //LOG(CAM_WARNING, FormatString("COrbitGroup::COrbitGroup - Unable to schedule - group: %u OrderLine: %u - CanSchedulePhase1 == CanSchedulePhase2 == false, Disabling....", Group(), OrderLine()));
		Disable();
	}
}

//--------------------------------------------------------------------
void COrbitGroup::CalcCanSchedulePhase1() //[ScheduleUnplacedSpots - see above]
{
    m_CanSchedulePhase1 = false;
    //if (m_pBLL->IsDynamic(Time()) == false)
	//{
	//	return;
	//}
    if (AreAllRetailOrbitsActive() == false) //[CheckOrbitsRetailActiveNess]
	{
		return;
	}
	m_CanSchedulePhase1 = true;
    if(HasNotBumped() && m_pBLL->InPhase2() && IsLeadSpotMakeGood())
    {
        if(m_pBLL->HasQuantityAllowed(Time()) == false)
		{
			m_CanSchedulePhase1 = false;
		}
    }
}

//--------------------------------------------------------------------
void COrbitGroup::CalcCanSchedulePhase2() //else if (pOL->m_ucOrbits == 'Y' && SpotGroups[GROUP][0]->ucType.get() == 'N' && It->second->IsSpotMovementEnabled())
{
	m_CanSchedulePhase2 = false;
	/*
	if (m_pBLL->IsDynamic(Time()) && IsLeadSpotNormal() && m_pBLL->IsSpotMovementEnabled())
	{
		m_CanSchedulePhase2 = true;
	}
	*/
	if (IsLeadSpotNormal() && m_pBLL->IsSpotMovementEnabled())
	{
		m_CanSchedulePhase2 = true;
	}
}

//--------------------------------------------------------------------
bool COrbitGroup::AreAllRetailOrbitsActive() //[CheckOrbitsRetailActiveNess]
{
    //when a retail orbits spot is scheduled for a day, we need to check to see the headnets are still active
    //since for retail line we do not enforce all the headnets need to be active for the whole date range
    if(m_pBLL->NotBillingModeRetail())
        return true;
	return AreBreakItemsActive();
}

//--------------------------------------------------------------------
int COrbitGroup::OnBumped()
{
    int res = CGroup::OnBumped();
    //////if ( res == GroupPartial) //we don't allow partial bumps for orbits
    //////    res = GroupDisable;
    //TODO: instead of disabling we could just set the items right, check after testing...
    return res;
}

//--------------------------------------------------------------------
int COrbitGroup::OnSchedule()
{
    m_pBLL = CBLLService::GetPtr(OrderLine());
    if (m_pBLL == 0)
	{
        //LOG(CAM_WARNING, FormatString("COrbitGroup::OnSchedule - Unable get cache - group: %u OrderLine: %u - Invalid BLL cache object, Disabling....", Group(), OrderLine()));
		Disable();
		return GroupDisable;
	}

	int res = GroupFailed;
	if (CanMoveAccrossNetworks == false) //phase1, schedule as non-orbit,
	{
		//orbit in phase1, we have already doe the check, otherwise this function should not be called
		//------------------------------------------------------------

		if (m_CanSchedulePhase1 == true)
		{
			CBreakItemGroupPtr pBIG = new CBreakItemGroup(GetLead(), true);
			if (CBreakItemGroupInit(pBIG) == false)
			{
				res = GroupFailed;
				m_CanSchedulePhase1 = false; //don't even try to retry....
			}
			else
			{
				int PreviousScheduled = m_Scheduled.size();
				//int PreviousScheduled = m_ScheduledCount;
				bool result = CBreakItemGroupSchedule(pBIG);
				if (result == true)
				{
					#ifdef _DEBUG
					LOG(CAM_TRACE, FormatString("COrbitGroup CGroup:::OnSchedule - Scheduled - group: %u OrderLine: %u", Group(), OrderLine()));
					#endif
				}
				else
				{
					LOG(CAM_WARNING, FormatString("COrbitGroup CGroup:::OnSchedule - Unable to schedule - group: %u OrderLine: %u", Group(), OrderLine()));
					res = GroupFailed;
				}
				res = ProcessLastScheduleOperationResult(PreviousScheduled);
			}
			delete pBIG;
		}
		return res;
	}

	res = GroupDisable;
    //this will upate the data ONLY if it changed...

	//we're replacing whatever may be scheduled, so we need to delete those lines
	DiscardChanges();
	ReturnAll(); //return newly added to pool, and we'll leave the original items in place

	res = GroupFailed;
	CGroupOrbitBuilder * pGroupBuilder = new CGroupOrbitBuilder(at(0), OrderLine(), at(0)->ulHeadnetID.get(), Time());
	if (pGroupBuilder == 0)
		return GroupDisable;

	CBreakItemGroupPtr pBIG = 0;
    while(1)
    {
        int count = pGroupBuilder->Next();
        if (count <= 0) //could not create group to schedule,
        {
//#ifdef _DEBUG
//            LOG(CAM_TRACE, FormatString("COrbitGroup::OnSchedule - Next could not create group - group: %u OrderLine: %u", Group(), OrderLine()));
//#endif
            //finish up...events will handle if line should exhaust...
            break;
        }

		//we know how many to create, there are four types of records,
		//but for our purposes, we'll consider only 3,
		//  a) DB (Clean and Dirty), these are the main ones we deal with
		//  b) ToDelete, first to go from the pool
		//  c) ToAdd, the pool might need to create New records here
		//in this function, we might need to return some ToAdd to the pool,
		//  or place some ToDelete, if the group will be smaller
		//at this point, we're not going to be updating our main vector container,
		//for obvious reasons, we just use another container
		//to keep track of what's getting or not getting added, again,
		//nothing to do with the ToDelete items except to change them to Dirty
		//now we have a full group, we need to initialize the data,
		//data has already been initialized with data from the lead,
		//and cleared relevant fields
		//however, we still need to assing the headnet data

		Resize(count);

		pBIG = new CBreakItemGroup(GetLead(), true);

		if (CBreakItemGroupInit(pBIG) == false)
		{
			delete pBIG;
			pBIG = 0;
			continue;
		}
        pGroupBuilder->Fill(*this, pBIG);
		//if (m_pBLL->IsDynamic(Time()) && IsLeadSpotNormal() && m_pBLL->IsSpotMovementEnabled())
		//{
		//	m_CanSchedulePhase2 = true;
		//}

		Dump("New Orbit OnSchedule");
        if (CBreakItemGroupCScheduleGroup(pBIG))
        {
			m_pBLL->IncrementPlacedQuantity(pBIG);
            CCrossedNetworks::GetRef().Add(OrderLine());
#ifdef _DEBUG
            LOG(CAM_TRACE, FormatString("COrbitGroup::OnSchedule - Scheduled - group: %u OrderLine: %u", Group(), OrderLine()));
#endif
			res = GroupScheduled;
/*
			if (m_Scheduled.size() > 0)
			{
				CBreakItemDeletions& bid = CBreakItemDeletions::GetRef();
				for (IteratorTBreakItemsVector it = m_Scheduled.begin(); it != m_Scheduled.end(); ++it)
				{
					CBreakItemPtr pBI = (*it);
					ASSERT(pBI);
					bid.push_back(pBI);
				}
		#ifdef _DEBUG
				LOG(CAM_WARNING, FormatString("COrbitGroup::OnSchedule - CGroup was partially scheduled! Marked %i scheduled break items for deletion for group: %u OrderLine: %u", m_Scheduled.size(), Group(), OrderLine()));
		#endif
				m_Scheduled.clear();
			}*/
			break;
        }
    } //end while loop
	delete pGroupBuilder;

	if (res != GroupScheduled)
	{
		if(m_pBLL->InPhase2())
		{
			m_pBLL->RemoveDayScheduleTimeAndCorrectBreakItemTime(pBIG);
		}
		delete pBIG;
		pBIG = 0;

		//it failed, so let's go ahead and return newly created items, they have not been saved
		if (m_pBLL->NotBillingModeRetail())
		{
			LOG(CAM_WARNING, FormatString("COrbitGroup::OnSchedule - Unable to schedule - group: %u OrderLine: %u - NotBillingModeRetail, Disabling....", Group(), OrderLine()));
			return GroupDisable;
		}
		else
		{
			LOG(CAM_WARNING, FormatString("COrbitGroup::OnSchedule - Unable to schedule - group: %u OrderLine: %u", Group(), OrderLine()));
			//TODO: what do we do here?
			return GroupFailed;
			return GroupDisable;
		}
	}
	else
	{
		delete pBIG;
		pBIG = 0;
	}
    //return res;
	//at this point the group is rebuilt and it should only scheduled items,
	//but, we want to check, because it would be better to schedule some than to schedule none? TODO: check this...
    return ProcessLastScheduleOperationResult(0);
}

//--------------------------------------------------------------------
bool COrbitGroup::HeadNetInactive(CBreakItemPtr pBI) const { return IsHeadNetActive(pBI->ulHeadnetID.get(), pBI->ulTimedate.get()) == FALSE; }

//--------------------------------------------------------------------

//--------------------------------------------------------------------
void CGroupOrbitBuilder::Fill(TBreakItemsVector& BIV, CBreakItemGroupPtr pBIG)
{
    SBreakItemBuffer Copy;
    Copy.SetBufferData(BIV.at(0));

    int idx = 0;
    int group_size = m_NextGroupCount;
    for (IteratorTBreakItemsVector it = BIV.begin(); it != BIV.end(); ++it)
    {
        CBreakItemPtr pBI = (*it);
        ASSERT(pBI);
		Copy.SetBreakItemData(pBI);
        m_OrbitGroupHeadnets[idx++].Assign(pBI); //copy New headnet data
        group_size--;
        if (group_size == 0)
            break;
    }
}

//--------------------------------------------------------------------
CGroupOrbitBuilder::CGroupOrbitBuilder(CBreakItemPtr pBILead, ULONG p_ulOrderlineVID, ULONG p_ulHeadnetID, CTime p_CTime) //[ScheduleUnplacedSpots - early part, getting cache info, day scheduled, etc...]
    : m_LeadTime(p_CTime)
	, m_ulOrderlineVID(p_ulOrderlineVID)
	, m_ulHeadnetIDLead(p_ulHeadnetID)
	, m_ulNetworkID(0)
{
	m_ulNetworkID = GetNetworkFromHeadnet(m_ulHeadnetIDLead);

	GetScheduleCache()->GetScheduleGroupHeadNets(m_ulOrderlineVID, m_ulHeadnetIDLead, m_LeadTime, m_HeadNetGroup);

    CSchOrderLine& OLCache = ::GetScheduleCache()->GetOrderLineCache(m_ulOrderlineVID);
    CRNArray& RNArray = OLCache.GetRNArrayReload(m_LeadTime);
    m_pRNArray = &RNArray;
    OLCache.ReloadNetworks();
    m_Networks = OLCache.GetNetworkList();

    IteratorTULONGVector it = find(m_Networks.begin(), m_Networks.end(), m_ulNetworkID);
    ++it;
    if (it != m_Networks.end())
        rotate(m_Networks.begin(), it, m_Networks.end()); //this to be the last network, where we stop, or... remove
    m_Networks.pop_back(); //this way when we iterate we don't have to check
}

//--------------------------------------------------------------------
int CGroupOrbitBuilder::Next() //[CreateBreakItemsForNetwork]
{
    for(IteratorTULONGVector it = m_Networks.begin(); it != m_Networks.end(); ++it)
    {
        int rnCount = CalcRetailHeadNetsCount(*it);
        if (rnCount)
        {
            m_NextGroupCount = CalcGroupHeadnets(rnCount);
            if (m_NextGroupCount == 0)
			{
				m_Networks.erase(it++);
                continue;
			}
            //we have now a set of headnets to create the New group to try
			m_Networks.erase(it++);
            return m_NextGroupCount;
        }
    }
    return 0;
}

//--------------------------------------------------------------------
int CGroupOrbitBuilder::CalcGroupHeadnets(int p_Count) //[CreateBreakItemsForNetwork]
{
    m_OrbitGroupHeadnets.clear();
    if (m_OrbitGroupHeadnets.capacity() < p_Count)
        m_OrbitGroupHeadnets.reserve(p_Count);

    OrderOptions options;
    options.SetRetailUnitLead();
    ULONG flagLead = options.GetULONG();
    options.ReSetRetailUnitLead();
    ULONG flag = options.GetULONG();

    CBLLServicePtr pBLL = CBLLService::GetPtr(m_ulOrderlineVID);
    if (pBLL == 0)
		return 0;

	m_ulHeadnetIDLead = 0;
    for (int i = 0; i<m_RetailNets.size(); i++)
    {
        CRetailNet * pRN = m_RetailNets.at(i);
        ASSERT(pRN);
        double dRetailunitrate = pRN->GetRetailUnitRate();
        ULONG ulRetailUnitID = pRN->m_ulRetailUnitID;

        for (int j = 0; j< pRN->m_HNL.GetSize(); j++)
        {
            ULONG ulHeadnetID = pRN->m_HNL[j].m_HeadNetID;
            ASSERT(ulHeadnetID);
            if(pBLL->IsOrderLineUniformRegion())
            {
                m_OrbitGroupHeadnets.push_back(SOrbitGroupHeadnets(ulHeadnetID, ulRetailUnitID, flag, dRetailunitrate));
            }
            else
            {
                if(m_ulHeadnetIDLead == 0 && pRN->GetLeadHeadEndID() == GetHeadEndID_fromHN(ulHeadnetID))
                {
                    m_ulHeadnetIDLead = ulHeadnetID;
                    m_OrbitGroupHeadnets.insert(m_OrbitGroupHeadnets.begin(), SOrbitGroupHeadnets(m_ulHeadnetIDLead, ulRetailUnitID, flagLead, dRetailunitrate));
                }
                else
                {
                    m_OrbitGroupHeadnets.push_back(SOrbitGroupHeadnets(ulHeadnetID, ulRetailUnitID, flag, dRetailunitrate));
                }
            }
        }
    }
    return m_OrbitGroupHeadnets.size();
}

//--------------------------------------------------------------------
int CGroupOrbitBuilder::CalcRetailHeadNetsCount(int p_ulNetworkIDToAttempt) //[CreateBreakItemsForNetwork]
{
    if (p_ulNetworkIDToAttempt == 0)
        return 0;
    CBLLServicePtr pBLL = CBLLService::GetPtr(m_ulOrderlineVID);
    if (pBLL == 0)
		return 0;

	//CRNArray& RNArray = OLCache.GetRNArray(m_LeadTime);

    m_RetailNets.clear();
    if (m_RetailNets.capacity() < m_pRNArray->size())
        m_RetailNets.reserve(m_pRNArray->size());

    int i;
    for (i = 0; i<m_pRNArray->size(); i++)
    {
        CRetailNet * pRN = m_pRNArray->GetAt(i);
        ASSERT(pRN);
        if (pRN && pRN->m_ulNetworkID == p_ulNetworkIDToAttempt)
            m_RetailNets.push_back(pRN);
    }

    int count = 0;
    for (i = 0; i<m_RetailNets.size(); i++)
    {
        CRetailNet * pRN = m_RetailNets.at(i);
        ASSERT(pRN);
        for (int j = 0; j< pRN->m_HNL.GetSize(); j++)
        {
            if(pBLL->IsOrderLineUniformRegion())
            {
                count++;
				m_RetailNets.push_back(pRN);
            }
            else
            {
                ULONG HeadNetID = pRN->m_HNL[j].m_HeadNetID;
                if(IsHeadNetActive(HeadNetID, m_LeadTime) == TRUE)
                {
                    count++;
					m_RetailNets.push_back(pRN);
                }
                else
                {
                    if(pRN->GetLeadHeadEndID() == GetHeadEndID_fromHN(HeadNetID))
                    {
						m_RetailNets.clear();
                        return 0;
                    }
                    count++;
					m_RetailNets.push_back(pRN);
                }
            }
        }
    }
    return count;
}

//--------------------------------------------------------------------
ULONG CGroupOrbitBuilder::GetNetworkFromHeadnet(ULONG p_HeadnetID) const //[CreateBreakItemsForNetwork]
{
    if (p_HeadnetID == 0)
        return 0;
    ConstIteratorTULONGMap it = m_HeadNetToNetwork.find(p_HeadnetID);
    if (it != m_HeadNetToNetwork.end())
        return it->second;
    ULONG network = GetNetworkID_fromHN(p_HeadnetID);
    m_HeadNetToNetwork[p_HeadnetID] = network;
    return network;
}

