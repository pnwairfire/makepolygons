#include <string>
#include <iostream>

#include <string.h>


using namespace std;



void trim(string& str)  
{  
    // find first and last non-whitespace chars 
     size_t first = str.find_first_not_of(" \t");  
     size_t last = str.find_last_not_of(" \t");  

     // if no non-whitespace chars return empty 
     if(first == string::npos)   
         str = "";   
     else  
         str = str.substr(first, last - first + 1);
}



void toLowerCase(string* inStr)
{
    for (unsigned int i = 0; i < inStr->length(); i++)
        (*inStr)[i] = tolower((*inStr)[i]);
}



bool isNumber(string* numStr)
{
    // first char can have '-' or '+'
    char currChar = (*numStr)[0];
    if (!isdigit(currChar) && currChar != '.' && currChar != '-' && currChar != '+')
        return false;

    // rest of chars must be digit or dot
    for (unsigned int i = 1; i < numStr->length(); i++) {
        currChar = (*numStr)[i];
        if (!isdigit(currChar) && currChar != '.')
            return false;
    }

    return true;
}



string* removeFromLastOccurrence(char* inStr, char last)
{
    string* orig = new string(inStr);
    int lastDot = orig->find_last_of(last, orig->length() - 1);
    if (lastDot < orig->length() - 1) {
        string retStr = orig->substr(0, lastDot);
        return new string(retStr);
    } else
        return new string(inStr);
}



string* pointToFilename(char* path, bool sansExt = false)
{
    char* lastSeparator = strrchr(path, '/');
    if (lastSeparator == NULL)
        lastSeparator = strrchr(path, '\\');

    char* retFName = lastSeparator == NULL ? path : lastSeparator + 1;

    return sansExt ? removeFromLastOccurrence(retFName, '.') : new string(retFName);
}

