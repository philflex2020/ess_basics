/*
 BaseUtils
 p wilshire
 02062022

  checkUri   
  filter out /ess 
                 /full
                 /naked
                 /.... etc  
    look for blocks
 */

#include <vector>
#include <ostream>
#include <iostream>

#pragma once

class dbSj;

static std::map<dbSj*,dbSj*> dbSjMap;
struct cstr{ const char* str;};

class baseUtils {

public:
    baseUtils() ; //int& flag, const*char*base, const char* uri);
    ~baseUtils();
    checkUri(const char* uri);
    std::map<dbSj*,dbSj*> dbSjMap;


};