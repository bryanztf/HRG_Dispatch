#ifndef TASKGROUP_H
#define TASKGROUP_H

#include <vector>
#include <opencv2/opencv.hpp>
#include "AStar.hpp"

class Task;
class Map;
class Manager;
class Agent;
class TaskGroup
{
public:
    TaskGroup();
    TaskGroup( Manager *pManager );

    ~TaskGroup();

    AStar::TaskGroupStatue     GetStatue();
    bool  ChangeTaskGroupStatue(AStar::TaskGroupStatue  eNextStatue);

    virtual  Task* GetCurrentTask(){};
    virtual  void  analysisTaskCommand(){};
    virtual  void  initializerTaskCommand(){};

    void  ExecuteTaskGroup();

    Manager*               m_pManager;
    Agent*                 m_pAgent;

    Task*                  m_pTaskHead;
    AStar::TaskGroupStatue m_eTGCurStatue;
    AStar::TaskGroupStatue m_eTGPreStatue;

    std::map<std::string, bool>  m_VarBolList;
};

#endif // TASKGROUP_H
