#include "acrosspoint.h"
#include "path.h"
#include "agent.h"

AcrossPoint::AcrossPoint()
{
    for(int i =0; i< 8; i++)
    {
        m_bDirection.push_back(false);
        m_pPaths.push_back(NULL);
    }
    //sem_init(&m_uOccupyLock,0,1);
    m_bOccupyLock = false;

    m_tempApplyList.clear();
    //m_pApplication = NULL;
    m_pProcessingPath  = NULL;
    m_pProcessingApply = NULL;

    m_dStepID = 0;
}

AcrossPoint::~AcrossPoint()
{
    for(int i =0; i< 8; i++)
    {
        if(m_pPaths[i] != NULL)
        {
            free(m_pPaths[i]);
            m_pPaths[i] =NULL;
        }
    }
    //sem_destroy(&m_uOccupyLock);
}

int  AcrossPoint::GetJoinNum()
{
    int    dJoinNum = m_bDirection[0] + m_bDirection[1] + m_bDirection[2] + m_bDirection[3] +
                      m_bDirection[4] + m_bDirection[5] + m_bDirection[6] + m_bDirection[7];
    return dJoinNum;
}

bool  AcrossPoint::OccupyLock()
{
    bool   flag = false;
    if( m_bOccupyLock == false )
    {
        m_bOccupyLock = true;
        flag    = true;
    }

    return  flag;
}

bool  AcrossPoint::ReleaseLock()
{
    bool   flag = false;
    if(m_bOccupyLock == true)
    {
        m_bOccupyLock = false;
        flag    = true;
    }

    return flag;
}

bool  AcrossPoint::CheckLock()
{
    return m_bOccupyLock;
}


/*
void AcrossPoint::HandlingApply(  )
{
    int dSize = m_tempApplyList.size();
    if(dSize > 0)
    {
        //COMBPATH_WAIT                0,
        //COMBPATH_WAIT_DISPATCH       1,
        //COMBPATH_MOVE_ACROSS         2,
        //COMBPATH_MOVE_PATH           3,
        //COMBPATH_WAIT_NEXT_APPROVE   4,
        //COMBPATH_WAIT_PASS           5,
        //COMBPATH_PASS_OR_END         6

        //0 WAIT {receive apply}  1 WAIT_DISPATCH {approve: across & path & agent Priority ; if is frist apply jump 3}
        //2 MOVE_ACROSS {check pass across;set pre apply pass 6} 3 MOVE_PATH {check arrive at path end; if is last apply jump 6}
        //4 WAIT_NEXT_APPROVE{set next applyto 1;then wait next apply approve(change to 2)}
        //5 WAIT_PASS{when next apply pass across}  6 PASS_OR_END

        //step1: frist check if current apply is over
        if(m_pApplication != NULL)  //none apply is dealing
        {
            CombPathStatue  eStatue = m_pApplication->pCombPath->GetStatue();
            if( eStatue > COMBPATH_MOVE_ACROSS )
                m_pApplication = NULL;
        }

        //step2: find the highest priority agent's apply.
        if(m_pApplication == NULL)  //none apply is dealing
        {
            float  fMaxPriority = 0;
            int    dMaxID = -1;
            for(int i =0; i< dSize; i++ )
            {
                if(m_tempApplyList[i].pCombPath->GetStatue() == COMBPATH_WAIT_DISPATCH )
                {
                    float  tempPriority = m_tempApplyList[i].pAgent->GetPriority();
                    if(tempPriority > fMaxPriority)
                    {
                        fMaxPriority = tempPriority;
                        dMaxID = i;
                    }
                }
            }

            if(dMaxID == -1)
                m_pApplication = &(m_tempApplyList[dMaxID]);
        }

        //step3: just deal this WAIT_DISPATCH statue apply
        CombPathStatue  eStatue = m_pApplication->pCombPath->GetStatue();
        switch( eStatue )
        {
        case COMBPATH_WAIT_DISPATCH:
            m_pApplication->pCombPath->m_pPath->ApplyPath( m_pApplication );
            break;

        default:
            break;
        }
    }

}
*/

void  AcrossPoint::HandlingApply_New()
{
    if( (NULL == m_pProcessingApply) && !CheckLock() )
    {
        //classify apply
        std::vector<ApplyInfor>  UndealApplyList;
        std::vector<ApplyInfor>  DealedApplyList;
        for( int i =0; i < m_tempApplyList.size(); i++ )
        {
            CombPathStatue  eStatue = m_tempApplyList[i].pCombPath->GetStatue();
            if( COMBPATH_WAIT_DISPATCH == eStatue )
                UndealApplyList.push_back(m_tempApplyList[i]);
            else
                DealedApplyList.push_back(m_tempApplyList[i]);
        }

        //sort apply by its' agent's Priority
        int   dMaxid       = -1;
        float fMaxPriority = -1;
        for(int i=0 ; i< UndealApplyList.size(); i++)
        {
            int j=i;
            for( ; j < UndealApplyList.size(); j++)
            {
                float fPriority = UndealApplyList[j].pAgent->GetPriority();
                if( fPriority > fMaxPriority )
                {
                    dMaxid = j;
                    fMaxPriority = fPriority;
                }
            }

            std::swap(UndealApplyList[i], UndealApplyList[j]);
        }

        if(UndealApplyList.size() > 0) //if have some application not be processed
        {
            //choose the Priority max and feasible apply as ProcessingApply application
            for(int i =0; i < UndealApplyList.size(); i++)
            {
                Agent*      pAgent     = UndealApplyList[i].pAgent;
                CombPath*   pTCombPath = UndealApplyList[i].pCombPath;
                Path*       pPath      = pTCombPath->m_pPath;

                if( pTCombPath->m_pPath->m_bMonopoly )
                {
                    if(
                          ( (pPath->CheckLock()) && (0 == pPath->m_AGVOccupyList.size()) )   ||    //this path has none agent occupy
                          (!(pPath->CheckLock()) && (1 == pPath->m_AGVOccupyList.size()) && pPath->IfAGVAlreadyOnThisPath(pAgent)) //this path had been occupyed, but the agent is itself
                            )
                    {
                        OccupyLock();           //lock across
                        pPath->OccupyLock();    //lock monopoly path
                        pTCombPath->ChangeStatue(COMBPATH_MOVE_PATH);
                        pPath->m_dDirection = pTCombPath->m_dDirection;
                        m_pProcessingApply  = &UndealApplyList[i];
                    }
                }
                else
                {
                    //path not used or apply's direction is same with path's direction
                    if(  (pPath->m_dDirection == -1) ||
                        ((pPath->m_dDirection == pTCombPath->m_dDirection ) && !(pPath->m_bLock))
                         )
                    {
                        OccupyLock();    //lock across
                        pTCombPath->ChangeStatue(COMBPATH_MOVE_ACROSS);
                        pPath->m_dDirection = pTCombPath->m_dDirection;
                        m_pProcessingApply  = &UndealApplyList[i];
                    }
                }

                if(m_pProcessingApply)
                    break;
            }



            //last step could not find a feasible apply, so deal the first conflict apply(max Priority)
            if(NULL == m_pProcessingApply)
            {
                Agent*      pAgent     = UndealApplyList[0].pAgent;
                CombPath*   pTCombPath = UndealApplyList[0].pCombPath;
                Path*       pPath      = pTCombPath->m_pPath;

                if( pTCombPath->m_pPath->m_bMonopoly )
                {
                    float fPriority = pTCombPath->m_pPath->m_AGVOccupyList[0]->GetPriority();
                    if(pAgent->GetPriority() > fPriority)
                    {
                        //0: if apply path have some agv is in TASK_PRO mode, then wait for process over
                        //1: calculate apply path's occupy agents 's dodge pos
                        //2: move agent to dodge pos,set apply path direction to -1

                    }
                    else
                    {
                        //just wait, do nothing
                    }
                }
                else
                {
                    //conflict occur
                    switch( m_dStepID )
                    {
                    //0: check if this apply path lock
                    case 0:
                        {
                            if( !(pPath->CheckLock()) )
                            {
                                m_dStepID++;
                                pPath->OccupyLock();
                            }
                        }
                        break;

                    //1: if apply path have some agv is in TASK_PRO mode, then wait for process over
                    case 1:
                        {
                            bool bflag = true;
                            for(int i=0; i< pPath->m_AGVOccupyList.size(); i++)
                            {
                                if( AGENT_STATE_DEALINGTASK_MOVE != pPath->m_AGVOccupyList[i]->GetState() )
                                {
                                    bflag = false;
                                    break;
                                }

                                if( AGENT_STATE_BREAKDOWN == pPath->m_AGVOccupyList[i]->GetState() )
                                {
                                    //error occur, warning
                                }
                            }

                            if( bflag )
                                m_dStepID++;
                        }
                        break;

                    //2: calculate apply path's occupy agents 's dodge pos
                    case 2:
                        pPath->SendOccupyAGVtoDodgeMode();
                        break;

                    //3: move all agent to dodge pos,set apply path direction to -1

                    //4:
                    default:
                        break;
                    }



                }

            }


        }
    }
}

/*
void  AcrossPoint::TrafficControl_HandlingApply()
{
    if(NULL == m_pProcessingApply)
    {
        //classify apply
        std::vector<ApplyInfor>  InPutApplyList;
        std::vector<ApplyInfor>  OutPutApplyList;
        for( int i =0; i < m_tempApplyList.size(); i++ )
        {
            CombPathStatue  eStatue = m_tempApplyList[i].pCombPath->GetStatue();
            if( COMBPATH_WAIT_DISPATCH == eStatue )
                InPutApplyList.push_back(m_tempApplyList[i]);
            else
                OutPutApplyList.push_back(m_tempApplyList[i]);
        }

        //just need to deal InApplyList,the apply that agent ont the m_pProcessingPath would be deal
        for(int i =0 ; i < InPutApplyList.size(); i++)
        {
            Agent*  pAgent = InPutApplyList[i].pAgent;

            for(int j =0; j < m_pProcessingPath->m_AGVOccupyList.size(); j++)
            {
                if(pAgent == m_pProcessingPath->m_AGVOccupyList[i])
                {
                    m_pProcessingApply = &InPutApplyList[i];
                    break;
                }
            }
        }
    }

    if( m_pProcessingApply )
    {
        Agent*      pAgent     = m_pProcessingApply->pAgent;
        CombPath*   pTCombPath = m_pProcessingApply->pCombPath;
        Path*       pPath      = pTCombPath->m_pPath;
        if(!CheckLock()) // across must empty
        {
            if( pTCombPath->m_pPath->m_bMonopoly )
            {
                if(
                      ( (pPath->CheckLock()) && (0 == pPath->m_AGVOccupyList.size()) )   ||    //this path has none agent occupy
                      (!(pPath->CheckLock()) && (1 == pPath->m_AGVOccupyList.size()) && pPath->IfAGVAlreadyOnThisPath(pAgent)) //this path had been occupyed, but the agent is itself
                        )
                {
                    OccupyLock();           //lock across
                    pPath->OccupyLock();    //lock monopoly path
                    pTCombPath->ChangeStatue(COMBPATH_MOVE_PATH);
                    pPath->m_dDirection = pTCombPath->m_dDirection;
                }
                else
                {
                    //just wait for this path release
                }
            }
            else
            {
                //path not used or apply's direction is same with path's direction
                if(  (pPath->m_dDirection == -1) ||
                    ((pPath->m_dDirection == pTCombPath->m_dDirection ) && !(pPath->m_bLock))
                     )
                {
                    OccupyLock();    //lock across
                    pTCombPath->ChangeStatue(COMBPATH_MOVE_ACROSS);
                    pPath->m_dDirection = pTCombPath->m_dDirection;
                }
                //conflict occur
                else
                {
                    switch(m_dStepID)
                    {
                    //0: check if this apply path lock
                    case 0:
                        {
                            if(pPath->CheckLock())
                            {
                                m_dStepID++;
                                pPath->OccupyLock();
                            }
                        }
                        break;

                    //1: if apply path have some agv is in TASK_PRO mode, then wait for process over
                    case 1:
                        {
                            bool bflag = true;
                            for(int i=0; i< pPath->m_AGVOccupyList.size(); i++)
                            {
                                if( AGENT_STATE_DEALINGTASK_MOVE != pPath->m_AGVOccupyList[i]->GetState() )
                                    bflag = false;


                                if( AGENT_STATE_BREAKDOWN == pPath->m_AGVOccupyList[i]->GetState() )
                                {
                                    //error occur, warning
                                }
                            }

                            if(bflag)
                                m_dStepID++;
                        }
                        break;

                    //2: calculate apply path's occupy agents 's dodge pos
                    case 2:
                        pPath->SendOccupyAGVtoDodgeMode();
                        break;

                    //3: move all agent to dodge pos,set apply path direction to -1

                    //4:
                    default:
                        break;

                    }

                }
            }
        }
    }

}

void  AcrossPoint::TrafficControl()
{
    if(NULL != m_pProcessingPath)
    {
        if(m_pProcessingPath->m_AGVOccupyList.size() > 0)
            TrafficControl_HandlingApply();
        else
        {
            m_pProcessingPath->ResetDirection();
            m_pProcessingPath = NULL;
        }
    }
    // find the average occupy agent Priority max path to process
    else
    {
        float   fMaxPriority = 0;
        for(int i=0; i < 8; i++)
        {
            if( ( NULL != m_pPaths[i] ) &&
                ( -1 != m_pPaths[i]->m_dDirection )  &&
                ( m_pPaths[i]->m_dAcrossID[m_pPaths[i]->m_dDirection] == m_dID )  // this path's direction is toward this AcrossPoint
                    )
            {
                float fPriority = m_pPaths[i]->AverageOccupyAGVPriority();

                if(fPriority > fMaxPriority)
                {
                    fMaxPriority = fPriority;
                    m_pProcessingPath = m_pPaths[i];
                }
            }
        }
    }
}

*/
