#include "manager.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>


#define random(x) (rand()%x)

Manager::Manager()
{
    m_eAPModel       = AGV_PMODEL_PARTICLE;
    m_ePriorityBasis = BASIS_SYNTHETICAL;
    m_dSystemClock   = 0;
    m_fBaseTime      = 7;

    m_eSystemRunType = SYSTEM_SIMU;

    m_htid           = 0;
    m_bThreadover    = false;
    sem_init(&m_uKill_thread,0,0);

    m_ConflictList.clear();
}

Manager::~Manager()
{
    if(	m_htid != 0 )
    {
        while(m_bThreadover == false)
        {
            sem_post(&m_uKill_thread);
            usleep(20000);
        }
        m_htid = 0;
    }
    sem_destroy(&m_uKill_thread);

    for(int i=0; i < m_vAgentList.size(); i++)
    {
        free(m_vAgentList[i]);
        m_vAgentList[i] = NULL;
    }

    for(int i=0; i < m_pMapList.size(); i++)
    {
        free(m_pMapList[i]);
        m_pMapList[i] = NULL;
    }

}


void  Manager::StartDispatch()
{

    if(	m_htid != 0 )
    {
        while(m_bThreadover == false)
        {
            sem_post(&m_uKill_thread);
            usleep(20000);
        }
        m_htid = 0;
    }

    if(m_htid == 0)
    {
        /* ��������� producer ������ consumer ���������*/
        /* ��������������������������������������������*/
        pthread_create(&m_htid,NULL,TimeSliceScan,(void*)this );
    }
}

int   Manager::InquiryWMS(string sMatrialName,  Vec2i&  cPosition)
{
    int a =0;
    return a;
}

void  Manager::ReleaseWMS(string sMatrialName)
{
    int a =0;
}

void* Manager::TimeSliceScan( void *arg )
{
    Manager*   pOwner = (Manager*)arg;
    bool  run_flag = true;
    std::cout<<"Manager RUN!"<<std::endl;

    struct timeval  tv1,tv2;
    struct timezone tz;
    gettimeofday(&tv1 , &tz);
    gettimeofday(&tv2 , &tz);


    float fBaseTime,fBaseTime_Turn;
    pOwner->GetBaseTime( fBaseTime, fBaseTime_Turn );
    fBaseTime *=1000000;

    while( run_flag )
    {
        run_flag = sem_trywait(&(pOwner->m_uKill_thread));
        if( run_flag == false )
            break;

        pOwner->CheckServiceRequest();

        gettimeofday(&tv2 , &tz);
        double  dUsedTimeUSec  = (tv2.tv_sec - tv1.tv_sec)*1000000 + (tv2.tv_usec - tv1.tv_usec);

        if(dUsedTimeUSec >= fBaseTime)
        {
            tv1 = tv2;

            pOwner->RefleshAllAgentPos();
            pOwner->DistributeDealTaskGroup();
            pOwner->DistributeApply();


            //pOwner->SetTimeStamp();
            //pOwner->PriorityTactics();
            //pOwner->CheckALLAGVStatueChange();
            //pOwner->DetectConflict();
            //pOwner->DealConflict();
        }
    }

    pOwner->m_bThreadover = true;
    cout<<"Manager TimeSliceScan thread overed!"<<endl;
}

/*
void  Manager::AddAgent(Agent  agv)
{
    m_vAgentList.push_back(agv);
}
*/

Agent*  Manager::FindAgentByID(int  dID)
{
    Agent*  tempHead = NULL;
    for(uint i =0; i < m_vAgentList.size(); i++ )
    {
        if(m_vAgentList[i]->GetID() == dID)
            tempHead = m_vAgentList[i];
    }
    return  tempHead;
}

void  Manager::GetBaseTime( float& fBaseTime, float& fBaseTime_Turn )
{
    fBaseTime      = m_fBaseTime;
    fBaseTime_Turn = m_fBaseTime_Turn;
}

/*
void  Manager::ReadParaFromConfigureFile(string  configureName)
{
    cv::FileStorage   fsr;
    fsr.open( configureName, cv::FileStorage::READ );
}
*/

void  Manager::CheckSimuRequest(std::vector<RealTaskGroup*> pTaskGroupQueue)
{
    if(m_eSystemRunType == SYSTEM_SIMU)
    {
        for(auto it : m_ServicePointList)
        {
            it.CheckSimulationTrigger();
            bool bflag = it.GetCallFlag();
            if( bflag )
            {
                RealTaskGroup*  tempTaskGroup = new RealTaskGroup(this);
                tempTaskGroup->setTaskGroupName(it.m_sName);
                pTaskGroupQueue.push_back(tempTaskGroup);
            }
        }
    }
}

void  Manager::ListenToMES( std::vector<RealTaskGroup*> pTaskGroupQueue )
{
    if(m_eSystemRunType == SYSTEM_REAL)
    {
        //******//
    }
}

void   Manager::CheckServiceRequest()
{
    if(m_eSystemRunType == SYSTEM_REAL)
        ListenToMES(m_pRealTaskGroupQueue);
    else
        CheckSimuRequest(m_pRealTaskGroupQueue);
}

void  Manager::RefleshAllAgentPos()
{
    for(uint i=0; i< m_vAgentList.size(); i++)
        m_vAgentList[i]->RefleshtPosition();
}

void   Manager::AddInforToContent( bool bRunInfor, string inforAdd )
{
    string temp = inforAdd + "\n\r";
    if(bRunInfor)
        m_stringRunInfor += temp;
    else
        m_stringErrInfor += temp;
}

string Manager::GetInformation( bool bRunInfor )
{
    string temp;
    if(bRunInfor)
        temp = m_stringRunInfor;
    else
        temp = m_stringErrInfor;

    return temp;
}

void   Manager::ClearInforContent( bool bRunInfor )
{
    if(bRunInfor)
        m_stringRunInfor.clear();
    else
        m_stringErrInfor.clear();
}



void  Manager::ConfigureInitialize()
{
    string            taskfile = "SystemConfigureFile.yml";
    cv::FileStorage   fsr;

    if( (access( taskfile.c_str(), 0 )) != -1 )
        fsr.open( taskfile, cv::FileStorage::READ );
    else
        cout<<"configure file do't exist!"<<endl;

    string   KeyNode = "APModel";
    string   command_content;
    fsr[KeyNode]>>command_content;
    if( command_content == "PARTICLE" )
        m_eAPModel = AGV_PMODEL_PARTICLE;
    else if( command_content == "SUBSTANCE" )
        m_eAPModel = AGV_PMODEL_SUBSTANCE;

    KeyNode = "PriorityBasis";
    fsr[KeyNode]>>command_content;
    if( command_content == "SYNTHETICAL" )
        m_ePriorityBasis = BASIS_SYNTHETICAL;
    else if( command_content == "FIXED" )
        m_ePriorityBasis = BASIS_FIXED;
    else if( command_content == "RANDOM" )
        m_ePriorityBasis = BASIS_RANDOM;

    KeyNode = "SystemRunType";
    fsr[KeyNode]>>command_content;
    if( command_content == "REAL" )
        m_eSystemRunType = SYSTEM_REAL;
    else if( command_content == "SIMU" )
        m_eSystemRunType = SYSTEM_SIMU;

    Initialize_AGV(fsr);
    Initialize_ServicePoints(fsr);
    Initialize_WayPoints(fsr);

}

void Manager::Initialize_Map(cv::FileStorage  fsr)
{
    string   command_content = "start";
    for(int i = 1; !command_content.empty(); i++)
    {
        char    id[10];
        sprintf(id,"%d",i);
        string   ID  = id;
        string   Key = "Map_";
        string   KeyNode = Key+ID;
        command_content.clear();
        fsr[KeyNode]>>command_content;
        if( !command_content.empty() )
        {
            string  content_copy = command_content;
            int OccurID   = content_copy.find(";");
            string  sName = content_copy.substr(0, OccurID);

            Map*  pMap = new Map();
            pMap->m_GridMap = cv::imread(sName);
            m_pMapList.push_back(pMap);
        }
    }
}


void Manager::Initialize_AGV(cv::FileStorage  fsr)
{
    string   command_content = "start";
    for(int i = 1; !command_content.empty(); i++)
    {
        char    id[10];
        sprintf(id,"%d",i);
        string   ID  = id;
        string   Key = "AGV_";
        string   KeyNode = Key+ID;
        command_content.clear();
        fsr[KeyNode]>>command_content;
        if( !command_content.empty() )
        {
            Agent*  pAGV = new(Agent);

            string  content_copy = command_content;
            int OccurID   = content_copy.find(";");
            string  sName = content_copy.substr(0, OccurID);
            content_copy  = content_copy.substr(OccurID+1, content_copy.size());

            OccurID       = content_copy.find(";");
            string  sID   = content_copy.substr(0, OccurID);
            int     dID   = atoi(sID.c_str());
            content_copy  = content_copy.substr(OccurID+1, content_copy.size());

            OccurID       = content_copy.find(";");
            string  sIPad = content_copy.substr(0, OccurID);
            content_copy  = content_copy.substr(OccurID+1, content_copy.size());

            OccurID        = content_copy.find(";");
            string  sWidth = content_copy.substr(0, OccurID);
            int     dWidth = atoi(sWidth.c_str());
            content_copy   = content_copy.substr(OccurID+1, content_copy.size());

            OccurID        = content_copy.find(";");
            string  sHigh  = content_copy.substr(0, OccurID);
            int     dHigh  = atoi(sHigh.c_str());
            content_copy   = content_copy.substr(OccurID+1, content_copy.size());

            OccurID        = content_copy.find(";");
            string  sLSpeed= content_copy.substr(0, OccurID);
            int     dLSpeed= atoi(sLSpeed.c_str());
            content_copy   = content_copy.substr(OccurID+1, content_copy.size());

            OccurID        = content_copy.find(";");
            string  sASpeed= content_copy.substr(0, OccurID);
            int     dASpeed= atoi(sASpeed.c_str());
            content_copy   = content_copy.substr(OccurID+1, content_copy.size());

            OccurID        = content_copy.find(";");
            string  sAcTime= content_copy.substr(0, OccurID);
            int     dAcTime= atoi(sAcTime.c_str());
            content_copy   = content_copy.substr(OccurID+1, content_copy.size());

            OccurID        = content_copy.find(";");
            string  sDcTime= content_copy.substr(0, OccurID);
            int     dDcTime= atoi(sDcTime.c_str());
            content_copy   = content_copy.substr(OccurID+1, content_copy.size());

            pAGV->InitializeParameter( this, sName, dID, sIPad, AGENT_STATUE_WAIT, dWidth,
                                       dHigh, dLSpeed, dASpeed, dAcTime, dDcTime, 0);

            m_vAgentList.push_back(pAGV);
        }
    }

}

void Manager::Initialize_ServicePoints(cv::FileStorage  fsr)
{
    string   command_content = "start";
    for(int i = 1; !command_content.empty(); i++)
    {
        char    id[10];
        sprintf(id,"%d",i);
        string   ID  = id;
        string   Key = "ServicePoint_";
        string   KeyNode = Key+ID;
        command_content.clear();
        fsr[KeyNode]>>command_content;
        if( !command_content.empty() )
        {
            string  content_copy = command_content;
            int OccurID   = content_copy.find(";");
            string  sName = content_copy.substr(0, OccurID);
            content_copy  = content_copy.substr(OccurID+1, content_copy.size());

            OccurID       = content_copy.find(";");
            string  sCallPitchTime   = content_copy.substr(0, OccurID);
            int     dCallPitchTime   = atoi(sCallPitchTime.c_str());
            content_copy  = content_copy.substr(OccurID+1, content_copy.size());

            OccurID      = content_copy.find(",");
            string  sL_X = content_copy.substr(0, OccurID);
            int     dL_X = atoi(sL_X.c_str());
            content_copy = content_copy.substr(OccurID+1, content_copy.size());

            OccurID      = content_copy.find(";");
            string  sL_Y = content_copy.substr(0, OccurID);
            int     dL_Y = atoi(sL_Y.c_str());
            content_copy = content_copy.substr(OccurID+1, content_copy.size());

            ServicePoint  tempPoint(sName,false,dCallPitchTime);
            tempPoint.x = dL_X;
            tempPoint.y = dL_Y;
            m_ServicePointList.push_back(tempPoint);
        }
    }
}

void Manager::Initialize_WayPoints(cv::FileStorage  fsr)
{
    string   command_content = "start";
    for(int i = 1; !command_content.empty(); i++)
    {
        char    id[10];
        sprintf(id,"%d",i);
        string   ID  = id;
        string   Key = "WayPoint_";
        string   KeyNode = Key+ID;
        command_content.clear();
        fsr[KeyNode]>>command_content;
        if( !command_content.empty() )
        {
            string  content_copy = command_content;
            int OccurID   = content_copy.find(";");
            string  sName = content_copy.substr(0, OccurID);
            content_copy  = content_copy.substr(OccurID+1, content_copy.size());

            OccurID      = content_copy.find(",");
            string  sL_X = content_copy.substr(0, OccurID);
            int     dL_X = atoi(sL_X.c_str());
            content_copy = content_copy.substr(OccurID+1, content_copy.size());

            OccurID      = content_copy.find(";");
            string  sL_Y = content_copy.substr(0, OccurID);
            int     dL_Y = atoi(sL_Y.c_str());
            content_copy = content_copy.substr(OccurID+1, content_copy.size());

            WayPoint  tempPoint;
            tempPoint.m_sName = sName;
            tempPoint.x = dL_X;
            tempPoint.y = dL_Y;
            m_WayPointList.push_back(tempPoint);
        }
    }
}

void Manager::StaticBaseTime()
{
    float fLength = m_pMapList[0]->GetGridScale();
    float fMaxVelocity, fMaxAngularVelocity;
    fMaxVelocity=fMaxAngularVelocity=0;
    float fMinAcctime,fMinDectime;
    fMinAcctime = fMinDectime = 1000;
    for(auto tempAgent : m_vAgentList)
    {
        float fLineVelocity,fAngularVelocity;
        tempAgent->GetMeanVelocity( fLineVelocity,fAngularVelocity);
        if(fMaxVelocity <  fLineVelocity)
            fMaxVelocity =  fLineVelocity;
        if(fMaxAngularVelocity < fAngularVelocity)
            fMaxAngularVelocity = fAngularVelocity;

        float fAcctime,fDectime;
        tempAgent->GetAccDecTime( fAcctime,fDectime );
        if(fMinAcctime > fAcctime)
            fMinAcctime = fAcctime;
        if(fMinDectime > fDectime)
            fMinDectime = fDectime;
    }

    float fBaseTime;
    if(fMaxVelocity!=0)
        fBaseTime = fLength / fMaxVelocity;
    else
        fBaseTime = 7;    //7 seconds


    float fBaseTime_turn;
    if(fMaxAngularVelocity!=0)
        fBaseTime_turn =  2*fMinAcctime + 2*fMinDectime + fLength / fMaxVelocity + 90 / fMaxAngularVelocity;
    else
        fBaseTime_turn = 14;    //7 seconds
}

/*
void  Manager::SetTimeStamp()
{
    for(uint i =0; i < m_vAgentList.size(); i++)
        m_vAgentList[i]->SetTimeStampToPath( m_eAPModel );
}

void   Manager::CheckALLAGVStatueChange()
{
    for(uint i =0; i < m_vAgentList.size(); i++)
        m_vAgentList[i]->CheckStatueChange();
}


void   Manager::PriorityTactics()
{
    int*   tempPriority = new int[m_vAgentList.size()];

    switch(m_ePriorityBasis)
    {
    case BASIS_SYNTHETICAL:
        {
            uint   MaxPathLength = 0;
            for(uint i =0; i < m_vAgentList.size(); i++)
            {
                Path   path;
                AgentStatue eCurStatue;
                m_vAgentList[i]->GetStatuePath(path, eCurStatue);
                int  dlength = path.GetRemainderPathLength();
                if(MaxPathLength < dlength)
                    MaxPathLength = dlength;
            }

            if(MaxPathLength !=0 )
            {
                float one_fourth_MAX_PRIO_NUM = MAX_PRIO_NUM / 4;
                float fscale = one_fourth_MAX_PRIO_NUM / MaxPathLength;
                for(uint i =0; i < m_vAgentList.size(); i++)
                {
                    Path   path;
                    AgentStatue eCurStatue;
                    m_vAgentList[i]->GetStatuePath(path, eCurStatue);
                    float fPrio = one_fourth_MAX_PRIO_NUM * 2 - path.GetRemainderPathLength() * fscale;
                    if(fPrio < 0)
                        fPrio = 0;

                    if(
                          (eCurStatue == AGENT_STATUE_PROCESS_DODGE_GO)    ||
                          (eCurStatue == AGENT_STATUE_PROCESS_DODGE_STOP)  ||
                          (eCurStatue == AGENT_STATUE_PROCESS_DODGE_BACK)
                       )
                        fPrio += one_fourth_MAX_PRIO_NUM;

                    tempPriority[i] = fPrio;
                    m_vAgentList[i]->SetPriority(tempPriority[i]);
                }

                for(uint i =0; i< (m_vAgentList.size() - 1); i++)
                {
                    float  pre_data = m_vAgentList[i]->GetPriority();
                    float  fValue   = pre_data;
                    for( uint j = (i+1); j< m_vAgentList.size(); j++ )
                    {
                        float  cur_data = m_vAgentList[j]->GetPriority();
                        if( cur_data == pre_data )
                            if( (fValue + 0.01) < MAX_PRIO_NUM )
                            {
                                m_vAgentList[j]->SetPriority(fValue + 0.01);
                                fValue += 0.01;
                            }
                    }
                }
            }
            else
            {
                for(uint i =0; i < m_vAgentList.size(); i++)
                    m_vAgentList[i]->SetPriority(i);
            }
        }
        break;

    case BASIS_FIXED:
        break;

    case BASIS_RANDOM:
        memset(tempPriority, 0, sizeof(int) * m_vAgentList.size());
        for(uint i =0; i < m_vAgentList.size(); i++)
        {
            tempPriority[i] = random( MAX_PRIO_NUM );
            bool  bsame = true;
            while(bsame)
            {
                uint j =0;
                for( ; j< i; j++)
                {
                    if(tempPriority[j] == tempPriority[i])
                        break;
                }

                if(j == i)
                    bsame = false;
                else
                    tempPriority[i] = random(MAX_PRIO_NUM);
            }
            m_vAgentList[i]->SetPriority(tempPriority[i]);
        }
        break;

    default:
        break;
    }

    free(tempPriority);
    tempPriority = NULL;
}
*/


void  Manager::DistributeApply( )
{
    for(int i =0; i < m_ApplyList.size(); i++)
    {
        Path*   pPath = m_ApplyList[i].pCombPath->m_pPath;
        pPath->m_ApplyList.push_back(m_ApplyList[i]);
    }
    m_ApplyList.clear();

    for( auto  it : m_pMapList )
        it->DealApplyList();
}
/*
void   Manager::ExcuteTaskGroup()
{
    for(int i =0; i < m_vAgentList.size(); i++ )
    {
        Agent*      pAgent  = m_vAgentList[i];
        AgentState  eStatue = pAgent->GetState();
        if( (AGENT_STATE_DODGE == eStatue) ||
            (AGENT_STATE_DEALINGTASK_MOVE == eStatue) )
            pAgent->MoveToNextPos();
    }
}
*/
Agent* Manager::GetOneIdleAgent()
{
    Agent*  pAgent = NULL;
    for( int i = 0;  i< m_vAgentList.size(); i++ )
    {
        AgentState eState = m_vAgentList[i]->GetState();
        if( (AGENT_STATE_WAIT == eState) || (AGENT_STATE_CHARGING == eState) )
        {
            pAgent = m_vAgentList[i];
            break;
        }
    }

    return pAgent;
}

void   Manager::DistributeDealTaskGroup()
{
    std::vector<RealTaskGroup*>::iterator    iter;
    for( iter =m_pRealTaskGroupQueue.begin(); iter != m_pRealTaskGroupQueue.end(); iter++ )
    {
        RealTaskGroup*      pTaskGroup = *iter;
        TaskGroupStatue      eTGStatue = pTaskGroup->GetStatue();

        switch( eTGStatue )
        {
        case TG_STATUE_WAIT:
            {
                //get one idel AGV, then Distribute this Task To this AGV
                Agent*  pAgent = GetOneIdleAgent();
                if( NULL != pAgent )
                {
                    //correlation AGV and taskgroup
                    pTaskGroup->m_pAgent = pAgent;
                    //change agv and taskgroup statue
                    pTaskGroup->ChangeTaskGroupStatue( TG_STATUE_RUN );
                    pAgent->ChangeState( AGENT_STATE_DEALINGTASK_MOVE );
                }
            }
            break;

        case TG_STATUE_RUN:
            {
                Agent*      pAgent = pTaskGroup->m_pAgent;
                AgentState  eState = pAgent->GetState();
                if((AGENT_STATE_DEALINGTASK_MOVE==eState)||(AGENT_STATE_DEALINGTASK_PRO==eState))
                {
                    pTaskGroup->ExecuteTaskGroup();
                }
            }

            break;

        //if this task group is finish, then release the source
        case TG_STATUE_FINISH:
            {
                Agent* pAGV = pTaskGroup->m_pAgent;
                pAGV->m_pTaskGroup = NULL;
                pAGV->ChangeState( AGENT_STATE_WAIT );
                m_pRealTaskGroupQueue.erase( iter );
                free( pTaskGroup );
            }
            break;

        case TG_STATUE_PAUSE:
            break;

        default:
            break;
        }
    }

    std::vector<DodgeTaskGroup*>::iterator   iter_A;
    for( iter_A =m_pDodgeTaskGroupQueue.begin(); iter_A != m_pDodgeTaskGroupQueue.end(); iter_A++ )
    {
        DodgeTaskGroup*  pTaskGroup = *iter_A;
        TaskGroupStatue   eTGStatue = pTaskGroup->GetStatue();

        switch( eTGStatue )
        {
        case TG_STATUE_WAIT:
            break;

        case TG_STATUE_RUN:
            {
                Agent*      pAgent = pTaskGroup->m_pAgent;
                AgentState  eState = pAgent->GetState();
                if(AGENT_STATE_DODGE == eState)
                {
                    pTaskGroup->ExecuteTaskGroup();
                }
            }
            break;

        //if this task group is finish, then release the source
        case TG_STATUE_FINISH:
            {
                Agent* pAGV = pTaskGroup->m_pAgent;
                pAGV->m_pTaskGroup = NULL;
                pAGV->ChangeState( pAGV->GetPreState() );
                m_pDodgeTaskGroupQueue.erase( iter_A );
                free( pTaskGroup );
            }
            break;

        case TG_STATUE_PAUSE:
            break;

        default:
            break;
        }
    }
}
