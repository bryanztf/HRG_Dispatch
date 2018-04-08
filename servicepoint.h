#ifndef SERVICEPOINT_H
#define SERVICEPOINT_H

//#include "AStar.hpp"

#include "waypoint.h"
using namespace AStar;

class ServicePoint : public WayPoint
{
public:
    ServicePoint();
    ServicePoint(std::string  sName, bool bCall, int dCallPitchTime);

   ~ServicePoint();

    void          SetCallFlag();
    bool          GetCallFlag();
    //used for simulation
    void          CheckSimulationTrigger();

    //used for simulation
    int           m_dCallPitchTime;      //second
private:
    bool          m_bCall;
    time_t        m_dLastTimeCall;
    sem_t         m_uClock;
};

#endif // SERVICEPOINT_H
