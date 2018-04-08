#include <stdio.h>

#include "map.h"
#include "path.h"
#include "agent.h"
#include "manager.h"
#include "taskgroup.h"

TaskGroup::TaskGroup()
{
    m_pManager = NULL;
    m_pAgent   = NULL;
    m_pTaskHead= NULL;

    m_eTGCurStatue= TG_STATUE_WAIT;
    m_eTGPreStatue= TG_STATUE_WAIT;

}

TaskGroup::TaskGroup( Manager *pManager ): m_pManager(pManager)
{
    m_pAgent   = NULL;
    m_pTaskHead= NULL;

    m_eTGCurStatue= TG_STATUE_WAIT;
    m_eTGPreStatue= TG_STATUE_WAIT;
}

TaskGroup::~TaskGroup()
{
    m_pManager =NULL;
    m_pAgent   =NULL;
}

AStar::TaskGroupStatue   TaskGroup::GetStatue()
{
    return m_eTGCurStatue;
}

bool  TaskGroup::ChangeTaskGroupStatue(AStar::TaskGroupStatue  eNextStatue)
{
    bool  bflag = false;
    switch( m_eTGCurStatue )
    {
    case TG_STATUE_WAIT:
        switch( eNextStatue )
        {
        case TG_STATUE_RUN:
            m_eTGPreStatue = m_eTGCurStatue;
            m_eTGCurStatue = eNextStatue;
            bflag = true;
            break;
        default:
            break;
        }
        break;
    case TG_STATUE_RUN:
        switch(eNextStatue)
        {
        case TG_STATUE_FINISH:
            m_eTGPreStatue = m_eTGCurStatue;
            m_eTGCurStatue = eNextStatue;
            bflag = true;
            break;
        case TG_STATUE_PAUSE:
            m_eTGPreStatue = m_eTGCurStatue;
            m_eTGCurStatue = eNextStatue;
            bflag = true;
            break;
        default:
            break;
        }
        break;
    case TG_STATUE_FINISH:
        break;
    case TG_STATUE_PAUSE:
        switch(eNextStatue)
        {
        case TG_STATUE_WAIT:
            m_eTGPreStatue = m_eTGCurStatue;
            m_eTGCurStatue = eNextStatue;
            bflag = true;
            break;
        case TG_STATUE_RUN:
            m_eTGPreStatue = m_eTGCurStatue;
            m_eTGCurStatue = eNextStatue;
            bflag = true;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    return  bflag;
}


void  TaskGroup::ExecuteTaskGroup()
{
    AgentState eState = m_pAgent->GetState();

    if(( AGENT_STATE_DEALINGTASK_MOVE == eState ) || (AGENT_STATE_DEALINGTASK_PRO == eState))
    {
        if( (NULL == m_pTaskHead) || (TASK_STATUE_FINISH == m_pTaskHead->GetStatue()) )
            analysisTaskCommand();
        else
            m_pTaskHead->ExecuteTask();
    }
    else if( AGENT_STATE_DODGE == eState )
    {
        if( (NULL == m_pTaskHead) || (TASK_STATUE_FINISH == m_pTaskHead->GetStatue()) )
            analysisTaskCommand();
        else
            m_pTaskHead->ExecuteTask();
    }
}


