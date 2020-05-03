#include "Utils.h"
#include <climits>
#include <sstream>

using namespace std;

int Utils::MaxIntStr(const string &s)
{
    stringstream ss(s);
    string item;
    int retInt = INT_MIN;
    int tempInt;
    while (getline(ss, item, ' ')) {
        if (item.back() == '*') item=item.substr(0,item.length()-1);
        tempInt = atoi(item.c_str());
        if (tempInt > retInt) retInt = tempInt;
    }
    return retInt;
}

int Utils::MinIntStr(const string &s)
{
    stringstream ss(s);
    string item;
    int retInt = INT_MAX;
    int tempInt;
    while (getline(ss, item, ' ')) {
        if (item.back() == '*') item=item.substr(0,item.length()-1);
        tempInt = atoi(item.c_str());
        if (tempInt < retInt) retInt = tempInt;
    }
    return retInt;
}

vector<string> Utils::split(const string &s, char delim) {
    stringstream ss(s);
    string item;
    vector<string> elems;
    while (getline(ss, item, delim)) {
        elems.push_back(move(item)); // C++11
    }
    return elems;
}

// string trimming
//void Utils::trimRight( std::string& str )
//{
//    const std::string whiteSpaces( " \f\n\r\t\v" );
//    std::string::size_type pos = str.find_last_not_of( whiteSpaces );
//    str.erase( pos + 1 );    
//}
//
//void Utils::trimLeft( std::string& str )
//{
//    const std::string whiteSpaces( " \f\n\r\t\v" );
//    std::string::size_type pos = str.find_first_not_of( whiteSpaces );
//    str.erase( pos + 1 );    
//}