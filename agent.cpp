#include "agent.h"
#include "manager.h"
#include "map.h"



std::map<int, std::string> Agent::m_vStatueList = { {AGENT_STATUE_WAIT, "WAIT"},
                                                   {AGENT_STATUE_CHARGING, "CHARGING"},
                                                   {AGENT_STATUE_PROCESS_NORMAL_RUN, "PROCESS_NORMAL_RUN"},
                                                   {AGENT_STATUE_PROCESS_DODGE_GO, "PROCESS_DODGE_GO"},
                                                   {AGENT_STATUE_PROCESS_DODGE_STOP, "PROCESS_DODGE_STOP"},
                                                   {AGENT_STATUE_PROCESS_DODGE_BACK, "PROCESS_DODGE_BACK"},
                                                   {AGENT_STATUE_BREAKDOWN, "BREAKDOWN"} };

std::map<int, std::string> Agent::m_vStateList = { {AGENT_STATE_WAIT, "WAIT"},
                                                   {AGENT_STATE_CHARGING, "CHARGING"},
                                                   {AGENT_STATE_DEALINGTASK_MOVE, "DEALINGTASK_MOVE"},
                                                   {AGENT_STATE_DEALINGTASK_PRO, "DEALINGTASK_PRO"},
                                                   {AGENT_STATE_DODGE, "DODGE"},
                                                   {AGENT_STATE_PAUSE, "PAUSE"},
                                                   {AGENT_STATE_BREAKDOWN, "BREAKDOWN"} };




Agent::Agent()
{
    m_sName = "";
    m_dID   = 0;
    m_eStatue = AGENT_STATUE_WAIT;
    m_ePreStatue = AGENT_STATUE_WAIT;

    m_fAGVWidth  = 1;
    m_fAGVLength = 1;

    m_fLineVelocity    = 0.3;
    m_fAngularVelocity = 15;

    m_fAccTime  = 1;
    m_fDecTime  = 1;

    m_fPriority = 0;
    m_dDodgeMode  = -1;

    m_pManager = NULL;
    m_pTaskGroup = NULL;
    m_pMap     = NULL;
}

void   Agent::InitializeParameter( Manager* pManager, string sAgentName, int dID, string sIP, AgentStatue  eStatue, float fAGVWidth, float fAGVLength,
                                   float fLineVelocity, float fAngularVelocity, float fAccTime, float fDecTime, float fPriority)
{
    m_pManager = pManager;

    m_sName   = sAgentName;
    m_eStatue = eStatue;
    m_dID     = dID;
    m_sIP     = sIP;

    m_fAGVWidth  = fAGVWidth;
    m_fAGVLength = fAGVLength;

    m_fLineVelocity    = fLineVelocity;
    m_fAngularVelocity = fAngularVelocity;

    m_fAccTime  = fAccTime;
    m_fDecTime  = fDecTime;

    m_fPriority = fPriority;

    m_pMap     = NULL;
}

AgentStatue  Agent::GetStatue()
{
    return  m_eStatue;
}

AgentStatue  Agent::GetPreStatue()
{
    return  m_ePreStatue;
}

bool   Agent::ChangeStatue( AgentStatue eNextStatue )
{
    switch(m_eStatue)
    {
    case AGENT_STATUE_WAIT:
        switch(eNextStatue)
        {
        case AGENT_STATUE_CHARGING:
            m_ePreStatue = m_eStatue;
            m_eStatue    = eNextStatue;
            break;
        case AGENT_STATUE_BREAKDOWN:
            m_ePreStatue = m_eStatue;
            m_eStatue    = eNextStatue;
            break;
        case AGENT_STATUE_PROCESS_NORMAL_RUN:
            m_ePreStatue = m_eStatue;
            m_eStatue    = eNextStatue;
            break;
        default:
            break;
        }
        break;

    case AGENT_STATUE_CHARGING:
        switch(eNextStatue)
        {
        case AGENT_STATUE_WAIT:
            m_ePreStatue = m_eStatue;
            m_eStatue    = eNextStatue;
            break;
        case AGENT_STATUE_BREAKDOWN:
            m_ePreStatue = m_eStatue;
            m_eStatue    = eNextStatue;
            break;
        default:
            break;
        }
        break;

    case AGENT_STATUE_PROCESS_NORMAL_RUN:
        switch(eNextStatue)
        {
        case AGENT_STATUE_WAIT:
            m_ePreStatue = m_eStatue;
            m_eStatue    = eNextStatue;
            break;
        case AGENT_STATUE_BREAKDOWN:
            m_ePreStatue = m_eStatue;
            m_eStatue    = eNextStatue;
            break;

        case AGENT_STATUE_PROCESS_DODGE_GO:
            m_ePreStatue = m_eStatue;
            m_eStatue    = eNextStatue;
            break;

        case AGENT_STATUE_PROCESS_NORMAL_PAUSE:
            m_ePreStatue = m_eStatue;
            m_eStatue    = eNextStatue;
            break;
        default:
            break;
        }
        break;

    case AGENT_STATUE_PROCESS_NORMAL_PAUSE:
        switch(eNextStatue)
        {
        case AGENT_STATUE_PROCESS_NORMAL_RUN:
            m_ePreStatue = m_eStatue;
            m_eStatue    = eNextStatue;
            break;
        case AGENT_STATUE_BREAKDOWN:
            m_ePreStatue = m_eStatue;
            m_eStatue    = eNextStatue;
            break;
        default:
            break;
        }
        break;

    case AGENT_STATUE_PROCESS_DODGE_GO:
        switch(eNextStatue)
        {
        case AGENT_STATUE_PROCESS_DODGE_STOP:
            m_ePreStatue = m_eStatue;
            m_eStatue    = eNextStatue;
            break;
        case AGENT_STATUE_BREAKDOWN:
            m_ePreStatue = m_eStatue;
            m_eStatue    = eNextStatue;
            break;
        default:
            break;
        }
        break;


    case AGENT_STATUE_PROCESS_DODGE_STOP:
        switch(eNextStatue)
        {
        case AGENT_STATUE_PROCESS_DODGE_BACK:
            m_ePreStatue = m_eStatue;
            m_eStatue    = eNextStatue;
            break;
        case AGENT_STATUE_BREAKDOWN:
            m_ePreStatue = m_eStatue;
            m_eStatue    = eNextStatue;
            break;
        default:
            break;
        }
        break;

    case AGENT_STATUE_PROCESS_DODGE_BACK:
        switch(eNextStatue)
        {
        case AGENT_STATUE_PROCESS_NORMAL_RUN:
            m_ePreStatue = m_eStatue;
            m_eStatue    = eNextStatue;
            break;
        case AGENT_STATUE_BREAKDOWN:
            m_ePreStatue = m_eStatue;
            m_eStatue    = eNextStatue;
            break;
        default:
            break;
        }
        break;

    case AGENT_STATUE_BREAKDOWN:
        if(eNextStatue == m_ePreStatue)
        {
            m_eStatue = m_ePreStatue;
            m_ePreStatue = AGENT_STATUE_BREAKDOWN;
        }
        else
        {
            std::cout<<"AGV_ID "<<m_dID<<" is break down now,and pre statue is "<<m_vStatueList[m_ePreStatue]
                     <<".So this AGV can't be change to "<<m_vStatueList[eNextStatue]<<std::endl;
        }
        break;
    default:
        break;
    }
}

Vec2i  Agent::GetCurrentMapPosition()
{
    return  m_sCurMapPos;
}

Vec2i Agent::GetCurrentGridPosition()
{
    return  m_sCurGridPos;
}


void   Agent::SetCurrentPosition(Vec2i CurMapPos)
{
    m_sCurMapPos = CurMapPos;

    if(m_pManager != NULL)
    {
        m_sCurGridPos = m_pMap->Rasterize(CurMapPos);
    }
}

void   Agent::RefleshtPosition()
{
    Vec2i curMapPos;

    //****Connect to agv, and grab it's position****//
    curMapPos = ObtainPosFromAGV();
    SetCurrentPosition(curMapPos);
    //CheckStatueChange();
}

/*
CoordinateList   Agent::CalcPathForDodgeBack()
{
    CoordinateList  dodgeBack;
    CoordinateList  dodgeGo = m_DodgePath.GetPath();
    Vec2i  startGridPos, dodgePos;
    m_DodgePath.GetExtremePoint(startGridPos, dodgePos);
    Task* temphead = m_pTaskGroup->GetTheProcessingTask();

    if((m_dDodgeMode ==3)&&(m_dDodgeMode ==1))
    {
        int  length1 = dodgeBack.size();
        int  length2 = CONFLICT_WINDOWS_HALF * 2 - length1;
        int  dIndex  = temphead->m_path.FindPoint(dodgePos);
        for(int i = dIndex; i < ( dIndex + length2); i++ )
        {
            Vec2i  tempPos;
            if( temphead->m_path.GetPointByID(i,tempPos) )
                dodgeBack.push_back(tempPos);
        }
    }
    else
    {
        for( int i = dodgeGo.size()-1; i>=0; i--)
            dodgeBack.push_back(dodgeGo[i]);

        int  length1 = dodgeBack.size();
        int  length2 = CONFLICT_WINDOWS_HALF * 2 - length1;
        if(length2 > 0)
        {
            int  dIndex  = temphead->m_path.FindPoint(startGridPos);
            for(int i = (dIndex + 1); i < ( dIndex + length2 + 1 ); i++ )
            {
                Vec2i  tempPos;
                if( temphead->m_path.GetPointByID(i,tempPos) )
                    dodgeBack.push_back(tempPos);
            }
        }
        else
        {
            CoordinateList  tempList;
            for(int i=0; i< CONFLICT_WINDOWS_HALF * 2; i++)
                tempList.push_back(dodgeBack[i]);
            dodgeBack = tempList;
        }

    }
    return  dodgeBack;
}


CoordinateList   Agent::CalcPathForDodgeBack()
{
    Vec2i    curGridPos = GetCurrentGridPosition();
    Task*      temphead = m_pTaskGroup->GetTheProcessingTask();
    CoordinateList  curTaskPathList = temphead->m_path.GetPath();

    //find the nearest point on the path from curPos;
    bool  bfind = false;
    int   j =0;
    for( ; j< curTaskPathList.size(); j++ )
    {
        int   delta_x  = abs(curTaskPathList[j].x - curTaskPathList[j].x);
        int   delta_y  = abs(curTaskPathList[j].y - curTaskPathList[j].y);
        bool  bfindCur = false;
        if((1 == delta_x)&&(1 == delta_y))
        {
            bfind = true;
            bfindCur = true;
        }

        if( bfind && !bfindCur )
            break;
    }

    CoordinateList   dodgeBack;
    int dstart=0;
    if( (j-1) >= 0 )
        dstart = j-1;
    else
        dstart = j;
    dodgeBack.push_back(curGridPos);
    for(int i = dstart; i < curTaskPathList.size(); i++ )
        dodgeBack.push_back(curTaskPathList[i]);
}

void   Agent::CheckStatueChange()
{
    AgentStatue eCurStatue = GetStatue();
    Vec2i startGridPos, endGridPos, endMapPos;
    Vec2i curMapPos = GetCurrentMapPosition();


    if(
        (eCurStatue != AGENT_STATUE_CHARGING) &&
        (eCurStatue != AGENT_STATUE_WAIT)     &&
        (eCurStatue != AGENT_STATUE_BREAKDOWN)
            )
    {
        if(eCurStatue == AGENT_STATUE_PROCESS_NORMAL_RUN)
        {
            Task* temphead = m_pTaskGroup->GetTheProcessingTask();
            temphead->m_path.GetExtremePoint(startGridPos, endGridPos);
            endMapPos  = m_pMap->CalculateGridCenter(endGridPos);
            float  dis = m_pMap->CalculateRealDistance(curMapPos, endMapPos);
            TaskStatue   taskStatue = temphead->GetStatue();
            if((MAX_ARRIVAL_DIS > dis) && (TASK_STATUE_MOVEING == taskStatue))
                temphead->ChangeTaskStatue(TASK_STATUE_PROCESS);
        }
        else if(
           ( eCurStatue == AGENT_STATUE_PROCESS_DODGE_GO   )    ||
           ( eCurStatue == AGENT_STATUE_PROCESS_DODGE_STOP )
                )
        {
            m_DodgePath.GetExtremePoint(startGridPos, endGridPos);
            endMapPos  = m_pMap->CalculateGridCenter(endGridPos);
            float  dis = m_pMap->CalculateRealDistance(curMapPos, endMapPos);
            if(MAX_ARRIVAL_DIS > dis)
            {
                if(eCurStatue == AGENT_STATUE_PROCESS_DODGE_GO)
                    ChangeStatue( AGENT_STATUE_PROCESS_DODGE_STOP );
                else if(eCurStatue == AGENT_STATUE_PROCESS_DODGE_STOP)
                {
                    //create a temp path for dodge back test, then use this path for judge if collision clear

                    CoordinateList tempPath= CalcPathForDodgeBack();
                    Path DodgeBackTestPath;
                    Task* temphead = m_pTaskGroup->GetTheProcessingTask();
                    DodgeBackTestPath.SetPath(tempPath);
                    DodgeBackTestPath.SetExtremePoint(tempPath[0],tempPath[tempPath.size()-1]);
                    DodgeBackTestPath.CalPathInflexion();
                    DodgeBackTestPath.SetTimeStampToPath(m_pMap, m_pManager->m_eAPModel,
                                                         GetCurrentGridPosition(), GetCurrentMapPosition(),
                                                         GetStatue(), temphead->GetStatue(),
                                                         m_fAccTime,m_fDecTime,m_fLineVelocity,m_fAngularVelocity,
                                                         m_pMap->GetGridScale());

                    std::vector<Agent*>   otherAGVs;
                    for(int i=0; i< m_pManager->m_vAgentList.size(); i++)
                        if(m_pManager->m_vAgentList[i]->GetID() != m_dID)
                            otherAGVs.push_back((m_pManager->m_vAgentList[i]));

                    int  dsafeNode = m_pMap->TimeOverLap_SpecialAGVPathToOtherAGVs(m_dID, DodgeBackTestPath, otherAGVs);
                    if(dsafeNode > MAX_FINDDODGE_BACKWARDLENGTH)
                    {
                        m_DodgePath = DodgeBackTestPath;
                        ChangeStatue( AGENT_STATUE_PROCESS_DODGE_BACK );
                    }
                }
            }
        }
        else if(eCurStatue == AGENT_STATUE_PROCESS_DODGE_BACK)
        {
            Vec2i  curPos   = GetCurrentGridPosition();
            Task*  temphead = m_pTaskGroup->GetTheProcessingTask();
            if( temphead->m_path.FindPoint(curPos) >= 0 )
            {
                temphead->m_path.SetTimeStampToPath(m_pMap, m_pManager->m_eAPModel,
                                                    GetCurrentGridPosition(), GetCurrentMapPosition(),
                                                    GetStatue(), temphead->GetStatue(),
                                                    m_fAccTime,m_fDecTime,m_fLineVelocity,m_fAngularVelocity,
                                                    m_pMap->GetGridScale( ));
                ChangeStatue( AGENT_STATUE_PROCESS_NORMAL_RUN );
            }
        }

    }
}

*/

void   Agent::GetAccDecTime(float  &fAccTime, float  &fDecTime)
{
    fAccTime = m_fAccTime;
    fDecTime = m_fDecTime;
}

void   Agent::GetMeanVelocity(float &fLineVelocity,float &fAngularVelocity)
{
    fLineVelocity    = m_fLineVelocity;
    fAngularVelocity = m_fAngularVelocity;
}

int    Agent::GetID()
{
    return  m_dID;
}

string Agent::GetName()
{
    return  m_sName;
}

float  Agent::GetPriority()
{
    return  m_fPriority;
}

void   Agent::SetPriority(float fPriority)
{
    m_fPriority = fPriority;
}

/*
void   Agent::GetStatuePath(Path path, AgentStatue eCurStatue)
{
   eCurStatue = GetStatue();

   if(
       (eCurStatue != AGENT_STATUE_CHARGING) &&
       (eCurStatue != AGENT_STATUE_WAIT)     &&
       (eCurStatue != AGENT_STATUE_BREAKDOWN)
           )
   {
       if(eCurStatue == AGENT_STATUE_PROCESS_NORMAL_RUN)
       {
           Task* temphead = m_pTaskGroup->GetTheProcessingTask();
           path = temphead->m_path;
       }
       else if(
          (eCurStatue == AGENT_STATUE_PROCESS_DODGE_GO)    ||
          (eCurStatue == AGENT_STATUE_PROCESS_DODGE_STOP)  ||
          (eCurStatue == AGENT_STATUE_PROCESS_DODGE_BACK)
               )
       {
           path = m_DodgePath;
       }
   }
}


void   Agent::SetTimeStampToPath(AgvPhysicalModel ePModel)
{
    AgentStatue   eAGVStatue  = GetStatue();

    float  fBaseTime, fBaseTimeTurn;
    m_pManager->GetBaseTime( fBaseTime, fBaseTimeTurn);


    if(
        (eAGVStatue != AGENT_STATUE_CHARGING) &&
        (eAGVStatue != AGENT_STATUE_WAIT)
            )
    {
        if( eAGVStatue == AGENT_STATUE_PROCESS_NORMAL_RUN )
        {
            Task* temphead = m_pTaskGroup->GetTheProcessingTask();
            temphead->m_path.SetTimeStampToPath( m_pMap, ePModel, GetCurrentGridPosition(), GetCurrentMapPosition(),
                                                 GetStatue(), temphead->GetStatue(),
                                                 m_fAccTime,m_fDecTime,m_fLineVelocity,m_fAngularVelocity, m_pMap->GetGridScale( ) );
        }
        else if(
           ( eAGVStatue == AGENT_STATUE_PROCESS_DODGE_GO )    ||
           ( eAGVStatue == AGENT_STATUE_PROCESS_DODGE_BACK )
                )
        {
            Task* temphead = m_pTaskGroup->GetTheProcessingTask();
            m_DodgePath.SetTimeStampToPath(m_pMap, ePModel, GetCurrentGridPosition(), GetCurrentMapPosition(),
                                           GetStatue(), temphead->GetStatue(),
                                           m_fAccTime,m_fDecTime,m_fLineVelocity,m_fAngularVelocity, m_pMap->GetGridScale( ) );
        }
        else if( eAGVStatue == AGENT_STATUE_PROCESS_DODGE_STOP )
        {
            time_t now_time;
            now_time = time(NULL);
            CoordinateList Pathlist = m_DodgePath.GetPath();
            Pathlist[Pathlist.size()-1].TimeIn  = now_time;
            Pathlist[Pathlist.size()-1].TimeOut = now_time + CONFLICT_WINDOWS_HALF * fBaseTime;
            m_DodgePath.SetPath(Pathlist);
        }
        else if( eAGVStatue == AGENT_STATUE_BREAKDOWN )
        {
            time_t now_time;
            now_time = time(NULL);
            Vec2i  stopPos  = GetCurrentGridPosition();
            stopPos.TimeIn  = now_time;
            stopPos.TimeOut = now_time + MIN_BREAKDOWN_TIMES * fBaseTime ;
            m_posBreakPause = stopPos;
        }
        else if( eAGVStatue == AGENT_STATUE_PROCESS_NORMAL_PAUSE )
        {
            time_t now_time;
            now_time = time(NULL);
            Vec2i  stopPos  = GetCurrentGridPosition();
            stopPos.TimeIn  = now_time;
            stopPos.TimeOut = now_time + CONFLICT_WINDOWS_HALF * fBaseTime ;
            m_posBreakPause = stopPos;
        }
    }
}
*/

AgentState    Agent::GetState()
{
    return  m_eState;
}

AgentState    Agent::GetPreState()
{
    return  m_ePreState;
}

bool   Agent::ChangeState( AgentState  eNextState )
{
    bool   bflag = false;
    switch(m_eState)
    {
    case AGENT_STATE_WAIT:
        switch(eNextState)
        {
        case AGENT_STATE_CHARGING:
            m_ePreState = m_eState;
            m_eState    = eNextState;
            bflag = true;
            break;
        case AGENT_STATE_DEALINGTASK_MOVE:
            m_ePreState = m_eState;
            m_eState    = eNextState;
            bflag = true;
            break;
        default:
            break;
        }
        break;

    case AGENT_STATE_CHARGING:
        switch(eNextState)
        {
        case AGENT_STATE_WAIT:
            m_ePreState = m_eState;
            m_eState    = eNextState;
            bflag = true;
            break;
        default:
            break;
        }
        break;

    case AGENT_STATE_DEALINGTASK_MOVE:
        switch(eNextState)
        {

        case AGENT_STATE_BREAKDOWN:
            m_ePreState = m_eState;
            m_eState    = eNextState;
            bflag = true;
            break;

        case AGENT_STATE_DODGE:
            m_ePreState = m_eState;
            m_eState    = eNextState;
            bflag = true;
            break;

        case AGENT_STATE_PAUSE:
            m_ePreState = m_eState;
            m_eState    = eNextState;
            bflag = true;
            break;

        case AGENT_STATE_DEALINGTASK_PRO:
            m_ePreState = m_eState;
            m_eState    = eNextState;
            bflag = true;
            break;

        default:
            break;
        }
        break;


    case  AGENT_STATE_DEALINGTASK_PRO:
        switch(eNextState)
        {
        case AGENT_STATE_BREAKDOWN:
            m_ePreState = m_eState;
            m_eState    = eNextState;
            bflag = true;
            break;

        case AGENT_STATE_PAUSE:
            m_ePreState = m_eState;
            m_eState    = eNextState;
            bflag = true;
            break;

        case AGENT_STATE_WAIT:
            m_ePreState = m_eState;
            m_eState    = eNextState;
            bflag = true;
            break;

        default:
            break;
        }
        break;

    case AGENT_STATE_DODGE:
        switch(eNextState)
        {
        case AGENT_STATE_DEALINGTASK_MOVE:
            m_ePreState = m_eState;
            m_eState    = eNextState;
            bflag = true;
            break;
        case AGENT_STATE_PAUSE:
            m_ePreState = m_eState;
            m_eState    = eNextState;
            bflag = true;
            break;
        case AGENT_STATE_BREAKDOWN:
            m_ePreState = m_eState;
            m_eState    = eNextState;
            bflag = true;
            break;
        default:
            break;
        }
        break;

    case AGENT_STATE_PAUSE:
        switch(eNextState)
        {
        case AGENT_STATE_DODGE:
            m_ePreState = m_eState;
            m_eState    = eNextState;
            bflag = true;
            break;
        case AGENT_STATE_DEALINGTASK_MOVE:
            m_ePreState = m_eState;
            m_eState    = eNextState;
            bflag = true;
            break;

        case AGENT_STATE_DEALINGTASK_PRO:
            m_ePreState = m_eState;
            m_eState    = eNextState;
            bflag = true;
            break;

        case AGENT_STATE_BREAKDOWN:
            m_ePreState = m_eState;
            m_eState    = eNextState;
            bflag = true;
            break;
        default:
            break;
        }
        break;

    case AGENT_STATE_BREAKDOWN:
        switch(eNextState)
        {
        case AGENT_STATE_WAIT:
            m_ePreState = m_eState;
            m_eState    = eNextState;
            bflag = true;
            break;
        default:
            std::cout<<"AGV_ID "<<m_dID<<" can't be change to "<<m_vStateList[eNextState]<<std::endl;
            break;
        }
        break;
    default:
        break;
    }

    return  bflag;
}


void  Agent::SendCommandToAGV(std::string  command)
{
    //send command to agv, by socket or other bus
}

string  Agent::ListenFeedBackFromAGV()
{
    //listen feed back from agv, by socket or other bus
}



/*
void   Agent::SetDodgePos(Vec2i DodgePos)
{
    m_sDodgePos = DodgePos;
}

Vec2i  Agent::GetDodgePos()
{
    return  m_sDodgePos;
}

bool   Agent::FindPath()
{
    bool flag = false;
    if(
            ( GetStatue() == AGENT_STATUE_PROCESS_NORMAL ) ||
            ( GetStatue() == AGENT_STATUE_PROCESS_STOP )
       )
    {
        Vec2i  curGridpos  = GetCurrentGridPosition();
        Vec2i  curTaskGoal, curTaskStart;
        m_pTaskGroup->GetTheProcessingTask()->m_path.GetExtremePoint(curTaskStart,curTaskGoal);
        flag = m_pTaskGroup->GetTheProcessingTask()->FindPath(curGridpos, curTaskGoal);
        m_pTaskGroup->GetTheProcessingTask()->SetTimeStampToPath();
    }
    else if( GetStatue() == AGENT_STATUE_PROCESS_DODGE )
    {
        Vec2i  curGridpos  = GetCurrentGridPosition();
        Vec2i  curDodgePos = GetDodgePos();
        flag = m_pTaskGroup->GetTheProcessingTask()->FindPath(curGridpos, curDodgePos);
        m_pTaskGroup->GetTheProcessingTask()->SetTimeStampToPath();
    }

    return flag;
}

bool   Agent::GetPath(CoordinateList& path)
{
    bool  flag = false;
    if(
            ( GetStatue() == AGENT_STATUE_PROCESS_NORMAL ) ||
            ( GetStatue() == AGENT_STATUE_PROCESS_DODGE )  ||
            ( GetStatue() == AGENT_STATUE_PROCESS_STOP )
       )
    {
        path = m_pTaskGroup->GetTheProcessingTask()->GetPath();
        flag = true;
    }
    return  flag;
}
*/

