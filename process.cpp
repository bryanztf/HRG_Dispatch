#include "process.h"

#include "agent.h"
#include "taskgroup.h"


static std::map<int, std::string>  sProStatueList= {  {PT_LASERREFLO,"LASERREFLO"},
                                                      {PT_CAMERALO, "CAMERALO"},
                                                      {PT_PLACE,"PT_PLACE"},
                                                      {PT_PICKUP,"PICKUP"},
                                                      {PT_SENDDODGEPOS,"SENDDODGEPOS"},
                                                      {PT_DODGE,"DODGE"} };

Process::Process(ProcessType  proType, TaskGroup* pTaskGroup,  time_t fTime )
{
    m_fTime = time(NULL);
    m_eProType   = proType;
    m_eProStatue = PS_WAIT;
    m_ePreProStatue = PS_WAIT;

    m_pTaskGroup = pTaskGroup;

    m_bVarPara  = false;
    m_sVarParaName.clear();
    m_sCommandPlus.clear();
}

Process::Process( const Process& source )
{
    m_fTime         = source.m_fTime;
    m_eProType      = source.m_eProType;
    m_eProStatue    = source.m_eProStatue;
    m_ePreProStatue = source.m_ePreProStatue;

    m_pTaskGroup    = source.m_pTaskGroup;

    m_bVarPara      = source.m_bVarPara;
    m_sVarParaName  = source.m_sVarParaName;
    m_sCommandPlus  = source.m_sCommandPlus;
}

Process&  Process::operator = ( const Process& source )
{
    m_fTime         = source.m_fTime;
    m_eProType      = source.m_eProType;
    m_eProStatue    = source.m_eProStatue;
    m_ePreProStatue = source.m_ePreProStatue;

    m_pTaskGroup    = source.m_pTaskGroup;

    m_bVarPara      = source.m_bVarPara;
    m_sVarParaName  = source.m_sVarParaName;
    m_sCommandPlus  = source.m_sCommandPlus;
    return *this;
}

void   Process::SetVarPara(bool bResult)
{
    m_bVarPara = bResult;
}

bool   Process::GetVarPara()
{
    return m_bVarPara;
}

void   Process::SetVarParaName(std::string  VarParaName)
{
    m_sVarParaName = VarParaName;
}

std::string   Process::GetVarParaName()
{
    return m_sVarParaName;
}

std::string   Process::GetCommandPlusName()
{
    return m_sCommandPlus;
}

void   Process::SetCommandPlusName(std::string  CommandPlusName)
{
    m_sCommandPlus = CommandPlusName;
}

bool   Process::ChangeStatue( ProcessStatue  eNextStatue )
{
    bool  bflag = false;
    switch(m_eProStatue)
    {
    case PS_WAIT:
        switch(eNextStatue)
        {
        case PS_WAIT_ONGOING:
            m_ePreProStatue = m_eProStatue;
            m_eProStatue    = eNextStatue;
            bflag = true;
            break;
        default:
            break;
        }
        break;

    case PS_ONGOING:
        switch(eNextStatue)
        {
        case PS_STOP:
            m_ePreProStatue = m_eProStatue;
            m_eProStatue    = eNextStatue;
            bflag = true;
            break;
        case PS_ERROR:
            m_ePreProStatue = m_eProStatue;
            m_eProStatue    = eNextStatue;
            bflag = true;
            break;
        case PS_FINISH:
            m_ePreProStatue = m_eProStatue;
            m_eProStatue    = eNextStatue;
            bflag = true;
            break;
        default:
            break;
        }
        break;

    case PS_STOP:
        switch(eNextStatue)
        {
        case PS_ONGOING:
            m_ePreProStatue = m_eProStatue;
            m_eProStatue    = eNextStatue;
            bflag = true;
            break;
        default:
            break;
        }
        break;

    case PS_ERROR:
        switch(eNextStatue)
        {
        case PS_ONGOING:
            m_ePreProStatue = m_eProStatue;
            m_eProStatue    = eNextStatue;
            bflag = true;
            break;
        default:
            break;
        }
        break;

    case PS_FINISH:
        break;

    case PS_WAIT_ONGOING:
        switch(eNextStatue)
        {
        case PS_WAIT:
            m_ePreProStatue = m_eProStatue;
            m_eProStatue    = eNextStatue;
            bflag = true;
            break;
        case PS_ONGOING:
            m_ePreProStatue = m_eProStatue;
            m_eProStatue    = eNextStatue;
            bflag = true;
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }

    return  bflag;
}

ProcessStatue  Process::GetStatue()
{
    return m_eProStatue;
}

void   Process::ExcuteProcess()
{
    //PS_WAIT      0  ->1
    //PS_ONGOING   1  ->2   ->3   ->4
    //PS_STOP      2  ->1
    //PS_ERROR     3  ->1
    //PS_FINISH    4

    ProcessStatue  eProStatue = GetStatue();
    switch(eProStatue)
    {
    case PS_WAIT:
        {
            string  infor = sProStatueList[m_eProType];
            infor += m_sCommandPlus;
            m_pTaskGroup->m_pAgent->SendCommandToAGV(infor);
            ChangeStatue( PS_WAIT_ONGOING );
            m_fTime = time(NULL);
        }
        break;

    case PS_WAIT_ONGOING:
        {
            time_t  curTime = time(NULL);
            if( (curTime - m_fTime) > 15 )
                ChangeStatue( PS_WAIT );
            else
            {
                string  sFeedBack = m_pTaskGroup->m_pAgent->ListenFeedBackFromAGV();
                int OccurID = sFeedBack.find("Start");
                if(OccurID>=0)
                    ChangeStatue( PS_ONGOING );
            }
        }
        break;

    case PS_ONGOING:
        {
            string  sFeedBack = m_pTaskGroup->m_pAgent->ListenFeedBackFromAGV();
            if(!sFeedBack.empty()  && !m_sVarParaName.empty() )
            {
                if( AnalysisFeedBackString(sFeedBack, m_bVarPara) )
                {
                    m_pTaskGroup->m_VarBolList[m_sVarParaName] = m_bVarPara;
                    ChangeStatue( PS_FINISH );
                }
            }
        }
        break;

    case PS_STOP:
        break;

    case PS_ERROR:
        break;

    case PS_FINISH:
        break;

    default:
        break;
    }


}

bool   Process::AnalysisFeedBackString(std::string  sFeedBackString, bool  &bflag)
{

}
