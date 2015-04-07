#include "thresholdraster.h"



ThresholdRaster::ThresholdRaster(IRaster* input, CutpointScale* cuts) : derivedFrom(input), cutpoints(cuts)
{
    cout << "Assigning threshold values to input raster";
    makeSameSizeRaster(derivedFrom);
    int cnt = 0;
    int nPixels = pixelCount();
    for(int xy = 0; xy < nPixels; xy++) {
        if(derivedFrom->isNoData(xy)) {
            raster[xy] = 0;
        } else {
            //float pixel = (float) **((double**) derivedFrom->getObjectAt(xy));
            float pixel = (float) derivedFrom->getDoubleAt(xy);
            //float pixel = ((Raster<float>*) derivedFrom)->getAt(xy);
            raster[xy] = cuts->getCategoryIndex(pixel);
        }
        if (++cnt % 50000 == 0)
            cout << ".";
    }
    cout << "...Done.\n";
}



Category* ThresholdRaster::getCategoryAt(int xy)
{
    return cutpoints->getCategoryByIndex(raster[xy]);
}

