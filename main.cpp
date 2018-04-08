#include "task.h"
#include <opencv2/opencv.hpp>
#include "taskgroup.h"
#include "map.h"
#include "manager.h"
#include "dodgepos.h"

int main(int argc, char *argv[])
{
/*
    Task   temp;
    temp.SetTimeStampToPath();
*/

/*
    string   mapConfigFile = "TaskDefine.yml";
    cv::FileStorage fsr( mapConfigFile, cv::FileStorage::WRITE );


    fsr<<"command_1"<<"DEFINE: WPNAME";
    fsr<<"command_11"<<"MOVE:CURPOS，KEYPOS_R;PROC:NULL;";
    fsr<<"command_12"<<"MOVE:KEYPOS_R,KEYPOS_A;PROC:NULL;";
    fsr<<"command_13"<<"MOVE:KEYPOS_A,VARPOS_GM=QUERYWMS(Matrial);PROC:LASERLO,CAMERALO,PICKUP;";
    fsr<<"command_14"<<"MOVE:VARPOS_GM,KEYPOS_A;PROC:NULL;";
    fsr<<"command_15"<<"MOVE:KEYPOS_A,KEYPOS_WPNAME_1;PROC:LASERLO,PLACE;";
    fsr<<"command_16"<<"MOVE:KEYPOS_WPNAME_1,KEYPOS_WPNAME_2;PROC:VARBOOL_Fexist=CAMERALO;";
    fsr<<"command_17"<<"JUDGE:VARBOOL_Fexist?JUMP:command_8,JUMP:command_14;";
    fsr<<"command_18"<<"MOVE:NULL;PROC:PICKUP;";
    fsr<<"command_19"<<"MOVE:KEYPOS_WPNAME_2,KEYPOS_P;PROC:NULL;";
    fsr<<"command_20"<<"MOVE:KEYPOS_P,VARPOS_PF=QUERYWMS(Frame);PROC:PLACE;";
    fsr<<"command_21"<<"MOVE:VARPOS_PF,KEYPOS_R;PROC:NULL;";
    fsr<<"command_22"<<"MOVE:KEYPOS_R,VARPOS_RP=QUERYWMS(Rest);PROC:NULL;";
    fsr<<"command_23"<<"END;";
    fsr<<"command_24"<<"MOVE:KEYPOS_WPNAME_2,KEYPOS_R;PROC:NULL;";
    fsr<<"command_25"<<"MOVE:KEYPOS_R,VARPOS_RP=QUERYWMS(Rest);PROC:NULL;";
    fsr<<"command_26"<<"END;";
    fsr.release();

    string   mapConfigFile = "TaskDefine.yml";
    cv::FileStorage fsr( mapConfigFile, cv::FileStorage::READ );
    if( fsr.isOpened() )
        cout<<"open it"<<endl;

    string  Command;
    fsr["command_15"]>>Command;
    cout<<Command<<endl;
    fsr.release();
*/

/*
    std::map<string, int>   temptest;
    temptest = {
               {"qwer",1},
               {"asdf",2}
               };

    //auto it = temptest.find("asdf");
    //std::cout<<it->first<<std::endl;

    int  abc =temptest["asff"];


    int b =0;
*/


/*
    TaskGroup   tempTaskGroup;
    tempTaskGroup.initializerTaskCommand();
    tempTaskGroup.analysisTaskCommand();

    string  ConfigFile = "SystemConfigureFile.yml";
    cv::FileStorage fsr( ConfigFile, cv::FileStorage::WRITE );

    fsr<<"APModel"<<"PARTICLE";
    fsr<<"PriorityBasis"<<"PATHLENGTH";
    fsr<<"SystemRunType"<<"SIMU";

    fsr<<"Map_1"<<"landrovermap_A.bmp";

    fsr<<"AGV_1"<<"AGV_1;1;192.168.0.101;1000;2000;500;20;0.6;0.6;";
    fsr<<"AGV_2"<<"AGV_2;2;192.168.0.102;1000;2000;500;20;0.6;0.6;";
    fsr<<"AGV_3"<<"AGV_3;3;192.168.0.103;1000;2000;500;20;0.6;0.6;";

    fsr<<"ServicePoint_1"<<"Head_1;10800;34,43;";
    fsr<<"ServicePoint_2"<<"Body_1;900;66,43;";
    fsr<<"ServicePoint_3"<<"Crank_1;3600;95,43;";
    fsr<<"ServicePoint_4"<<"Sleeve_1;4800;7,6;";

    fsr<<"WayPoint_1"<<"Head_2;27,40;";
    fsr<<"WayPoint_2"<<"Body_2;59,40;";
    fsr<<"WayPoint_3"<<"Crank_2;88,40;";
    fsr<<"WayPoint_4"<<"Sleeve_2;10,3;";
    fsr<<"WayPoint_5"<<"Wait_A;35,50;";
    fsr<<"WayPoint_6"<<"Wait_P;18,49;";
    fsr<<"WayPoint_7"<<"Wait_R;43,49;";

    fsr.release();
*/


    cv::Mat  gridmap = cv::imread("landrovermap_A.bmp");
    Map   temp;
    std::vector<Vec2i>        evilWayExtremePointList;
    std::vector<Vec2i>        airLineExtremePointList;
    int  gridsize = 5;
    temp.m_GridMap = gridmap.clone();
    temp.FindAcrossExtremePoint( gridmap, gridsize, cv::Scalar(255,0,0), temp.m_AcrossPointList, airLineExtremePointList, evilWayExtremePointList);
    temp.FindPathFromExtremeAcrossPoint(gridmap, gridsize, cv::Scalar(255,0,0), temp.m_AcrossPointList, airLineExtremePointList, evilWayExtremePointList);
    temp.CalculateMapCostMatrix(temp.m_AcrossPointList, temp.m_CostMatrix);
    temp.FindAllDodgePos(gridsize);

    std::vector<CombPath> combPathList;
    Vec2i StartPos(40,44);
    Vec2i EndPos(30,49);
    temp.GeneralSubPath(StartPos, EndPos, combPathList );

    cv::Mat     vectorPath = gridmap.clone();
    cv::Scalar  m_color[6] = {cv::Scalar(96,0,191),
                              cv::Scalar(0,255,255),
                              cv::Scalar(128,128,255),
                              cv::Scalar(0,64,0),
                              cv::Scalar(128,64,0),
                              cv::Scalar(200,200,200)};

    //show all paths
    int  dColorIndex = 0;
    for(int  j=0; j <  temp.m_AcrossPointList.size(); j++)
    {
        cout<<"across point "<<temp.m_AcrossPointList[j].m_dID<<": X "<<temp.m_AcrossPointList[j].x<<"  Y "<<temp.m_AcrossPointList[j].y<<endl;
        for(int i =0; i< 8; i++)
        {
            if( temp.m_AcrossPointList[j].m_pPaths[i] != NULL )
            {
                Path *phead = temp.m_AcrossPointList[j].m_pPaths[i];
                CoordinateList   pathlist = phead->GetPath();
                for( auto iteator : pathlist )
                {
                    cv::Rect Roi(iteator.x*gridsize+1,iteator.y*gridsize+1,gridsize-1,gridsize-1);
                    cv::Mat  SubMap = vectorPath( Roi );
                    SubMap.setTo(m_color[dColorIndex]);
                }
                dColorIndex++;
                dColorIndex %= 5;
                cout<<"    path： across "<<phead->m_dAcrossID[0]<<" to "<<"across "<<phead->m_dAcrossID[1]<<endl;
            }
        }
    }

    //show all dodge pos
    for( auto iteator : temp.m_DodgePosList )
    {
        cv::Rect Roi(iteator.second.x*gridsize+1,iteator.second.y*gridsize+1,gridsize-1,gridsize-1);
        cv::Mat  SubMap = vectorPath( Roi );
        SubMap.setTo(m_color[5]);
    }


    //show cost matrix
    for(int i=0; i < temp.m_CostMatrix.size(); i++)
    {
        for(int j=0; j < temp.m_CostMatrix[i].size(); j++)
        {
            cout<<" ";
            if( (temp.m_CostMatrix[i][j] >= 0)&&(temp.m_CostMatrix[i][j] < 10) )
                cout<<"  "<<temp.m_CostMatrix[i][j];
            if( (temp.m_CostMatrix[i][j] >= 10)&&(temp.m_CostMatrix[i][j] < 100) )
                cout<<" "<<temp.m_CostMatrix[i][j];
            else if(temp.m_CostMatrix[i][j] == INT_MAX)
            {
                cout<<" "<<-1;
            }

        }
        cout<<endl;
    }

    cv::imshow("Map&Path", vectorPath);
    cv::waitKey(0);



/*
    Manager   temp;
    temp.ConfigureInitialize();
*/

/*
    Map   temp;

    std::vector<int>  tempVector;
    tempVector= {0,2,4,1,INT_MAX,INT_MAX,INT_MAX};
    temp.m_CostMatrix.push_back(tempVector);
    tempVector.clear();
    tempVector= {2,0,INT_MAX,INT_MAX,10,INT_MAX,INT_MAX};
    temp.m_CostMatrix.push_back(tempVector);
    tempVector.clear();
    tempVector= {4,INT_MAX,0,5,INT_MAX,1,INT_MAX};
    temp.m_CostMatrix.push_back(tempVector);
    tempVector.clear();
    tempVector= {1,INT_MAX,5,0,INT_MAX,1,INT_MAX};
    temp.m_CostMatrix.push_back(tempVector);
    tempVector.clear();
    tempVector= {INT_MAX,10,INT_MAX,INT_MAX,0,INT_MAX,6};
    temp.m_CostMatrix.push_back(tempVector);
    tempVector.clear();
    tempVector= {INT_MAX,INT_MAX,1,1,INT_MAX,0,1};
    temp.m_CostMatrix.push_back(tempVector);
    tempVector.clear();
    tempVector= {INT_MAX,INT_MAX,INT_MAX,INT_MAX,6,1,0};
    temp.m_CostMatrix.push_back(tempVector);

    tempVector.clear();
    tempVector= {0,0,8,4,0,0,0};
    temp.m_PunishCostMatrix.push_back(tempVector);
    tempVector.clear();
    tempVector= {0,0,0,0,0,0,0};
    temp.m_PunishCostMatrix.push_back(tempVector);
    tempVector.clear();
    tempVector= {8,0,0,0,0,0,0};
    temp.m_PunishCostMatrix.push_back(tempVector);
    tempVector.clear();
    tempVector= {4,0,0,0,0,12,0};
    temp.m_PunishCostMatrix.push_back(tempVector);
    tempVector.clear();
    tempVector= {0,0,0,0,0,0,0};
    temp.m_PunishCostMatrix.push_back(tempVector);
    tempVector.clear();
    tempVector= {0,0,0,12,0,0,0};
    temp.m_PunishCostMatrix.push_back(tempVector);
    tempVector.clear();
    tempVector= {0,0,0,0,0,0,0};
    temp.m_PunishCostMatrix.push_back(tempVector);

    std::vector<int> AcrossList;
    temp.FindShortestAcrossList(0, 2 , 7, AcrossList);
*/


/*
    DodgePos  temp1(1,2,100);
    DodgePos  temp2(1,3,101);

    vector<DodgePos>  tempList;
    tempList.push_back(temp1);
    tempList.push_back(temp2);

    DodgePos  temp3,temp4;
    temp3 = tempList[0];
    temp4 = tempList[1];

    temp3 = temp4;

    std::map<int , DodgePos>  tempDodgelist;
    tempDodgelist.insert(make_pair(temp1.m_dKey,temp1));
    tempDodgelist.insert(make_pair(temp2.m_dKey,temp2));

    DodgePos  temp5,temp6;
    temp5=tempDodgelist[temp1.m_dKey];
    temp6=tempDodgelist[temp2.m_dKey];

    DodgePos  temp7 = tempDodgelist[99];
*/
    int  a = 0;
}
