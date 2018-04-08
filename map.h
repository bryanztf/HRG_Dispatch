#ifndef MAP_H
#define MAP_H

#include "AStar.hpp"
#include <opencv2/opencv.hpp>
#include <string>
#include "path.h"
#include "acrosspoint.h"
#include "combpath.h"
#include "dodgepos.h"

using namespace AStar;


class Manager;

class Agent;
class Map
{
public:
    Map();
    ~Map();

    void   CreatGridMap( std::string mapConfigFile, float GridScale );

    void   AddOneFixedPath( Path fixedPath );
    Path&  GetPathByExtremePoint( Vec2i startpos, Vec2i endpos );

    void   AddOneKeyPos( std::string  KeyName, Vec2i  KeyPos );
    bool   GetOneKeyPosByName( std::string  KeyName,  Vec2i&  KeyPos );

    float  GetGridScale();
    Vec2i  Rasterize(Vec2i mapLocation);
    Vec2i  CalculateGridCenter(Vec2i GridPos);
    float  CalculateRealDistance(Vec2i Location1, Vec2i Location2);

    //void   ConflictStatic(ConflictMap& ConflictList);
    //void   DealConflict(ConflictMap& ConflictList);
    //int    DealConflict_FindDodge(Agent* dodgeAgv,  Vec2i  collPos, std::vector<Agent*>  agentlist, OccupyTimeElement* pOTMap, Vec2i& dodgeGridPos, Path& dodgePath );
    //int    DealConflict_FindDodgePoint_Case1_AlongPath( Agent* dodgeAgv,Vec2i nearestCollPos, OccupyTimeElement* pOTMap, Vec2i& dodgePos, Path& dodgePath);
    //int    DealConflict_FindDodgePoint_Case2_AlongBackwardPath( Agent* dodgeAgv,Vec2i nearestCollPos, OccupyTimeElement* pOTMap, Vec2i& dodgePos, Path& dodgePath);
    //bool   DealConflict_FindDodgePoint_Case2_BesidePath( Agent* dodgeAgv,Vec2i nearestCollPos, OccupyTimeElement* pOTMap, Vec2i& dodgePos, Path& dodgePath);
    //bool   DealConflict_FindDodgePoint_Case4_BesideBackwardPath( Agent* dodgeAgv,Vec2i nearestCollPos, OccupyTimeElement* pOTMap, Vec2i& dodgePos, Path& dodgePath);

    int    TimeOverLap_SpecialAGVPathToOtherAGVs(int dSpecialAGV_ID, Path SpecialAGVPath, std::vector<Agent*>  pOtherAgentList);

/*
    void   Test_1(){ std::cout<<"Test_1"<<std::endl;}
    void   Test_2(){ std::cout<<"Test_2"<<std::endl;}
    void   Test(int a);
*/

    //topology map     start
    std::vector<AcrossPoint>         m_AcrossPointList;
    std::vector<std::vector<int>>    m_CostMatrix;
    std::vector<std::vector<int>>    m_PunishCostMatrix;
    cv::Mat                          m_GridMap;
    std::map<int,DodgePos>           m_DodgePosList;

    void  FindAcrossExtremePoint( cv::Mat  gridMap, int  gridsize,  cv::Scalar color, std::vector<AcrossPoint>  &acrossPointList,
                                  std::vector<Vec2i> &extremePointList, std::vector<Vec2i> &evilWayExtremePointList );
    void  FindPathFromExtremeAcrossPoint(cv::Mat  gridMap, int  gridsize,  cv::Scalar color,std::vector<AcrossPoint>  &acrossPointList,
                                         std::vector<Vec2i> &airLineExtremePointList, std::vector<Vec2i> &evilWayExtremePointList);
    void  CalculateMapCostMatrix(std::vector<AcrossPoint> &acrossPointList, std::vector<std::vector<int>> &CostMatrix);
    void  FindAllDodgePos(int  gridsize);

    void  CalculateMapPunishCostMatrix(std::vector<AcrossPoint> &acrossPointList, std::vector<std::vector<int>> &PunishCostMatrix);
    void  FindShortestAcrossList(int startAcrossID, int EndAcrossID2, int dAcrossNum, std::vector<int> &AcrossList );

    int   ChooseNextAcross( int* distance, int n, bool *found );
    bool  IsThisColor(cv::Mat  gridMap, Vec2i node, cv::Scalar color);
    bool  StatisticsPixelConnectivity(cv::Mat  gridMap, int  gridsize, cv::Scalar color, Vec2i gridIDNode,
                                      std::vector<bool> &ConnectivityList, int &dConnectivityAirLine, int &dConnectivityEvilWay);
    void  ReleaseAcrossPoint( std::vector<AcrossPoint>  &AcrossPointList );
    void  GeneralSubPath(Vec2i StartPos, Vec2i EndPos, std::vector<CombPath>& combPathList );
    Path* CheckPointBelong(Vec2i& Pos);
    void  DealApplyList();
    void  FindPathDodgePos(Path* pPath, int  gridsize);
    //topology map     end


    //A_Star function
    void   setDiagonalMovement(bool enable_);
    void   setHeuristic(HeuristicFunction heuristic_);
    CoordinateList findPath(Vec2i source_, Vec2i target_);
    void   addCollision(Vec2i coordinates_, uchar Value, u_char *pGridMap = NULL);
    void   removeCollision(Vec2i coordinates_, u_char *pGridMap = NULL);

    Manager*   m_pManager;
private:
    void   ClearOTMap();

    void   ConflictStatic_SignAllAGVPath(std::vector<Agent*>  pAgentList, OccupyTimeElement*  pOTMap=NULL);
    void   ConflictStatic_SignPath( int AgentID, CoordinateList  path, OccupyTimeElement*  pOTMap=NULL );
    void   ConflictStatic_AnalysisConflict( ConflictMap& ConflictList );
    void   PriorityOptimiz(std::vector<Conflict>  &conflist);
    void   PriorityOptimiz_sort(std::vector<Conflict>  &conflist);
    void   DealConflict_HoldInWindows(ConflictMap& ConflictList);
    bool   DealConflict_CheckTimeOverlap(std::vector<AgentOccupyTime>  AOTList,int dAGVID, Vec2i node);


    //A_Star function
    bool  detectCollision(Vec2i coordinates_);
    Node* findNodeOnList(NodeSet& nodes_, Vec2i coordinates_);
    void  releaseNodes(NodeSet& nodes_);



    float    m_fScale;
    Size     m_cGridSize;

    cv::Mat  m_slamMap; // read from yaml file form laser slam generation
    u_char  *m_pGridMap;

    std::vector<Path>        m_fixedPaths;
    std::map<string,Vec2i>   m_KeyPoses;


    //A_Star parameter
    uint                 m_dDirection;
    HeuristicFunction    m_heuristic;
    CoordinateList       m_directionList;

    OccupyTimeElement*   m_pOTMap;

};

#endif // MAP_H
