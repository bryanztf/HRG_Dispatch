#include "combpath.h"

CombPath::CombPath()
{
    m_pAcross    = NULL;
    m_dDirection = -1;
    m_pPath      = NULL;
    m_statue     = COMBPATH_WAIT;
    m_dID        = -1;
}

CombPath::CombPath( AcrossPoint*  pAcross_,Vec2i staPos_,Vec2i endPos_,int dDirection_, Path* pPath_,CombPathStatue statue_, int dID_)
{
    m_staPos= staPos_;
    m_endPos= endPos_;
    m_dDirection = dDirection_;
    m_pPath = pPath_;
    m_pAcross = pAcross_;
    m_statue = statue_;
    m_dID    = dID_;
}

CombPathStatue  CombPath::GetStatue()
{
    return m_statue;
}

bool  CombPath::ChangeStatue(CombPathStatue  dNextStatue)
{
    bool   flag = false;

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

    switch(m_statue)
    {
    case COMBPATH_WAIT:
        {
            switch(dNextStatue)
            {
            case COMBPATH_WAIT_DISPATCH:
                m_statue = dNextStatue;
                flag = true;
                break;
            default:
                break;
            }
        }
        break;

    case COMBPATH_WAIT_DISPATCH:
        {
            switch(dNextStatue)
            {
            case COMBPATH_MOVE_ACROSS:
                m_statue = dNextStatue;
                flag = true;
                break;

            case COMBPATH_MOVE_PATH:
                m_statue = dNextStatue;
                flag = true;
                break;

            default:
                break;
            }
        }
        break;

    case COMBPATH_MOVE_ACROSS:
        {
            switch(dNextStatue)
            {
            case COMBPATH_MOVE_PATH:
                m_statue = dNextStatue;
                flag = true;
                break;
            default:
                break;
            }
        }
        break;

    case COMBPATH_MOVE_PATH:
        {
            switch(dNextStatue)
            {
            case COMBPATH_WAIT_NEXT_APPROVE:
                m_statue = dNextStatue;
                flag = true;
                break;

            case COMBPATH_PASS_OR_END:
                m_statue = dNextStatue;
                flag = true;
                break;

            default:
                break;
            }
        }
        break;

    case COMBPATH_WAIT_NEXT_APPROVE:
        {
            switch(dNextStatue)
            {
            case COMBPATH_WAIT_PASS:
                m_statue = dNextStatue;
                flag = true;
                break;

            default:
                break;
            }
        }
        break;


    case COMBPATH_WAIT_PASS:
        {
            switch(dNextStatue)
            {
            case COMBPATH_PASS_OR_END:
                m_statue = dNextStatue;
                flag = true;
                break;

            default:
                break;
            }
        }
        break;


    case COMBPATH_PASS_OR_END:
        break;

    default:
        break;

    }

    return  flag;
}
