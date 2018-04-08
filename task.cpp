#include <iostream>
#include <cmath>
#include "agent.h"
#include "map.h"
#include "task.h"
#include "manager.h"

#define  CV_PI   3.1415926

std::map<int, std::string> Task::m_vStatueList = {
                                                   {TASK_STATUE_MOVEING, "MOVEING"},
                                                   {TASK_STATUE_PROCESS, "PROCESS"},
                                                   {TASK_STATUE_FINISH, "FINISH"},
                                                   {TASK_STATUE_PAUSE, "PAUSE"} };

Task::Task()
{
    m_dID = 0;
    m_eCurStatue= TASK_STATUE_MOVEING;
    m_ePreStatue= TASK_STATUE_MOVEING;

    m_ActionList.clear();

    m_pManager   = NULL;
    m_pAgent     = NULL;
}

Task::Task( Manager* pManager, Agent* pAgent ):  m_pManager(pManager),m_pAgent(pAgent)
{
    m_dID = 0;
    m_eCurStatue= TASK_STATUE_MOVEING;
    m_ePreStatue= TASK_STATUE_MOVEING;
    m_ActionList.clear();
}

/*
bool  Task::FindPath( Vec2i  start, Vec2i end )
{
    bool  bflag = false;

    if( (m_eCurStatue  == TASK_STATUE_WAIT) || (m_eCurStatue  == TASK_STATUE_MOVEING) )
    {
        m_vPath.clear();
        m_vPath = m_pMap->findPath(start, end);
        if( m_vPath.size() == 0 )
        {
            CalPathInflexion();
            bflag = true;
        }
    }
    else
        bflag = false;
    return  bflag;
}


void   Task::SetTimeStampToPath()
{
    if(m_eCurStatue == TASK_STATUE_MOVEING)
    {
        time_t now_time;
        now_time = time(NULL);
        bool   bfind = false;
        float  deltaTime = 0;
        CoordinateList path = m_path.GetPath();
        for( int i =0; i< path.size(); i++ )
        {
            Vec2i*  tempNode = &(path[i]);

            if( bfind )
            {
                float Acctime,Dectime;
                float fLineVelocity,fAngularVelocity;
                float fScale;

                m_pMap->GetGridScale(fScale);
                m_pAgent->GetAccDecTime( Acctime,Dectime);
                m_pAgent->GetMeanVelocity(fLineVelocity,fAngularVelocity);

                if(tempNode->rotate_angle == 0)
                {
                    tempNode->TimeIn  = now_time + deltaTime;
                    deltaTime +=  fScale / fLineVelocity;
                    tempNode->TimeOut = now_time + deltaTime;
                }
                else
                {
                    tempNode->TimeIn  = now_time + deltaTime;
                    deltaTime += 2*Acctime + 2*Dectime + fScale / fLineVelocity + tempNode->rotate_angle / fAngularVelocity;
                    tempNode->TimeOut = now_time + deltaTime;
                }
                continue;
            }
            else
            {
                tempNode->TimeIn  = 0;
                tempNode->TimeOut = 0;
            }

            Vec2i  curGridPos = m_pAgent->GetCurrentGridPosition();
            if( *tempNode == curGridPos )
            {
                //the current grid cost time calculate
                bfind = true;

                float Acctime,Dectime;
                float fLineVelocity,fAngularVelocity;
                float fScale;

                m_pMap->GetGridScale(fScale);
                m_pAgent->GetAccDecTime( Acctime,Dectime);
                m_pAgent->GetMeanVelocity(fLineVelocity,fAngularVelocity);

                tempNode->TimeIn  = now_time + deltaTime;
                if(i < (path.size() - 1) )
                {
                    bool bAlign  = (curGridPos.angle == tempNode->angle);

                    if(tempNode->rotate_angle == 0)
                    {
                        Vec2i  NextGridCenter = m_pMap->CalculateGridCenter(path[i+1]);
                        float Dis = m_pMap->CalculateRealDistance(m_pAgent->GetCurrentMapPosition(), NextGridCenter) -  fScale / 2;
                        deltaTime += Dis / fLineVelocity;
                    }
                    else
                    {
                        Vec2i  curGridCenter = m_pMap->CalculateGridCenter(path[i]);
                        if(bAlign)
                        {
                            float Dis = fScale / 2 - m_pMap->CalculateRealDistance(m_pAgent->GetCurrentMapPosition(), curGridCenter);
                            deltaTime += Dis / fLineVelocity;
                        }
                        else
                        {
                            float Dis = m_pMap->CalculateRealDistance(m_pAgent->GetCurrentMapPosition(), curGridCenter);
                            if(Dis > 0.1)
                            {
                                deltaTime += tempNode->rotate_angle / fAngularVelocity + 2*Acctime + 2*Dectime + fScale / (2*fLineVelocity) + Dis / fLineVelocity;
                            }
                            else
                            {
                                float rotate_angle = (m_pAgent->GetCurrentMapPosition().angle - tempNode->angle );
                                deltaTime += rotate_angle / fAngularVelocity + Acctime + Dectime + fScale / (2*fLineVelocity);
                            }
                        }
                    }
                }
                else //now agent on the last node of this path
                {
                    Vec2i  curGridCenter = m_pMap->CalculateGridCenter(path[i]);
                    float Dis = m_pMap->CalculateRealDistance(m_pAgent->GetCurrentMapPosition(), curGridCenter);
                    deltaTime += Dis / fLineVelocity;
                }
                tempNode->TimeOut = now_time + deltaTime;
            }
        }
        m_path.SetPath( path );
    }
}


CoordinateList  Task::GetPath()
{
    return  m_vPath;
}

void   Task::SetStart(Vec2i start)
{
    m_sStart = start;
}

Vec2i  Task::GetStart()
{
    return m_sStart;
}

void   Task::SetGoal(Vec2i goal)
{
    m_sGoal = goal;
}

Vec2i  Task::GetGoal()
{
    return m_sGoal;
}
*/

TaskStatue   Task::GetStatue()
{
    return  m_eCurStatue;
}

bool  Task::Start()
{
    bool   bflag = false;

    if(!m_path.GetPath().empty())
    {
         bflag = ChangeTaskStatue(TASK_STATUE_MOVEING);
    }
    else
        cout<<"Error: Task "<<m_dID<<" can't be started, because the path is empty. please first set the path!"<<endl;

    return bflag;
}

bool  Task::Pause()
{
    bool   bflag = ChangeTaskStatue(TASK_STATUE_PAUSE);
    return bflag;
}

bool  Task::Resume()
{
    bool   bflag = ChangeTaskStatue(m_ePreStatue);
    return bflag;
}

bool  Task::ChangeTaskStatue(TaskStatue  eNextStatue)
{
    bool  bflag = false;

    switch(m_eCurStatue)
    {
/*
    case TASK_STATUE_WAIT:
        switch(eNextStatue)
        {
        case TASK_STATUE_MOVEING:
            m_ePreStatue = m_eCurStatue;
            m_eCurStatue = eNextStatue;
            bflag = true;
            break;
        default:
            cout<<"Error: Task "<<m_dID<<" is wait."<<"Can't be change to "<<m_vStatueList[eNextStatue]<<endl;
            break;
        }
        break;
*/

    case TASK_STATUE_MOVEING:
        switch(eNextStatue)
        {
        case TASK_STATUE_PROCESS:
            m_ePreStatue = m_eCurStatue;
            m_eCurStatue = eNextStatue;
            bflag = true;
            break;

        case TASK_STATUE_PAUSE:
            m_ePreStatue = m_eCurStatue;
            m_eCurStatue = eNextStatue;
            bflag = true;
            break;

        default:
            cout<<"Error: Task "<<m_dID<<" is moving."<<"Can't be change to "<<m_vStatueList[eNextStatue]<<endl;
            break;
        }
        break;

    case TASK_STATUE_PROCESS:
        switch(eNextStatue)
        {
        case TASK_STATUE_FINISH:
            m_ePreStatue = m_eCurStatue;
            m_eCurStatue = eNextStatue;
            bflag = true;
            break;

        case TASK_STATUE_PAUSE:
            m_ePreStatue = m_eCurStatue;
            m_eCurStatue = eNextStatue;
            bflag = true;
            break;

        default:
            cout<<"Error: Task "<<m_dID<<" is processing."<<"Can't be change to "<<m_vStatueList[eNextStatue]<<endl;
            break;
        }
        break;

    case TASK_STATUE_FINISH:
        cout<<"Error: Task "<<m_dID<<" is finish."<<endl;
        break;

    case TASK_STATUE_PAUSE:
        if(m_ePreStatue == eNextStatue)
        {
            m_ePreStatue = m_eCurStatue;
            m_eCurStatue = eNextStatue;
            bflag = true;
        }
        else
            cout<<"Error: Task "<<m_dID<<" is pause."<<"It's last statue is "<<m_vStatueList[m_ePreStatue]<<",so it can't be change to "<<m_vStatueList[eNextStatue]<<endl;
        break;

    default:
        break;
    }
    return bflag;
}

CombPath*  Task::GetExecuteCombPath()
{
    CombPath* pCurExecuteCombPath = NULL;
    for(int i =0; i < m_combPathList.size(); i++)
    {
        if( ( COMBPATH_WAIT_PASS != m_combPathList[i].GetStatue() ) && (COMBPATH_PASS_OR_END != m_combPathList[i].GetStatue() ) )
        {
            pCurExecuteCombPath = &(m_combPathList[i]);
            break;
        }
    }

    return    pCurExecuteCombPath;
}

void  Task::ExecuteTask( )
{
    TaskStatue  eTaskStatue = GetStatue();

    switch(eTaskStatue)
    {
    case TASK_STATUE_MOVEING:
        {
            CombPath*   pCurExecuteCombPath = GetExecuteCombPath();
            if(NULL != pCurExecuteCombPath)
                ExecuteCombPath(pCurExecuteCombPath);
            else
                ChangeTaskStatue(TASK_STATUE_PROCESS);
        }
        break;

    case TASK_STATUE_PROCESS:
        {
            if(m_ActionList.size() != 0)
            {
                Process*  pProHead = NULL;
                for(int i = 0; m_ActionList.size(); i++)
                {
                    ProcessStatue  eStatue = m_ActionList[i].GetStatue();
                    if( PS_FINISH != eStatue )
                    {
                        pProHead = &m_ActionList[i];
                        break;
                    }
                }

                if(pProHead)
                    pProHead->ExcuteProcess();
                else
                    ChangeTaskStatue(TASK_STATUE_FINISH);
            }
            else
                ChangeTaskStatue(TASK_STATUE_FINISH);
        }
        break;

    case TASK_STATUE_FINISH:
        break;

    case TASK_STATUE_PAUSE:
        break;

    default:
        break;
    }

}

void  Task::ExecuteCombPath( CombPath*   pCurExecuteCombPath )
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

     CombPathStatue   curstatue = pCurExecuteCombPath->GetStatue();
     switch(curstatue)
     {
     //this case occur only when combpath start execute
     case COMBPATH_WAIT:
         {
             //send apply path to system
             ApplyInfor  tempApply;
             tempApply.pAgent    = m_pAgent;
             tempApply.pCombPath = pCurExecuteCombPath;
             m_pManager->m_ApplyList.push_back(tempApply);
             pCurExecuteCombPath->ChangeStatue( COMBPATH_WAIT_DISPATCH );
         }
         break;

     case COMBPATH_WAIT_DISPATCH:
         break;

     case COMBPATH_MOVE_ACROSS:
         {
            //if current path is not empty
            if( 0 != pCurExecuteCombPath->m_pPath->GetPathSize() )
            {
                Vec2i  curPos = m_pAgent->GetCurrentGridPosition();
                Vec2i  staPos = pCurExecuteCombPath->m_staPos;
                int    dDirection = -1;

                //normal: enter current path, and exit pre path source
                int    dLength = pCurExecuteCombPath->m_pPath->CalLengthTwoPos(curPos, staPos, dDirection);
                if(
                     ( dLength == 0 )  ||   //arrived at start pos
                     (( dLength > 0 )&&( dDirection != pCurExecuteCombPath->m_dDirection )) //had already passed the start pos of this path
                        )
                {
                    if(pCurExecuteCombPath->m_dID > 0)//must true
                    {
                        //release the current across source
                        pCurExecuteCombPath->m_pAcross->ReleaseLock();

                        //release source of pre path
                        if( (0 == m_combPathList[pCurExecuteCombPath->m_dID - 1].m_pPath->GetPathSize()) &&
                            (m_combPathList[pCurExecuteCombPath->m_dID - 1].m_pAcross->CheckLock()) )
                            m_combPathList[pCurExecuteCombPath->m_dID - 1].m_pAcross->ReleaseLock();  //release the last across source
                        m_combPathList[pCurExecuteCombPath->m_dID - 1].ChangeStatue( COMBPATH_PASS_OR_END );
                        m_combPathList[pCurExecuteCombPath->m_dID - 1].m_pPath->ReleaseAgent( m_pAgent );
                        if( m_combPathList[pCurExecuteCombPath->m_dID - 1].m_pPath->m_bMonopoly )
                            m_combPathList[pCurExecuteCombPath->m_dID - 1].m_pPath->ReleaseLock();
                    }

                    pCurExecuteCombPath->ChangeStatue( COMBPATH_MOVE_PATH );
                    pCurExecuteCombPath->m_pPath->AddAgent(m_pAgent);
                }
            }
            else
            {
                pCurExecuteCombPath->ChangeStatue( COMBPATH_WAIT_NEXT_APPROVE );
                pCurExecuteCombPath->m_pPath->AddAgent(m_pAgent);
            }
         }
         break;

     case COMBPATH_MOVE_PATH:   //check arrive at path end; if is last apply jump 6
         {
            if(0==pCurExecuteCombPath->m_dID)
                pCurExecuteCombPath->m_pPath->AddAgent(m_pAgent);

            Vec2i   curPos = m_pAgent->GetCurrentGridPosition();
            Vec2i   endPos = pCurExecuteCombPath->m_endPos;
            int     dDirection = -1;
            int     dLength = pCurExecuteCombPath->m_pPath->CalLengthTwoPos(curPos, endPos, dDirection);

            if(dLength == 0)
            {
                if(pCurExecuteCombPath->m_dID == (m_combPathList.size() - 1) )
                    pCurExecuteCombPath->ChangeStatue(COMBPATH_PASS_OR_END);
                else
                    pCurExecuteCombPath->ChangeStatue(COMBPATH_WAIT_NEXT_APPROVE);
            }
         }
         break;

     case COMBPATH_WAIT_NEXT_APPROVE:   //set next applyto 1;then wait next apply approve(change to >= 2)
         {
             if( pCurExecuteCombPath->m_dID != ( m_combPathList.size() - 1 ) )
             {
                 if( m_combPathList[pCurExecuteCombPath->m_dID + 1].GetStatue()  ==  COMBPATH_WAIT )
                 {
                     ApplyInfor  tempApply;
                     tempApply.pAgent    = m_pAgent;
                     tempApply.pCombPath = &m_combPathList[pCurExecuteCombPath->m_dID + 1];
                     tempApply.pCombPath->ChangeStatue( COMBPATH_WAIT_DISPATCH );
                     m_pManager->m_ApplyList.push_back(tempApply);
                 }

                 //Next apply approve
                 CombPathStatue nextStatue = m_combPathList[pCurExecuteCombPath->m_dID + 1].GetStatue();
                 if( nextStatue > COMBPATH_WAIT_DISPATCH )
                     pCurExecuteCombPath->ChangeStatue( COMBPATH_WAIT_PASS );
             }
         }
         break;

     case COMBPATH_WAIT_PASS:
         break;

     case COMBPATH_PASS_OR_END:
         break;

     default:
         break;
     }


}














