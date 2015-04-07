#include <algorithm>
#include <stdexcept>

#include "region.h"
#include "regionmap.h"

using namespace std;



Region::Region(RegionMap* map, int rID, Category* cat)
    : regionMap(map), regionID(rID), category(cat)
{
    parent = NULL;
    polygon = NULL;
    children = new list<Region*>();
    adjacentRegions = NULL;
}



int Region::getRegionID()
{
    return regionID;
}



Category* Region::getCategory()
{
    return category;
}



void Region::addAdjacent(const Region* other)
{
    if (adjacentRegions == NULL)
        adjacentRegions = new list<Region*>();

    if (this == other)
        return;
        
    // Keep track of what other regions we're touching
    if (!touches(other)) {
        adjacentRegions->push_back((Region *)other);
        findParent();
    }
}



void Region::addChild(const Region* newChild)
{
    if (adjacentRegions == NULL)
        adjacentRegions = new list<Region*>();

    if (parent == newChild)
        return;

    if (!hasChild(newChild))
        children->push_back((Region *)newChild);

    findParent();
}



void Region::removeChild(const Region* oldChild)
{
    children->remove((Region *)oldChild);
    findParent();
}



void Region::findParent()
{
    int touchCount = 0;
    Region* possibleParent = NULL;

    if (adjacentRegions != NULL) {
        for (list<Region*>::iterator touch = adjacentRegions->begin(); touch != adjacentRegions->end();  touch++) {
            Region* currRegion = *touch;
            if (currRegion == NULL)
                continue;
            if (hasChild(currRegion))
                continue;
            touchCount++;
            possibleParent = currRegion;
        }
    }

    if (touchCount == 1) {
         // Hey, we're only touching one other polygon -- it must be our parent!
        parent = possibleParent;
        if(!parent->hasChild(this)) parent->addChild(this);
    } else if (parent != NULL) {
        // Oops, we thought this was our parent, but now we're touching another
        // polygon, so we need to remove ourself from our parent's list of children.
        if(parent->hasChild(this)) parent->removeChild(this);
        parent = NULL;
    }
}



bool Region::touches(int otherRegionID)
{
    return touches(regionMap->getRegion(otherRegionID));
}



bool Region::touches(const Region* otherRegion)
{
    if (adjacentRegions == NULL)
        return false;
    list<Region*>::iterator it = find(adjacentRegions->begin(), adjacentRegions->end(), otherRegion);
    
    return it !=  adjacentRegions->end();
}



bool Region::hasChild(const Region* maybeAChild)
{
    list<Region*>::iterator it = find(children->begin(), children->end(), maybeAChild);

    return it != children->end();
}



Polygon* Region::toPolygon()
{
    if (category->getName()->compare("**NoData**") == 0)
        return NULL;

    if(polygon == NULL) {
        list<Point*>* outerRing = NULL;
        list<list<Point*>*>* innerRings = new list<list<Point*>*>();            
        Coord* first = new Coord(0, 0);

        // Find the first point (topmost leftmost) of this region
        for(int y = 0; (y < regionMap->getHeight()) && (outerRing == NULL); y++)
            for(int x = 0; (x < regionMap->getWidth()) && (outerRing == NULL); x++)
                if(regionMap->getRegionIDAt(x, y) == regionID) {
                    first->X = x;
                    first->Y = y;

                    // Trace the outer ring
                    //Console.Write("Tracing outer ring for polygon #{0}...", regionID);
                    outerRing = traceRing(first, true);
                    //Console.WriteLine("[ok]");
                }
        if(outerRing == NULL) {
            throw new runtime_error("Algorithm bug: Region not found!");
        }

        // Find and trace inner rings
        for(int y = first->Y; y < regionMap->getHeight(); y++) {
            bool foundRegion = false;
            bool insideRegion = false;
            for(int x = 0; x < regionMap->getWidth(); x++) {
                int pixelRegion = regionMap->getRegionIDAt(x, y);
                // Skip pixels until we're inside our region
                if(!insideRegion) {
                    if(pixelRegion == regionID) {
                        insideRegion = true;
                        foundRegion = true;
                    } else {
                        continue;
                    }
                }
                // We've found a different region inside our region!
                if(pixelRegion != regionID) {
                    insideRegion = false;
                    
                    // If we already know we touch this other region, then we don't 
                    // need to trace the ring, since we've already found it in a previous
                    // ring (possibly the outer ring).
                    if (touches(pixelRegion))
                        continue;
                    
                    // Trace the inner ring
                    //Console.Write("Tracing inner ring #{1} for polygon #{0} (against region #{4} at {2}, {3})...", regionID, innerRings.Count + 1, x, y, pixelRegion);
                    addAdjacent(regionMap->getRegion(pixelRegion));
                    //List<Point> innerRing = TraceRing(new Coord(x, y + 1), true);
                    list<Point*>* innerRing = traceRing(new Coord(x, y), false);
                    //Console.WriteLine("[ok]");
                    innerRings->push_back(innerRing);
                }
            }
            // If we never found our region inside the current line then we don't need
            // to look at any further lines.
            if(!foundRegion)
                break;
        }

        polygon = new Polygon(this, outerRing, innerRings);
    }
    return polygon;
}



list<Point*>*   Region::traceRing(Coord* firstPoint, bool isRightHanded)
{
    list<Point*>* ring = new list<Point*>;

    // initial vector is eastward for right-handed ring, westward for left-handed
    Vector vec;
    if (isRightHanded) {
        vec.X = 1;
        vec.Y = 0;
    } else {
        vec.X = 0;
        vec.Y = 1;
    }

    // start with first point
    Coord p = *firstPoint;

    Point q;
    Coord outside, inside;
    Coord left;
    Coord right;
    int leftRegion, rightRegion;
    do {

        // The coordinate (p.X, p.Y) is actually on an edge between pixels.
        // Calculate the coordinates of some of the pixels around us.

        // The coordinate of the pixel to our left (outside the region)
        outside.X = p.X + (vec.Y + vec.X - 1) / 2;
        outside.Y = p.Y + (vec.Y - vec.X - 1) / 2;

        // The coordinate of the pixel to our right (inside the region)
        inside.X = p.X + (vec.X - vec.Y - 1) / 2;
        inside.Y = p.Y + (vec.X + vec.Y - 1) / 2;

        q.X = p.X;
        q.Y = p.Y;

        // OK, now we've computed q; add it to our ring
        ring->push_back(regionMap->getPixelCoord(q.X, q.Y));

        // This also means that the current region touches the region under
        // the outside pixel
        if(outside.X >= 0 && outside.X < regionMap->getWidth() && outside.Y >= 0 && outside.Y < regionMap->getHeight()) {
            addAdjacent(regionMap->getRegion(regionMap->getRegionIDAt(outside.X, outside.Y)));
        } else {
            addAdjacent(NULL);
        }

        // Move our current point (p) forward along our vector.  
        p = p + vec;

        // Make sure we don't go off the edge of the raster!
        if (p.X < 0)
            p.X = 0;
        if (p.X >= regionMap->getWidth())
            p.X = regionMap->getWidth();
        if (p.Y < 0)
            p.Y = 0;
        if( p.Y >= regionMap->getHeight())
            p.Y = regionMap->getHeight();


        // Now we need to decide if we are going to continue moving in the
        // direction we've been moving.  We move forward from our inside and 
        // outside pixels to get the pixels ahead and to the right and ahead 
        // and to the left.  If they are still respectively inside and outside 
        // the region, then we can keep moving forward in the same direction.
        left = outside + vec;
        right = inside + vec;

        // Find the region that would be under the new left pixel
        if(left.X >= 0 && left.X < regionMap->getWidth() && left.Y >= 0 && left.Y < regionMap->getHeight())
            leftRegion = regionMap->getRegionIDAt(left.X, left.Y);
        else
            leftRegion = -1;

        // Find the region that would be under the new right pixel
        if(right.X >= 0 && right.X < regionMap->getWidth() && right.Y >= 0 && right.Y < regionMap->getHeight())
            rightRegion = regionMap->getRegionIDAt(right.X, right.Y);
        else
            rightRegion = -1;

        // What are the pixels ahead-left and ahead-right?
        if (isRightHanded) {

            if (rightRegion != regionID) {
                // Ahead-right is not part of this region -- make a right turn
                vec.turnRight();
            } else if( leftRegion == regionID) {
                // Ahead-left is part of this region -- make a left turn
                vec.turnLeft();
            } // else keep on going in the same direction

        } else {

            if (leftRegion == regionID) {
                // Ahead-left is part of this region -- make a left turn
                vec.turnLeft();
            } else if (rightRegion != regionID) {
                // Ahead-right is not part of this region -- make a right turn
                vec.turnRight();
            } // else keep on going in the same direction

        }

        if (ring->size() > regionMap->pixelCount())
            throw new runtime_error("Algorithm bug: incorrect start pixel location!");
 
    } while (p != *firstPoint);

    return ring;
}

