#include "map.h"
#include <math.h>
#include <iostream>
#include <string>
#include <memory.h>
#include "manager.h"


//typedef int (Map::*dodge_Func)(Agent* dodgeAgv,Vec2i nearestCollPos, OccupyTimeElement* pOTMap, Vec2i& dodgePos, Path& dodgePath);
//dodge_Func    dodgefunction[2] = {
                                  //&Map::DealConflict_FindDodgePoint_Case1_AlongPath,
                                  //&Map::DealConflict_FindDodgePoint_Case2_BesidePath,
                                  //&Map::DealConflict_FindDodgePoint_Case2_AlongBackwardPath,
                                  //&Map::DealConflict_FindDodgePoint_Case4_BesideBackwardPath
                                  //};

/*
typedef void (Map::*Func_Test)();
Func_Test    function_test[2] = {
                              &Map::Test_1,
                              &Map::Test_2,
                                  };
*/

Map::Map()
{
    m_fScale = 0;
    m_pOTMap = NULL;
    m_pManager = NULL;
    m_pGridMap = NULL;

    setDiagonalMovement(false);
    setHeuristic(&Heuristic::manhattan);
    m_directionList = {
        { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 },
        { -1, 1}, { 1, 1 }, { 1, -1 }, { -1, -1},
    };


}

Map::~Map()
{
    ReleaseAcrossPoint( m_AcrossPointList );

    if(m_pOTMap != NULL)
    {
        free(m_pOTMap);
        m_pOTMap = NULL;
    }

    if(m_pGridMap != NULL)
    {
        free(m_pGridMap);
        m_pGridMap = NULL;
    }
}

void  Map::setDiagonalMovement(bool enable_)
{
    m_dDirection = (enable_ ? 8 : 4);
}

void  Map::setHeuristic(HeuristicFunction heuristic_)
{
    m_heuristic = std::bind(heuristic_, std::placeholders::_1, std::placeholders::_2);
}

bool  Map::detectCollision(Vec2i coordinates_)
{
    if (coordinates_.x < 0 || coordinates_.x >= m_cGridSize.width ||
        coordinates_.y < 0 || coordinates_.y >= m_cGridSize.heigh ||
        m_pGridMap[coordinates_.y * m_cGridSize.width + coordinates_.x]  != 0 )
    {
        return true;
    }
    return false;
}

Node* Map::findNodeOnList(NodeSet& nodes_, Vec2i coordinates_)
{
    for (auto node : nodes_)
    {
        if (node->coordinates == coordinates_)
            return node;
    }
    return nullptr;
}

void  Map::releaseNodes(NodeSet& nodes_)
{
    for (auto it = nodes_.begin(); it != nodes_.end();)
    {
        delete *it;
        it = nodes_.erase(it);
    }
}

void  Map::addCollision(Vec2i coordinates_, uchar Value, u_char *pGridMap)
{
    if(pGridMap == NULL)
        pGridMap = m_pGridMap;
    pGridMap[coordinates_.y * m_cGridSize.width + coordinates_.x] = Value;
}

void  Map::removeCollision(Vec2i coordinates_, u_char *pGridMap)
{
    if(pGridMap == NULL)
        pGridMap = m_pGridMap;
    m_pGridMap[coordinates_.y * m_cGridSize.width + coordinates_.x] = 0;
}

CoordinateList Map::findPath(Vec2i source_, Vec2i target_)
{
    Node *current = nullptr;
    NodeSet openSet, closedSet;
    openSet.insert(new Node(source_));

    bool  bfind = false;
    while (!openSet.empty())
    {
        current = *openSet.begin();
        for (auto node : openSet)
        {
            if (node->getScore() <= current->getScore())
            {
                current = node;
            }
        }

        if (current->coordinates == target_)
        {
            bfind = true;
            break;
        }

        closedSet.insert(current);
        openSet.erase(std::find(openSet.begin(), openSet.end(), current));

        for (uint i = 0; i < m_dDirection; ++i)
        {
            Vec2i newCoordinates( current->coordinates );
            newCoordinates + m_directionList[i];
            //if neighbor in closedSet or Collision
            //    continue
            // Ignore the neighbor which is already evaluated.
            if ( detectCollision(newCoordinates) ||
                 findNodeOnList(closedSet, newCoordinates) )
                continue;

            uint totalCost = current->G + ((i < 4) ? 10 : 14);

            Node *successor = findNodeOnList(openSet, newCoordinates);
            if (successor == nullptr)
            {
                successor = new Node(newCoordinates, current);
                successor->G = totalCost;
                successor->H = m_heuristic(successor->coordinates, target_);
                openSet.insert(successor);
            }
            else if (totalCost < successor->G)
            {
                successor->parent = current;
                successor->G = totalCost;
            }
        }
    }

    CoordinateList path;
    if(bfind)
    {
        while (current != nullptr)
        {
            path.push_back(current->coordinates);
            current = current->parent;
        }
    }

    releaseNodes(openSet);
    releaseNodes(closedSet);

    return path;
}

void   Map::CreatGridMap(std::string mapConfigFile, float GridScale)
{
    m_fScale = GridScale;
    cv::FileStorage fsr( mapConfigFile, cv::FileStorage::READ );
    if( fsr.isOpened() )
        cout<<"open it"<<endl;

    string imgName;
    fsr["image"]>>imgName;
    cout<<imgName<<endl;

    float res ;
    fsr["resolution"]>>res; // m/pixel
    float occTh = (float)fsr["occupied_thresh"];
    float freeTh = (float)fsr["free_thresh"];
    fsr.release();

    m_slamMap = cv::imread(imgName, 0);

    int width  = m_slamMap.cols;
    int height = m_slamMap.rows;

    float  rW = width*res; //  real map width (m)
    float  rH  = height*res;// real map height (m)

    // cout<<rW<<" "<<rH<<endl;
    m_cGridSize.width = (int)(rW/m_fScale);
    m_cGridSize.heigh = (int)(rH/m_fScale);

    if(m_pGridMap != NULL)
    {
        free(m_pGridMap);
        m_pGridMap = NULL;
    }
    m_pGridMap = (u_char*)malloc(sizeof(u_char)* m_cGridSize.heigh * m_cGridSize.width);//为二维数组分配
    memset(m_pGridMap, 0, sizeof(u_char)* m_cGridSize.heigh * m_cGridSize.width);

    for(int i=0; i<width; i++)
    {
        for(int j=0; j<height; j++)
        {
            float occ =  (255 - m_slamMap.at<uchar>(j,i) )/255.0;
            if( occ >= occTh )
            {
                float x = i*res;
                float y = j*res;
                int   w = (int) (x/m_fScale);
                int   h = (int) (y/m_fScale);
                m_pGridMap[ h * m_cGridSize.width + w ] = 255;
            }
        }
    }
}

float   Map::GetGridScale()
{
    return  m_fScale;
}

Vec2i  Map::Rasterize(Vec2i mapLocation)
{
    Vec2i  tempGridPos;

    tempGridPos.x = mapLocation.x / m_fScale;
    if( ( tempGridPos.x * m_fScale ) < mapLocation.x )
        tempGridPos.x += 1;

    tempGridPos.y = mapLocation.y / m_fScale;
    if( ( tempGridPos.y * m_fScale ) < mapLocation.y )
        tempGridPos.y += 1;

    tempGridPos.angle = mapLocation.angle / 45;
    if(  abs( ( tempGridPos.angle * 45 ) - mapLocation.angle ) < 10 )
    {
        tempGridPos.angle *= 45;
    }
    else
    {
        tempGridPos.angle += 1;
        tempGridPos.angle *= 45;
    }

    return  tempGridPos;
}

Vec2i  Map::CalculateGridCenter(Vec2i GridPos)
{
    Vec2i GridCenter;
    GridCenter.x = GridPos.x * m_fScale + 0.5 * m_fScale;
    GridCenter.y = GridPos.y * m_fScale + 0.5 * m_fScale;
    return GridCenter;
}

float  Map::CalculateRealDistance(Vec2i Location1, Vec2i Location2)
{
    float  Dis = sqrt( (Location1.x - Location2.x)*(Location1.x - Location2.x) +
                       (Location1.y - Location2.y)*(Location1.y - Location2.y) );
    return Dis;
}



void   Map::ConflictStatic_SignPath( int AgentID, CoordinateList  path, OccupyTimeElement*  pOTMap)
{
    if(pOTMap == NULL)
    {
        pOTMap = m_pOTMap;
    }

    for( auto node : path )
    {
        if( ( node.TimeIn != 0 ) && ( node.TimeOut != 0 ) )
        {
            AgentOccupyTime   tempAOT;
            tempAOT.AgentID = AgentID;
            tempAOT.TimeIn  = node.TimeIn;
            tempAOT.TimeOut = node.TimeOut;
            pOTMap[node.y * m_cGridSize.width + node.x].Used += 1;
            pOTMap[node.y * m_cGridSize.width + node.x].x = node.x;
            pOTMap[node.y * m_cGridSize.width + node.x].y = node.y;
            pOTMap[node.y * m_cGridSize.width + node.x].AOTList.push_back(tempAOT);
        }
    }

}

void   Map::ClearOTMap()
{
    if(m_pOTMap != NULL)
    {
        free(m_pOTMap);
        m_pOTMap = NULL;
    }

    m_pOTMap = new OccupyTimeElement[ m_cGridSize.width * m_cGridSize.heigh ];
}

/*
void   Map::ConflictStatic_SignAllAGVPath(std::vector<Agent*>  pAgentList, OccupyTimeElement*  pOTMap)
{
    float fBaseTime, fBaseTime_Turn;
    m_pManager->GetBaseTime( fBaseTime, fBaseTime_Turn );

    for( auto agent : pAgentList )
    {
        AgentStatue   eAGVStatue =  agent->GetStatue();
        if(
            (eAGVStatue != AGENT_STATUE_CHARGING) &&
            (eAGVStatue != AGENT_STATUE_WAIT)
                )
        {
            if( eAGVStatue == AGENT_STATUE_PROCESS_NORMAL_RUN )
            {
                Task* temphead = agent->m_pTaskGroup->GetTheProcessingTask();

                CoordinateList path = temphead->m_path.GetPath();
                int    dID = agent->GetID();
                ConflictStatic_SignPath(dID, path, pOTMap);


                if(temphead->GetStatue() == )
                {
                    CoordinateList path = temphead->m_path.GetPath();
                    int    dID = agent->GetID();
                    ConflictStatic_SignPath(dID, path, pOTMap);
                }
                else
                {
                    time_t now_time;
                    now_time = time(NULL);
                    CoordinateList Pathlist;
                    Vec2i   tempNode = agent->GetCurrentGridPosition();
                    tempNode.TimeIn  = now_time;
                    tempNode.TimeOut = now_time + CONFLICT_WINDOWS_HALF * fBaseTime;
                    Pathlist.push_back(tempNode);
                    int    dID = agent->GetID();
                    ConflictStatic_SignPath(dID, Pathlist, pOTMap);
                }


            }
            else if(
               (eAGVStatue == AGENT_STATUE_PROCESS_DODGE_GO)    ||
               (eAGVStatue == AGENT_STATUE_PROCESS_DODGE_STOP)  ||
               (eAGVStatue == AGENT_STATUE_PROCESS_DODGE_BACK)
                    )
            {
                CoordinateList path = agent->m_DodgePath.GetPath();
                int    dID = agent->GetID();
                ConflictStatic_SignPath(dID, path, pOTMap);
            }
            else if(
               (eAGVStatue == AGENT_STATUE_BREAKDOWN)      ||
               (eAGVStatue == AGENT_STATUE_PROCESS_NORMAL_PAUSE)
                    )
            {
                int    dID = agent->GetID();
                CoordinateList path;
                path.push_back(agent->m_posBreakPause);
                ConflictStatic_SignPath(dID, path, pOTMap);
            }
        }
    }
}
*/

/*
void   Map::ConflictStatic(ConflictMap& ConflictList)
{
    ClearOTMap();
    std::vector<Agent*> pAgentList;
    for(int i=0; i< m_pManager->m_vAgentList.size();i++)
        pAgentList.push_back( (m_pManager->m_vAgentList[i]) );

    ConflictStatic_SignAllAGVPath(pAgentList);
    ConflictStatic_AnalysisConflict( ConflictList );
}

int   Map::TimeOverLap_SpecialAGVPathToOtherAGVs(int dSpecialAGV_ID, Path SpecialAGVPath, std::vector<Agent*>  pOtherAgentList)
{
    bool  bOverLap = false;

    OccupyTimeElement *pOTMap = new OccupyTimeElement[ m_cGridSize.width * m_cGridSize.heigh ];
    ConflictStatic_SignAllAGVPath(pOtherAgentList, pOTMap);

    CoordinateList  pathlist = SpecialAGVPath.GetPath();
    int  dsafeNodeNum = 0;
    for(auto it : pathlist)
    {
        OccupyTimeElement*  pOTElement = &(pOTMap[ it.y * m_cGridSize.width + it.x ]);
        for(auto itAOT : pOTElement->AOTList)
        {
            if(itAOT.AgentID != dSpecialAGV_ID)
            {
                if( (it.TimeOut>itAOT.TimeIn)&&(it.TimeOut<itAOT.TimeOut)  ||
                    (it.TimeIn>itAOT.TimeIn)&&(it.TimeIn<itAOT.TimeOut) )
                {
                    bOverLap = true;
                    break;
                }
            }
        }

        if(bOverLap)
            break;

        dsafeNodeNum++;
    }

    if(pOTMap != NULL)
    {
        free(pOTMap);
        pOTMap = NULL;
    }

    if( ( dsafeNodeNum == pathlist.size() ) && (dsafeNodeNum < MAX_FINDDODGE_BACKWARDLENGTH) )
        dsafeNodeNum += MAX_FINDDODGE_BACKWARDLENGTH;

    return  dsafeNodeNum;
}
*/

void   Map::PriorityOptimiz_sort(std::vector<Conflict>  &conflist)
{
    for(int it=0; it < conflist.size(); it++)
    {
        Agent* tempHead_A = m_pManager->FindAgentByID( conflist[it].conflictAgent_A.AgentID );
        Agent* tempHead_B = m_pManager->FindAgentByID( conflist[it].conflictAgent_B.AgentID );

        if( tempHead_A->GetPriority() <= tempHead_B->GetPriority() )
        {
            Conflict  tempdata(conflist[it].x, conflist[it].y, conflist[it].conflictAgent_B, conflist[it].conflictAgent_A);
            conflist[it] = tempdata;
        }
    }
}

void   Map::PriorityOptimiz(std::vector<Conflict>  &conflist)
{
    std::map<int,int>  mapAGVID_ColNum;

    for(auto  it : conflist)
    {
        if(mapAGVID_ColNum.end() == mapAGVID_ColNum.find(it.conflictAgent_A.AgentID) )
            mapAGVID_ColNum.insert(std::make_pair(it.conflictAgent_A.AgentID, 1));
        else
            mapAGVID_ColNum[it.conflictAgent_A.AgentID] += 1;

        if(mapAGVID_ColNum.end() == mapAGVID_ColNum.find(it.conflictAgent_B.AgentID) )
            mapAGVID_ColNum.insert(std::make_pair(it.conflictAgent_B.AgentID, 1));
        else
            mapAGVID_ColNum[it.conflictAgent_B.AgentID] += 1;
    }

    int    colAGVNUM = mapAGVID_ColNum.size();

    switch(colAGVNUM)
    {
    case 2:

        break;

    case 3:
        {
        /*********************************************/
        /* 3AGV, 3 times collosion: _____            */
        /*                          _____            */
        /*                          _____            */
        /*                                           */
        /*                                           */
        /* 3AGV, 2 times collosion: _____            */
        /*                      _____   _____        */
        /*                                           */
        /*********************************************/
            if(conflist.size() == 2)
            {
                int  MaxColAGVID = 0;
                int  MaxColNUM   = 0;
                for( auto it : mapAGVID_ColNum )
                {
                    if(it.second > MaxColNUM)
                    {
                        MaxColAGVID = it.first;
                        MaxColNUM   = it.second;
                    }
                }
                Agent* pMaxColAGV = m_pManager->FindAgentByID( MaxColAGVID );
                pMaxColAGV->SetPriority(0);

            }

            if(conflist.size() == 3)
            {
                float prioMax = 0;
                int   maxPrioAGVID = -1;
                for( auto it : mapAGVID_ColNum )
                {
                    Agent* pColAGV  = m_pManager->FindAgentByID( it.first );
                    float tempvalue = pColAGV->GetPriority();

                    if(tempvalue > prioMax)
                    {
                        prioMax = tempvalue;
                        maxPrioAGVID =  it.first;
                    }
                }

                std::vector<Conflict>  tempconflist;
                for(auto  it : conflist)
                {
                    if((it.conflictAgent_A.AgentID ==  maxPrioAGVID)||(it.conflictAgent_B.AgentID ==  maxPrioAGVID))
                        tempconflist.push_back(it);
                }
                conflist.clear();
                conflist = tempconflist;
            }
        }
        break;

    case 4:
        {
            /*********************************************/
            /* 4AGV, 6 times collosion: _____            */
            /*                          _____            */
            /*                          _____            */
            /*                          _____            */
            /*                                           */
            /*                                           */
            /* 4AGV, 5 times collosion: _____            */
            /*                      _____   _____        */
            /*                          _____            */
            /*                                           */
            /*                                           */
            /* 4AGV, 4 times collosion: _____            */
            /*                      _____   _____        */
            /*                              _____        */
            /*                                           */
            /*                                           */
            /* 4AGV, 3 times collosion: _____   _____    */
            /*                      _____   _____        */
            /*                                           */
            /*                                           */
            /* 4AGV, 2 times collosion: _____      _____ */
            /*                      _____       _____    */
            /*                                           */
            /*********************************************/


            if(conflist.size() == 6) //3,3,3,3
            {
                float prioMax = 0;
                int   maxPrioAGVID = -1;
                for( auto it : mapAGVID_ColNum )
                {
                    Agent* pColAGV  = m_pManager->FindAgentByID( it.first );
                    float tempvalue = pColAGV->GetPriority();

                    if(tempvalue > prioMax)
                    {
                        prioMax = tempvalue;
                        maxPrioAGVID =  it.first;
                    }
                }

                std::vector<Conflict>  tempconflist;
                for(auto  it : conflist)
                {
                    if((it.conflictAgent_A.AgentID ==  maxPrioAGVID)||(it.conflictAgent_B.AgentID ==  maxPrioAGVID))
                        tempconflist.push_back(it);
                }
                conflist.clear();
                conflist = tempconflist;
            }

            if(conflist.size() == 5) //2,2,3,3
            {
                int  MaxColNUM   = 0;
                for( auto it : mapAGVID_ColNum )
                {
                    if(it.second > MaxColNUM)
                        MaxColNUM   = it.second;
                }

                int  PriorityValue = 0;
                for(auto it : mapAGVID_ColNum )
                {
                    if(it.second == MaxColNUM)
                    {
                        Agent* pMaxColAGV = m_pManager->FindAgentByID( it.first );
                        pMaxColAGV->SetPriority(PriorityValue);
                        PriorityValue += 0.01;
                    }
                }

                std::vector<Conflict>  tempconflist;
                for(auto  it : conflist)
                {
                    if((MaxColNUM !=  mapAGVID_ColNum[it.conflictAgent_A.AgentID])||(MaxColNUM !=  mapAGVID_ColNum[it.conflictAgent_B.AgentID]))
                        tempconflist.push_back(it);
                }
                conflist.clear();
                conflist = tempconflist;

            }

            if(conflist.size() == 4) //just analysis case 1,2,2,3; case 2,2,2,2 not exist
            {
                int  MaxColNUM      = 0;
                int  MaxNUMColAGVID = -1;
                for( auto it : mapAGVID_ColNum )
                {
                    if(it.second > MaxColNUM)
                    {
                        MaxColNUM   = it.second;
                        MaxNUMColAGVID = it.first;
                    }
                }
                Agent* pMaxColAGV = m_pManager->FindAgentByID( MaxNUMColAGVID );
                pMaxColAGV->SetPriority(0);

                int  ColTwoTimesID1=-1;
                int  ColTwoTimesID2=-1;
                for(auto it : mapAGVID_ColNum)
                {
                    if(it.second == 2)
                    {
                        if(-1==ColTwoTimesID1)
                            ColTwoTimesID1 = it.first;
                        else
                            ColTwoTimesID2 = it.first;
                    }
                }

                Agent* pColAGV1  = m_pManager->FindAgentByID( ColTwoTimesID1 );
                Agent* pColAGV2  = m_pManager->FindAgentByID( ColTwoTimesID2 );
                int  replanAGVID = pColAGV1->GetPriority() > pColAGV2->GetPriority() ? ColTwoTimesID2 : ColTwoTimesID1;

                std::vector<Conflict>  tempconflist;
                for(auto  it : conflist)
                {
                    if(
                       ((MaxNUMColAGVID == it.conflictAgent_A.AgentID) &&  ( replanAGVID == it.conflictAgent_B.AgentID)) ||
                       ((replanAGVID    == it.conflictAgent_A.AgentID) &&(MaxNUMColAGVID == it.conflictAgent_B.AgentID))
                      )
                        continue;
                    else
                        tempconflist.push_back(it);
                }
                conflist.clear();
                conflist = tempconflist;

            }

            if(conflist.size() == 3) //just analysis case 1,2,2,1;
            {
                int  MaxColNUM = 0;
                for( auto it : mapAGVID_ColNum )
                {
                    if(it.second > MaxColNUM)
                        MaxColNUM   = it.second;
                }

                int  PriorityValue = 0;
                for(auto it : mapAGVID_ColNum )
                {
                    if(it.second == MaxColNUM)
                    {
                        Agent* pMaxColAGV = m_pManager->FindAgentByID( it.first );
                        pMaxColAGV->SetPriority(PriorityValue);
                        PriorityValue += 0.01;
                    }
                }

                std::vector<Conflict>  tempconflist;
                for(auto  it : conflist)
                {
                    if((MaxColNUM !=  mapAGVID_ColNum[it.conflictAgent_A.AgentID])||(MaxColNUM !=  mapAGVID_ColNum[it.conflictAgent_B.AgentID]))
                        tempconflist.push_back(it);
                }
                conflist.clear();
                conflist = tempconflist;
            }
        }
        break;

    default:
        break;
    }

    PriorityOptimiz_sort(conflist);
}

void   Map::ConflictStatic_AnalysisConflict( ConflictMap& conflictmap )
{
    int  congflictID =0;
    for(uint i =0; i < m_cGridSize.width * m_cGridSize.heigh; i++)
    {
        if( m_pOTMap[i].Used > 1 )
        {
            std::vector<Conflict>  conflist;
            for(uint j =0; j< m_pOTMap[i].Used; j++)
            {
                for(uint k =j+1; k< m_pOTMap[i].Used; k++)
                {
                    AgentOccupyTime  tempA = m_pOTMap[i].AOTList[j];
                    AgentOccupyTime  tempB = m_pOTMap[i].AOTList[k];

                    bool  bconflict = false;
                    if( (tempA.TimeIn < tempB.TimeOut) && (tempA.TimeIn > tempB.TimeIn) )
                        bconflict = true;
                    if( (tempB.TimeOut < tempA.TimeOut) && (tempB.TimeOut > tempA.TimeIn) )
                        bconflict = true;

                    if( bconflict )
                    {
                        Conflict newConflict( m_pOTMap[i].x,  m_pOTMap[i].y, tempA, tempB );
                        conflist.push_back( newConflict );
                    }
                }
            }

            if( conflist.size()>0 )
            {
                PriorityOptimiz(conflist);
                conflictmap.insert(std::make_pair( congflictID,conflist ));
                congflictID++;
            }
        }
    }
}

void   Map::DealConflict_HoldInWindows(ConflictMap& ConflictList)
{
    time_t  now_time;
    now_time = time(NULL);
    float  fBaseTime, fBaseTimeTurn;
    m_pManager->GetBaseTime( fBaseTime, fBaseTimeTurn);
    float  advanceTime = CONFLICT_WINDOWS_HALF * fBaseTime;

    //just hold these conflict which in "CONFLICT_WINDOWS"
    for (auto it = ConflictList.begin(); it != ConflictList.end(); )
    {
        for(auto conflictElement = it->second.begin();  conflictElement != it->second.end();)
        {
            time_t Min_Time = ( conflictElement->conflictAgent_A.TimeIn > conflictElement->conflictAgent_B.TimeIn ?
                                conflictElement->conflictAgent_B.TimeIn : conflictElement->conflictAgent_A.TimeIn );
            if( ((Min_Time - now_time) > 0) && ((Min_Time - now_time) >  advanceTime) )
                conflictElement = it->second.erase( conflictElement );
        }

        if(it->second.size() == 0)
            it = ConflictList.erase(it);
    }
}

/*
void   Map::DealConflict(ConflictMap& ConflictList)
{
    DealConflict_HoldInWindows(ConflictList);

    std::cout<<ConflictList.size()<<" points would occur collision."<<std::endl;
    std::map<Agent*, int>  WaitFordodgeAGVHeadID;   //this map save the dodge agv id
    for( auto conflictPos = ConflictList.begin(); conflictPos != ConflictList.end();)
    {
        for(auto conflictEle = conflictPos->second.begin(); conflictEle != conflictPos->second.end();)
        {
            Agent* replanhead = m_pManager->FindAgentByID( conflictEle->conflictAgent_B.AgentID );

            if( WaitFordodgeAGVHeadID.end() == WaitFordodgeAGVHeadID.find(replanhead) )
                WaitFordodgeAGVHeadID.insert(make_pair(replanhead, replanhead->GetID()));
        }
    }

    std::map<Agent*, int>  NormalAGVHeadID;  //this map save the normal agv id
    std::vector<Agent*>    NormalAGVHead;
    for(int i=0; i < m_pManager->m_vAgentList.size(); i++)
    {
         if( NormalAGVHeadID.end() == WaitFordodgeAGVHeadID.find(( m_pManager->m_vAgentList[i]) ) )
         {
            NormalAGVHeadID.insert(make_pair( ( m_pManager->m_vAgentList[i] ), m_pManager->m_vAgentList[i]->GetID() ));
            NormalAGVHead.push_back( ( m_pManager->m_vAgentList[i] ) );
         }
    }

    AgentConfPos   conflictAgents;
    OccupyTimeElement *pOTMap = new OccupyTimeElement[ m_cGridSize.width * m_cGridSize.heigh ];
    ConflictStatic_SignAllAGVPath(NormalAGVHead, pOTMap);
    //statical collision to map, key by the collision agent's head
    for( auto conflictPos = ConflictList.begin(); conflictPos != ConflictList.end();)
    {
        for(auto conflictEle = conflictPos->second.begin(); conflictEle != conflictPos->second.end();)
        {
            Agent* noplanhead = m_pManager->FindAgentByID( conflictEle->conflictAgent_A.AgentID );
            Agent* replanhead = m_pManager->FindAgentByID( conflictEle->conflictAgent_B.AgentID );

            Vec2i  temp(conflictEle->x, conflictEle->y);
            if(conflictAgents.end() == conflictAgents.find(replanhead) )
            {
                AgentPosCollision  tempData;
                tempData.collPos = temp;
                tempData.agentlist.push_back(noplanhead);
                conflictAgents.insert(std::make_pair(replanhead, tempData));
            }
            else
            {
                auto iterator = conflictAgents.find(replanhead);
                iterator->second.agentlist.push_back(noplanhead);
            }
        }

        //to every collision agent, change gridmap,because the path is fixed,so find the dodge point
        for( auto iterator = conflictAgents.begin();  iterator != conflictAgents.end(); )
        {
            Vec2i  dodgePos;
            Path   dodgePath;
            int   bfind = DealConflict_FindDodge( iterator->first,  iterator->second.collPos, iterator->second.agentlist,  pOTMap, dodgePos, dodgePath);
            if( bfind > 0 )
            {
                //modify this agv's dodge path's time stamp, and add this path's OT list to OTMap
                iterator->first->ChangeStatue( AGENT_STATUE_PROCESS_DODGE_GO);
                iterator->first->m_DodgePath  = dodgePath;
                iterator->first->m_dDodgeMode = bfind;

                std::vector<Agent*>  templist;
                templist.push_back(iterator->first);
                ConflictStatic_SignAllAGVPath(templist, pOTMap);
            }
            else  // error: can't find dodge point
            {
                //stop this AGV first;
                iterator->first->ChangeStatue(AGENT_STATUE_BREAKDOWN);

                //put out warnning information
                string  errorInfor = "ERROR: Fatal error!";
                char cinfor[100];
                sprintf(cinfor, "No dodge point could be found! AGV_%d had been stoped now,operator must drive it go away from road!",iterator->first->GetID());
                errorInfor += cinfor;
                m_pManager->AddInforToContent(false, errorInfor);
            }
        }
    }

    if(pOTMap != NULL)
    {
        free(pOTMap);
        pOTMap = NULL;
    }

}
*/

/*
int   Map::DealConflict_FindDodge(Agent* dodgeAgv,  Vec2i  collPos, std::vector<Agent*>  agentlist, OccupyTimeElement* pOTMap, Vec2i& dodgeGridPos, Path& dodgePath)
{
    int    dfind = 0;
    Vec2i  nearestCollPos = collPos;

    uchar  *pGridMap = (u_char*)malloc(sizeof(u_char)* m_cGridSize.heigh * m_cGridSize.width);
    memcpy( pGridMap, m_pGridMap, sizeof(u_char)* m_cGridSize.heigh * m_cGridSize.width );
    for( auto it : agentlist )
    {
        Task* temphead = it->m_pTaskGroup->GetTheProcessingTask();
        Path path = temphead->m_path;
        for( auto posEle : path.GetPath() )
            addCollision( posEle, CONFLICT_SIGN_VALUE);
    }

    for(int i =0; i < 2; i++)
    {
        dfind = (this->*dodgefunction[i])( dodgeAgv, nearestCollPos, pOTMap, dodgeGridPos, dodgePath );
        if(dfind > 0)
            break;
    }

    memcpy( m_pGridMap, pGridMap, sizeof(u_char)* m_cGridSize.heigh * m_cGridSize.width );
    if( pGridMap != NULL )
    {
        free(pGridMap);
        pGridMap = NULL;
    }
    return  dfind;
}

int   Map::DealConflict_FindDodgePoint_Case2_AlongBackwardPath( Agent* dodgeAgv,Vec2i nearestCollPos, OccupyTimeElement* pOTMap, Vec2i& dodgePos, Path& dodgePath )
{
    int bflag =0;
    AgentStatue  eAgvStatue = dodgeAgv->GetStatue();
    Task*  temphead = NULL;
    Path*  path     = NULL;
    if(AGENT_STATUE_PROCESS_NORMAL_RUN == eAgvStatue)
    {
        path     = &(temphead->m_path);
        temphead = dodgeAgv->m_pTaskGroup->GetTheProcessingTask();
    }
    else if(
            (AGENT_STATUE_PROCESS_DODGE_GO   == eAgvStatue) ||
            (AGENT_STATUE_PROCESS_DODGE_STOP == eAgvStatue) ||
            (AGENT_STATUE_PROCESS_DODGE_BACK == eAgvStatue)
            )  //the dodge agv is alreday in dodge statue, this statue can't be deal,so error
    {
        return bflag;
    }


    float fAccTime,fDecTime,fLineVelocity,fAngularVelocity;
    dodgeAgv->GetAccDecTime(fAccTime,fDecTime);
    dodgeAgv->GetMeanVelocity(fLineVelocity,fAngularVelocity);

    int dIndex = 0;
    path->GetCurGridPosIndex(dIndex);
    CoordinateList  pathlist = path->GetPath();
    CoordinateList  dodgeTestList;
    for(int i= dIndex; i > 0; i--)
    {
        if((dIndex -i) >MAX_FINDDODGE_BACKWARDLENGTH)
            break;

        Vec2i  tempNode = pathlist[i];
        dodgeTestList.push_back(tempNode);
        dodgePath.SetExtremePoint(pathlist[dIndex],tempNode);
        dodgePath.SetPath(dodgeTestList);
        dodgePath.CalPathInflexion();
        dodgePath.SetTimeStampToPath( this, m_pManager->m_eAPModel,
                                      dodgeAgv->GetCurrentGridPosition(),
                                      dodgeAgv->GetCurrentMapPosition(),
                                      dodgeAgv->GetStatue(), temphead->GetStatue(),
                                      fAccTime,fDecTime,fLineVelocity,fAngularVelocity,
                                      GetGridScale() );

        dodgePath.GetPointByID((dodgePath.GetPathSize() - 1),tempNode);
        if( m_pGridMap[pathlist[i].y * m_cGridSize.width + pathlist[i].x] == 0 )
        {
            //time check
            if(!DealConflict_CheckTimeOverlap(pOTMap[tempNode.y * m_cGridSize.width + tempNode.x].AOTList, dodgeAgv->GetID(), tempNode ))
            {
                bflag    = 3;
                dodgePos = tempNode;
                break;
            }
        }
        else
        {
            CoordinateList  directionList = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };
            for (uint j = 0; j < 4; ++j)
            {
                Vec2i newCoordinates = pathlist[i];
                newCoordinates + directionList[j];

                bool bfail = detectCollision(newCoordinates);
                if(!bfail)
                {
                    bflag    = 4;
                    dodgePos = newCoordinates;
                    break;
                }
            }

            if( 4==bflag )
            {
                dodgeTestList.push_back(dodgePos);
                dodgePath.SetExtremePoint(pathlist[dIndex], dodgePos);
                dodgePath.SetPath(dodgeTestList);
                dodgePath.CalPathInflexion();
                dodgePath.SetTimeStampToPath(this, m_pManager->m_eAPModel,
                                             dodgeAgv->GetCurrentGridPosition(),
                                             dodgeAgv->GetCurrentMapPosition(),
                                             dodgeAgv->GetStatue(), temphead->GetStatue(),
                                             fAccTime,fDecTime,fLineVelocity,fAngularVelocity,
                                             GetGridScale() );
                break;
            }
        }
    }

    return  bflag;
}



int   Map::DealConflict_FindDodgePoint_Case1_AlongPath( Agent* dodgeAgv,Vec2i nearestCollPos, OccupyTimeElement* pOTMap, Vec2i& dodgePos, Path& dodgePath)
{
    int   bflag  = 0;
    AgentStatue  eAgvStatue = dodgeAgv->GetStatue();
    Path   *path     = NULL;
    Task   *temphead = NULL;
    if(AGENT_STATUE_PROCESS_NORMAL_RUN == eAgvStatue)
    {
        path     = &(temphead->m_path);
        temphead = dodgeAgv->m_pTaskGroup->GetTheProcessingTask();
    }
    else if(
            (AGENT_STATUE_PROCESS_DODGE_GO   == eAgvStatue) ||
            (AGENT_STATUE_PROCESS_DODGE_STOP == eAgvStatue) ||
            (AGENT_STATUE_PROCESS_DODGE_BACK == eAgvStatue)
            )  //the dodge agv is alreday in dodge statue, this statue can't be deal,so error
    {
        return bflag;
    }

    float fAccTime,fDecTime,fLineVelocity,fAngularVelocity;
    dodgeAgv->GetAccDecTime(fAccTime,fDecTime);
    dodgeAgv->GetMeanVelocity(fLineVelocity,fAngularVelocity);

    int   dCurIndex = 0;
    Vec2i curNode;
    path->GetCurGridPosIndex(dCurIndex);
    path->GetPointByID(dCurIndex, curNode);
    int  dNearestCollIndex = path->GetSpeGridPosIndex(nearestCollPos);
    for( int i= (dNearestCollIndex-1); i >= dCurIndex; i-- )
    {
        Vec2i tempNode;
        path->GetPointByID(i, tempNode);

        CoordinateList  pathlist;
        for( int j= dCurIndex; j<=i ;j++ )
        {
            Vec2i tempNodeD;
            path->GetPointByID(j, tempNodeD);
            pathlist.push_back(tempNodeD);
        }

        if( m_pGridMap[tempNode.y * m_cGridSize.width + tempNode.x] == 0 )
        {
            //time check
            if(!DealConflict_CheckTimeOverlap(pOTMap[tempNode.y * m_cGridSize.width + tempNode.x].AOTList, dodgeAgv->GetID(), tempNode))
            {
                bflag    = 1;
                dodgePos = tempNode;
                Vec2i  colPos;
                path->GetPointByID(dNearestCollIndex, colPos);

                dodgePath.SetExtremePoint( curNode, tempNode );
                dodgePath.SetPath(pathlist);
                dodgePath.CalPathInflexion();
                dodgePath.SetTimeStampToPath(this, m_pManager->m_eAPModel,
                                             dodgeAgv->GetCurrentGridPosition(),
                                             dodgeAgv->GetCurrentMapPosition(),
                                             dodgeAgv->GetStatue(), temphead->GetStatue(),
                                             fAccTime,fDecTime,fLineVelocity,fAngularVelocity,
                                             GetGridScale());
                break;
            }
        }
        else
        {
            CoordinateList  directionList = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };
            for (uint j = 0; j < 4; ++j)
            {
                Vec2i newCoordinates = tempNode;
                newCoordinates + directionList[j];

                bool bfail = detectCollision(newCoordinates);
                if(!bfail)
                {
                    bflag    = 2;
                    dodgePos = newCoordinates;

                    pathlist.push_back(newCoordinates);
                    dodgePath.SetExtremePoint(curNode, newCoordinates);
                    dodgePath.SetPath(pathlist);
                    dodgePath.CalPathInflexion();
                    dodgePath.SetTimeStampToPath(this, m_pManager->m_eAPModel,
                                                 dodgeAgv->GetCurrentGridPosition(),
                                                 dodgeAgv->GetCurrentMapPosition(),
                                                 dodgeAgv->GetStatue(), temphead->GetStatue(),
                                                 fAccTime,fDecTime,fLineVelocity,fAngularVelocity,
                                                 GetGridScale());
                    break;
                }
            }
        }
    }
    return  bflag;
}
*/


bool  Map::DealConflict_CheckTimeOverlap(std::vector<AgentOccupyTime>  AOTList, int dAGVID, Vec2i node)
{
    bool  bNoOverLap     = true;
    float  fBaseTime, fBaseTimeTurn;
    m_pManager->GetBaseTime( fBaseTime, fBaseTimeTurn);

    for( auto tempAOT :  AOTList )
    {
        if(tempAOT.AgentID == dAGVID)
            continue;

        if((MIN_DODGETIME_TIMES*fBaseTime) > (tempAOT.TimeIn - node.TimeOut))
        {
            bNoOverLap = false;
            break;
        }
    }
    return  !bNoOverLap;
}


void   Map::AddOneFixedPath( Path fixedPath )
{
    m_fixedPaths.push_back(fixedPath);
}

Path&  Map::GetPathByExtremePoint( Vec2i startpos, Vec2i endpos )
{
    Path  result;
    for( auto tempPath : m_fixedPaths )
    {
        Vec2i s_pos,e_pos;
        tempPath.GetExtremePoint(s_pos,e_pos);
        if((s_pos == startpos)&&(e_pos == endpos))
        {
            result = tempPath;
            break;
        }
    }
    return result;
}

void   Map::AddOneKeyPos( std::string  KeyName, Vec2i  KeyPos )
{
    m_KeyPoses.insert(std::make_pair(KeyName, KeyPos));
}

bool Map::GetOneKeyPosByName( std::string  KeyName,  Vec2i&  KeyPos )
{
    bool flag =false;
    auto iterator = m_KeyPoses.begin();
    for( ;iterator != m_KeyPoses.end(); )
    {
        if(iterator->first == KeyName)
        {
            KeyPos = iterator->second;
            flag   = true;
            break;
        }
    }

    return flag;
}


bool  Map::StatisticsPixelConnectivity(cv::Mat  gridMap, int  gridsize, cv::Scalar color,Vec2i gridIDNode,
                                       std::vector<bool> &ConnectivityList,
                                       int &dConnectivityAirLine, int &dConnectivityEvilWay)
{
    int  dHalfsize = (gridsize/2)*2 < gridsize ? ( (gridsize/2) + 1 ) : (gridsize/2);
    dConnectivityAirLine = dConnectivityEvilWay = 0;
    for(int k =0 ; k < 8; k++)
    {
        Vec2i  tempNode1 = gridIDNode;
        tempNode1 + m_directionList[k];
        Vec2i  tempNode2((tempNode1.x*gridsize + dHalfsize),(tempNode1.y*gridsize+dHalfsize));
        bool bresult  = false;
        if((tempNode2.x < gridMap.cols) && (tempNode2.x >= 0) &&
           (tempNode2.y < gridMap.rows) && (tempNode2.y >= 0) ) //in the map
            bresult = IsThisColor(gridMap, tempNode2, color);

        ConnectivityList.push_back(bresult);
        if(bresult)
        {
            if(k<4)
                dConnectivityAirLine++;
            else
                dConnectivityEvilWay++;
        }
    }

    //check if downside case occur, this case is forbiden
    //  1 1
    //  * 1
    bool flag = true;
    if(ConnectivityList[0] && ConnectivityList[5] && ConnectivityList[1])
        flag = false;
    if(ConnectivityList[1] && ConnectivityList[6] && ConnectivityList[2])
        flag = false;
    if(ConnectivityList[2] && ConnectivityList[7] && ConnectivityList[3])
        flag = false;
    if(ConnectivityList[3] && ConnectivityList[4] && ConnectivityList[0])
        flag = false;

    return flag;
}

bool  Map::IsThisColor(cv::Mat  gridMap, Vec2i node, cv::Scalar color)
{
    bool bflag = false;

    if(
    (gridMap.ptr<cv::Vec3b>(node.y)[node.x][0]==color[0]) &&
    (gridMap.ptr<cv::Vec3b>(node.y)[node.x][1]==color[1]) &&
    (gridMap.ptr<cv::Vec3b>(node.y)[node.x][2]==color[2])   )
        bflag = true;

    return  bflag;
}

void  Map::FindPathFromExtremeAcrossPoint(cv::Mat  gridMap, int  gridsize,  cv::Scalar color,std::vector<AcrossPoint>  &acrossPointList,
                                     std::vector<Vec2i> &airLineExtremePointList, std::vector<Vec2i> &evilWayExtremePointList)
{
    std::vector<Vec2i>  ExtremePointList = evilWayExtremePointList;
    for(int i =0; i < airLineExtremePointList.size(); i++)
        ExtremePointList.push_back(airLineExtremePointList[i]);

    //find paths start from Extreme Points
    for(int i =0; i< ExtremePointList.size(); i++)
    {
        CoordinateList  pathlist;
        Vec2i CurNode = ExtremePointList[i];
        bool  bEnd = false;
        for(;!bEnd;)
        {
            //if the curNode is across point,stop this path scan,then link this path to the across
            for(int i =0; i < acrossPointList.size(); i++)
            {
                if((acrossPointList[i].x == CurNode.x)&&(acrossPointList[i].y == CurNode.y))
                {
                    Vec2i  direction( ( pathlist[pathlist.size()-1].x - CurNode.x ), (pathlist[pathlist.size()-1].y - CurNode.y) );
                    int    j =0;
                    for(; j < 8 ; j++)
                        if( m_directionList[j] == direction )
                            break;

                    Path*   ptempPath = new Path();
                    ptempPath->m_pMap = this;
                    ptempPath->SetPath(pathlist);
                    if(pathlist.size()>0)
                    {
                        ptempPath->SetExtremePoint(pathlist[0], pathlist[pathlist.size()-1]);
                        ptempPath->m_bMonopoly = true;
                        ptempPath->m_dAcrossID[0]  = -1;
                        ptempPath->m_dAcrossID[1]  = i;
                    }
                    acrossPointList[i].m_pPaths[j] = ptempPath;
                    bEnd = true;
                    break;
                }
            }

            //curNode is not a across node, then add curNode to path,then generate the next node
            if(!bEnd)
            {
                pathlist.push_back(CurNode);

                std::vector<bool>   ConnectivityList;
                int dConnectivityAirLine, dConnectivityEvilWay;
                StatisticsPixelConnectivity(gridMap,gridsize,color,CurNode,ConnectivityList,dConnectivityAirLine,dConnectivityEvilWay);

                // generate the next node
                for(int i =0; i< 8; i++)
                {
                    Vec2i tempNode = CurNode;
                    if(ConnectivityList[i])
                    {
                        tempNode + m_directionList[i];
                        bool  bfind = false;
                        for(auto it : pathlist)
                        {
                            if(it == tempNode)
                            {
                                bfind = true;
                                break;
                            }
                        }

                        if(!bfind)
                        {
                            CurNode = tempNode;
                            break;
                        }
                    }
                }
            }
        }
    }

    //find paths start from across Points
    for(int i =0; i < acrossPointList.size(); i++ )
    {
        for(int j =0; j < acrossPointList[i].m_bDirection.size(); j++)
        {
            //find direction need to find path
            if( (acrossPointList[i].m_bDirection[j]) && (acrossPointList[i].m_pPaths[j]==NULL) )
            {
                CoordinateList  pathlist;
                Vec2i  acrossNode_A(acrossPointList[i].x, acrossPointList[i].y);
                Vec2i  CurNode = acrossNode_A;
                CurNode + m_directionList[j];

                Path*  pHead = NULL;
                bool   bEnd = false;
                for(;!bEnd;)
                {
                    //if the curNode is across point,stop this path scan,then link this path to the across
                    for(int i =0; i < acrossPointList.size(); i++)
                    {
                        if((acrossPointList[i].x == CurNode.x)&&(acrossPointList[i].y == CurNode.y))
                        {
                            Vec2i  direction( ( pathlist[pathlist.size()-1].x - CurNode.x ), (pathlist[pathlist.size()-1].y - CurNode.y) );
                            int    j =0;
                            for(; j < 8 ; j++)
                                if( m_directionList[j] == direction )
                                    break;

                            Path*   ptempPath = new Path();
                            ptempPath->m_pMap = this;
                            ptempPath->SetPath(pathlist);
                            if(pathlist.size()>0)
                            {
                                ptempPath->SetExtremePoint(pathlist[0], pathlist[pathlist.size()-1]);
                                ptempPath->m_bMonopoly = false;
                            }
                            //ptempPath->m_dAcrossID[0]  = -1;
                            ptempPath->m_dAcrossID[1]  = i;
                            acrossPointList[i].m_pPaths[j] = ptempPath;
                            pHead = ptempPath;
                            bEnd  = true;
                            break;
                        }
                    }

                    //curNode is not a across node, then add curNode to path,then generate the next node
                    if(!bEnd)
                    {
                        pathlist.push_back(CurNode);

                        std::vector<bool>   ConnectivityList;
                        int dConnectivityAirLine, dConnectivityEvilWay;
                        StatisticsPixelConnectivity(gridMap,gridsize,color,CurNode,ConnectivityList,dConnectivityAirLine,dConnectivityEvilWay);

                        // generate the next node
                        for(int i =0; i< 8; i++)
                        {
                            Vec2i tempNode = CurNode;
                            if(ConnectivityList[i])
                            {
                                tempNode + m_directionList[i];

                                bool  bfind = false;
                                for(auto it : pathlist)
                                {
                                    if(it == tempNode)
                                    {
                                        bfind = true;
                                        break;
                                    }
                                }

                                if((pathlist.size()==1)&&(acrossNode_A==tempNode))
                                    bfind = true;

                                if(!bfind)
                                {
                                    CurNode = tempNode;
                                    break;
                                }
                            }
                        }
                    }
                }
                pHead->m_dAcrossID[0] = i;
                acrossPointList[i].m_pPaths[j] = pHead;
            }
        }
    }

}

void  Map::FindAcrossExtremePoint( cv::Mat  gridMap, int  gridsize, cv::Scalar color, std::vector<AcrossPoint>  &acrossPointList,
                                   std::vector<Vec2i> &airlineExtremePointList,  std::vector<Vec2i> &evilWayExtremePointList)
{
    int dWidth    = gridMap.cols / gridsize;
    int dHeigh    = gridMap.rows / gridsize;
    int dHalfsize = (gridsize/2)*2 < gridsize ? ( (gridsize/2) + 1 ) : (gridsize/2);

    for(int  j = 0; j < dHeigh; j++)
    {
        for(int  i = 0; i < dWidth; i++)
        {
            Vec2i  tempNode((i*gridsize+dHalfsize),(j*gridsize + dHalfsize));

            if( IsThisColor(gridMap, tempNode, color) )
            {
                std::vector<bool> ConnectivityList;
                Vec2i  gridIDNode(i,j);
                int dConnectivityAirLine, dConnectivityEvilWay;
                bool  flag = StatisticsPixelConnectivity(gridMap,gridsize,color,gridIDNode,ConnectivityList,dConnectivityAirLine,dConnectivityEvilWay);

                if(!flag)
                {
                    //error
                    cout<<"path can't be set as: 1 1"<<endl;
                    cout<<"                      1 1"<<endl;
                    return;
                }

                switch(dConnectivityAirLine)
                {
                case 4:
                case 3:
                {
                    AcrossPoint  tempNode;
                    tempNode.x = i;
                    tempNode.y = j;
                    tempNode.m_bDirection[0] = ConnectivityList[0];
                    tempNode.m_bDirection[1] = ConnectivityList[1];
                    tempNode.m_bDirection[2] = ConnectivityList[2];
                    tempNode.m_bDirection[3] = ConnectivityList[3];
                    tempNode.m_dID = acrossPointList.size();
                    acrossPointList.push_back(tempNode);
                }
                break;

                case 2:
                {
                    AcrossPoint  tempNode;
                    tempNode.x = i;
                    tempNode.y = j;
                    tempNode.m_bDirection[0] = ConnectivityList[0];
                    tempNode.m_bDirection[1] = ConnectivityList[1];
                    tempNode.m_bDirection[2] = ConnectivityList[2];
                    tempNode.m_bDirection[3] = ConnectivityList[3];
                    if(ConnectivityList[4] && !ConnectivityList[3] && !ConnectivityList[0])
                        tempNode.m_bDirection[4] = ConnectivityList[4];
                    if(ConnectivityList[5] && !ConnectivityList[0] && !ConnectivityList[1])
                        tempNode.m_bDirection[5] = ConnectivityList[5];
                    if(ConnectivityList[6] && !ConnectivityList[1] && !ConnectivityList[2])
                        tempNode.m_bDirection[6] = ConnectivityList[6];
                    if(ConnectivityList[7] && !ConnectivityList[2] && !ConnectivityList[3])
                        tempNode.m_bDirection[7] = ConnectivityList[7];

                    if(tempNode.m_bDirection[4] || tempNode.m_bDirection[5] || tempNode.m_bDirection[6] || tempNode.m_bDirection[7])
                    {
                        tempNode.m_dID = acrossPointList.size();
                        acrossPointList.push_back(tempNode);
                    }

                }
                break;

                case 1:
                {
                    AcrossPoint  tempNode;
                    tempNode.x = i;
                    tempNode.y = j;
                    tempNode.m_bDirection[0] = ConnectivityList[0];
                    tempNode.m_bDirection[1] = ConnectivityList[1];
                    tempNode.m_bDirection[2] = ConnectivityList[2];
                    tempNode.m_bDirection[3] = ConnectivityList[3];
                    if(ConnectivityList[4] && !ConnectivityList[3] && !ConnectivityList[0])
                        tempNode.m_bDirection[4] = ConnectivityList[4];
                    if(ConnectivityList[5] && !ConnectivityList[0] && !ConnectivityList[1])
                        tempNode.m_bDirection[5] = ConnectivityList[5];
                    if(ConnectivityList[6] && !ConnectivityList[1] && !ConnectivityList[2])
                        tempNode.m_bDirection[6] = ConnectivityList[6];
                    if(ConnectivityList[7] && !ConnectivityList[2] && !ConnectivityList[3])
                        tempNode.m_bDirection[7] = ConnectivityList[7];

                    if( 2 == (tempNode.m_bDirection[4] + tempNode.m_bDirection[5] + tempNode.m_bDirection[6] + tempNode.m_bDirection[7]) )
                    {
                        tempNode.m_dID = acrossPointList.size();
                        acrossPointList.push_back(tempNode);
                    }
                    else if(!tempNode.m_bDirection[4] && !tempNode.m_bDirection[5] && !tempNode.m_bDirection[6] && !tempNode.m_bDirection[7])
                        airlineExtremePointList.push_back(gridIDNode);

                }
                break;

                case 0:
                {
                    switch(dConnectivityEvilWay)
                    {
                    case 4:
                    case 3:
                        {
                        AcrossPoint  tempNode;
                        tempNode.x = i;
                        tempNode.y = j;
                        tempNode.m_bDirection[5] = ConnectivityList[5];
                        tempNode.m_bDirection[6] = ConnectivityList[6];
                        tempNode.m_bDirection[7] = ConnectivityList[7];
                        tempNode.m_bDirection[8] = ConnectivityList[8];
                        tempNode.m_dID = acrossPointList.size();
                        acrossPointList.push_back(tempNode);
                        }
                        break;
                    case 2:
                        break;
                    case 1:
                        evilWayExtremePointList.push_back(gridIDNode);
                        break;
                    default:
                        break;
                    }
                }
                break;

                }
            }
        }
    }
}


void  Map::ReleaseAcrossPoint( std::vector<AcrossPoint>   &AcrossPointList )
{
    for(int itA = 0; itA < AcrossPointList.size(); itA++)
    {
        for(int i =0; i < 8; i++)
        {
            if(AcrossPointList[itA].m_pPaths[i] != NULL)
            {
                Path*  tempHead  = AcrossPointList[itA].m_pPaths[i];
                bool   bFindSame = false;
                for(int itB = 0; itB < AcrossPointList.size(); itB++)
                {
                    if(itA != itB)
                    {
                        for(int j =0; j < 8; j++)
                        {
                            if(AcrossPointList[itB].m_pPaths[j] == tempHead)
                            {
                                free(AcrossPointList[itB].m_pPaths[j]);
                                AcrossPointList[itB].m_pPaths[j] = NULL;
                                bFindSame = true;
                                break;
                            }
                        }

                        if(bFindSame)
                            break;
                    }
                }

                if(!bFindSame)
                    free(AcrossPointList[itA].m_pPaths[i]);

                AcrossPointList[itA].m_pPaths[i] = NULL;
            }
        }
    }
}

void  Map::GeneralSubPath(Vec2i StartPos, Vec2i EndPos, std::vector<CombPath>& combPathList )
{
    Path*   tempPath1 = CheckPointBelong(StartPos);
    Path*   tempPath2 = CheckPointBelong(EndPos);

    //start end in one path
    if(tempPath1 == tempPath2)
    {
        int  dDirection = -1;
        tempPath1->CalDirection( StartPos, EndPos, dDirection);

        CombPath  tempPath(NULL,StartPos,EndPos,dDirection,tempPath1,COMBPATH_WAIT,0);
        combPathList.push_back(tempPath);
    }
    else  //start and end pos not in one path, calculate the across points list
    {
        int Path1_Across = -1;
        int Path2_Across = -1;

        int Across1[2];
        int Across2[2];
        if(tempPath1->m_bMonopoly)  //this path has only one Extreme pos
            Path1_Across = (tempPath1->m_dAcrossID[0] == -1) ? tempPath1->m_dAcrossID[1] : tempPath1->m_dAcrossID[0];
        else
        {
            Across1[0] = tempPath1->m_dAcrossID[0];
            Across1[1] = tempPath1->m_dAcrossID[1];
        }

        if(tempPath2->m_bMonopoly) //this path has only one Extreme pos
            Path2_Across = (tempPath2->m_dAcrossID[0] == -1) ? tempPath2->m_dAcrossID[1] : tempPath2->m_dAcrossID[0];
        else
        {
            Across2[0] = tempPath2->m_dAcrossID[0];
            Across2[1] = tempPath2->m_dAcrossID[1];
        }

        std::vector<int> AcrossList;
        if(( -1 != Path1_Across )&&(-1 != Path2_Across))
        {
            FindShortestAcrossList(Path1_Across, Path2_Across, m_AcrossPointList.size(),  AcrossList );
        }
        else if(( -1 != Path1_Across )&&(-1 == Path2_Across))
        {
            std::vector<int> AcrossList_A;
            std::vector<int> AcrossList_B;
            FindShortestAcrossList(Path1_Across, Across2[0], m_AcrossPointList.size(),  AcrossList_A );
            FindShortestAcrossList(Path1_Across, Across2[1], m_AcrossPointList.size(),  AcrossList_B );
            AcrossList = ( AcrossList_A.size() > AcrossList_B.size() ) ? AcrossList_B : AcrossList_A;
        }
        else if(( -1 == Path1_Across )&&(-1 != Path2_Across))
        {
            std::vector<int> AcrossList_A;
            std::vector<int> AcrossList_B;
            FindShortestAcrossList(Across1[0], Path2_Across, m_AcrossPointList.size(),  AcrossList_A );
            FindShortestAcrossList(Across1[1], Path2_Across, m_AcrossPointList.size(),  AcrossList_B );
            AcrossList = ( AcrossList_A.size() > AcrossList_B.size() ) ? AcrossList_B : AcrossList_A;
        }
        else if(( -1 == Path1_Across )&&(-1 == Path2_Across))
        {
            std::vector<int> AcrossList_A;
            std::vector<int> AcrossList_B;
            std::vector<int> AcrossList_C;
            std::vector<int> AcrossList_D;

            FindShortestAcrossList(Across1[0], Across2[0], m_AcrossPointList.size(), AcrossList_A );
            FindShortestAcrossList(Across1[0], Across2[1], m_AcrossPointList.size(), AcrossList_B );
            FindShortestAcrossList(Across1[1], Across2[0], m_AcrossPointList.size(), AcrossList_C );
            FindShortestAcrossList(Across1[1], Across2[1], m_AcrossPointList.size(), AcrossList_D );

            std::vector<int> AcrossList_E;
            std::vector<int> AcrossList_F;
            AcrossList_E = ( AcrossList_A.size() > AcrossList_B.size() ) ? AcrossList_B : AcrossList_A;
            AcrossList_F = ( AcrossList_C.size() > AcrossList_D.size() ) ? AcrossList_D : AcrossList_C;
            AcrossList   = ( AcrossList_E.size() > AcrossList_F.size() ) ? AcrossList_F : AcrossList_E;
        }

        if(AcrossList.size() > 0)
        {

            Vec2i         staPos;
            Vec2i         endPos;
            Path*         tempPathHead = NULL;
            int           dDirection = -1;
            staPos = StartPos;
            tempPathHead = tempPath1;
            tempPathHead->CalDirection(AcrossList[0], dDirection,  endPos);

            CombPath  tempPath(NULL,staPos,endPos,dDirection,tempPathHead,COMBPATH_WAIT,0);
            combPathList.push_back(tempPath);

            for(int i =0 ; i <AcrossList.size(); i++ )
            {
                Vec2i         staPos;
                Vec2i         endPos;
                Path*         tempPathHead = NULL;
                int           dDirection = -1;

                if( i != (AcrossList.size()-1) )  //not the last across
                {
                    int Across1 = AcrossList[i];
                    int Across2 = AcrossList[i+1];
                    for(int j =0; j < 8; j++)
                    {
                        if( NULL != m_AcrossPointList[Across1].m_pPaths[j] )
                        {
                            bool flag1 = (m_AcrossPointList[Across1].m_pPaths[j]->m_dAcrossID[0] == Across1 ) &&
                                         (m_AcrossPointList[Across1].m_pPaths[j]->m_dAcrossID[1] == Across2 );
                            bool flag2 = (m_AcrossPointList[Across1].m_pPaths[j]->m_dAcrossID[0] == Across2 ) &&
                                         (m_AcrossPointList[Across1].m_pPaths[j]->m_dAcrossID[1] == Across1 );

                            if(flag1)
                            {
                                if( 0 != m_AcrossPointList[Across1].m_pPaths[j]->GetPathSize() )
                                {
                                    tempPathHead = m_AcrossPointList[Across1].m_pPaths[j];
                                    m_AcrossPointList[Across1].m_pPaths[j]->GetExtremePoint(staPos,endPos);
                                    dDirection = 1;
                                }
                                break;
                            }

                            if(flag2)
                            {
                                if( 0 != m_AcrossPointList[Across1].m_pPaths[j]->GetPathSize() )
                                {
                                    tempPathHead = m_AcrossPointList[Across1].m_pPaths[j];
                                    m_AcrossPointList[Across1].m_pPaths[j]->GetExtremePoint(endPos,staPos);
                                    dDirection = 0;
                                }
                                break;
                            }
                        }
                    }

                    CombPath  tempPath(&m_AcrossPointList[Across1],staPos,endPos,dDirection,tempPathHead,COMBPATH_WAIT,(i+1));
                    combPathList.push_back(tempPath);
                }
                else   //the last across
                {
                    if(tempPath2->m_dAcrossID[0] ==  AcrossList[i])
                    {
                        tempPath2->GetPointByID(0, staPos);
                        endPos = EndPos;
                        tempPathHead = tempPath2;
                        dDirection = 1;
                    }
                    if(tempPath2->m_dAcrossID[1] ==  AcrossList[i])
                    {
                        tempPath2->GetPointByID(tempPath2->GetPathSize()-1, staPos);
                        endPos = EndPos;
                        tempPathHead = tempPath2;
                        dDirection = 0;
                    }

                    CombPath  tempPath(&m_AcrossPointList[AcrossList[i]],staPos,endPos,dDirection,tempPathHead,COMBPATH_WAIT,(i+1));
                    combPathList.push_back(tempPath);
                }
            }
        }
    }
}

Path* Map::CheckPointBelong(Vec2i& Pos)
{
    bool  bfind = false;
    Path  *pPath=NULL;
    for(int itA = 0; itA < m_AcrossPointList.size(); itA++)
    {
        for(int i =0; i < 8; i++)
        {
            if(m_AcrossPointList[itA].m_pPaths[i] != NULL)
            {
                for(int j =0; j< m_AcrossPointList[itA].m_pPaths[i]->GetPathSize();j++)
                {
                    Vec2i result;
                    m_AcrossPointList[itA].m_pPaths[i]->GetPointByID(j, result);

                    if(Pos == result)
                    {
                        bfind = true;
                        pPath = m_AcrossPointList[itA].m_pPaths[i];
                        return  pPath;
                    }
                }
            }
        }
    }

    if(bfind == false)
    {
        for(int itA = 0; itA < m_AcrossPointList.size(); itA++)
        {
            for(int i =0; i < 8; i++)
            {
                if(m_AcrossPointList[itA].m_pPaths[i] != NULL)
                {
                    for(int j =0; j< m_AcrossPointList[itA].m_pPaths[i]->GetPathSize();j++)
                    {
                        Vec2i result;
                        m_AcrossPointList[itA].m_pPaths[i]->GetPointByID(j, result);
                        int  dis = sqrt((Pos.x - result.x)*(Pos.x - result.x) + (Pos.y - result.y)*(Pos.y - result.y));

                        if(dis == 1)
                        {
                            bfind = true;
                            pPath = m_AcrossPointList[itA].m_pPaths[i];
                            Pos   = result;
                            return  pPath;
                        }
                    }
                }
            }
        }
    }

    return  pPath;
}

void  Map::CalculateMapPunishCostMatrix(std::vector<AcrossPoint> &acrossPointList, std::vector<std::vector<int>> &PunishCostMatrix)
{
    if(PunishCostMatrix.size()>0)
    {
        for( int i=0; i<PunishCostMatrix.size(); i++ )
            PunishCostMatrix[i].clear();
        PunishCostMatrix.clear();
    }

    if(acrossPointList.size() > 0)
    {

        for( int i=0; i<acrossPointList.size(); i++ )
        {
            vector<int>  temp;
            for( int j=0; j<acrossPointList.size(); j++ )
                temp.push_back(0);

            PunishCostMatrix.push_back(temp);
        }

        for(int i =0; i < acrossPointList.size(); i++)
        {
            for(int j=0; j< 8; j++)
            {
                if( acrossPointList[i].m_pPaths[j] != NULL )
                {
                    Path*  tempHead = acrossPointList[i].m_pPaths[j];
                    int    across1 = tempHead->m_dAcrossID[0];
                    int    across2 = tempHead->m_dAcrossID[1];
                    int    dPunishFactor = tempHead->m_AGVOccupyList.size() * PATHLENGTH_PUNISHFACTOR;
                    if((-1 != across1)&&(-1 != across2))
                    {
                        PunishCostMatrix[across1][across2] = dPunishFactor;
                        PunishCostMatrix[across2][across1] = dPunishFactor;
                    }
                }
            }
        }
    }
}

void  Map::CalculateMapCostMatrix(std::vector<AcrossPoint> &acrossPointList, std::vector<std::vector<int>> &CostMatrix)
{

    if(CostMatrix.size()>0)
    {
        for( int i=0; i<CostMatrix.size(); i++ )
            CostMatrix[i].clear();
        CostMatrix.clear();
    }

    if(acrossPointList.size() > 0)
    {
        for( int i=0; i<acrossPointList.size(); i++ )
        {
            vector<int>  temp;
            for( int j=0; j<acrossPointList.size(); j++ )
            {
                if(i==j)
                    temp.push_back(0);
                else
                    temp.push_back(INT_MAX);
            }

            CostMatrix.push_back(temp);
        }

        for(int i =0; i < acrossPointList.size(); i++)
        {
            for(int j=0; j< 8; j++)
            {
                if( acrossPointList[i].m_pPaths[j] != NULL )
                {
                    Path*  tempHead = acrossPointList[i].m_pPaths[j];
                    int    across1 = tempHead->m_dAcrossID[0];
                    int    across2 = tempHead->m_dAcrossID[1];
                    int    dLength = tempHead->GetPathSize() + 1;
                    if((-1!=across1)&&(-1!=across2))
                    {
                        CostMatrix[across1][across2] = dLength;
                        CostMatrix[across2][across1] = dLength;
                    }
                }
            }
        }
    }

}

void  Map::FindShortestAcrossList(int startAcrossID, int endAcrossID2, int dAcrossNum, std::vector<int>  &AcrossList )
{
    int   i,u,w;
    int   n = dAcrossNum;
    bool  found[n];
    int   distance[n];
    std::vector<std::vector<int>>   pathlist;
    for(int i=0; i< n; i++)
    {
        std::vector<int>  temp;
        pathlist.push_back(temp);
    }

    //bool  bPathLengthPunish = m_pManager->m_bPathLengthPunish;
    bool  bPathLengthPunish = false;

    for(i=0;i<n;i++)
    {
        found[i]=false;
        distance[i]=m_CostMatrix[startAcrossID][i];
        if(bPathLengthPunish)
            distance[i] += m_PunishCostMatrix[startAcrossID][i];

        pathlist[i].push_back(startAcrossID);
    }

    found[startAcrossID]=true;
    distance[startAcrossID]=0;
    for(i=0;i<n-1;i++)
    {
        u=ChooseNextAcross(distance,n,found);
        found[u]=true;
        pathlist[u].push_back(u);
        for(w=0;w<n;w++)
            if(!found[w])
            {

                int dPunishLength = bPathLengthPunish ? m_PunishCostMatrix[u][w] : 0;
                if( (INT_MAX!=m_CostMatrix[u][w]) && ( (distance[u]+m_CostMatrix[u][w] + dPunishLength) < distance[w]) )
                {
                    distance[w] = bPathLengthPunish ? distance[u]+m_CostMatrix[u][w] + dPunishLength : m_CostMatrix[u][w]+distance[u];
                    pathlist[w].clear();
                    pathlist[w] = pathlist[u];
                }
            }
    }

/*
    for(int j=0; j< n; j++)
    {
        cout<<"from "<<startAcrossID<<" to "<<j<<" :";
        for(int k=0; k< pathlist[j].size(); k++)
        {
            cout<<" "<<pathlist[j][k];
        }
        cout<<endl;
    }
*/
    AcrossList = pathlist[endAcrossID2];

}

int Map::ChooseNextAcross( int* distance, int n, bool *found )
{
    int i,min,minpos;
    min    = INT_MAX;
    minpos = -1;
    for(i=0;i<n;i++)
    {
        if( (0 < distance[i]) && (distance[i]<min) && (!found[i]) )
        {
            min    = distance[i];
            minpos = i;
        }
    }
    return minpos;
}

void  Map::DealApplyList()
{
    std::vector<ApplyInfor>  AloneApplyList;

    //clear all across 's tempApplyList
    for(auto it : m_AcrossPointList)
        it.m_tempApplyList.clear();

    //induce all applications from path to across's tempApplyList(across + path) and to AloneApplyList(path only)
    for(auto it : m_AcrossPointList)
    {
        for(int i =0; i < 8; i++)
        {
            if( (it.m_pPaths[i] != NULL) && (it.m_pPaths[i]->m_ApplyList.size() != 0) )
            {
                Path* pPath = it.m_pPaths[i];
                std::vector<ApplyInfor>::iterator ita;
                for(ita= pPath->m_ApplyList.begin(); ita != pPath->m_ApplyList.end(); ++ita )
                {
                    if( NULL == ita->pCombPath->m_pAcross )  // the first combpath
                    {
                        CombPathStatue  eApplyStatue = ita->pCombPath->GetStatue();
                        if(eApplyStatue != COMBPATH_PASS_OR_END)
                            AloneApplyList.push_back(*ita);
                        else
                            ita = pPath->m_ApplyList.erase(ita);
                    }
                    else
                    {
                        CombPathStatue  eApplyStatue = ita->pCombPath->GetStatue();
                        if(eApplyStatue != COMBPATH_PASS_OR_END)
                            ita->pCombPath->m_pAcross->m_tempApplyList.push_back(*ita);
                        else
                            ita = pPath->m_ApplyList.erase(ita);
                    }
                }
            }
        }
    }

    //deal these AloneApplyList(path only) first combpath
    for(int i =0; i< AloneApplyList.size(); i++)
    {
        CombPathStatue  eApplyStatue = AloneApplyList[i].pCombPath->GetStatue();
        if(eApplyStatue == COMBPATH_WAIT_DISPATCH)
            AloneApplyList[i].pCombPath->m_pPath->ApplyOnlyPath( &AloneApplyList[i]);
    }

    //deal applys on across as unit
    for(int i =0; i < m_AcrossPointList.size(); i++)
        //m_AcrossPointList[i].HandlingApply();
        m_AcrossPointList[i].HandlingApply_New();
}


void  Map::FindAllDodgePos( int  gridsize)
{
    std::map<Path*, bool>  pathList;

    for(int i = 0; i< m_AcrossPointList.size(); i++)
    {
        for(int j =0; j< 8; j++)
        {
            if( (m_AcrossPointList[i].m_pPaths[j] != NULL) && !(pathList[m_AcrossPointList[i].m_pPaths[j]]) )
            {
                FindPathDodgePos(m_AcrossPointList[i].m_pPaths[j], gridsize);
                pathList.insert(make_pair(m_AcrossPointList[i].m_pPaths[j], true));
            }
        }
    }

}

void Map::FindPathDodgePos(Path* pPath, int  gridsize)
{
    int dWidth    = m_GridMap.cols / gridsize;
    int dHeigh    = m_GridMap.rows / gridsize;
    int dHalfsize = (gridsize/2)*2 < gridsize ? ( (gridsize/2) + 1 ) : (gridsize/2);
    for(int i=0; i < pPath->GetPathSize(); i++)
    {
        Vec2i  PathPos;
        pPath->GetPointByID(i, PathPos);
        for(int j=0; j< 4; j++)
        {
            Vec2i  tempPos = PathPos;
            tempPos + m_directionList[j];
            Vec2i  tempPos1((tempPos.x*gridsize+dHalfsize),(tempPos.y*gridsize + dHalfsize));

            bool  bflag = IsThisColor(m_GridMap, tempPos1, cv::Scalar(255,255,255));
            if( bflag )
            {
                int dKeyV = tempPos.y * dWidth + tempPos.x;
                DodgePos  tempDodgePos(tempPos.x, tempPos.y, dKeyV);
                m_DodgePosList.insert(make_pair(dKeyV, tempDodgePos));
                pPath->m_DodgePosIDList.push_back(dKeyV);
            }
        }
    }

}

/*
void   Map::Test(int a)
{
    (this->*function_test[a])(  );
}
*/

/*
bool   Map::DealConflict_FindDodgePoint_Case4_BesideBackwardPath( Agent* dodgeAgv, Vec2i nearestCollPos, OccupyTimeElement* pOTMap, Vec2i& dodgePos, Path& dodgePath )
{
    bool bflag =false;
    Task*  temphead = dodgeAgv->m_pTaskGroup->GetTheProcessingTask();
    Path   path = temphead->m_path;

    int dCurIndex = 0;
    path.GetCurGridPosIndex(dCurIndex);
    CoordinateList  pathlist = path.GetPath();
    for(int i= dCurIndex; i > 0; i--)
    {
        for (uint j = 0; j < m_dDirection; ++j)
        {
            Vec2i newCoordinates = pathlist[i];
            newCoordinates + m_directionList[j];

            bool bfail = detectCollision(newCoordinates);
            if(!bfail)
            {
                bflag    = true;
                dodgePos = newCoordinates;
                break;
            }
        }

        if(bflag)
            break;
    }

    return  bflag;
}
*/
/*
bool   Map::DealConflict_FindDodgePoint_Case2_BesidePath( Agent* dodgeAgv,Vec2i nearestCollPos, OccupyTimeElement* pOTMap, Vec2i& dodgePos, Path& dodgePath )
{
    bool bflag =false;
    Task*  temphead = dodgeAgv->m_pTaskGroup->GetTheProcessingTask();
    Path   path = temphead->m_path;

    int dCurIndex = 0;
    path.GetCurGridPosIndex(dCurIndex);
    Vec2i curNode;
    path.GetPointByID(dCurIndex, curNode);
    int dColIndex = path.GetSpeGridPosIndex(nearestCollPos);
    CoordinateList  pathlist = path.GetPath();

    for(int i= (dColIndex - 1); i >= dCurIndex; i--)
    {
        //Vec2i  tempNode = pathlist[i];
        for (uint j = 0; j < m_dDirection; ++j)
        {
            Vec2i newCoordinates = pathlist[i];
            newCoordinates + m_directionList[j];

            bool bfail = detectCollision(newCoordinates);
            if(!bfail)
            {
                bflag    = true;
                dodgePos = newCoordinates;

                CoordinateList  pathlist;
                for( int j= dCurIndex; j<=i ;j++ )
                {
                    Vec2i tempNodeD;
                    path.GetPointByID(j, tempNodeD);
                    pathlist.push_back(tempNodeD);
                }
                pathlist.push_back(newCoordinates);
                dodgePath.SetExtremePoint(curNode, newCoordinates);
                dodgePath.SetPath(pathlist);

                break;
            }
        }

        if(bflag)
            break;
    }

    return  bflag;
}
*/
