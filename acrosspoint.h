#ifndef ACROSSPOINT_H
#define ACROSSPOINT_H

#include "AStar.hpp"
#include "combpath.h"

using namespace AStar;

class Path;
class AcrossPoint : public Vec2i
{
public:
    AcrossPoint();
    ~AcrossPoint();

    int  m_dID;
    int  GetJoinNum();

    bool                     m_bOccupyLock;
    std::vector<bool>        m_bDirection;
    std::vector<Path*>       m_pPaths;


    //topology map     start
    bool        OccupyLock();
    bool        ReleaseLock();
    bool        CheckLock();


    std::vector<ApplyInfor>  m_tempApplyList;

    //ApplyInfor*              m_pApplication; //not used
    //void     HandlingApply();  //not used
    int      m_dStepID;

    Path*        m_pProcessingPath;
    ApplyInfor*  m_pProcessingApply;
    //void     TrafficControl();
    //void     TrafficControl_HandlingApply();
    void     HandlingApply_New();
    //topology map     end
};



#endif // ACROSSPOINT_H
