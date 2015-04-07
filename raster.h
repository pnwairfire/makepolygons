#ifndef STI_MP_RASTER_H_
#define STI_MP_RASTER_H_



#include <string>
//#include <typeinfo.h>
#include <limits>


#include "primitives.h"
#include "rasterutil.h"

using namespace std;



class IRaster
{
public:
    virtual int pixelCount() = 0;
    virtual double getDoubleAt(int, int) = 0;
    virtual double getDoubleAt(int) = 0;
    virtual void* getObjectAt(int, int) = 0;
    virtual void* getObjectAt(int) = 0;
    virtual bool isNoData(int, int) = 0;
    virtual bool isNoData(int) = 0;
    virtual Point* getPixelCoord(double, double) = 0;
    virtual int getWidth() = 0;
    virtual int getHeight() = 0;
    virtual double getCellSizeX() = 0;
    virtual double getCellSizeY() = 0;
    virtual double getMinX() = 0;
    virtual double getMinY() = 0;
    virtual double getSkewX() = 0;
    virtual double getSkewY() = 0;
    virtual const string* getSpatialRef() = 0;
    virtual void setSpatialRef(string*) = 0;
    virtual double getNoDataValue() = 0;
    
protected:
    virtual void        makeSameSizeRaster(IRaster*) = 0;
    virtual IRaster*    makeResizedRaster(double factor) = 0;
};



template<class T>
class Raster : public IRaster
{
protected:
    T*      raster;
    int     width;
    int     height;
    double  cellSizeX;
    double  cellSizeY;
    double  minX;
    double  minY;
    double  skewX;
    double  skewY;
    string* spatialRef;
    double  noDataValue;

public:
    Raster() {}
    Raster(T*, int, int, double, double, double, double, double, double, string*, double);

    int getWidth() { return width; }
    int getHeight() { return height; };
    double getCellSizeX() { return cellSizeX; }
    double getCellSizeY() { return cellSizeY; }
    double getMinX() { return minX; }
    double getMinY() { return minY; }
    double getSkewX() { return skewX; }
    double getSkewY() { return skewY; }
    const string* getSpatialRef() { return spatialRef; }
    void setSpatialRef(string* sr) { spatialRef = sr; }
    double getNoDataValue() { return noDataValue; }

    int pixelCount();
    T getAt(int, int);
    T getAt(int);
    double getDoubleAt(int, int);
    double getDoubleAt(int);
    void* getObjectAt(int, int);
    void* getObjectAt(int);
    T* rasterData();
    bool isNoData(int, int);
    bool isNoData(int);
    Point* getPixelCoord(double, double);
    
protected:
    void        makeSameSizeRaster(IRaster*);
    Raster<T>*  makeResizedRaster(double factor);
};



template<class T>
Raster<T>::Raster(T* c, int w, int h, double csx, double csy, double mx, double my, double skx, double sky, string* spr, double ndv)          
                  : raster(c), width(w), height(h), cellSizeX(csx), cellSizeY(csy), minX(mx), minY(my), skewX(skx), skewY(sky), spatialRef(spr), noDataValue(ndv)
{}



template<class T>
int Raster<T>::pixelCount()
{
    return width * height;
}

template<class T>
T Raster<T>::getAt(int x, int y)
{
    return raster[y * width + x];
}

template<class T>
T Raster<T>::getAt(int xy)
{
    return raster[xy];
}

template<class T>
double Raster<T>::getDoubleAt(int x, int y)
{
    return (double) raster[y * width + x];
}

template<class T>
double Raster<T>::getDoubleAt(int xy)
{
    return (double) raster[xy];
}

template<class T>
void* Raster<T>::getObjectAt(int x, int y) {
    return &raster[y * width + x];
}

template<class T>
void* Raster<T>::getObjectAt(int xy) {
    return &raster[xy];
}

template<class T>
T* Raster<T>::rasterData()
{
    return raster;
}


template<class T>
bool Raster<T>::isNoData(int x, int y)
{
    return noDataValue == NULL_DOUBLE_ ? false : getAt(x, y) == noDataValue;
}



template<class T>
bool Raster<T>::isNoData(int xy)
{
    return noDataValue == NULL_DOUBLE_ ? false : getAt(xy) == noDataValue;
}



template<class T>
Point* Raster<T>::getPixelCoord(double x, double y)
{
    double geoX = minX + (x * cellSizeX) + (y * skewX);
    double geoY = minY + (x * skewY) + (y * cellSizeY);
    return new Point(geoX, geoY);
}



template<class T>
void Raster<T>::makeSameSizeRaster(IRaster* other)
{
    raster = new T[other->pixelCount()];
    width = other->getWidth();
    height = other->getHeight();
    cellSizeX = other->getCellSizeX();
    cellSizeY = other->getCellSizeY();
    minX = other->getMinX();
    minY = other->getMinY();
    skewX = other->getSkewX();
    skewY = other->getSkewY();
    spatialRef = new string(*(other->getSpatialRef()));
}



template<class T>
Raster<T>* Raster<T>::makeResizedRaster(double factor)
{
    Raster<T>* other = new Raster<T>();
    other->width = width * (int)factor;
    other->height = height * (int)factor;
    other->raster = new T[other->width * other->height];
    other->cellSizeX = cellSizeX / factor;
    other->cellSizeY = cellSizeY / factor;
    other->minX = minX - (cellSizeX / 2) + (other->cellSizeX / 2);
    other->minY = minY - (cellSizeY / 2) + (other->cellSizeY / 2);
    other->skewX = skewX / factor;
    other->skewY = skewY / factor;
    other->spatialRef = spatialRef;
    other->noDataValue = noDataValue;

    return other;
}



#endif /* STI_MP_RASTER_H_ */

