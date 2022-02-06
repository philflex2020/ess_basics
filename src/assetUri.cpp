#ifndef ASSET_CPP
#define ASSET_CPP
#include <string>
#include <cstring>
#include "assetUri.h"
//#include "formatters.hpp"
/******************************************************
 *              
 *                 assetUri.h
 *  p.wilshire
 *  01/10/2021
 *   rework 02/05/2022
 *   test with test_assetUri
 *
 *  added setParam("foo[1]", true);
 *      sets a bit in a param
 * 
 *    
 ******************************************************/
struct cstr{ const char* str;};

/***********************************************
 *                 assetUri
 ***********************************************/
assetUri::assetUri(const char* uri, const char* var)
{
    sUri = nullptr;
    origvar = nullptr;
    vecUri = nullptr;
    //nfrags = getNfrags();
    Var = nullptr;
    Param = nullptr;
    index = -1;
    setValue = true;
    setup(uri, var);
}

assetUri::~assetUri()
{
    free((void*)Uri);
    if(sUri)free((void*)sUri);
    free((void*)origuri);
    if(origvar)free(origvar);
    if(vecUri)free(vecUri);
    uriVec.clear();
}

void assetUri::single(void)
{
    if(origuri)
    {
        sUri = strdup(origuri);
        char*sp = sUri;
        sVar = nullptr;
        while(*sp++)
        {
            if (*sp == '/')
            {
                sVar=sp;
            }
        }
        if(sVar)
        {
            *sVar = 0;
            sVar++;
        }
    }
}

void assetUri::show(void )
{
    std::cout << "uri :[" << Uri <<"]"<< std::endl;
    std::cout << "var :[" << Var <<"]"<< std::endl;
    std::cout << "param :[" << Param <<"]"<< std::endl;
}
// comp:a:b:c
// Var == c but there is a vec with c:b:a on it 
//var can become a vec
void assetUri::setup(const char* uri, const char* var)
{
    Uri = strdup(uri);
    origuri = strdup(uri);
    if(var)origvar = strdup(var);
    nfrags = getNfrags();
    index = -1;
    char *sp = strstr(Uri,(char*)":");
    if(sp)
    {
        *sp++ = 0;
        Var = sp;
    }
    else
    {
        Var = origvar;
    }
    if(Var)
    {
        sp = strstr(Var,(char *)"@");
        if(sp)
        {
            *sp++ = 0;
            Param = sp;
        }
    }
    else
    {
        sp = strstr(Uri,(char *)"@");
        if(sp)
        {
            *sp++ = 0;
            Param = sp;
        //recover Var
            while ((*sp != '/') && (sp != Uri)) sp--;
            if(*sp == '/')
            {
                *sp++ = 0;
                Var = sp;
            }
        }

    }
    // now search for an index in var /a/b/c:d[23]  
    if(Var)
    {
        sp = strstr(Var,(char *)"[");
        if(sp)
        {
            *sp++ = 0;
            index = (int)strtol(sp, nullptr, 10);
        }

    }
    if(Param)
    {
        sp = strstr(Param,(char *)"[");
        if(sp)
        {
            *sp++ = 0;
            index = (int)strtol(sp, nullptr, 10);
            printf(" Param sp remaining >>%s<< index %d\n", sp, index);

        }
    }
    if(!Var&&!Param)
    {
        sp = strstr(Uri,(char *)"[");
        if(sp)
        {
            *sp++ = 0;
            index = (int)strtol(sp, nullptr, 10);
            printf(" Param sp remaining >>%s<< index %d\n", sp, index);
            //recover Var
            while ((*sp != '/') && (sp != Uri)) sp--;
            if(*sp == '/')
            {
                *sp++ = 0;
                Var = sp;
            }
        }

    }
    // now search for an index in param /a/b/c:d[23]  
    

}
// do this after discovering var
int assetUri::setupVarVec()
{
    if(varVec.size() > 0)
        return (int)varVec.size();
    char* spUri  = strdup(Var);
    char* sp  = spUri;//strdup(Var);
    char *sp1;
    sp++;
    do
    {
        sp1 = strstr(sp,(char*)":");
        if(sp1)
        {
            *sp1++ = 0;
            varVec.push_back(sp);
            if (1) printf("push back [%s]", sp);
            sp = sp1;
        }
    } while(sp1 && (sp1[0] != '@'));
    if(sp!=spUri)
    {
        varVec.push_back(sp);
        if (1) printf("push back last [%s]", sp);

    }
    return (int)varVec.size();
}

int assetUri::setupUriVec()
{
    if(uriVec.size() > 0)
        return (int)uriVec.size();
    vecUri = strdup(origuri);
    char *sp = vecUri;
    char *sp1;
    sp++;
    do
    {
        sp1 = strstr(sp,(char*)"/");
        if(sp1)
        {
            *sp1++ = 0;
            uriVec.push_back(sp);
            //if (0) FPS_PRINT_INFO("push back [{}]", cstr{sp});
            sp = sp1;
        }
    } while(sp1);
    if(sp!=vecUri)
    {
        uriVec.push_back(sp);
        //if (0) FPS_PRINT_INFO("push back last [{}]", cstr{sp});

    }
    return (int)uriVec.size();
}
// was pull_one_uri
char* assetUri::pullOneUri(int idx)
{
    int ix = setupUriVec();
    if (idx > ix) return nullptr;
    return uriVec[idx];
}

//char* pull_pfrag(const char* uri, int idx);
char* assetUri::pullPfrag(int idx)
{
    int ix = setupUriVec();
    if (idx > ix) return nullptr;
    if (idx == 0) return strdup(Uri);
    std::string res= "";
    int i = 0;
    while(i<idx && i<ix)
    {
        res += "/";
        res +=uriVec[i++];
    }

    char* sp = (char*)strdup(res.c_str());
    return sp;
}

char* assetUri::pullPvar(int idx)
{
    int ix = setupUriVec();
    if (idx > ix) return nullptr;
    if (idx == 0) return nullptr;

    std::string res= "";
    while(idx<ix)
    {
        res += "/";
        res +=uriVec[idx];
        idx++;
    }
    char* spr = (char*)res.c_str();

    char* sp = (char*)strdup(spr+1);
    return sp;
}

//char* pull_first_uri(const char* uri, int n = 1)
char* assetUri::pullFirstUri(int n)
{
    return pullPfrag(n);
}

//char* pull_last_uri(const char* uri, int n = 1)
char* assetUri::pullLastUri(int n)
{
    int ix = setupUriVec();
    if((ix - n) >= 0)
        return (char *)strdup(uriVec[ix-n]);
    return nullptr;
}

//char* pull_uri(const char* uri, int idx)
char* assetUri::pullUri(int idx)
{
    // needs to be freed
    return pullPfrag(idx);
}

int assetUri::getNfrags()
{
    int nfrags = 0;
    char* sp = origuri;
    while (*sp)
    {
        if (*sp++ == '/') nfrags++;
    }
    return nfrags;
}

#endif
