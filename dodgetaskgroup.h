#ifndef DODGETASKGROUP_H
#define DODGETASKGROUP_H

#include "AStar.hpp"
#include "taskgroup.h"

using namespace AStar;

class DodgeTaskGroup : public TaskGroup
{
public:
    DodgeTaskGroup();
    DodgeTaskGroup( Manager *pManager );

    ~DodgeTaskGroup();

    void     SetKeyPar( Vec2i  pathPos, int dDodgePosID );

    virtual  Task* GetCurrentTask();
    virtual  void  analysisTaskCommand();
    virtual  void  initializerTaskCommand();

    void     ReleaseTaskList();

private:

    Vec2i    m_pathPos;
    int      m_DodgePosID;

    std::vector<Task*>  m_pTaskList;
};

#endif // DODGETASKGROUP_H
