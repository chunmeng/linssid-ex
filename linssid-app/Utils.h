
#ifndef _UTILS_H
#define	_UTILS_H

#include <string>
#include <vector>

// Host all the utility functions
class Utils {
public:
    static int MaxIntStr(const std::string &s);
    static int MinIntStr(const std::string &s);
    static std::vector<std::string> split(const std::string &s, char delim);
    static inline void waste(int);
};

inline void Utils::waste(int)
{
    // This silliness is to ignore an argument function's return code without
    // having the compiler whine about it.
}

#endif /* _UTILS_H */