#include "Utils.h"
#include <sstream>
#include <climits>

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