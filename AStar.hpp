/*
    Copyright (c) 2015, Damian Barczynski <daan.net@wp.eu>
    Following tool is licensed under the terms and conditions of the ISC license.
    For more information visit https://opensource.org/licenses/ISC.
*/
#ifndef __ASTAR_HPP_8F637DB91972F6C878D41D63F7E7214F__
#define __ASTAR_HPP_8F637DB91972F6C878D41D63F7E7214F__

#include <vector>
#include <functional>
#include <set>
#include <opencv2/opencv.hpp>
#include <string>
#include <semaphore.h>

namespace AStar
{
    enum CombPathStatue
    {
        COMBPATH_WAIT          = 0,
        COMBPATH_WAIT_DISPATCH,
        COMBPATH_MOVE_ACROSS,
        COMBPATH_MOVE_PATH,
        COMBPATH_WAIT_NEXT_APPROVE,
        COMBPATH_WAIT_PASS,
        COMBPATH_PASS_OR_END
    };

    enum PriorityBasis
    {
        BASIS_SYNTHETICAL = 0,
        BASIS_FIXED,
        BASIS_RANDOM
    };

    enum AgentStatue
    {
        AGENT_STATUE_WAIT      = 0,
        AGENT_STATUE_CHARGING,
        AGENT_STATUE_PROCESS_NORMAL_RUN,
        AGENT_STATUE_PROCESS_NORMAL_PAUSE,
        AGENT_STATUE_PROCESS_DODGE_GO,
        AGENT_STATUE_PROCESS_DODGE_STOP,
        AGENT_STATUE_PROCESS_DODGE_BACK,
        AGENT_STATUE_BREAKDOWN
    };

    //topology map     start
    enum AgentState
    {
        AGENT_STATE_WAIT      = 0,
        AGENT_STATE_CHARGING,
        AGENT_STATE_DEALINGTASK_MOVE,
        AGENT_STATE_DEALINGTASK_PRO,
        AGENT_STATE_DODGE,
        AGENT_STATE_PAUSE,
        AGENT_STATE_BREAKDOWN
    };
    //topology map     end

    enum AgvPhysicalModel
    {
        AGV_PMODEL_PARTICLE = 0,
        AGV_PMODEL_SUBSTANCE,
    };

    /**
     this enum "Process_Type" used to describe the agent(agv)'s action.

     \param  PT_LASERREFLECTERLOCATION   the agent use the location information from laser reflector to move to the right position.
     \param  PT_CAMERALOCATION           the agent use the location information from camera to move to the right position.
     \param  PT_PLACE                    the agent place goods
     \param  PT_PICKUP                   the agent pick up goods
    */
    enum ProcessType
    {
        PT_LASERREFLO = 0,
        PT_CAMERALO,
        PT_PLACE,
        PT_PICKUP,
        PT_SENDDODGEPOS,
        PT_DODGE
    };

    /**
     this enum "Process_Type" used to describe the action's statue.

     \param  PS_WAIT              the action is wait for process
     \param  PS_ONGOING           the action is in hand.
     \param  PS_STOP              the action is stop
     \param  PS_ERROR             the action is encounter a failure
     \param  PS_FINISH            the action is finish
    */
    enum ProcessStatue
    {
        PS_WAIT= 0,
        PS_ONGOING,
        PS_STOP,
        PS_ERROR,
        PS_FINISH,
        PS_WAIT_ONGOING
    };


    /**
     this enum "SystemRunType" used to describe the system's work statue.

     \param  SYSTEM_REAL   run used real agent.
     \param  SYSTEM_SIMU   simulate.
    */
    enum SystemRunType
    {
        SYSTEM_REAL = 0,
        SYSTEM_SIMU,
    };

    enum TaskStatue
    {
        //TASK_STATUE_WAIT = 0,
        TASK_STATUE_MOVEING =0 ,
        TASK_STATUE_PROCESS,   //grab  putdown
        TASK_STATUE_FINISH,
        TASK_STATUE_PAUSE
    };

    enum TaskGroupStatue
    {
        TG_STATUE_WAIT = 0,
        TG_STATUE_RUN,
        TG_STATUE_FINISH,
        TG_STATUE_PAUSE
    };


    class AgentOccupyTime
    {
    public:
        int     AgentID;
        time_t  TimeIn;
        time_t  TimeOut;
        AgentOccupyTime(){ AgentID = 0; TimeIn = 0; TimeOut = 0;}
        AgentOccupyTime& operator = (const AgentOccupyTime& source)
        { AgentID = source.AgentID;TimeIn  = source.TimeIn;
          TimeOut = source.TimeOut;return  *this; }
    };

    class Conflict
    {
    public:
        int     x, y;
        AgentOccupyTime  conflictAgent_A;
        AgentOccupyTime  conflictAgent_B;

        Conflict(int  x_, int y_, AgentOccupyTime  conflict_A, AgentOccupyTime  conflict_B)
        { x = x_;y = y_; conflictAgent_A = conflict_A; conflictAgent_B = conflict_B; }

        Conflict(){ x=0; y=0; }

    };

    class OccupyTimeElement
    {
    public:
        int     Used;
        int     x, y;
        std::vector<AgentOccupyTime>  AOTList;
        OccupyTimeElement(){ Used =0; x = 0; y= 0; AOTList.clear(); }
    };

    class Size
    {
    public:
        int width,heigh;
        Size(){ width=0; heigh=0; }
        Size& operator = (const Size& source){ width=source.width; heigh=source.heigh; }
    };

    class Vec2i
    {
    public:
        int x, y;
        int angle;

        int rotate_angle;

        time_t  TimeIn;
        time_t  TimeOut;

        Vec2i( ){ x=0; y=0; angle=0; rotate_angle =0; TimeIn=0; TimeOut=0;}
        Vec2i(int X, int Y ){ x=X; y=Y; angle=0; rotate_angle =0; TimeIn=0; TimeOut=0;}
        Vec2i(const Vec2i& source){ x = source.x;
                                    y = source.y;
                                    angle = source.angle;
                                    rotate_angle =source.rotate_angle;
                                    TimeIn=source.TimeIn;
                                    TimeOut=source.TimeOut;}

        bool   operator == (const Vec2i& coordinates_);
        bool   operator != (const Vec2i& coordinates_);
        Vec2i& operator +  (const Vec2i& right_);
        Vec2i& operator =  (const Vec2i& source);
    };

    using uint = unsigned int;
    using HeuristicFunction = std::function<uint(Vec2i, Vec2i)>;
    using CoordinateList    = std::vector<Vec2i>;
    using ConflictMap       = std::map<int,std::vector<Conflict>>;

    struct Node
    {
        uint   G, H;
        Vec2i  coordinates;
        Node   *parent;

        Node();

        Node(Vec2i coord_, Node *parent_ = nullptr);
        uint getScore();
    };

    using NodeSet = std::set<Node*>;

    /*
    class Generator
    {
        bool  detectCollision(Vec2i coordinates_);
        Node* findNodeOnList(NodeSet& nodes_, Vec2i coordinates_);
        void  releaseNodes(NodeSet& nodes_);

    public:
        Generator();
        ~Generator();

        void setWorldSize(Vec2i worldSize_);
        void setDiagonalMovement(bool enable_);
        void setHeuristic(HeuristicFunction heuristic_);
        CoordinateList findPath(Vec2i source_, Vec2i target_);

        void addCollision(Vec2i coordinates_);
        void removeCollision(Vec2i coordinates_);
        void clearCollisions();

    private:
        HeuristicFunction heuristic;
        CoordinateList    direction;
        Vec2i             worldSize;
        u_char            *Obstacal;
        uint directions;
    };
    */

    class Heuristic
    {
        static Vec2i getDelta(Vec2i source_, Vec2i target_);

    public:
        static uint manhattan(Vec2i source_, Vec2i target_);
        static uint euclidean(Vec2i source_, Vec2i target_);
        static uint octagonal(Vec2i source_, Vec2i target_);
    };
}

#endif // __ASTAR_HPP_8F637DB91972F6C878D41D63F7E7214F__
