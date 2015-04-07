#ifndef STI_MP_REGIONMAP_H_
#define STI_MP_REGIONMAP_H_

#include "raster.h"
#include "thresholdraster.h"
#include "region.h"


class RegionMap : public Raster<int>
{
private:
    ThresholdRaster*    derivedFrom;
    Region**            regions;
    int                 nRegions;

    void                cluster();

public:
    RegionMap(ThresholdRaster*);
    Region* getRegion(int regionID);
    int getRegionIDAt(int x, int y);
    int getNumRegions();
};


#endif /* STI_MP_REGIONMAP_H_ */

