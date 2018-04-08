#include <stdio.h>
#include "map.h"
#include "path.h"
#include "agent.h"
#include "manager.h"
#include "realtaskgroup.h"

using namespace std;
using namespace AStar;

static std::map< std::string, int>  sProStatueList= { {"LASERREFLO",PT_LASERREFLO,},
                                                      {"CAMERALO",  PT_CAMERALO},
                                                      {"PT_PLACE",  PT_PLACE},
                                                      {"PICKUP",    PT_PICKUP},
                                                      {"DODGE",    PT_DODGE} };

RealTaskGroup::RealTaskGroup():TaskGroup()
{
    m_VarBolList.clear();
    m_VarPosList.clear();
    m_name.clear();
    m_sDefineName.clear();
    m_dCommandID = 0;
}

RealTaskGroup::RealTaskGroup( Manager *pManager ):TaskGroup( pManager )
{
    m_VarBolList.clear();
    m_VarPosList.clear();
    m_name.clear();
    m_sDefineName.clear();
    m_dCommandID = 0;
}

RealTaskGroup::~RealTaskGroup()
{
    if(m_pTaskHead != NULL)
    {
        delete m_pTaskHead;
        m_pTaskHead = NULL;
    }
}

Task* RealTaskGroup::GetCurrentTask()
{
    return m_pTaskHead;
}

void  RealTaskGroup::analysisTaskCommand()
{
    bool   bFindMove = false;
    Task*  pTaskHead = NULL;

    while( !bFindMove )
    {
        char    id[10];
        sprintf(id,"%d",m_dCommandID);
        string  commandID = "command_";
        string  ID = id;
        commandID += ID;

        string   command_content;
        m_fsr[commandID]>>command_content;

        if(command_content.empty())
            m_dCommandID++;
        else
        {
            int OccurID = command_content.find(";");
            string   KeyValue;
            string   KeyParam;
            string   KeyContent = command_content.substr(0, OccurID);
            command_content = command_content.substr(OccurID+1,command_content.size());

            if(KeyContent == "END")
            {
                ChangeTaskGroupStatue(TG_STATUE_FINISH);
                m_dCommandID = 0;
            }
            else
            {
                OccurID = KeyContent.find(":");
                if(OccurID>=0)
                {
                    KeyValue = KeyContent.substr(0, OccurID);
                    KeyParam = KeyContent.substr(OccurID + 1 , KeyContent.size());

                    if( KeyValue == "MOVE" )
                    {
                        bool flag = analysisTaskCommand_move(pTaskHead, KeyParam);
                        if( flag )
                        {
                            OccurID = command_content.find(":");
                            if(OccurID>=0)
                            {
                                KeyValue = command_content.substr(0, OccurID);
                                KeyParam = command_content.substr(OccurID + 1 , command_content.size());

                                if( KeyValue == "PROC" )
                                    analysisTaskCommand_proc(pTaskHead, KeyParam);
                            }
                            bFindMove = true;
                            m_dCommandID++;
                        }
                    }

                    if( KeyValue == "DEFINE" )
                    {
                        analysisTaskCommand_define( KeyParam );
                        m_dCommandID++;
                    }

                    if( KeyValue == "JUDGE" )
                        analysisTaskCommand_judge( KeyParam );
                }
            }
        }
    }

    if(bFindMove)
    {
        if(m_pTaskHead)
            free(m_pTaskHead);
        m_pTaskHead = NULL;
        m_pTaskHead = pTaskHead;
    }
}

void  RealTaskGroup::initializerTaskCommand()
{
    closedTaskDescribeFile();
    string   taskfile = "TaskDescribe_" + m_name + ".yml";

    if( (access( taskfile.c_str(), 0 )) != -1 )
        m_fsr.open( taskfile, cv::FileStorage::READ );
    else
        m_fsr.open( "TaskDescribe.yml", cv::FileStorage::READ );
}

void  RealTaskGroup::setTaskGroupName(std::string missionName)
{
    m_name = missionName;
}

void  RealTaskGroup::closedTaskDescribeFile()
{
    if( m_fsr.isOpened() )
        m_fsr.release();
}

bool  RealTaskGroup::analysisTaskCommand_move( Task* pTaskHead, std::string command_content )
{
    bool  flag = false;
    string  commandContent = command_content;

    int OccurID =commandContent.find("NULL");
    if(OccurID < 0)
    {
        //flag = true;
        OccurID  = commandContent.find(",");
        string  startstr = commandContent.substr(0, OccurID);
        commandContent = commandContent.substr(OccurID+1, commandContent.size());
        OccurID  = commandContent.find(";");
        string  endstr   = commandContent.substr(0, OccurID);

        Vec2i  startPos, endPos;
        bool flag1 = analysisTaskCommand_move_para(startstr, startPos);
        bool flag2 = analysisTaskCommand_move_para(endstr, endPos);

        if(flag1 && flag2)
        {
            flag = true;
            //*********  start a task   **********//
            /*
            Path  fixedpath = m_pMap->GetPathByExtremePoint( startPos, endPos );
            if( fixedpath.GetPathSize() > 0 )
            {
                pTaskHead = new Task( m_pMap, m_pAgent );
                pTaskHead->m_path = fixedpath;
                pTaskHead->Start();
            }
            else
            {
                std::cout<<"Path from: ["<<startPos.x<<","<<startPos.y<<"] to ["<<endPos.x
                         <<","<<endPos.y<<"] isn't exist, please check out!"<<std::endl;
            }
            */

            pTaskHead = new Task( m_pManager, m_pAgent );
            m_pAgent->m_pMap->GeneralSubPath(startPos, endPos, pTaskHead->m_combPathList );
        }
    }
    else
    {
        pTaskHead = new Task( m_pManager, m_pAgent );
        //pTaskHead->ChangeTaskStatue(TASK_STATUE_MOVEING);
        pTaskHead->ChangeTaskStatue(TASK_STATUE_PROCESS);
        flag = true;
    }

    return  flag;
}

bool  RealTaskGroup::analysisTaskCommand_move_para( std::string& paraName, AStar::Vec2i& position )
{
    bool    bresult = false;
    string  substr = paraName.substr(0,6);

    if(substr == "CURPOS")
    {
        /*  Test  */
        /*  position = m_pAgent->GetCurrentGridPosition();   */
        bresult = true;
    }
    else if(substr == "KEYPOS")
    {
        int  OccurID = paraName.find("_");
        string  para = paraName.substr( (OccurID+1), (paraName.size()-OccurID-1) );
        paraName = para;

        OccurID = paraName.find("_");
        if(OccurID >=0)
        {
            string  sDefineName = paraName.substr( 0, OccurID );
            string  suffixName  = paraName.substr( OccurID, (paraName.size()-OccurID) );

            if(
                (!sDefineName.empty() && !m_sDefineName.empty()) &&
                (sDefineName == m_sDefineName)
                    )
                paraName = m_name + suffixName;
        }
        /*  Test  */
        /*   bresult = m_pMap->GetOneKeyPosByName( paraName,  position);   */
        if(!bresult)
            std::cout<<"Key Position: "<<paraName<<" don't exist!"<<std::endl;
    }
    else if(substr == "VARPOS")
    {
        int  OccurID = paraName.find("_");
        string  para = paraName.substr( (OccurID+1), (paraName.size()-OccurID-1) );
        paraName = para;
        OccurID  = paraName.find("=");
        if(OccurID >= 0)
        {
            /*   query WMS to grab one position  */
            string Checkstr = paraName.substr( (OccurID+1), (paraName.size()-OccurID-1) );
            para     = paraName.substr( 0, OccurID );
            paraName = para;

            OccurID  = Checkstr.find("(");
            string Funstr = Checkstr.substr( 0, OccurID );
            Checkstr = Checkstr.substr( (OccurID+1), (Checkstr.size()-OccurID-1) );
            OccurID  = Checkstr.find(")");
            string Stockstr = Checkstr.substr( 0, OccurID );

            if(Funstr == "QUERYWMS")
            {
                int  result = 0;
                result = m_pManager->InquiryWMS(Stockstr, position);

                if(result == 1)
                {
                    bresult = true;
                    m_VarPosList.insert(std::make_pair(paraName, position));
                }
            }
        }
        else
        {
            /* find this var_pos in this Taskgroup  */
            auto iterator = m_VarPosList.begin();
            for( ;iterator != m_VarPosList.end(); )
            {
                if(iterator->first == paraName)
                {
                    position = iterator->second;
                    bresult   = true;
                    break;
                }
            }
        }
    }

    return bresult;
}

bool  RealTaskGroup::analysisTaskCommand_define( std::string command_content )
{
    string  commandContent = command_content;

    int OccurID =commandContent.find(";");
    if(OccurID >= 0)
    {
        string  defineName = commandContent.substr(0, OccurID);
        m_sDefineName = defineName;
    }

    return true;
}
void  RealTaskGroup::analysisTaskCommand_proc( Task* pTaskHead, std::string command_content )
{
    int OccurID = command_content.find(";");
    string KeyParam = command_content.substr(0 , OccurID);

    if(KeyParam == "NULL")
    {
         //Process  temp( PT_PICKUP );
         //temp.ChangeStatue(PS_ONGOING);
         //temp.ChangeStatue(PS_FINISH);
         pTaskHead->m_ActionList.clear();
    }
    else
    {
        int OccurID=0;
        command_content = KeyParam;
        do
        {
            OccurID = command_content.find(",");
            if(OccurID >= 0)
            {
                KeyParam = command_content.substr(0 , OccurID);
                command_content = command_content.substr(OccurID+1, command_content.size());
            }
            else
                KeyParam = command_content;

            int OccurID2 = KeyParam.find("=");
            if(OccurID2 < 0)
            {
                ProcessType  proType = (ProcessType)(sProStatueList[KeyParam]);
                Process  temp( proType, this );
                /*  Test  */
                pTaskHead->m_ActionList.push_back(temp);
            }
            else
            {
                string  varParaString = KeyParam.substr(0 , OccurID2);
                string  proTypeString = KeyParam.substr(OccurID2+1, KeyParam.size());

                OccurID2 = varParaString.find("_");

                string  varResultName = varParaString.substr(OccurID2+1, varParaString.size());
                bool    bResult = false;
                m_VarBolList.insert(make_pair(varResultName, bResult));

                ProcessType  proType = (ProcessType)(sProStatueList[proTypeString]);
                Process  temp( proType, this);
                temp.SetVarParaName(varResultName);
                /*  Test  */
                pTaskHead->m_ActionList.push_back(temp);
            }
        }
        while(OccurID >= 0);

    }
}

bool  RealTaskGroup::analysisTaskCommand_judge(std::string command_content)
{
    int OccurID = command_content.find("?");
    string varParamString = command_content.substr(0 , OccurID);
    command_content = command_content.substr(OccurID + 1 , command_content.size());
    OccurID = command_content.find(";");
    command_content = command_content.substr(0 , OccurID);

    OccurID = varParamString.find("_");
    string varParamName   = varParamString.substr(OccurID + 1 , varParamString.size());
    string  commandID;
    if(m_VarBolList[varParamName])
    {
        OccurID = command_content.find(",");
        string  commandIDString = command_content.substr(0 , OccurID);
        OccurID = commandIDString.find("_");
        commandID = commandIDString.substr(OccurID + 1 , commandIDString.size());
        m_dCommandID = atoi(commandID.c_str());
    }
    else
    {
        OccurID = command_content.find(",");
        string  commandIDString = command_content.substr(OccurID + 1 , command_content.size());
        OccurID = commandIDString.find("_");
        commandID = commandIDString.substr(OccurID + 1 , commandIDString.size());
        m_dCommandID = atoi(commandID.c_str());
    }

    return true;
}
