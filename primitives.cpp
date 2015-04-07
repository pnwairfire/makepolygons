#include <list>
#include "primitives.h"



/* ----------------------- Vector ------------------------- */

Vector::Vector() : X(0), Y(0)
{}

Vector::Vector(int x, int y) : X(x), Y(y)
{}

void Vector::turnLeft()
{
    int x = X;
    X = Y;
    Y = -x;
} 

void Vector::turnRight()
{
    int x = X;
    X = -Y;
    Y = x;
}



/* ----------------------- Coord ------------------------- */

Coord::Coord() : X(0), Y(0)
{}

Coord::Coord(int x, int y) : X(x), Y(y)
{}



int Coord::getHashCode()
{
    return X ^ Y;
}



/* ----------------------- Point ------------------------- */

Point::Point() : X(0), Y(0)
{}

Point::Point(double x, double y) : X(x), Y(y)
{}



/* ----------------------- Polygon ------------------------- */

Polygon::Polygon(Region* defrom, list<Point*>* oring, list<list<Point*>*>* irings) : derivedFrom(defrom), outerRing(oring), innerRings(irings)
{}

