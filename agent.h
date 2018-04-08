#ifndef AGENT_H
#define AGENT_H

#include <string>
#include "task.h"
#include "AStar.hpp"
#include "realtaskgroup.h"
#include "agent.h"

using namespace std;
using namespace AStar;


class Manager;
class Agent
{
public:
    Agent();

    void   InitializeParameter( Manager* pManager, string sAgentName, int dID, string sIP, AgentStatue  eStatue, float fAGVWidth, float fAGVLength,
                                float fLineVelocity, float fAngularVelocity, float fAccTime, float fDecTime, float fPriority);

    AgentStatue  GetStatue();
    AgentStatue  GetPreStatue();
    bool   ChangeStatue( AgentStatue  eNextStatue );

    void   SetCurrentPosition(Vec2i CurMapPos);
    Vec2i  GetCurrentMapPosition();
    Vec2i  GetCurrentGridPosition();
    void   GetAccDecTime(float  &fAccTime, float  &fDecTime);
    void   GetMeanVelocity(float &fLineVelocity,float &fAngularVelocity);
    void   SetTimeStampToPath(AgvPhysicalModel  ePModel);
    float  GetPriority();
    void   SetPriority(float fPriority);

    void   GetStatuePath(Path path, AgentStatue eCurStatue);

    int    GetID();
    string GetName();

    //void   MoveToNextPos();

    //void   SetDodgePos(Vec2i DodgePos);
    //Vec2i  GetDodgePos();

    void   RefleshtPosition();
    virtual Vec2i  ObtainPosFromAGV(){}
    void   CheckStatueChange();
    CoordinateList CalcPathForDodgeBack();

    void    SendCommandToAGV(std::string  command);
    string  ListenFeedBackFromAGV();

    //topology map     start
    AgentState    m_eState;
    AgentState    m_ePreState;
    static std::map<int, std::string>  m_vStateList;

    AgentState    GetState();
    AgentState    GetPreState();
    bool   ChangeState( AgentState  eNextState );
    //topology map     end


    Manager*       m_pManager;
    Map*           m_pMap;
    RealTaskGroup*     m_pTaskGroup;

    Path           m_DodgePath;
    int            m_dDodgeMode;
    Vec2i          m_posBreakPause;

    static std::map<int, std::string>  m_vStatueList;
private:
    string         m_sName;
    string         m_sIP;
    int            m_dID;
    AgentStatue    m_eStatue;
    AgentStatue    m_ePreStatue;

    //agv's contour
    float          m_fAGVWidth;
    float          m_fAGVLength;

    float          m_fLineVelocity;
    float          m_fAngularVelocity;

    float          m_fAccTime;
    float          m_fDecTime;

    float          m_fPriority;
    Vec2i          m_sCurMapPos;
    Vec2i          m_sCurGridPos;
    //Vec2i          m_sDodgePos;
};

#endif // AGENT_H
