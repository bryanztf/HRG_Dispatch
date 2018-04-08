#ifndef WAYPOINT_H
#define WAYPOINT_H

#include "AStar.hpp"
using namespace AStar;

class WayPoint : public Vec2i
{
public:
    WayPoint();

    std::string  m_sName;
};

#endif // WAYPOINT_H
