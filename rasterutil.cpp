#include <limits>
#include <iostream>
#include <algorithm>

#include "rasterutil.h"
#include "primitives.h"

using namespace std;



/* ----------------------- Category ------------------------- */

Category::Category(string* n, float t)
{
    name = n;
    threshold = t;
}

Category::~Category() {
    delete name;
}

string* Category::getName()
{
    return name;
}



float Category::getThreshold()
{
    return threshold;
}



void Category::setNextCategory(Category* n)
{
    nextCategory = n;
}


float Category::minThreshold()
{
    return threshold;
}



float Category::maxThreshold()
{
    return nextCategory == NULL ? numeric_limits<float>::max( ) : nextCategory->minThreshold();
}




/* ----------------------- CutpointScale ------------------------- */

bool categoryCompare(Category* first, Category* second)
{
    return first->getThreshold() < second->getThreshold();
}

CutpointScale::CutpointScale(vector<Category*>* cuts)
{   
    sort(cuts->begin(), cuts->end(), categoryCompare);
    Category* empty = new Category(new string("**NoData**"), NULL_FLOAT_);
    this->numCats = cuts->size() + 1;
    this->cutpoints = new Category*[numCats];
    this->cutpoints[0] = empty;
    for(int i = 1; i < this->numCats; i++) {
        this->cutpoints[i] = cuts->at(i - 1);
        this->cutpoints[i - 1]->setNextCategory(this->cutpoints[i]);
    }
    this->cutpoints[this->numCats - 1]->setNextCategory(NULL);
}



Category* CutpointScale::defaultCategory()
{
    return cutpoints[0];
}



Category* CutpointScale::getCategoryByIndex(int ind)
{
    return cutpoints[ind];
}



Category* CutpointScale::getCategory(float value)
{
    return cutpoints[getCategoryIndex(value)];
}



int CutpointScale::getCategoryIndex(float value)
{
    for(int i = this->numCats - 1; i >= 0; i--) {
        if(cutpoints[i]->minThreshold() <= value) {
            return i;
        }
    }

    cout << "WARNING: Value " << value << " in input data does not correspond to any defined categories.\n   Assigning category " << defaultCategory()->getName();
    return 0;
}



//float CutpointScale::getAverageCategoryWidth()
//{
//  float sumWidths = 0;
//  int nWidths = 0;
//  list<Category*>::iterator catIt;
//  for (catIt = cutpoints->begin(); catIt != cutpoints->end(); catIt++) {
//      Category cat = **catIt;
//      float width = cat.maxThreshold() - cat.minThreshold();
//      if (width == numeric_limits<float>::max( ))
//          continue;
//      sumWidths += width;
//      nWidths++;
//  }
//
//  return sumWidths / nWidths;
//}
    


