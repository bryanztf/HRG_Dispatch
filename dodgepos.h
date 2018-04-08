#ifndef DODGEPOS_H
#define DODGEPOS_H

#include "AStar.hpp"

using namespace AStar;

class DodgePos : public Vec2i
{
public:
    DodgePos();
    DodgePos(int X, int Y, int dKey);
    DodgePos(const DodgePos& source);

    DodgePos& operator =  (const DodgePos& source);

    bool        OccupyLock();
    bool        ReleaseLock();
    bool        CheckLock();

    int         m_dKey;
private:
    bool        m_bOccupyLock;
};

#endif // DODGEPOS_H
