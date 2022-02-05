/*
 AssetUri
 uri decoder
 */
 #include <vector>


#pragma once

class assetUri {

public:
    assetUri(const char* uri, const char* var= nullptr);
    ~assetUri();
    void setup();
    int setupUriVec();
    void single();
    char* pullOneUri(int idx);
    char* pullPfrag(int idx);
    char* pullPvar(int idx);
    char* pullFirstUri(int n = 1);
    char* pullLastUri(int n = 1);
    char* pullUri(int idx);
    int getNfrags();


    char* Uri;
    char* origuri;
    char *vecUri;
    char* origvar;
    int nfrags;
    char* Var;
    char* Param;
    char* sUri;   // single decode /a/b/c into /a/b  c
    char* sVar;

    std::vector<char *> uriVec;
    int index;
    bool setValue;

};