#ifndef MAJORNODE_H
#define MAJORNODE_H

class  Point
{
private:
    int x;
    int y;

public:
    Point()
    {   x=0; y=0;}

    Point( int H, int V )
    {   x=H; y=V;}

    Point( Point& source)
    {
        source.GetData(x,y);
    }

    Point& operator=( Point& source)
    {
        source.GetData(x,y);
        return *this;
    }

    void GetData(int &X, int &Y)
    {
        X = x;
        Y = y;
    }
};

class Node
{
public:
    Node();
    Node(Point nodeCenter,  int time = 0 );

    void   SetNodeInfor(Point nodeCenter,  int time );
    void   GetNodeInfor(Point &nodeCenter,  int &time );

    void   SetNodeScore( float  fScore,float  gScore );
    void   GetNodeScore( float  &fScore, float  &gScore );

private:
    Point  m_c_nodeCenter;
    int    m_d_time;

    float  m_fScore;
    float  m_gScore;

};

#endif // MAJORNODE_H
