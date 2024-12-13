// UserDefine.cc --- 
// 

#include "UserDefine.hh"

bool valid(int det, int id)
{
    if (det == 0 && id < 16 && id != 2) return true;
    return false;
}

// 
// UserDefine.h ends here
