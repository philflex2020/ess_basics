
#include <vector>
#include <string>

#pragma once
// todo 
//    track owners only delete when owners are all gone
//    add set/get val with locks
class dbVal {

public:
    dbVal(){};
    ~dbVal(){};
    enum dtype { DB_BASE, DB_NONE, DB_OBJ,DB_ARRAY,DB_STRING,DB_DOUBLE, DB_INT,DB_BOOL,DB_NULL,DB_END};
    dtype dbtype;
    std::vector<void*> ownvec;
    std::string valuestring;
union {
    double valuedouble;
    double valueint;
    bool valuebool;
    };
};