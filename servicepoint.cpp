#include "servicepoint.h"

ServicePoint::ServicePoint()
{
    m_sName.clear();
    m_dCallPitchTime = 600;    //second
    m_bCall          = false;
    m_dLastTimeCall  = time(NULL);
    sem_init(&m_uClock,0,1);
}

ServicePoint::ServicePoint(std::string  sName, bool bCall, int dCallPitchTime): m_bCall(bCall),m_dCallPitchTime(dCallPitchTime)
{
    m_sName = sName;
    m_dLastTimeCall = time(NULL);
    sem_init(&m_uClock,0,1);
}

ServicePoint::~ServicePoint()
{
    sem_destroy(&m_uClock);
}

void  ServicePoint::SetCallFlag()
{
    sem_wait(&m_uClock);
    m_bCall = true;
    sem_post(&m_uClock);
}

bool  ServicePoint::GetCallFlag()
{
    bool  bflag;
    sem_wait(&m_uClock);
    bflag   = m_bCall;
    m_bCall = false;
    sem_post(&m_uClock);
    return bflag;
}

void  ServicePoint::CheckSimulationTrigger()
{
    time_t  dCurTime = time(NULL);
    if( (dCurTime - m_dLastTimeCall) >= m_dCallPitchTime)
    {
        SetCallFlag();
        m_dLastTimeCall = dCurTime;
    }
}
