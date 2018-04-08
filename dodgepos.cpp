#include "dodgepos.h"

DodgePos::DodgePos()
{
    m_dKey = 0;
    m_bOccupyLock = false;
}

DodgePos::DodgePos(int X, int Y, int dKey):Vec2i(X,Y)
{
    m_dKey = dKey;
    m_bOccupyLock = false;
}

DodgePos::DodgePos(const DodgePos& source):Vec2i(source)
{
    m_dKey        = source.m_dKey;
    m_bOccupyLock = source.m_bOccupyLock;
}

DodgePos&  DodgePos::operator =  (const DodgePos& source)
{
    Vec2i::operator=(source);

    if(this == &source)
        return *this;

    m_dKey        = source.m_dKey;
    m_bOccupyLock = source.m_bOccupyLock;

    return *this;
}

bool  DodgePos::CheckLock()
{
    return  m_bOccupyLock;
}

bool  DodgePos::OccupyLock()
{
    bool   flag = false;
    if(m_bOccupyLock == false)
    {
        m_bOccupyLock = true;
        flag    = true;
    }

    return  flag;
}

bool  DodgePos::ReleaseLock()
{
    bool   flag = false;
    if(m_bOccupyLock == true)
    {
        m_bOccupyLock = false;
        flag    = true;
    }

    return flag;
}


