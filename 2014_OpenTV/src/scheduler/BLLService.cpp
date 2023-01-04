/////////////////////////////////////////////////////////////////////////////////
//
// CBLLService  Version 1.0
//
// OpenTV
// KevinG
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <DayUsed.h>
#include "..\ScheduleParams.h"
#include "..\DayScheduled.h"
#include "..\ScheduleGroup.h"
#include "..\ScheduleCache.h"

#include "BLLService.h"

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// static
//////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------
TDayScheduledMap*   CBLLService::m_pTDayScheduledMap    = 0;
MODE                CBLLService::m_Mode                 = NONE;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// CBLLService
//////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------
CBLLService::CBLLService(ULONG p_ulOrderlineVID) //[ScheduleUnplacedSpots - early part, getting cache info, day scheduled, etc...]
    : m_pDayScheduled(0)
    , m_pOrderLine(0)
    , m_pCompany(0)
    , m_ulOrderlineVID(p_ulOrderlineVID)
{
    m_pOrderLine = GetScheduleCache()->GetSC_OrderLine(m_ulOrderlineVID);
    if (m_pOrderLine == 0)
    {
        CSchOrderLine & OLCache = GetScheduleCache()->GetOrderLineCache(m_ulOrderlineVID);
        m_pOrderLine = OLCache.GetOrderLine();
        if (m_pOrderLine == 0)
            LOG(CAM_WARNING, FormatString("CBLLService::CBLLService - Unable to get cache for OrderLine: %u", m_ulOrderlineVID));
    }

    m_pCompany = ::GetScheduleCache()->GetCompanyEntityFromOrderLineID(m_ulOrderlineVID);
    if (!m_pCompany)
    {
        ULONG CompanyEntityID = GetScheduleCache()->GetCompanyEntityID(m_pOrderLine->m_ulOrdernumber);
        if (CompanyEntityID)
            m_pCompany = GetSystemSetup()->GetCompanyEntity(CompanyEntityID);
    }

    if (m_pTDayScheduledMap)
    {
        IteratorTDayScheduledMap it = m_pTDayScheduledMap->find(m_ulOrderlineVID);
        if (it != m_pTDayScheduledMap->end())
            m_pDayScheduled = it->second;
        if (m_pDayScheduled == 0)
        	LOG(CAM_WARNING, FormatString("CBLLService::CBLLService - Unable to get day scheduled info for OrderLine: %u", m_ulOrderlineVID));
    }
}

//--------------------------------------------------------------------
void CBLLService::GetScheduled(CUnplacedList& p_UPL)
{
    //this gets us the huge list of inventory break items, not really used anymore...
    ::GetScheduleCache()->GetBreakItems(p_UPL);
}

//--------------------------------------------------------------------
int CBLLService::GetScheduledULONG p_ulOrderlineVID, ULONG p_ulGroupID, vector<CBreakItemPtr> * pBIV)
{
    //this is used during Schedule() and Bump() so that we may track break items as a whole
    return ::GetScheduleCache()->ExistsBreakItemGroup(p_ulOrderlineVID, p_ulGroupID, pBIV);
}

//--------------------------------------------------------------------
void CBLLService::BumpGroup(ULONG p_ulOrderlineVID, const p_ulGroupID)
{
    ::GetScheduleCache()->UnplaceGroup(p_ulOrderlineVID, p_ulGroupID); //ok, we don't wanna use this thing...
}

//--------------------------------------------------------------------
bool CBLLService::IsDynamic(CTime p_CTime) const
{
    if(IsDynamic() || IsSameDate(p_CTime))
    {
#ifdef _DEBUG
    //  LOG(CAM_TRACE, FormatString("CBLLService::IsDynamic - FALSE - ulOrderlineVID:%u", m_ulOrderlineVID));
#endif
        return false;
    }
    return true;
}

//--------------------------------------------------------------------
int CBLLService::Priority() const
{
    //this is calculated so that during group construction the priority queue can schedule the highest priority
    //items first thereby avoiding bumping, however, it was brought to my attention that doing this would
    //cause the highest priority items to always be scheduled leaving the bottom feeders no chance of ever
    //coming to light. Also there is supposed to be only one bump per group per pass. One thing to consider is
    //that the higher priority items are set to bump lower priority ones, so without a priority sort we may see
    //too much bumping
    //TODO: we might have to test these hypotheses
    FALSE_IF_INVALID(m_pOrderLine);
    //calc priority, use customer, order, order line and break item, also it should use the rate
    // maybe it needs to calculate the whole group rate, the added benefit of this would be that there
    // should be a lot less bumping going on, no pun intended...
    int priority = m_pOrderLine->m_ulPriority;
    CUSTOMERINFO * pCustInfo = GetScheduleCache()->GetCustomerCache().GetCustomerInfo(m_pOrderLine->m_ulCustomerID);
    if (pCustInfo)
        priority += pCustInfo->m_ulDefaultPriority;
    //the group rate is also added later at the time of schedule group building, since the group items may vary
    return priority;
}

//--------------------------------------------------------------------
bool CBLLService::HasQuantityAllowed(CTime p_CTime) const
{
    //this is part of: (from the first main loop)
    /*
                    bool IsBumpedSpot = i > UnplaceListOrgSize? true: false;
                    if(!IsBumpedSpot && It->second->InPhase2() && pOL->m_ucOrbits == 'Y' && SpotGroups[GROUP][0]->ucType.get() == 'M')
                    {
                        int nSpotsAllowed = It->second->GetQtyAllowed(SpotGroups[GROUP][0]->ulTimedate.get(), 0, 0);

                        if(nSpotsAllowed <=0)
                        {
                            CanPlaceMore = false;
                        }
                    }

     */
    //to determine if an orbit group can be scheduled in the first phase, part of the first check, later all headnet activeness is checked
    FALSE_IF_INVALID(m_pDayScheduled);
    if(m_pDayScheduled->GetQtyAllowed(p_CTime, 0, 0) > 0)
    {
        return true;
    }
#ifdef _DEBUG
    LOG(CAM_TRACE, FormatString("CBLLService::HasQuantityAllowed FALSE - ulOrderlineVID:%u", m_ulOrderlineVID));
#endif
    return false;
}

//--------------------------------------------------------------------
bool CBLLService::EnableInventoryOverrides() const
{
    if (CanOverride() == false)
        return false;
    // look up for the particular company entity...
    FALSE_IF_INVALID(m_pCompany);
    return m_pCompany->EnableInventoryOverrides();
}

//--------------------------------------------------------------------
bool CBLLService::EnforceUniformRegion(UCHAR p_SpotType, bool p_IsMovePhase) const
{
    if (p_IsMovePhase)
    {
        TRUE_IF_INVALID(m_pOrderLine);
            // It's better to assume it's uniform region.
        if (IsBillingModeThreshold())
            return true;
    }
    if (p_SpotType=='B')
        return IsCompanyEnforceUniformRegionForBonus();
    else
        return IsCompanyEnforceUniformRegion();
}

//--------------------------------------------------------------------
bool CBLLService::IsSameDate(CTime p_CTime) const
{
    CTime CurrentProcessingDate = 0;
    try
    {
        IScheduleStatus * stat = ::GetScheduleCache()->STAT();
        if (stat != 0)
        {
            CurrentProcessingDate = CTime(stat->GetWeekStart()) +
                                    CTimeSpan(stat->GetProcessingDay(),0,0,0);
        }
    }
    catch (...)
    {
    }
    BOOL Result = (TruncDay(p_CTime) == TruncDay(CurrentProcessingDate));
    if (Result == FALSE)
    {
        CString dt = p_CTime.Format(_T("%y.%m.%d %H:%M:%S"));
        CString cur = CurrentProcessingDate.Format(_T("%y.%m.%d %H:%M:%S"));
#ifdef _DEBUG
        LOG(CAM_TRACE, FormatString("CBLLService::IsSameDate - FALSE - test date:%s cur date:%s - ulOrderlineVID:%u", dt, cur, m_ulOrderlineVID));
#endif
    }
    return Result == TRUE;
}

//--------------------------------------------------------------------
void CBLLService::IncrementPlacedQuantity(CBreakItemGroupPtr pBIG) //[IncrementPlacedQuantity]
{
    RETURN_IF_INVALID( m_pDayScheduled );
    ASSERT( pBIG );
    CBreakItemPtr pBILead = pBIG->at(0);
    ASSERT( pBILead );
    CTime Time = pBILead->ulTimedate.get();

    //if this is a retail group or if this is an orbit group, increment per group
    if (NotOrderLineUniformRegion() || ScheduledAsOrbit())
    {
        m_pDayScheduled->IncrementPlacedQty(Time,
                                pBILead->ulRetailunitID,
                                GetNetworkID_fromHN(pBILead->ulHeadnetID),
                                //GetNetworkFromHeadnet(pBILead->ulHeadnetID),
                                true // it's newly placed in this pass
                                );

    }
    else
    {
        ULONG ulPrevRetailUnitID = pBILead->ulRetailunitID;
        ULONG ulPrevNetworkID = GetNetworkID_fromHN(pBILead->ulHeadnetID);
        if (ulPrevRetailUnitID && ulPrevNetworkID)
        {
            m_pDayScheduled->IncrementPlacedQty(Time,
                                    pBILead->ulRetailunitID,
                                    GetNetworkID_fromHN(pBILead->ulHeadnetID),
                                    true // it's newly placed in this pass
                                    );

            for (int k=1; k<pBIG->GroupSize(); k++)
            {
                if (ulPrevRetailUnitID != pBIG->at(k)->ulRetailunitID ||
                    ulPrevNetworkID != GetNetworkID_fromHN(pBIG->at(k)->ulHeadnetID))
                {
                    if (pBIG->at(k) == 0 || pBIG->at(k)->ulRetailunitID == 0 || pBIG->at(k)->ulHeadnetID == 0)
                        continue;
                    m_pDayScheduled->IncrementPlacedQty(Time,
                                        pBIG->at(k)->ulRetailunitID,
                                        GetNetworkID_fromHN(pBIG->at(k)->ulHeadnetID),
                                        true // it's newly placed in this pass
                                        );
                    ulPrevRetailUnitID = pBIG->at(k)->ulRetailunitID;
                    ulPrevNetworkID = GetNetworkID_fromHN(pBIG->at(k)->ulHeadnetID);
                }
            }
        }
    }
}

//--------------------------------------------------------------------
void CBLLService::RemoveDayScheduleTimeAndCorrectBreakItemTime(CBreakItemGroupPtr pBIG) const
{
    RETURN_IF_INVALID( m_pDayScheduled );
    ASSERT( pBIG );
    try
    {
        bool lastGroup = false;
        if(InPhase2())
        {
            for (int i = 0; i<pBIG->size(); i++)
            {
                if (pBIG->at(i) == 0)
                    continue;
                CBreakItemPtr pBI = pBIG->at(i);
                ULONG ulId = pBI->ulId.get();
                //CTime Time = 0;
                CTime Time = m_pDayScheduled->FindBITime(ulId);
                // if Time == 0, it's not found.  This means it's not moved from
                //    a different day, so ulTimedate is never changed.  One scenario
                //    when this happens is the break item is bumped out of the schedule
                //    and scheduler is unable Find an appropriate break for it.
                if (Time > 0)
                {
                    pBI->ulTimedate.SetOnChange(Time);
                    pBI->ulExceptionVID.SetOnChange(0);
                    // Each break item in the group has its chance to be the lead.
                    //    So, if a group has three breakitems, ScheduleBreakItemGroup
                    //    will be called three times
    //TODO: what's the deal here with lastGroup, how to handle this best?
                    if (lastGroup)
                        m_pDayScheduled->RemoveBITime(ulId);
                }
            }
        }
    }
    EXCEPTION_HANDLER("CBLLService::RemoveDayScheduleTimeAndCorrectBreakItemTime()")
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

MODE CBLLService::GetMode() { return m_Mode; }
void CBLLService::SetMode(MODE p_Mode) { m_Mode = p_Mode; }
void CBLLService::SetTDayScheduledMap(TDayScheduledMap* p_pTDayScheduledMap) { m_pTDayScheduledMap = p_pTDayScheduledMap; }

bool CBLLService::Valid() const { return (m_pOrderLine != 0); }

bool CBLLService::CanOverride() const { return ::GetScheduleCache()->CanOverride(); }
bool CBLLService::IsDynamic() const { return ::GetScheduleCache()->GetEDSMode() == DYNAMIC_MODE; }

bool CBLLService::InPhase2() const { FALSE_IF_INVALID(m_pDayScheduled); return m_pDayScheduled->InPhase2(); }
bool CBLLService::ScheduledAsOrbit() const { FALSE_IF_INVALID(m_pDayScheduled); return m_pDayScheduled->IsOrbits(); }
bool CBLLService::IsSpotMovementEnabled() const { FALSE_IF_INVALID(m_pDayScheduled); return m_pDayScheduled->IsSpotMovementEnabled(); }

bool CBLLService::IsOrbit() const { FALSE_IF_INVALID(m_pOrderLine); return m_pOrderLine->m_ucOrbits == 'Y'; }
bool CBLLService::NonOrbit() const { FALSE_IF_INVALID(m_pOrderLine); return m_pOrderLine->m_ucOrbits != 'Y'; }
bool CBLLService::IsBillingModeRetail() const { FALSE_IF_INVALID(m_pOrderLine); return m_pOrderLine->m_ucBillingmode == 'R'; }
bool CBLLService::NotBillingModeRetail() const { FALSE_IF_INVALID(m_pOrderLine); return m_pOrderLine->m_ucBillingmode != 'R'; }
bool CBLLService::IsBillingModeThreshold() const { FALSE_IF_INVALID(m_pOrderLine); return m_pOrderLine->m_ucBillingmode == 'T'; }
bool CBLLService::IsBillingModeFractional() const { FALSE_IF_INVALID(m_pOrderLine); return m_pOrderLine->m_ucBillingmode == 'F'; }
bool CBLLService::IsOrderLineUniformRegion() const { FALSE_IF_INVALID(m_pOrderLine); return m_pOrderLine->m_ucUniformregion == 'Y'; }
bool CBLLService::NotOrderLineUniformRegion() const { FALSE_IF_INVALID(m_pOrderLine); return m_pOrderLine->m_ucUniformregion != 'Y'; }

bool CBLLService::GetExactTimeScheduling() const { return IsCompanyEnabledExactTimeUniformSchedule(); }
bool CBLLService::IsCompanyEnforceUniformRegion() const { FALSE_IF_INVALID(m_pCompany); return m_pCompany->ucEnforceuniformregion.CMP('Y')?true:false; }
bool CBLLService::IsCompanyEnforceUniformRegionForBonus() const { FALSE_IF_INVALID(m_pCompany); return m_pCompany->ucEnforceuniformregionbonuses.CMP('Y')?true:false; }
bool CBLLService::IsCompanyEnabledExactTimeUniformSchedule() const { FALSE_IF_INVALID(m_pCompany); return m_pCompany->ucEnableexcttimeuniformsch.CMP('Y')?true:false; }

