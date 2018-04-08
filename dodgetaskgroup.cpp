#include "dodgetaskgroup.h"
#include "manager.h"

DodgeTaskGroup::DodgeTaskGroup()
{
    m_pTaskList.clear();
}

DodgeTaskGroup::DodgeTaskGroup( Manager *pManager ):TaskGroup( pManager )
{
    m_pTaskList.clear();
}

DodgeTaskGroup::~DodgeTaskGroup( )
{
    ReleaseTaskList();
}


void  DodgeTaskGroup::SetKeyPar( Vec2i  pathPos, int dDodgePosID )
{
    m_pathPos    = pathPos;
    m_DodgePosID = dDodgePosID;
}

Task* DodgeTaskGroup::GetCurrentTask()
{
    Task*  tempHead = NULL;
    for(int i=0; i < m_pTaskList.size(); i++)
    {
        if( TASK_STATUE_FINISH != m_pTaskList[i]->GetStatue() )
        {
            tempHead = m_pTaskList[i];
            m_pTaskHead = tempHead;
            break;
        }
    }

    return tempHead;
}

void  DodgeTaskGroup::analysisTaskCommand()
{
    GetCurrentTask();

    bool  bFinish = true;
    for(int i=0; i < m_pTaskList.size(); i++)
    {
        if( TASK_STATUE_FINISH != m_pTaskList[i]->GetStatue() )
        {
            bFinish = false;
            break;
        }
    }

    if(bFinish)
        ChangeTaskGroupStatue(TG_STATUE_FINISH);
}

void  DodgeTaskGroup::initializerTaskCommand()
{
    Task*  pTaskHead = new Task( m_pManager, m_pAgent );
    Vec2i   startPos = m_pAgent->GetCurrentGridPosition();
    m_pAgent->m_pMap->GeneralSubPath(startPos, m_pathPos, pTaskHead->m_combPathList );
    pTaskHead->m_ActionList.clear();
    ProcessType  proType = PT_SENDDODGEPOS;
    Process  tempPro1( proType, this);
    Vec2i  dodgePos    = m_pAgent->m_pMap->CalculateGridCenter(m_pAgent->m_pMap->m_DodgePosList[m_DodgePosID]);
    char   commandChar_1[50];
    sprintf(commandChar_1, " %d %d", dodgePos.x, dodgePos.y);
    string  sCommandPlus = commandChar_1;
    tempPro1.SetCommandPlusName(sCommandPlus);
    pTaskHead->m_ActionList.push_back(tempPro1);
    m_pTaskList.push_back(pTaskHead);

    pTaskHead = new Task( m_pManager, m_pAgent );
    pTaskHead->ChangeTaskStatue(TASK_STATUE_PROCESS);
    pTaskHead->m_ActionList.clear();
    proType = PT_DODGE;
    Process  tempPro2( proType, this);
    pTaskHead->m_ActionList.push_back(tempPro2);
    m_pTaskList.push_back(pTaskHead);

    pTaskHead = new Task( m_pManager, m_pAgent );
    pTaskHead->ChangeTaskStatue(TASK_STATUE_PROCESS);
    pTaskHead->m_ActionList.clear();
    proType = PT_SENDDODGEPOS;
    Process  tempPro3( proType, this);
    Vec2i  pathPos    = m_pAgent->m_pMap->CalculateGridCenter(m_pathPos);
    char   commandChar_2[50];
    sprintf(commandChar_2, " %d %d", pathPos.x, pathPos.y);
    sCommandPlus = commandChar_2;
    tempPro3.SetCommandPlusName(sCommandPlus);
    pTaskHead->m_ActionList.push_back(tempPro3);
    m_pTaskList.push_back(pTaskHead);
}

void  DodgeTaskGroup::ReleaseTaskList()
{
    for(int i=0; i < m_pTaskList.size(); i++)
    {
        Task* tempHead = m_pTaskList[i];
        if(NULL != tempHead)
        {
            free(tempHead);
            m_pTaskList[i] = NULL;
        }
    }
    m_pTaskList.clear();
}
