#ifndef PROCESS_H
#define PROCESS_H

#include "AStar.hpp"

using namespace AStar;

class TaskGroup;
class Process
{
public:
    Process( ProcessType  proType, TaskGroup* pTaskGroup, time_t fTime = 0 );
    Process( const Process& source );
    Process&  operator = ( const Process& source );


    //the real agv's action, be different from the Process_Type, you can realize this function by facts
    //virtual void Action() {};
    ProcessStatue     GetStatue();
    bool   ChangeStatue( ProcessStatue  eNextStatue );

    void   SetVarPara(bool bResult);
    bool   GetVarPara();

    std::string   GetVarParaName();
    void   SetVarParaName(std::string  VarParaName);

    std::string   GetCommandPlusName();
    void   SetCommandPlusName(std::string  CommandPlusName);

    bool   AnalysisFeedBackString(std::string  sFeedBackString, bool  &bflag);
    void   ExcuteProcess();

    TaskGroup*     m_pTaskGroup;

private:
    time_t         m_fTime;

    ProcessType    m_eProType;

    ProcessStatue  m_eProStatue;
    ProcessStatue  m_ePreProStatue;


    bool           m_bVarPara;
    std::string    m_sVarParaName;

    std::string    m_sCommandPlus;
};

#endif // PROCESS_H
