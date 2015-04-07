#ifndef STI_MP_RASTERUTIL_H_
#define STI_MP_RASTERUTIL_H_

#include <string>
#include <vector>

using namespace std;



class Category
{
private:
    float threshold;
    Category* nextCategory;
    string* name;

public:
    ~Category();
    Category(string*, float);
    string* getName();
    float getThreshold();
    void setNextCategory(Category*);
    float minThreshold();
    float maxThreshold();

    operator float() { return threshold; }
    operator double() { return (double)threshold; }
};



class CutpointScale
{
private:
    Category** cutpoints;
    int numCats;

public:
    CutpointScale(vector<Category*>*);
    Category* defaultCategory();
    Category* getCategory(float);
    Category* getCategoryByIndex(int);
    int       getCategoryIndex(float);
    //float getAverageCategoryWidth();
};



#endif /* STI_MP_RASTERUTIL_H_ */

