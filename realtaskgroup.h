#ifndef REALTASKGROUP_H
#define REALTASKGROUP_H

#include "AStar.hpp"
#include "taskgroup.h"

class Task;
class Map;
class Manager;
class Agent;
class RealTaskGroup : public TaskGroup
{
public:
    RealTaskGroup();
    RealTaskGroup( Manager *pManager );

    ~RealTaskGroup();

    virtual Task*    GetCurrentTask();
    virtual void     analysisTaskCommand();
    virtual void     initializerTaskCommand();

    void  setTaskGroupName(std::string missionName);


 private:
    void  closedTaskDescribeFile( );
    bool  analysisTaskCommand_move( Task* pTaskHead, std::string command_content );
    bool  analysisTaskCommand_move_para( std::string& paraName, AStar::Vec2i& position );
    bool  analysisTaskCommand_define( std::string command_content );
    void  analysisTaskCommand_proc( Task* pTaskHead, std::string command_content );
    bool  analysisTaskCommand_judge(std::string command_content);

    cv::FileStorage                       m_fsr;
    std::string                           m_name;
    uint                                  m_dCommandID;
    std::map<std::string, AStar::Vec2i>   m_VarPosList;
    std::string                           m_sDefineName;


};

#endif // REALTASKGROUP_H
