#ifndef STI_MP_PRIMITIVESS_H_
#define STI_MP_PRIMITIVESS_H_

#include <list>

using namespace std;



#define NULL_DOUBLE_    numeric_limits<double>::min()
#define NULL_FLOAT_     numeric_limits<float>::min()


class Vector
{
public:
    int X;
    int Y;

    Vector();
    Vector(int x, int y);

    void turnLeft(); 
    void turnRight();
};



class Coord
{
public:
    int X;
    int Y;

    Coord();
    Coord(int, int);

    int getHashCode();


    // -------------- operator overloading ---------------
    Coord operator +(const Coord& other)
    {
        return Coord( X + other.X, Y + other.Y);
    }
    Coord operator +(const Vector& other)
    {
        return Coord( X + other.X, Y + other.Y);
    }
    bool operator ==(const Coord& other)
    {
        return X == other.X && Y == other.Y;
    }
    bool operator !=(const Coord& other)
    {
        return X != other.X || Y != other.Y;
    }
};



class Point
{
public:
    double X;
    double Y;

    Point();
    Point(double, double);
};



class Region;

class Polygon
{
public:
    Region*                 derivedFrom;
    list<Point*>*           outerRing;
    list<list<Point*>*>*    innerRings;
   
    Polygon(Region*, list<Point*>*, list<list<Point*>*>*);
};



#endif /* STI_MP_PRIMITIVESS_H_ */

