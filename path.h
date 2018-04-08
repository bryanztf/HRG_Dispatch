#ifndef PATH_H
#define PATH_H

#include <vector>
#include <opencv2/opencv.hpp>
#include "AStar.hpp"
#include "combpath.h"

using namespace std;
using namespace AStar;

class Map;
class Agent;
class Path
{
public:
    Path( );
    Path( const Path& source );
    Path&  operator = ( const Path& source );
    ~Path( );

    int    GetPathSize();
    int    GetRemainderPathLength();
    Vec2i  GetCurGridPosIndex(int & dIndex);
    int    GetSpeGridPosIndex(Vec2i specialGridPos);
    void   SetPath( CoordinateList Pathlist );
    CoordinateList   GetPath( );
    void   SetExtremePoint( Vec2i ExtremePos1, Vec2i ExtremePos2 );
    void   GetExtremePoint( Vec2i& ExtremePos1, Vec2i& ExtremePos2 );
    void   CalPathInflexion();
    int    FindPoint(Vec2i elementPos);  //return  -1 failed
    bool   GetPointByID(int posID, Vec2i &result);
    void   SetTimeStampToPath( Map* pMap, AgvPhysicalModel eModel, Vec2i curGridPos, Vec2i curMapPos, AgentStatue  eAgentStatue,
                               TaskStatue eTaskStatue, float Acctime,float Dectime,float fLineVelocity,float fAngularVelocity,float fScale);

    //sem_wait(&pOwner->m_pDipper->m_uTextBufferLock);
    //sem_post(&pOwner->m_pDipper->m_uTextBufferLock);
    //run_flag = sem_trywait(&(pOwner->m_uKill_thread));
    

    //topology map   start
    //void   CheckDirection();
    bool        m_bMonopoly;       //if path just have one ExtremePos linked to across

    bool        m_bLock;           // m_bMonopoly is true,this sem used to describe occupy;
                                   // m_bMonopoly is false,this sem used to describe confict occur

    int         m_dDirection;      //-1 none agent used this path    0  Direction is to m_dAcrossID[0]   1  Direction is to m_dAcrossID[1]
    int         m_dAcrossID[2];

    std::vector<Agent*>       m_AGVOccupyList;
    std::vector<Agent*>       m_AGVDodgeList;
    std::vector<int>          m_DodgePosIDList;    //from  m_ExtremePos[0] to m_ExtremePos[1]
    std::vector<ApplyInfor>   m_ApplyList;
    void   ApplyOnlyPath( ApplyInfor*  pApplication );

    bool   OccupyLock();
    bool   ReleaseLock();
    bool   CheckLock();

    int    GetSpeGridPosIndexWithDirection(Vec2i specialGridPos, int  dDirection);
    Vec2i  GetGridPosFromIndexDirection(int  dIndex, int  dDirection);
    void   SendOccupyAGVtoDodgeMode();

    bool   IfAGVAlreadyOnThisPath(Agent* pAGV);

    bool   ResetDirection();
    bool   SetDirection(int dDirection, Agent* pAgent = NULL);

    float  AverageOccupyAGVPriority();
    
    bool   CalDirection( int targetAcrossID, int &dDirection, Vec2i& extPos);
    bool   CalDirection( Vec2i staPos, Vec2i endPos, int &dDirection);
    int    CalLengthTwoPos(Vec2i  staNode_, Vec2i  endNode_, int& dDirection_);
    bool   ReleaseAgent(Agent* pPassedAgent);
    bool   AddAgent(Agent* pPassedAgent);
    float  GetMaxPriorityAgent();
    cv::Scalar  m_color;
    //topology map    end

    Map*        m_pMap;
private:
    void   CalPathInflexion_Point( Vec2i&  Pre_Node, Vec2i&  Cur_Node, Vec2i  Nex_Node );

    Vec2i            m_ExtremePos[2];
    CoordinateList   m_Pathlist;

};

#endif // PATH_H
