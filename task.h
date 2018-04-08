#ifndef TASK_H
#define TASK_H

#include "process.h"
#include "AStar.hpp"
#include <map>
#include <string>
#include "path.h"
#include "combpath.h"

using namespace AStar;
using namespace std;


class Agent;
class Manager;
class Task
{
public:
    Task();
    Task( Manager* pManager, Agent* pAgent );

    //void  Execute();
    bool  Start();
    bool  Pause();
    bool  Resume();

    //bool  FindPath( Vec2i  start, Vec2i end );
    //CoordinateList  GetPath();

    //void   SetStart(Vec2i start);
    //Vec2i  GetStart();
    //void   SetGoal(Vec2i goal);
    //Vec2i  GetGoal();

    TaskStatue     GetStatue();
    bool  ChangeTaskStatue(TaskStatue  eNextStatue);

    static std::map<int, std::string>  m_vStatueList;
    Manager*        m_pManager;
    Agent*          m_pAgent;
    Path            m_path;

    std::vector<Process>     m_ActionList;

    //topology map   start
    std::vector<CombPath>    m_combPathList;
    CombPath*  GetExecuteCombPath( );
    void       ExecuteCombPath(CombPath*  pCurExecuteCombPath);
    //topology map    end

    void       ExecuteTask( );



private:

    uint            m_dID;

    //Vec2i           m_sStart;
    //Vec2i           m_sGoal;
    //CoordinateList  m_vPath;


    TaskStatue     m_eCurStatue;
    TaskStatue     m_ePreStatue;


};

#endif // TASK_H
