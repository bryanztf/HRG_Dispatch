#ifndef COMBPATH_H
#define COMBPATH_H

#include "AStar.hpp"

using namespace AStar;



class Agent;
class Path;
class AcrossPoint;
class CombPath
{
public:
    CombPath();
    CombPath( AcrossPoint*  pAcross_, Vec2i staPos_,Vec2i endPos_, int dDirection_, Path* pPath_, CombPathStatue statue_, int dID_);

    AcrossPoint*   m_pAcross;

    Vec2i          m_staPos;
    Vec2i          m_endPos;
    int            m_dDirection;  //this move need path's direction
                                  //0  Direction is to m_dAcrossID[0]
                                  //1  Direction is to m_dAcrossID[1]

    Path*          m_pPath;
    int            m_dID;

    bool           ChangeStatue(CombPathStatue  dNextStatue);
    CombPathStatue GetStatue();

private:
    CombPathStatue m_statue;    //0 WAIT {receive apply}  1 WAIT_DISPATCH {approve: across & path & agent Priority ; if is frist apply jump 3}
                                //2 MOVE_ACROSS {check pass across;set pre apply pass 7} 3 MOVE_PATH {check arrive at path end; if is last apply jump 7}
                                //4 WAIT_NEXT_APPROVE{set next applyto 1;then wait next apply approve(change to 2)}
                                //5 WAIT_PASS{when next apply pass across}  6 PASS_OR_END

};


struct  ApplyInfor
{
    Agent     *pAgent;
    CombPath  *pCombPath;
};


#endif // COMBPATH_H
