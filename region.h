#ifndef STI_MP_REGION_H_
#define STI_MP_REGION_H_



#include "primitives.h"


using namespace std;


class RegionMap;
class Category;



class Region
{
private:
    RegionMap*      regionMap;
    int             regionID;
    Category*       category;
    Region*         parent;
    list<Region*>*  children;
    list<Region*>*  adjacentRegions;
    Polygon*        polygon;

    void            findParent();
    bool            hasChild(const Region*);
    void            removeChild(const Region*);
    list<Point*>*   traceRing(Coord*, bool);


public:

    Region(RegionMap*, int, Category*);

    // -------------- operator overloading ---------------
    bool operator ==(const Region& other)
    {
        return other.regionID == regionID;
    }
    bool operator !=(const Region& other)
    {
        return other.regionID != regionID;
    }

    int             getRegionID();
    Category*       getCategory();
    list<Region*>*  getChildren();
    void            addAdjacent(const Region*);
    void            addChild(const Region*);
    bool            touches(const Region*);
    bool            touches(int);
    Polygon*        toPolygon();
};



#endif /* STI_MP_REGION_H_ */

