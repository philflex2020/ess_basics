
#include <vector>
#include <string>

#ifndef DBSJ_TYPE
#define DBSJ_TYPE

//#pragma once
// todo 
//    track owners only delete when owners are all gone
//    add set/get val with locks
// baseUtils
int split_delim(std::vector<std::string>&v, const std::string& s, char c);

class dbSj;

static std::map<dbSj*,dbSj*> dbSjMap;

class dbSj {
public:
dbSj(){
    //child = nullptr;
    parent = nullptr;
    name = "";
    //valuebool = false;
    //valueint = 0;
    //valuestring = "";
    dbtype = DB_NONE;
    dbval = new dbVal;
    dbval->valuestring =  "";
    depth = 0;
    state = 0;
    item = 0;
    update = false;
    dbSjMap[this] = this;
    };

dbSj(int depth, dbSj * p){
    //child = nullptr;
    parent = p;
    name = "";
    //valuebool = false;
    //valueint = 0;
    //valuestring = "";
    dbtype = DB_NONE;
    dbval = new dbVal;
    dbval->valuestring =  "";
   
    depth = depth+1;
    update = false;
    dbSjMap[this] = this;
    };


~dbSj(){
    std::cout << " deleteing ["<<name<<"]"<< std::endl;
    //if(dbval)
    //    delete dbval;
    };
    std::string name;
    // todo try one of these
    std::vector<std::vector<dbVal*>> svalpvec;
    std::vector<std::vector<dbVal>> svalvec;
    //
    bool update;
    // todo move this out to dbVal
    enum dtype { DB_BASE, DB_NONE, DB_OBJ,DB_ARRAY,DB_STRING,DB_DOUBLE, DB_INT,DB_BOOL,DB_NULL,DB_END};
    dtype dbtype;

    // thse are the kids
    std::vector<dbSj*> dvec;
    std::map<std::string,dbSj*> dmap;

    //dbSj* child; 
    dbSj* parent;
    dbVal* dbval;

    int depth;  // depth of 2 has a value (in a param)
    
    int state;
    int item;
    //go all the way back 
    // /components/sel_735:value:depth
    // /components/sel_735:value:state
    // /components/sel_735:value:item
    // setsVal (T value)
    // svalvec[state][item].valuedouble = 234.5

    void setsVal(double val)
    {
        svalvec[state][item].valuedouble = val;
    }

    double getsdVal()
    { 
        return svalvec[state][item].valuedouble;
    }

    dbSj*getVar(dbSj* basedb, const char* uri)
    {
        dbSj*db = nullptr;
        dbSj*dbi = basedb;
        std::vector<std::string>v;
        split_delim(v,uri,':');
        for (auto xx : v)
        {
            if(dbi->dmap.find(xx)== dbi->dmap.end())
                return nullptr;
            dbi=dbi->dmap[xx];
            db = dbi;
        }
        return db;
    }

    dbSj*setVal(dbSj* basedb, const char* uri, double value)
    {
        //dbSj*db = nullptr;
        dbSj*dbi = basedb;
        std::vector<std::string>v;
        split_delim(v,uri,':');
        for (auto xx : v)
        {
            if(dbi->dmap.find(xx)== dbi->dmap.end())
            {
                //adddbSj(key);   
                dbi->adddbSj(xx.c_str());
                dbi=dbi->dmap[xx];
                dbi->dbtype=DB_OBJ;
            }
            else
            {
                dbi=dbi->dmap[xx];
            }
        }
        //dbi->setsVal(value);
        dbi->dbval->valuedouble=value;
        dbi->dbtype=DB_DOUBLE;
        return dbi;
    }

    void getroot()
    {
        dbSj* rootdb=this;
        std::vector<const char *>rvec;
        while (rootdb->parent)
        {
            if(rootdb->parent->name.size() > 0)
                rvec.push_back(rootdb->parent->name.c_str());
            rootdb=rootdb->parent;
            /* code */
        }
        std::string sroot;
        int idx = 0;
        for ( auto xx: rvec)
        {
            if(idx++ >0)
                sroot.insert(0,":");
            sroot.insert(0,xx);
        }
        root = sroot;
    }
    std::string root;

    const char* get_dtype()
    {
        switch (dbtype) {
            case DB_BASE:
                return "base";
            case DB_NONE:
                return "none";
            case DB_OBJ:
                return "obj";
            case DB_ARRAY:
                return "array";
            case DB_STRING:
                return "string";
            case DB_DOUBLE:
                return "double";
            case DB_INT:
                return "int";
            case DB_BOOL:
                return "bool";
            case DB_NULL:
                return "null";
            case DB_END:
                return "end";
        }
        return nullptr;
    }

    dbSj* find_key(const char*key)
    {
        auto db =  adddbSj(key);
        return db;
    }

    dbSj* find(const char* name)
    {
        //dbSj*db = nullptr;
        if(dmap.find(name) == dmap.end())
        {
            return nullptr;
        }
        return dmap[name];
    }

    dbSj* adddbSj(const char* name)
    {
        dbSj*db = nullptr;
        if(dmap.find(name) == dmap.end())
        {
            db = new dbSj(depth, this);
            db->depth = depth+1;
            dmap[name] = db;
            db->name = name;
            db->parent = this;
            dvec.push_back(db);
        }
        db = dmap[name];
        return db;        
    }

    dbSj* adddbSjType(const char* name, dtype _type)
    {
        dbSj*db = adddbSj(name);
        db->dbtype = _type;
        return db;        
    }

    dbSj* adddbSj(const char* name, double value)
    {
        dbSj*db=adddbSjType(name, DB_DOUBLE);
        db->dbval->valuedouble = value;
        db->update = true;
        return db;        
    }

    dbSj* adddbSj(const char* name, const char* value)
    {
        dbSj*db=adddbSjType(name, DB_STRING);
        db->dbval->valuestring = value;
        db->update = true;
        return db;        
    }

    dbSj* adddbSj(const char* name, bool value)
    {
        dbSj*db=adddbSjType(name, DB_BOOL);
        db->dbval->valuebool = value;
        db->update = true;
        return db;        
    }


} dbSj_t;

#endif
