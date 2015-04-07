#include <stdexcept>

#include "regionmap.h"




RegionMap::RegionMap(ThresholdRaster* tRaster) : derivedFrom(tRaster)
{
    cluster();
}



Region* RegionMap::getRegion(int regionID)
{
    return regions[regionID];
}



int RegionMap::getRegionIDAt(int x, int y)
{
    return raster[(y * width) + x];
}



int RegionMap::getNumRegions()
{
    return nRegions;
}



void RegionMap::cluster()
{

    cout << "Finding disjoint regions in thresholded raster...";
    ThresholdRaster* level = derivedFrom;
    makeSameSizeRaster((IRaster*)derivedFrom);

    int i = 0;
    int j = 0;
    vector<int>* tree = new vector<int>();
    int maxNum = -1;
    bool pathological = false;
    int progressTick = height / 5;

    // Walk through the input raster
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {

            // This is a little easier to follow if we use names for these
            int idxCurrPixel = (y * width) + x;
            int idxLeftPixel = (y * width) + x - 1;
            int idxAbovPixel = (y * width) + x - width;

            // Does the current pixel have the same category as the pixel left of it?
            i = ((x > 0) && (level->getAt(idxCurrPixel) == level->getAt(idxLeftPixel))) ? 1 : 0;
            // Does the current pixel have the same category as the pixel above it?
            j = ((y > 0) && (level->getAt(idxCurrPixel) == level->getAt(idxAbovPixel))) ? 1 : 0;

            switch(2 * j + i) {
                case 0:
                    // We're different from both -- that means this is a new region
                    maxNum++;
                    tree->push_back(maxNum);
                    raster[idxCurrPixel] = maxNum;
                    break;
                case 1:
                    // We're the same as the one to the left, so copy its value
                    raster[idxCurrPixel] = raster[idxLeftPixel];
                    break;
                case 2:
                    // We're the same as the one above, so copy its value
                    raster[idxCurrPixel] = raster[idxAbovPixel];
                    break;
                case 3:
                    // We're the same as both -- that means two regions may need to merge
                    int a = raster[idxLeftPixel];
                    int b = raster[idxAbovPixel]; 
                    int c = min(tree->at(a), tree->at(b));
                    tree->at(a) = tree->at(b) = tree->at(tree->at(a)) = tree->at(tree->at(b)) = c;
                    raster[idxCurrPixel] = c;
                    break;
            }
        }
        if (y % progressTick == 0)
            cout << ".";
    }

    //Console.WriteLine("Found {0} regions or parts of regions...", maxNum + 1);

    // Eliminate redundant region identifiers...
    for(int a = 0; a <= maxNum; a++)
        tree->at(a) = tree->at(tree->at(a));

    // ... re-map the regions in the raster accordingly ...
    for(int i = 0; i < width * height; i++) {
        int RegionID = tree->at(raster[i]);
        raster[i] = RegionID;
    }

    // ... coalesce contiguous regions ...
    int* remap = new int[maxNum + 1];
    for(i = 0; i <= maxNum; i++) {
        remap[i] = -1;
    }
    j = 0;
    for(int ii = 0; ii <= maxNum; ii++) {
        if(tree->at(ii) == ii) {
            remap[ii] = j;
            j++;
        }
    }

    // ... and re-map one more time
    Category** qualNum = new Category*[j];
    for (i = 0; i < j; i++)
        qualNum[i] = NULL;
    for(int xy = 0; xy < width * height; xy++) {
        int regionID = raster[xy];
        int remapRegionID = remap[regionID];
        raster[xy] = remapRegionID;
        // While we're at it, grab the Category that corresponds to each region
        Category* cat = level->getCategoryAt(xy);
        if (qualNum[remapRegionID] == NULL)
            qualNum[remapRegionID] = cat;
        else if (qualNum[remapRegionID] != cat)
            throw new runtime_error("Algorithm bug: mis-identified threshold raster!");
        if(xy > 0 && xy % width > 0 && raster[xy] != raster[xy - 1] && qualNum[raster[xy]] == qualNum[raster[xy - 1]])
            pathological = true;
    }

    delete remap;

    // Build Region objects
    regions = new Region*[j];
    for(i = 0; i < j; i++)
        regions[i] = new Region(this, i, qualNum[i]);
    nRegions = j;

    if(pathological) {
        cout << "[!!] Found " << j << " regions\n";
        cout << "    WARNING: Some region boundaries have been mis-identified.\n    Spurious polygons or incorrect polygons may result.\n";
    } else {
        cout << "[ok] Found " << j << " regions\n";
    }
}

