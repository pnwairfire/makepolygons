#ifndef STI_MP_THRESHOLDRASTER_H_
#define STI_MP_THRESHOLDRASTER_H_



#include <iostream>

#include "rasterutil.h"
#include "raster.h"

using namespace std;


class ThresholdRaster : public Raster<int>
{
private:
    IRaster* derivedFrom;
    CutpointScale* cutpoints;

public:
    ThresholdRaster(IRaster*, CutpointScale*);
    Category*   getCategoryAt(int);
};


#endif /* STI_MP_THRESHOLDRASTER_H_ */



