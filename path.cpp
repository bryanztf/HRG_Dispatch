#include "path.h"
#include "map.h"
#include "manager.h"

struct PathDodgePosCorrelation
{
    Vec2i  pos;

    int    IDDodgePos1;
    int    IDDodgePos2;

    PathDodgePosCorrelation()
    {
        IDDodgePos1 =-1;
        IDDodgePos2 =-1;
    }
};


Path::Path()
{
    m_bMonopoly = false;
    //sem_init(&m_uLock,0,1);
    m_bLock = false;
    m_dDirection = -1;
    m_dAcrossID[0]  = -1;
    m_dAcrossID[1]  = -1;
    m_color = cv::Scalar(255,255,255);
    m_AGVOccupyList.clear();

    m_AGVDodgeList.clear();
    m_pMap = NULL;
}

Path::Path( const Path& source )
{
    m_ExtremePos[0] = source.m_ExtremePos[0];
    m_ExtremePos[1] = source.m_ExtremePos[1];
    m_Pathlist = source.m_Pathlist;
    m_pMap = source.m_pMap;
}

Path&  Path::operator = (const Path& source)
{
    m_ExtremePos[0] = source.m_ExtremePos[0];
    m_ExtremePos[1] = source.m_ExtremePos[1];
    m_Pathlist = source.m_Pathlist;
    m_pMap = source.m_pMap;
    return *this;
}

Path::~Path()
{
    //sem_destroy(&m_uLock);
}

void   Path::SetPath( CoordinateList Pathlist)
{
    m_Pathlist = Pathlist;
}

void   Path::SetExtremePoint( Vec2i ExtremePos1, Vec2i ExtremePos2 )
{
    m_ExtremePos[0] =  ExtremePos1;
    m_ExtremePos[1] =  ExtremePos2;
}

void   Path::GetExtremePoint(Vec2i& ExtremePos1, Vec2i& ExtremePos2)
{
    ExtremePos1 = m_ExtremePos[0];
    ExtremePos2 = m_ExtremePos[1];
}

CoordinateList   Path::GetPath()
{
    return   m_Pathlist;
}

int  Path::GetPathSize()
{
    return  m_Pathlist.size();
}

int  Path::GetRemainderPathLength()
{
    int dPassPathLength =0;
    for(;dPassPathLength<m_Pathlist.size();dPassPathLength++)
    {
        if( (m_Pathlist[dPassPathLength].TimeIn != 0) || (m_Pathlist[dPassPathLength].TimeOut != 0) )
        {
            break;
        }
    }
    int dRemainderPathLength = m_Pathlist.size() - dPassPathLength;

    return dRemainderPathLength;
}

int    Path::GetSpeGridPosIndex(Vec2i specialGridPos)
{
    int dPassPathLength =0;
    for(;dPassPathLength<m_Pathlist.size();dPassPathLength++)
    {
        if( m_Pathlist[dPassPathLength] == specialGridPos)
            break;
    }

    if(dPassPathLength >= m_Pathlist.size())
        dPassPathLength = -1;

    return dPassPathLength;
}

Vec2i  Path::GetCurGridPosIndex(int& dIndex)
{
    int dPassPathLength =0;
    for(;dPassPathLength<m_Pathlist.size();dPassPathLength++)
    {
        if( (m_Pathlist[dPassPathLength].TimeIn != 0) || (m_Pathlist[dPassPathLength].TimeOut != 0) )
        {
            break;
        }
    }
    dIndex = dPassPathLength;

    return m_Pathlist[dIndex];
}

bool   Path::GetPointByID(int posID, Vec2i &result)
{
    bool bflag = false;
    if( (posID < m_Pathlist.size()) && (posID >= 0) )
    {
        bflag = true;
        result = m_Pathlist[posID];
    }

    return bflag;
}

int   Path::FindPoint( Vec2i elementPos )
{
    int it = 0;
    for(; it< m_Pathlist.size(); it++)
    {
        if( m_Pathlist[it] == elementPos )
            break;
    }

    if( it == m_Pathlist.size() )
        it = -1;

    return it;
}

void  Path::CalPathInflexion()
{
    if(m_Pathlist.size() > 0 )
    {
        m_Pathlist[0].rotate_angle = 0;
        Vec2i*  pPre_Node = &(m_Pathlist[0]);
        Vec2i*  pCur_Node = &(m_Pathlist[1]);
        Vec2i*  pNex_Node = &(m_Pathlist[2]);
        for(int i =1; i < (m_Pathlist.size()-1); i++ )
            CalPathInflexion_Point( *pPre_Node, *pCur_Node, *pNex_Node );
        pNex_Node->rotate_angle = 0;
    }
}

void   Path::SetTimeStampToPath( Map* pMap,  AgvPhysicalModel eModel,  Vec2i curGridPos,  Vec2i curMapPos , AgentStatue  eAgentStatue,
                                 TaskStatue eTaskStatue, float Acctime, float Dectime, float fLineVelocity, float fAngularVelocity, float fScale)
{
    time_t now_time;
    now_time = time(NULL);
    bool   bfind         = false;
    float  deltaTime     = 0;
    float  deltaTime_in  = 0;
    float  deltaTime_out = 0;
    float  fBaseTime     = 0;
    float  fBaseTime_Turn= 0;
    pMap->m_pManager->GetBaseTime( fBaseTime, fBaseTime_Turn );

    for( int i =0; i< m_Pathlist.size(); i++ )
    {
        Vec2i*  tempNode = &(m_Pathlist[i]);

        if( bfind )
        {
            if(tempNode->rotate_angle == 0)
            {
                switch(eModel)
                {
                case AGV_PMODEL_PARTICLE:
                    tempNode->TimeIn  = now_time + deltaTime;
                    deltaTime +=  fScale / fLineVelocity;
                    tempNode->TimeOut = now_time + deltaTime;
                    break;
                case AGV_PMODEL_SUBSTANCE:
                    tempNode->TimeIn  = now_time + deltaTime_in;
                    deltaTime_out     = deltaTime_in + fScale * 2 / fLineVelocity;
                    deltaTime_in     += fScale / fLineVelocity;
                    tempNode->TimeOut = now_time + deltaTime_out;
                    break;
                default:
                    break;
                }
            }
            else
            {
                switch(eModel)
                {
                case AGV_PMODEL_PARTICLE:
                    tempNode->TimeIn  = now_time + deltaTime;
                    deltaTime += 2*Acctime + 2*Dectime + fScale / fLineVelocity + tempNode->rotate_angle / fAngularVelocity;
                    tempNode->TimeOut = now_time + deltaTime;
                    break;
                case AGV_PMODEL_SUBSTANCE:
                    tempNode->TimeIn  = now_time + deltaTime_in;
                    deltaTime_out     = deltaTime_in + 2*Acctime + 2*Dectime + fScale * 2 / fLineVelocity + tempNode->rotate_angle / fAngularVelocity;
                    deltaTime_in     += 2*Acctime + 2*Dectime + tempNode->rotate_angle / fAngularVelocity + fScale / fLineVelocity;
                    tempNode->TimeOut = now_time + deltaTime_out;
                    break;
                default:
                    break;
                }
            }
            continue;
        }
        else
        {
            tempNode->TimeIn  = 0;
            tempNode->TimeOut = 0;
        }

        if( *tempNode == curGridPos )
        {
            //the current grid cost time calculate
            bfind = true;

            float Acctime,Dectime;
            float fLineVelocity,fAngularVelocity;
            float fScale;

            if( i < ( m_Pathlist.size() - 1 ) )
            {
                bool bAlign  = (curGridPos.angle == tempNode->angle);

                if(tempNode->rotate_angle == 0)
                {
                    Vec2i  NextGridCenter = pMap->CalculateGridCenter(m_Pathlist[i+1]);
                    float Dis = pMap->CalculateRealDistance(curMapPos, NextGridCenter) -  fScale / 2;
                    //deltaTime += Dis / fLineVelocity;

                    switch(eModel)
                    {
                    case AGV_PMODEL_PARTICLE:
                        tempNode->TimeIn = now_time + deltaTime;
                        deltaTime += Dis / fLineVelocity;
                        tempNode->TimeOut = now_time + deltaTime;
                        break;
                    case AGV_PMODEL_SUBSTANCE:
                        deltaTime_in = (Dis - 1.5* fScale)/fLineVelocity;
                        tempNode->TimeIn = now_time + deltaTime_in;
                        deltaTime_out  = deltaTime_in + ( Dis + fScale / 2 ) / fLineVelocity;
                        tempNode->TimeOut = now_time + deltaTime_out;
                        deltaTime_in += fScale / fLineVelocity;
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    Vec2i  curGridCenter = pMap->CalculateGridCenter(m_Pathlist[i]);
                    if(bAlign)
                    {
                        float Dis = fScale / 2 - pMap->CalculateRealDistance(curMapPos, curGridCenter);
                        //deltaTime += Dis / fLineVelocity;

                        switch(eModel)
                        {
                        case AGV_PMODEL_PARTICLE:
                            tempNode->TimeIn = now_time + deltaTime;
                            deltaTime += Dis / fLineVelocity;
                            tempNode->TimeOut = now_time + deltaTime;
                            break;
                        case AGV_PMODEL_SUBSTANCE:
                            deltaTime_in = ((Dis - 1.5* fScale)/fLineVelocity)-(2*Acctime + 2*Dectime + tempNode->rotate_angle / fAngularVelocity);
                            tempNode->TimeIn = now_time + deltaTime_in;
                            deltaTime_out  = Dis / fLineVelocity;
                            tempNode->TimeOut = now_time + deltaTime_out;
                            deltaTime_in = now_time + (Dis - fScale/2)/fLineVelocity;
                            break;
                        default:
                            break;
                        }

                    }
                    else
                    {
                        float Dis = pMap->CalculateRealDistance(curMapPos, curGridCenter);
                        if(Dis > 0.1)
                        {
                            switch(eModel)
                            {
                            case AGV_PMODEL_PARTICLE:
                                tempNode->TimeIn = now_time + deltaTime;
                                deltaTime += tempNode->rotate_angle / fAngularVelocity + 2*Acctime + 2*Dectime + fScale / (2*fLineVelocity) + Dis / fLineVelocity;
                                tempNode->TimeOut = now_time + deltaTime;
                                break;
                            case AGV_PMODEL_SUBSTANCE:
                                deltaTime_in = (Dis + 0.5* fScale)/fLineVelocity;
                                tempNode->TimeIn = now_time + deltaTime_in;
                                deltaTime_out  = tempNode->rotate_angle / fAngularVelocity + 2*Acctime + 2*Dectime + fScale / fLineVelocity + Dis / fLineVelocity;
                                tempNode->TimeOut = now_time + deltaTime_out;
                                deltaTime_in = now_time + Dis/fLineVelocity + tempNode->rotate_angle / fAngularVelocity + 2*Acctime + 2*Dectime;
                                break;
                            default:
                                break;
                            }
                        }
                        else
                        {
                            float rotate_angle = abs(curMapPos.angle - tempNode->angle );
                            switch(eModel)
                            {
                            case AGV_PMODEL_PARTICLE:
                                tempNode->TimeIn = now_time + deltaTime;
                                deltaTime += rotate_angle / fAngularVelocity + Acctime + Dectime + fScale / (2*fLineVelocity);
                                tempNode->TimeOut = now_time + deltaTime;
                                break;
                            case AGV_PMODEL_SUBSTANCE:
                                deltaTime_in = -(abs(tempNode->rotate_angle - rotate_angle)/fAngularVelocity + Acctime + Dectime + fScale / fLineVelocity );
                                tempNode->TimeIn = now_time + deltaTime_in;
                                deltaTime_out  = rotate_angle / fAngularVelocity + Acctime + Dectime + fScale / fLineVelocity;
                                tempNode->TimeOut = now_time + deltaTime_out;
                                deltaTime_in   = rotate_angle / fAngularVelocity + Acctime + Dectime;
                                break;
                            default:
                                break;
                            }
                        }
                    }
                }
            }
            else //now agent on the last node of this path
            {
                Vec2i  curGridCenter = pMap->CalculateGridCenter(m_Pathlist[i]);
                float Dis = pMap->CalculateRealDistance(curMapPos, curGridCenter);
                deltaTime += Dis / fLineVelocity;

                switch(eModel)
                {
                case AGV_PMODEL_PARTICLE:
                    tempNode->TimeIn  = now_time;
                    tempNode->TimeOut = now_time + deltaTime;
                    if( (AGENT_STATUE_PROCESS_DODGE_STOP == eAgentStatue) || (TASK_STATUE_FINISH == eTaskStatue) )
                        tempNode->TimeOut = now_time + MIN_DODGETIME_TIMES * fBaseTime;
                    break;
                case AGV_PMODEL_SUBSTANCE:
                    tempNode->TimeIn  = now_time;
                    tempNode->TimeOut = now_time + deltaTime;
                    if( (AGENT_STATUE_PROCESS_DODGE_STOP == eAgentStatue) || (TASK_STATUE_FINISH == eTaskStatue) )
                        tempNode->TimeOut = now_time + MIN_DODGETIME_TIMES * fBaseTime;
                    break;
                default:
                    break;
                }
            }
        }
    }

}

void  Path::CalPathInflexion_Point( Vec2i&  Pre_Node, Vec2i&  Cur_Node, Vec2i  Nex_Node )
{
    float delta_y1= Cur_Node.y-Pre_Node.y;
    float delta_x1= Cur_Node.x-Pre_Node.x;
    float delta_y2= Nex_Node.y-Cur_Node.y;
    float delta_x2= Nex_Node.x-Cur_Node.x;

    unsigned int Pre_situation=0;
    if((delta_x1>0)&&(delta_y1>0))
    {
        Pre_situation = 1;
    }
    else if((delta_x1<0)&&(delta_y1>0))
    {
        Pre_situation = 2;
    }
    else if((delta_x1<0)&&(delta_y1<0))
    {
        Pre_situation = 3;
    }
    else if((delta_x1>0)&&(delta_y1<0))
    {
        Pre_situation = 4;
    }
    else if((delta_x1>0)&&(delta_y1==0))
    {
        Pre_situation = 5;
    }
    else if((delta_x1<0)&&(delta_y1==0))
    {
        Pre_situation = 6;
    }
    else if((delta_x1==0)&&(delta_y1>0))
    {
        Pre_situation = 7;
    }
    else if((delta_x1==0)&&(delta_y1<0))
    {
        Pre_situation = 8;
    }

    switch (Pre_situation) {
    case 1:
        Pre_Node.angle = atan(delta_y1/delta_x1)*180/CV_PI;
        break;
    case 2:
        Pre_Node.angle = atan(delta_y1/delta_x1)*180/CV_PI+180;
    case 3:
        Pre_Node.angle = atan(delta_y1/delta_x1)*180/CV_PI+180;
    case 4:
        Pre_Node.angle = atan(delta_y1/delta_x1)*180/CV_PI+360;
    case 5:
        Pre_Node.angle = 0;
    case 6:
        Pre_Node.angle = 180;
    case 7:
        Pre_Node.angle = 90;
    case 8:
        Pre_Node.angle = 270;
    default:
        break;
    }

    unsigned int Cur_situation=0;
    if((delta_x2>0)&&(delta_y2>0))
    {
        Cur_situation = 1;
    }
    else if((delta_x2<0)&&(delta_y2>0))
    {
        Cur_situation = 2;
    }
    else if((delta_x2<0)&&(delta_y2<0))
    {
        Cur_situation = 3;
    }
    else if((delta_x2>0)&&(delta_y2<0))
    {
        Cur_situation = 4;
    }
    else if((delta_x2>0)&&(delta_y2==0))
    {
        Cur_situation = 5;
    }
    else if((delta_x2<0)&&(delta_y2==0))
    {
        Cur_situation = 6;
    }
    else if((delta_x2==0)&&(delta_y2>0))
    {
        Cur_situation = 7;
    }
    else if((delta_x2==0)&&(delta_y2<0))
    {
        Cur_situation = 8;
    }
    switch (Cur_situation) {
    case 1:
        Cur_Node.angle = atan(delta_y2/delta_x2)*180/CV_PI;
        break;
    case 2:
        Cur_Node.angle = atan(delta_y2/delta_x2)*180/CV_PI+180;
    case 3:
        Cur_Node.angle = atan(delta_y2/delta_x2)*180/CV_PI+180;
    case 4:
        Cur_Node.angle = atan(delta_y2/delta_x2)*180/CV_PI+360;
    case 5:
        Cur_Node.angle = 0;
    case 6:
        Cur_Node.angle = 180;
    case 7:
        Cur_Node.angle = 90;
    case 8:
        Cur_Node.angle = 270;
    default:
        break;
    }
    float temprotate_angle=Cur_Node.angle-Pre_Node.angle;
    if(temprotate_angle==0)
    {    //sem_wait(&pOwner->m_pDipper->m_uTextBufferLock);
        //sem_post(&pOwner->m_pDipper->m_uTextBufferLock);
        //run_flag = sem_trywait(&(pOwner->m_uKill_thread));
       Cur_Node.rotate_angle=0;
    }
    if(temprotate_angle!=0)
    {
        Cur_Node.rotate_angle=temprotate_angle;
    }
}

int   Path::CalLengthTwoPos(Vec2i  staNode_, Vec2i  endNode_, int& dDirection_)
{
    int  dlength = 0;
    dDirection_  = -1;

    if(staNode_ == endNode_)
    {
        dlength = 0;
    }
    else
    {
        for(int i=0; i< m_Pathlist.size(); i++)
        {
            if( (-1==dDirection_) && (staNode_ == m_Pathlist[i]) )
                dDirection_ = 1;
            else if( (-1==dDirection_) && (endNode_ == m_Pathlist[i]) )
                dDirection_ = 0;

            if( ( (staNode_ == m_Pathlist[i]) || (endNode_ == m_Pathlist[i]) ) && (0 == dlength) )
                dlength += 1;
            else if( (staNode_ != m_Pathlist[i]) && (endNode_ != m_Pathlist[i]) && (0 < dlength))
                dlength += 1;
            if( ( (staNode_ == m_Pathlist[i]) || (endNode_ == m_Pathlist[i]) ) && (0 < dlength) )
                break;
        }
    }

    return dlength;
}


bool   Path::OccupyLock()
{
    bool   flag = false;
    if(m_bLock == false)
    {
        m_bLock = true;
        flag    = true;
    }

    return  flag;
}


bool   Path::ReleaseLock()
{
    bool   flag = false;
    if(m_bLock == true)
    {
        m_bLock = false;
        flag    = true;
    }

    return flag;
}

bool   Path::CheckLock()
{
    return m_bLock;
}


bool  Path::CalDirection( int targetAcrossID , int &dDirection, Vec2i& extPos)
{
    bool  bflag = false;

    if(m_dAcrossID[0] == targetAcrossID)
    {
        bflag = true;
        dDirection = 0;
        extPos     = m_ExtremePos[0];
    }
    else if(m_dAcrossID[1] == targetAcrossID)
    {
        bflag = true;
        dDirection = 1;
        extPos     = m_ExtremePos[1];
    }

    return bflag;
}

bool  Path::CalDirection( Vec2i staPos, Vec2i endPos, int &dDirection)
{
    bool  flag  = false;
    int   dSize = GetPathSize();
    for( int i=0; i < dSize; i++ )
    {
        Vec2i  tempPos;
        GetPointByID(i,tempPos);
        if(tempPos == staPos)
        {
            dDirection = 1;
            flag       = true;
            break;
        }

        if(tempPos == endPos)
        {
            dDirection = 0;
            flag       = true;
            break;
        }
    }

    return flag;
}

bool Path::AddAgent(Agent* pPassedAgent)
{
    bool  bflag = true;
    std::vector<Agent*>::iterator it;
    for( it= m_AGVOccupyList.begin(); it != m_AGVOccupyList.end(); ++it )
    {
        if(*it == pPassedAgent)
        {
            bflag = false;
            break;
        }
    }

    if(bflag)
        m_AGVOccupyList.push_back(pPassedAgent);

    return bflag;
}

bool Path::ReleaseAgent(Agent* pPassedAgent)
{
    bool  bflag = false;
    std::vector<Agent*>::iterator it;
    for( it= m_AGVOccupyList.begin(); it != m_AGVOccupyList.end(); ++it )
    {
        if(*it == pPassedAgent)
        {
            it    = m_AGVOccupyList.erase(it);
            bflag = true;
            break;
        }
    }

    return bflag;
}

float Path::GetMaxPriorityAgent()
{
    float  fMaxPriority = -1;
    for(int i=0; m_AGVOccupyList.size(); i++)
    {
        float  tempValue = m_AGVOccupyList[i]->GetPriority();
        if(tempValue > fMaxPriority)
            fMaxPriority = tempValue;
    }

    return fMaxPriority;
}

void Path::ApplyOnlyPath( ApplyInfor* pApplication)
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

    Agent*     pAgent    = pApplication->pAgent;
    CombPath*  pCombPath = pApplication->pCombPath;
    Path*      pPath     = pApplication->pCombPath->m_pPath;

    if(pPath->m_bMonopoly)   //single extrem pos path apply
    {
        if(
              ( (pPath->CheckLock()) && ( 0 == pPath->m_AGVOccupyList.size() ) )   ||    //this path has none agent occupy
              (!(pPath->CheckLock()) && ( 1 == pPath->m_AGVOccupyList.size() ) && IfAGVAlreadyOnThisPath(pAgent) ) //this path had been occupyed, but the agent is itself
                )
        {

            pCombPath->ChangeStatue(COMBPATH_MOVE_PATH);
            pPath->OccupyLock();
            pPath->m_dDirection = pCombPath->m_dDirection;
            pPath->AddAgent(pAgent);
        }
    }
    else   //double extrem pos path apply
    {
        //path not use and this apply's direction same with this path's direction
        if(  ( pPath->m_dDirection == -1 ) ||
            (( pPath->m_dDirection == pCombPath->m_dDirection ) && !(pPath->CheckLock() )) ||
            (( pPath->m_dDirection != pCombPath->m_dDirection ) && !(pPath->CheckLock()) && (1 == pPath->m_AGVOccupyList.size()) && IfAGVAlreadyOnThisPath(pAgent))
             )
        {
            pCombPath->ChangeStatue(COMBPATH_MOVE_PATH);
            pPath->m_dDirection = pCombPath->m_dDirection;
            pPath->AddAgent(pApplication->pAgent);
        }
        //this apply's direction conflict with this path's direction
        else if( pPath->m_dDirection != pCombPath->m_dDirection )
        {
            //do noting,and wait
        }
    }

}

void  Path::SendOccupyAGVtoDodgeMode()
{
    int  dSize           = GetPathSize();
    int  dAGVNum         = m_AGVOccupyList.size();
    int  dDodgeDirection = m_dDirection;

    //sort agv by it's current position
    std::map<int, Agent*>   sortAGVList;
    for(int i =0; i < m_AGVOccupyList.size(); i++)
    {
        Vec2i  curPos = m_AGVOccupyList[i]->GetCurrentGridPosition();
        int    dIndex = GetSpeGridPosIndexWithDirection( curPos, dDodgeDirection );
        sortAGVList.insert(make_pair(dIndex, m_AGVOccupyList[i]));
    }

    //Path pos and DodgePos Correlation
    PathDodgePosCorrelation  ArrayCorrelation[dSize];
    for(int i=0; i< dSize; i++)
    {
        ArrayCorrelation[i].pos = GetGridPosFromIndexDirection(i, dDodgeDirection);
        for(int j =0 ;j < m_DodgePosIDList.size(); j++)
        {
            int   j_direction = (dDodgeDirection == 0)? j : (m_DodgePosIDList.size()-1-j);
            int   id  = m_DodgePosIDList[j_direction];
            float dis = m_pMap->CalculateRealDistance((Vec2i)(m_pMap->m_DodgePosList[id]), ArrayCorrelation[i].pos);
            if(dis<=1)
            {
                if( -1 == ArrayCorrelation[i].IDDodgePos1 )
                    ArrayCorrelation[i].IDDodgePos1 = id;
                else if( -1 == ArrayCorrelation[i].IDDodgePos2 )
                {
                    ArrayCorrelation[i].IDDodgePos2 = id;
                    break;
                }
            }
            else
                break;
        }
    }

    //search dodge pos for every agv on this path
    Agent*  pAgentList[dAGVNum];
    Vec2i   PathPos[dAGVNum];
    int     DodgePosID[dAGVNum];
    bool    Arrayflag[dAGVNum];

    int  dID = 0;
    std::map<int, Agent*>::iterator  iter;
    for( iter = sortAGVList.begin(); iter != sortAGVList.end(); iter++ )
    {
        Agent*     pAGV        = iter->second;
        pAgentList[dID]        = pAGV;
        CombPath*  pCombPath   = pAGV->m_pTaskGroup->GetCurrentTask()->GetExecuteCombPath();
        int        direction   = pCombPath->m_dDirection;
        int        dodgePosID1 = -1;
        int        dodgePosID2 = -1;

        bool bfind = false;
        for( int i = 0; i < dSize; i++ )
        {
            PathPos[dID] = GetGridPosFromIndexDirection(i, direction);
            dodgePosID1  = ArrayCorrelation[i].IDDodgePos1;
            dodgePosID2  = ArrayCorrelation[i].IDDodgePos2;

            bool bLockDodge1 = (dodgePosID1 == -1) ? true : m_pMap->m_DodgePosList[dodgePosID1].CheckLock();
            bool bLockDodge2 = (dodgePosID2 == -1) ? true : m_pMap->m_DodgePosList[dodgePosID2].CheckLock();

            if( !bLockDodge1 || !bLockDodge2 )
            {
                bfind = true;
                Arrayflag[dID]  = bfind;
                DodgePosID[dID] = !bLockDodge1 ? dodgePosID1 : dodgePosID2;
                m_pMap->m_DodgePosList[DodgePosID[dID]].OccupyLock();
                break;
            }
        }
        dID++;
    }

    //general dodge task group, and active it
    for(int i =0; i< dAGVNum; i++)
    {
        DodgeTaskGroup* tempHead = new DodgeTaskGroup(m_pMap->m_pManager);
        tempHead->SetKeyPar( PathPos[i], DodgePosID[i] );
        tempHead->m_pAgent = pAgentList[i];
        pAgentList[i]->ChangeState( AGENT_STATE_DODGE );
        m_pMap->m_pManager->m_pDodgeTaskGroupQueue.push_back(tempHead);
    }

}

Vec2i Path::GetGridPosFromIndexDirection(int  dIndex, int  dDirection)
{
    dIndex = (1 == dDirection) ? (GetPathSize() - dIndex -1) : dIndex;
    Vec2i  resultPos = GetCurGridPosIndex(dIndex);
    return resultPos;
}

int   Path::GetSpeGridPosIndexWithDirection(Vec2i specialGridPos, int  dDirection)
{
    int  dIndex = GetSpeGridPosIndex(specialGridPos);
    dIndex = (1 == dDirection) ? (GetPathSize() - dIndex -1) : dIndex;

    return dIndex;
}

bool Path::IfAGVAlreadyOnThisPath(Agent* pAGV)
{
    bool  bflag  =  false;
    for(int i =0; i < m_AGVOccupyList.size(); i++)
    {
        if(m_AGVOccupyList[i] == pAGV)
        {
            bflag = true;
            break;
        }
    }

    return bflag;
}

bool  Path::ResetDirection()
{
    bool bflag = false;
    if(0 == m_AGVOccupyList.size())
    {
        m_dDirection = -1;
        bflag = true;
    }

    return bflag;
}

bool  Path::SetDirection(int dDirection, Agent* pAgent)
{
    bool   bflag = false;
    //none agv occupy this path
    if( -1 == m_dDirection )
    {
        m_dDirection = dDirection;
        bflag = true;
    }
    //only one agv occupy this path, and this agv want change direction
    else
    {
        if( (m_AGVOccupyList.size() == 1)  &&
            (m_AGVOccupyList[0] == pAgent) &&
            (m_dDirection != dDirection) )
        {
            m_dDirection = dDirection;
            bflag = true;
        }
    }

    return bflag;
}

float  Path::AverageOccupyAGVPriority()
{
    float  fPriority = 0;
    for(int i=0; i< m_AGVOccupyList.size(); i++)
        fPriority += m_AGVOccupyList[i]->GetPriority();

    if(m_AGVOccupyList.size() > 1)
        fPriority /= m_AGVOccupyList.size();

    return  fPriority;
}
