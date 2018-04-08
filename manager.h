#ifndef MANAGER_H
#define MANAGER_H

#include "agent.h"
#include "task.h"
#include "map.h"
#include "AStar.hpp"
#include "servicepoint.h"
#include "realtaskgroup.h"
#include "dodgetaskgroup.h"

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <iostream>

#define  CONFLICT_WINDOWS_HALF  2
#define  CONFLICT_SIGN_VALUE    100
#define  MAX_PRIO_NUM           400
#define  MAX_ARRIVAL_DIS        0.2    //meter
#define  MIN_DODGETIME_TIMES    CONFLICT_WINDOWS_HALF*2
#define  MIN_BREAKDOWN_TIMES    CONFLICT_WINDOWS_HALF*2
#define  MAX_FINDDODGE_BACKWARDLENGTH   20
#define  PATHLENGTH_PUNISHFACTOR  4

struct  AgentPosCollision
{
    Vec2i  collPos;
    std::vector<Agent*>  agentlist;
};

using AgentConfPos = std::map<Agent*, AgentPosCollision>;

class Manager
{
public:
    Manager();
    ~Manager();

    /*
     * Ask WMS to besure the Stock's element location
    \param   sMatrialName   the matrial that need to be inquiry
    \param   the loaction of this element
    \return  0 this matrial don't exist    1 success   2 the stock's element is run out  3 monopolize source, need wait
   */
    int         InquiryWMS(string sMatrialName,  Vec2i&  cPosition);

    /*
     * tell WMS to release source
    \param   sMatrialName   the source that need to be release
   */
    void        ReleaseWMS(string sMatrialName);


    void        StartDispatch();
    //void        AddAgent(Agent  agv);
    Agent*      FindAgentByID(int  dID);
    void        GetBaseTime( float &fBaseTime, float &fBaseTime_Turn );

    void        AddInforToContent( bool bRunInfor, string inforAdd );
    string      GetInformation( bool bRunInfor );
    void        ClearInforContent( bool bRunInfor );
    void        StaticBaseTime();

    void        ConfigureInitialize();
    void        Initialize_Map(cv::FileStorage  fsr);
    void        Initialize_AGV(cv::FileStorage  fsr);
    void        Initialize_ServicePoints(cv::FileStorage  fsr);
    void        Initialize_WayPoints(cv::FileStorage  fsr);

    std::vector<Map*>   m_pMapList;
    vector<Agent*>      m_vAgentList;
    AgvPhysicalModel    m_eAPModel;

    //topology map     start
    std::vector<ApplyInfor>       m_ApplyList;
    std::vector<DodgeTaskGroup*>  m_pDodgeTaskGroupQueue;
    void           DistributeApply();
    bool           m_bPathLengthPunish;
    //topology map     end

private: 
    void           ListenToMES(std::vector<RealTaskGroup*> pTaskGroupQueue);
    void           CheckSimuRequest(std::vector<RealTaskGroup*> pTaskGroupQueue);
    //void          ReadParaFromConfigureFile(string  configureName);

    static  void  *TimeSliceScan(void *arg);
    //** check  deal  tasks **//
    void           CheckServiceRequest();
    void           DistributeDealTaskGroup();
    void           RefleshAllAgentPos();

    void           SetTimeStamp();
    void           PriorityTactics();
    void           CheckALLAGVStatueChange();
    void           DetectConflict();
    void           DealConflict();

    //void           ExcuteTaskGroup();
    Agent*         GetOneIdleAgent();


    bool           m_bThreadover;
    sem_t          m_uKill_thread;
    pthread_t      m_htid;
    uint           m_dSystemClock;

    float          m_fBaseTime;
    float          m_fBaseTime_Turn;
    PriorityBasis  m_ePriorityBasis;

    ConflictMap    m_ConflictList;
    string         m_stringErrInfor;
    string         m_stringRunInfor;

    std::vector<RealTaskGroup*>    m_pRealTaskGroupQueue;
    std::vector<ServicePoint>      m_ServicePointList;
    std::vector<WayPoint>          m_WayPointList;
    AStar::SystemRunType           m_eSystemRunType;
};

#endif // MANAGER_H
