#include "majornode.h"

Node::Node()
{

    m_d_time       = 0;
}


Node::Node(Point nodeCenter, int time )
{
    m_c_nodeCenter = nodeCenter;

    m_d_time       = time;
}

void  Node::SetNodeInfor(Point nodeCenter,  int time )
{
    m_c_nodeCenter = nodeCenter;

    m_d_time       = time;
}

void  Node::GetNodeInfor(Point &nodeCenter,  int &time )
{
     nodeCenter = m_c_nodeCenter;

     time       = m_d_time;
}

void   Node::SetNodeScore( float  fScore, float  gScore )
{
    m_fScore = fScore;
    m_gScore = gScore;
}

void   Node::GetNodeScore( float  &fScore, float  &gScore )
{
    fScore = m_fScore;
    gScore = m_gScore;
}
