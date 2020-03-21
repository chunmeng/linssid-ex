
#ifndef _UTILS_H
#define	_UTILS_H

#include <string>

// Host all the utility functions
class Utils {
public:
    static int MaxIntStr(const std::string &s);
    static int MinIntStr(const std::string &s);
    static inline void waste(int);
};

inline void Utils::waste(int)
{
    // This silliness is to ignore an argument function's return code without
    // having the compiler whine about it.
}

#endif /* _UTILS_H */