#ifndef DNP3_UTILS_H
#define DNP3_UTILS_H
/*
 * mapping shared library
 *      pwilshire      5/5/2020
 */

#include <cjson/cJSON.h>
#include <fims/libfims.h>

#include <map>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>


#ifndef FPS_ERROR_PRINT 
#define FPS_ERROR_PRINT printf
#define FPS_DEBUG_PRINT printf
#endif
#define MAX_SETUP_TICKS  50
#define DNP3_MASTER 0
#define DNP3_OUTSTATION 1

#define DNP3_UTILS_VERSION "0.9.4"

//const ControlCode StringToControlCode(const char* codeWord);
void addCjTimestamp(cJSON* cj, const char* ts);

struct char_cmp 
{
    bool operator () (const char* a,const char* b) const
    {
        return strcmp(a,b)<0;
    }
};


typedef struct maps_t maps;
typedef std::map<const char*, std::pair<bool*, maps**>, char_cmp> body_map;
typedef std::map<const char*, body_map*, char_cmp> uri_map;
//typedef std::vector<std::pair<const char*,int>> bits_map;

int variation_decode(const char* ivar);
const char *iotypToStr (int t);
int iotypToId (const char* t);
int countUris(const char* uri);

enum Type_of_Var{
    AnIn16,
    AnIn32,
    AnF32,
    Type_Crob,     // write master
    Type_Analog,  // write outstation
    Type_Binary,     //write outstation
    Type_AnalogOS,  // auto write outstation
    Type_BinaryOS,  //write outstation
    NumTypes
};

// used to manage char* compares in maps
struct char_dcmp {
    bool operator () (const char *a,const char *b) const
    {
        return strcmp(a,b)<0;
    }
};
// only allow 1 type for now
// TODO use EventAnalogVariation::Group32Var0; defs

enum {
    GroupUndef,
    Group1Var0,
    Group1Var1,
    Group30Var1,
    Group30Var2,
    Group30Var3,
    Group30Var4,
    Group30Var5,
    Group32Var0,
    Group32Var1,
    Group32Var2,
    Group32Var3,
    Group32Var4,
    Group32Var5,
    Group32Var6,
    Group32Var7,
    Group32Var8,
    NumVars
};

// flag options for reporting
#define PRINT_VALUE      1
#define PRINT_PARENT     2
#define PRINT_URI        4

// local copy of all inputs and outputs
// Also used with bit fields
// if so the "parent" slot is filled and the index refers to the bit number 
//see https://groups.google.com/forum/#!topic/automatak-dnp3/RvrrCaGM8-8
typedef struct DbVar_t {
    DbVar_t(const char *iname, int _type, int _offset, const char* iuri, const char*ivariation):site(NULL),type(_type), offset(_offset),site_offset(-1) {
        valuedouble = 0.0;
        crob = 0;
        bit = -1;
        parent = NULL;
        initSet = 0;
        //readb = NULL;
        linkb = NULL;  // used to link outstation responses to master vars
        linkback = NULL;
        linkuri = NULL;        
        clazz = 0;
        sign = false;
        scale = 0.0;
        pref = 0;
        deadband = 0.0;
        valuestring = nullptr;
        name = iname;

         
        if(iuri)
        {
            uri = strdup(iuri);
            //free((void *)iuri);
        }
        else
        {
            uri = NULL;
        }
        variation = variation_decode(ivariation);
    };

    ~DbVar_t()
    {
        //FPS_ERROR_PRINT(" DbVar delete name [%s] uri [%s]\n", name.c_str(), uri);
        if(uri)free((void *)uri);
        if(linkback) free((void *)linkback);
        if(linkuri) free((void *)linkuri);
    }

    void setEvar(const char* evar)
    {
        evariation = variation_decode(evar);
    }

    void setFloat(bool isFloat)
    {
        if(isFloat)
        {
            if (type == Type_Analog) 
            {
                variation = variation_decode("Group30Var5");
            }
            if (type == Type_Binary)
            {
                variation = variation_decode("Group1Var1");
            }
        }
    }

    void setClazz(int iclazz)
    {
        clazz = iclazz;
    }
    
    int addBit(const char*bit)
    {
        dbBits.push_back(std::make_pair(bit,0));
        return static_cast<int32_t>(dbBits.size());
    }

    int setBit(int idx, int val)
    {
        dbBits[idx].second = val;
        return val;
    }

    int setBit(const char* var, int val)
    {
        for (int i = 0; i < static_cast<int32_t>(dbBits.size()); i++)
        {
            if(strcmp(dbBits[i].first, var) == 0)
            {
                setBit(i,val);
                return 0;
            }
        }
        return -1;
    }

    int getBit(int idx)
    {
        int val = dbBits[idx].second;
        return val;
    }

    int getBit(const char* var)
    {
        for (int i = 0; i < static_cast<int32_t>(dbBits.size()); i++)
        {
            if(strcmp(dbBits[i].first, var) == 0)
            {
                return getBit(i);
            }
        }
        return -1;
    }

    // TODO turn these into char* .... no point the extra strcmp uses more resources
    std::string name;
    const char* site;    // furute use for site.
    const char* uri;
    int type;
    int variation;         // space to flag different DNP3 variation like Group30var5
    int evariation;         // space to flag different DNP3 variation like Group30var5
    int offset;
    int bit;              // used to indiate which bit in parent
    DbVar_t* parent;      // I'm a bit and this is daddy 
    int site_offset;     // future use for site offset
    // values , most types use valuedouble
    double valuedouble;

    uint8_t crob;
    int idx;      // type index
    int clazz;    // class

    // used for bit fields
    std::vector<std::pair<const char*,int>>/*bits_map*/dbBits;

    uint8_t initSet;
    DbVar_t* linkb;
    const char* linkback;
    const char* linkuri;
    bool sign;
    double scale;
    uint16_t pref;
    int vindex; // used when the compressed option is requested
    double deadband;
    char *valuestring;

} DbVar;

// map names to vars ... deprecated becaus name + uri is used to identify a unique item
typedef std::map<std::string, DbVar_t*> dbvar_map;
// map of idx to vars
typedef std::map<int, DbVar_t*>dbix_map;
// map a vector list of dbvars against a vector of uris no longer used

//typedef std::map<const char*,std::vector<DbVar_t*>, char_dcmp>duri_map;
typedef std::vector<DbVar_t*> dbvec;

// new thinking 
// we'll have a map of uris each with a pointer to a map of variables
// we'll have one of these for pubs , gets , and sets
// how do we group them together
// each varuable can have a uri or a default one establised for the component
// "compname" :{
//    "uri":" <some uri>",
//    "varibles":[
      // "varname":{.....},
      // "varname":{.....}
//    ]
// }
// 
// not going to do it this way
// the uri is fixed for a var
// we can add _get or _set to the uri for special uses.
// this structure is still good. the uri points to a map
// just like john did
// but it can be a real map we dont need to share them
// so  when we add a var we include the uri
//typedef std::map<std::string, dbvar_map> dburi_map;


// used in parseBody the int is a print flag to include "value"
typedef std::vector<std::pair<DbVar*,int>>dbs_type; // collect all the parsed vars here

// this is for the bits
typedef std::map<const char*, std::pair<DbVar_t*,int>, char_dcmp>bits_map;

// do conversion for large values unsigned ints
double getF32Val(DbVar *db);
int32_t getInt32Val(DbVar *db);
int16_t getInt16Val(DbVar *db);

bool extractInt32Val(double &dval, DbVar *db);
bool extractInt16Val(double &dval, DbVar *db);


int addVarToCj(cJSON* cj, DbVar*db);
int addVarToCj(cJSON* cj, std::pair<DbVar*,int>dbp);
int addVarToCj(cJSON* cj, const char *dname);
int addVarToCj(cJSON* cj, DbVar* db, int flag);
// the varlist is a map of vars associated with each uri
typedef struct varList_t {
    varList_t(const char* iuri)
    {
        uri = strdup(iuri);
    };
    ~varList_t(){
        // for each dbmap item delete second
        dbvar_map::iterator it;
        for (it= dbmap.begin() ;it != dbmap.end();++it)
        {
            // dbvar_map
            auto dvar = it->second;
            delete dvar;
        }
        if(uri) delete(uri);
    };

    void addDb(DbVar* db)
    {
        dbmap[db->name] = db;
    }
    int size()
    {
        return (int)dbmap.size();
    }

    const char* uri;
    dbvar_map dbmap;
} varList;

// this now replaces duri_map
typedef std::map<std::string, varList*> dburi_map;

enum {
    URI_FLAG_REPLY  = 1<<1,
    URI_FLAG_GET    = 1<<2,
    URI_FLAG_SET    = 1<<3,
    URI_FLAG_URIOK  = 1<<4,
    URI_FLAG_NAMEOK = 1<<5,
    URI_FLAG_SINGLE = 1<<6,
    URI_FLAG_SYSTEM = 1<<7,
    URI_FLAG_DUMP = 1<<8

};

typedef struct sysCfg_t {

    sysCfg_t() :name(NULL), protocol(NULL), id(NULL), ip_address(NULL), p_fims(NULL)
    {
        cj = NULL;
        cjPub = NULL;
        cjloaded = 0;
        debug = 0;
        scanreq = 0;
        num_subs = 0;
        unsol = 0;
        cjclass = NULL;
        base_uri = NULL;
        local_uri = NULL;
        defUri = NULL;
        freq1 = 0;
        freq2 = 0;
        freq3 = 0;
        frequency = 1000;
        rangeAStart = 0;
        rangeAStop = 0;
        rangeBStart = 0;
        rangeBStop = 0;
        rangeFreq = 0;

        unsolUpdate = false;
        useOffset = false;
        pref = 1;
        useVindex = false;
        deadband =  0.1;
        valuestring = nullptr;

        for (int i = 0; i <= static_cast<int32_t>(Type_of_Var::NumTypes) ; i++)
        {
            maxIdx[i] = 0;
        }
        conn_type = nullptr;
        // TLS stuff
        peerCertificate = nullptr;
        privateKey = nullptr;
        certificateChain = nullptr;
        caCertificate = nullptr;
        tls_folder_path = nullptr;
        // RTU stuff:
        baud = 0;
        dataBits = 0;
        stopBits = 0.0;
        parity = nullptr;
        flowType = nullptr;
        asyncOpenDelay = 0;
        deviceName = nullptr; // this might just be name really?
        valuestring = nullptr;
    }
    ~sysCfg_t()
    {
        FPS_DEBUG_PRINT(" %s closing \n", __FUNCTION__);

        //if(name)free(name);
        if(protocol)free(protocol);
        if(id)free(id);
        if(ip_address)free(ip_address);
        //if (pub) free(pub);
        if (name) free(name);

        cleanUpDbMaps();

        if (cj) cJSON_Delete(cj);
        if(base_uri) free(base_uri);
        if(local_uri) free(local_uri);
        if(defUri) free(defUri);

        //cleanUpDbMaps();
        if (conn_type) free(conn_type);
        // TLS stuff:
        if (peerCertificate) free(peerCertificate);
        if (privateKey) free(privateKey);
        if (certificateChain) free(certificateChain);
        if (caCertificate) free(caCertificate);
        if (tls_folder_path) free(tls_folder_path);

        // RTU stuff:
        if (parity) free(parity);
        if (flowType) free(flowType);
        if (deviceName) free(deviceName); // this might just be name really?
    };

    public:
        // now always creates a var. it is added to the uri map
        DbVar* newDbVar(const char *iname, int type, int offset, char* uri, char* variation) 
        {
            DbVar* db = NULL;
            db = new DbVar(iname, type, offset, uri, variation);
            if(uri == NULL)
            {
                db->uri = strdup(defUri);
            }
            //dbMap[name] = db;
            //
            //db->idx = static_cast<int32_t>(dbVec[type].size());
            // TODO auto assign see below
            // TODO deprecated need to set up the maps 
            // this is OK to keep a master list as a vector 
            // use to get dbs dbix_map dbMapIxs[Type_of_Var::NumTypes+1];
            dbVec[type].push_back(db);
            db->vindex = dbVec[type].size();
            if (offset > maxIdx[type])
                maxIdx[type] = offset;
     
            return db;
        };

        int32_t getTypeSize(int type) 
        {
            // we have an extra array do handle auto indexing
            if (type > NumTypes)
                return 0;
            //return static_cast<int32_t>(dbVec[type].size());
            // fix the BRP power bug getting the last value
            if(useVindex)return dbMapIxs[type].size();
            return static_cast<int32_t>(maxIdx[type]+1);
        }

        // allow this to auto configure for any if db->idx == -1
        void setDbIdxMap(DbVar* db, int inParse)
        {
            int opType = db->type;
            // do these ones after parsing
            if (((db->type == Type_Crob) 
                || (db->type == AnF32) 
                || (db->type == AnIn16) 
                || (db->type == AnIn32)) 
                && (db->idx == -1)
                && inParse)
            {
                // set the db->ix later
                
                FPS_DEBUG_PRINT(" **** %s setting name [%s] typ [%d] idx [%d] Later \n", __FUNCTION__, db->name.c_str(), db->type, db->idx );
                return;
            }
            // no need to repeat this
            //if ((inParse == 0) && (db->idx != -1))
            //    return;
            // slight wrinkle 
            // if useVindex is on we get the index back as the compressed index... ( sigh)
            // so we need to save the vIndex in the map to find the thing.
            //  need sys->who and db->vindex
            if(db->idx > -1)
            {
                //dbMapIxs[opType][db->idx] = db;
                // repeat here just to make sure
                // if(opType != db->type)
                if(debug)
                {
                    FPS_ERROR_PRINT(" **** %s setting name [%s] typ [%d] idx [%d] NOW \n", __FUNCTION__, db->name.c_str(), db->type, db->idx );
                }

                if((who == DNP3_MASTER) && useVindex && (db->type == Type_Analog || db->type== Type_Binary))
                    dbMapIxs[db->type][db->vindex] = db;
                else
                    dbMapIxs[db->type][db->idx] = db;

                return;
            }
            else
            {
                db->idx = 0;
            }
            opType = db->type;

            if ((db->type == AnF32) 
                || (db->type == AnIn16) 
                || (db->type == AnIn32)) 
            {
                opType = NumTypes;
            }
            // TODO review this
            // all outputs are merged into one type to ensure unique indexes
            while (dbMapIxs[opType].find(db->idx) != dbMapIxs[opType].end())
            {   
                db->idx++;
            }
            dbMapIxs[opType][db->idx] = db;
            // repat here just to make sure
            //if(opType != db->type)
            FPS_ERROR_PRINT(" **** %s setting name [%s] typ [%d] idx [%d] AGAIN NOW \n", __FUNCTION__, db->name.c_str(), db->type, db->idx );
            dbMapIxs[db->type][db->idx] = db;
        } 
        // given a uri and a db name recover the dbvar if we have one
        DbVar* getDbVar(const char *uri, const char* name)
        {
            std::string sname = name;
            std::string suri = uri;
            dburi_map::iterator it = dburiMap.find(suri);
            if(it != dburiMap.end())
            {
                 // dbvar_map
                auto dvar = it->second;
                auto dbm = dvar->dbmap;
                dbvar_map::iterator itd = dbm.find(sname);
            
                if (itd != dbm.end() )
                {
                    return itd->second;
                }
            }
            return NULL;
            
        };

        int setDb(DbVar *db, char* valuestring)
        {
            if(db->valuestring)
            {
                free (db->valuestring);
                if(valuestring)
                    db->valuestring= strdup(valuestring);
                else
                    db->valuestring = nullptr;
            }
            return 0;
        }

        // set a value  handles linkback as well
        int setDb(DbVar* db, double fval)
        {
            if(db != NULL)
            {
                if (db->linkback != NULL)
                {
                    if (db->linkb == NULL)
                    {
                        const char* luri = db->linkuri;
                        if (luri == NULL)
                            luri = db->uri;
                        db->linkb = getDbVar(luri, db->linkback);
                    }
                    if (db->linkb != NULL)
                    {
                        if (db->linkb->linkb == db)
                        {
                            FPS_ERROR_PRINT(" link loopback detected from [%s] to [%s] \n", db->name.c_str(), db->linkb->name.c_str());
                        }
                        else
                        {
                            setDb(db->linkb, fval);
                        }
                    }
                }
                db->initSet = 1;
                switch (db->type) 
                {
                    case Type_Analog:
                    case Type_AnalogOS:
                    case AnF32:
                    case AnIn32:
                    case AnIn16:
                    {
                        // also copy valueint or the group30var5 stuff
                        db->valuedouble = fval;
                        return 1;
                    }    
                    case Type_Binary:
                    case Type_BinaryOS:
                    {
                        db->valuedouble = fval;
                        //db->valueint = static_cast<int32_t>(fval);
                        return  1;
                    } 
                    case Type_Crob:
                    {
                    //    db->crob = ControlCodeToType(StringToControlCode(cj->valuestring));
                       return  1;
                    }
                    default:
                        return 0;
                }    
            }
            return 0;
        };

        int setDb(DbVar* db, int ival)
        {
            if(db != NULL)
            {
                if (db->linkback != NULL)
                {
                    if (db->linkb == NULL)
                    {
                        const char* luri = db->linkuri;
                        if (luri == NULL)
                            luri = db->uri;
                        db->linkb = getDbVar(luri, db->linkback);
                    }
                    if(db->linkb != NULL)
                    { 
                        if (db->linkb->linkb == db)
                        {
                            FPS_ERROR_PRINT(" link loopback detected from [%s] to [%s] \n", db->name.c_str(), db->linkb->name.c_str());
                        }
                        else
                        {
                            setDb(db->linkb, ival);
                        }
                    }
                }
                db->initSet = 1;

                switch (db->type) 
                {
                    case Type_Analog:
                    case Type_AnalogOS:
                    case AnF32:
                    case AnIn16:
                    case AnIn32:
                   {
                        // also copy valueint or the group30var5 stuff
                        db->valuedouble = (double)ival;
                        //db->valueint =  ival;
                        return 1;
                    }    
                    case Type_Binary:
                    case Type_BinaryOS:
                    {
                        db->valuedouble = (double)ival;
                        //db->valueint = ival;
                        return  1;
                    } 
                    case Type_Crob:
                    {
                        db->crob = ival;// ControlCodeToType(StringToControlCode(cj->valuestring));
                        return  1;
                    }
                    default:
                        return 0;
                }
            }
            return 0;
        };

        int setDbVar(DbVar* db, cJSON* cj)
        {
            if(db != NULL)
            {
                switch (db->type) 
                {
                    case Type_AnalogOS:
                    case Type_Analog:
                    case AnF32:
                    case AnIn32:
                    case AnIn16:
                    {
                        return setDb(db, cj->valuedouble);
                    }    
                    case Type_BinaryOS:
                    case Type_Binary:
                    {
                        if(debug > 0)
                            FPS_ERROR_PRINT(" setting the value of [%s] %s to  int %d sign %d \n"
                                , db->name.c_str(), iotypToStr(db->type), cj->valueint, db->sign );                

                        return setDb(db, cj->valueint);
                    } 
                    case Type_Crob:
                    {
                        // CROB must be a string
                        if (cj->valuestring)
                            return setDb(db, cj->valuestring);
                        // false is control code 4
                        else if (cj->valueint == 0)
                            return setDb(db, 4);
                        // true is control code 3
                        else if (cj->valueint == 1)
                            return setDb(db, 3);
                        else 
                        {
                            if ( debug > 0)
                            {
                               FPS_ERROR_PRINT(" %s Crob string or T/F missing for %s/%s\n", __FUNCTION__, db->uri, db->name.c_str());
                            }
                        }
                    }
                    default:
                        return 0;
                }
            }
            return 0;
        };

        int setDbVar(const char* uri, const char* name, cJSON* cj)
        {
            return setDbVar(getDbVar(uri, name),cj);
        };

        int setDbVar(const char* uri, const char *name, double dval)
        {
            DbVar* db = getDbVar(uri, name);
            if(db != NULL)
            {
                return setDb(db, dval);
            }
            return 0;
        };

        int setDbVar(const char* uri, const char *name, int ival)
        {
            DbVar* db = getDbVar(uri, name);
            if(db != NULL)
            {
                return setDb(db, ival);
            }
            return 0;
        };

        int setDbVarIx(int dbtype, int idx, double ival)
        {
            DbVar* db = getDbVarId(dbtype, idx);
            if (db != NULL)
            {
                if((db->scale > 0.0) || (db->scale < 0.0))
                {
                    ival /= db->scale;
                }
                if(debug)
                    FPS_ERROR_PRINT(" setting the double value of [%s] %s to %f sign %d \n"
                            , db->name.c_str(), iotypToStr(dbtype), ival, db->sign );                

                return setDb(db, ival);
            }
            else
            {
                FPS_ERROR_PRINT("  %s Error Set Double Variable %s idx %d value %f unknown Var\n"
                    , __FUNCTION__, iotypToStr(dbtype), idx, ival);                  
            }
            return 0;
        };

        int setDbVarIx(int dbtype, int idx, int ival)
        {
            DbVar* db = getDbVarId(dbtype, idx);
            if (db != NULL)
            {
                if((db->scale > 0.0) || (db->scale < 0.0))
                {
                    ival /= db->scale;
                }
                if(debug)
                    FPS_ERROR_PRINT(" setting the int value of [%s] %s to %d sign %d \n", db->name.c_str(), iotypToStr(dbtype), ival, db->sign );                

                return setDb(db, ival);
            }
            else
            {
                FPS_ERROR_PRINT(" %s Set INT Variable %s index  %d unknown \n", __FUNCTION__, iotypToStr(dbtype), ival);                  
            }            
            return 0;
        };

        int setDbVarIx(int dbtype, int idx, bool ival)
        {
            DbVar* db = getDbVarId(dbtype, idx);
            if (db != NULL)
            {
                FPS_ERROR_PRINT(" setting the int value of [%s] %s to %s sign %d \n"
                    , db->name.c_str(), iotypToStr(dbtype), ival?"true":"false", db->sign );
                int mval = ival?1:0;
                if(db->scale < 0.0)
                {
                    mval = ival?0:1;
                }                   
                return setDb(db, mval);
            }
            else
            {
                FPS_ERROR_PRINT(" %s Set INT Variable %s index  %d unknown \n", __FUNCTION__, iotypToStr(dbtype), ival);                  
            }            
            return 0;
        };

        //  fixed
        DbVar* getDbVarId(int dbtype, int idx)
        {
            if (dbtype > Type_of_Var::NumTypes) return NULL;
            if (dbMapIxs[dbtype].find(idx) != dbMapIxs[dbtype].end())
            {
                //use dbix_map
                return  dbMapIxs[dbtype][idx];
            }
            return NULL;
        };

        void cleanUpDbMaps()
        {
            for (int i = 0; i < static_cast<int32_t>(Type_of_Var::NumTypes); i++)
            {
                //dbVec[type].push_back(db);
                FPS_ERROR_PRINT(" cleanup dbvec [%d]\n", i);
                dbVec[i].clear();
            }
            int ix = 0;            
            dburi_map::iterator it;
            for (it = dburiMap.begin(); it != dburiMap.end(); ++it)
            {
                FPS_ERROR_PRINT(" cleanup dburiMap [%s] [%d]\n", it->first.c_str(), ix);

                //free((void *)it->first);
                delete it->second;
                ix++;

            }
            dburiMap.clear();            
        }

// deprecated
        void showDbMap()
        {
            FPS_ERROR_PRINT(" %s DbVars===> \n\n", __FUNCTION__);
            for (int i = 0; i < static_cast<int32_t>(Type_of_Var::NumTypes); i++)
            {
                if (dbVec[i].size() > 0)
                {
                    FPS_ERROR_PRINT("\n dnp3 type [%s]\n", iotypToStr(i)); 
                    for (int j = 0; j < static_cast<int32_t>(dbVec[i].size()); j++)
                    {
                        // TODO use dbMapIxs[Type_of_Var::NumTypes+1];
                        // this one is OK
                        // DbVar* db = getDbVarId(i, j);
                        DbVar* db = dbVec[i][j];
                        if((db != NULL) &&(db->type == i))
                        {
                            FPS_ERROR_PRINT(" idx [%s.%d] ->name :[%s] offset : [%d]  vIdx [%d]===> "
                                , iotypToStr(db->type)
                                , db->idx
                                , db->name.c_str()
                                , db->offset
                                , j
                                );

                        }
                        // try again from tne idx map
                        if(db)
                            db = getDbVarId(i, db->idx);
                        if(db != NULL)// &&(db->type == i))
                        {
                            if(debug > 0)
                            {
                                FPS_ERROR_PRINT(" OK ***** idx [%s.%d] j = [%d] Item Already in dbMapIxs\n"
                                    , iotypToStr(i)
                                    , db->idx
                                    , j
                                    );
                            }
                            else
                            {
                                FPS_ERROR_PRINT("\n");
                            }
                        }
                        else 
                        {
                            FPS_ERROR_PRINT(" Error ***** idx [%s]  type = %d j = [%d] Item Not in dbMapIxs\n"
                                , iotypToStr(i)
                                , i 
                                , j
                                );
                        }
                    }
                }
            }           
        }

        void addPubVar(DbVar* db, double val)
        {
            if (cjPub == NULL)
            {
                cjPub = cJSON_CreateObject();
                //FPS_ERROR_PRINT(" %s  created cjPub (double) \n", __FUNCTION__);
            }
            cJSON* cjuri = cJSON_GetObjectItem(cjPub, db->uri);
            if(cjuri == NULL)
            {
                //FPS_ERROR_PRINT(" %s  created cjuri [%s] \n", __FUNCTION__, db->uri);
                cjuri = cJSON_CreateObject();
                //cJSON_AddNumberToObject(cjv,"value", cmd.value);
                cJSON_AddItemToObject(cjPub, db->uri, cjuri);
                cjuri = cJSON_GetObjectItem(cjPub, db->uri);
            }
            if(cjuri != NULL)
            {
                if (db->pref == pref)
                {
                    cJSON_DeleteItemFromObjectCaseSensitive(cjuri, db->name.c_str());
                }
                cJSON_AddNumberToObject(cjuri, db->name.c_str(), val);
                db->pref = pref;
            }
        }

        void addPubVar(DbVar* db, bool val)
        {
            if (cjPub == NULL)
            {
                cjPub = cJSON_CreateObject();
                //FPS_ERROR_PRINT(" %s  created cjPub (bool) \n", __FUNCTION__);
            }
            cJSON* cjuri = cJSON_GetObjectItem(cjPub, db->uri);
            if(cjuri == NULL)
            {
                //FPS_ERROR_PRINT(" %s  created cjuri [%s] \n", __FUNCTION__, db->uri);
                cjuri = cJSON_CreateObject();
                cJSON_AddItemToObject(cjPub, db->uri, cjuri);
                cjuri = cJSON_GetObjectItem(cjPub, db->uri);
            }
            if(cjuri != NULL)
            {
                if (db->pref == pref)
                {
                    cJSON_DeleteItemFromObjectCaseSensitive(cjuri, db->name.c_str());
                }
                cJSON_AddBoolToObject(cjuri, db->name.c_str(), val);
                db->pref = pref;
            }
        }
//runs in fpsMasterApplication.h
        void pubUris(void)
        {
            if (cjPub == NULL)
            {
                return;
            }
            cJSON* obj;
            cJSON_ArrayForEach(obj, cjPub)
            {
                //obj->string is the URI 
                //obj->child are the data points
                addCjTimestamp(obj, "Timestamp");
                //char* out = cJSON_PrintUnformatted(obj->child);
                char* out = cJSON_PrintUnformatted(obj);
                if (out) 
                {
                    char tmp[1024];
                    snprintf(tmp, 1024,"%s", obj->string);

                    if(p_fims)
                    {
                        p_fims->Send("pub", tmp, NULL, out);
                    }
                    free(out);
                }
            }
            cJSON_Delete(cjPub);
            cjPub =  NULL;
        }

        void assignIdx()
        {
            if(debug > 0)
                FPS_ERROR_PRINT(" %s Assign DbVar idx ===> SKIPPED \n\n", __FUNCTION__);
            return;
            for (int i = 0; i < static_cast<int32_t>(Type_of_Var::NumTypes); i++)
            {
                // for outputs they are all on the Ain16 Vec
                //if (i == AnIn32) continue;
                //if (i == AnF32) continue;
                //if (i == Type_Crob) continue;

                if (dbVec[i].size() > 0)
                {
                    if(debug > 0)
                        FPS_ERROR_PRINT(" Assign dnp3 type [%s]\n", iotypToStr(i)); 
                    for (int j = 0; j < static_cast<int32_t>(dbVec[i].size()); j++)
                    {
                        //use dbix_map dbMapIxs[Type_of_Var::NumTypes+1];
                        //DbVar* db = getDbVarId(i, j);
                        DbVar* db = dbVec[i][j];
                        if(db != NULL)
                        {
                            setDbIdxMap(db, 0);
                            if (dbMapIxs[db->type].find(db->idx) == dbMapIxs[db->type].end()) 
                            {
                                FPS_ERROR_PRINT("***********   name %s type %d idx %d NOT in dbMapIxs  \n",  db->name.c_str(), db->type, db->idx);                
                            }
                            else
                            {
                                if(debug > 0)
                                    FPS_ERROR_PRINT("***********  name %s type %d idx %d Already in dbMapIxs  \n", db->name.c_str(), db->type, db->idx);                            
                            }
                        }
                        if(db != NULL)
                        {
                            if(debug > 0)
                                FPS_ERROR_PRINT(" idx set [%s.%d] ->name :[%s] offset : [%d] ===> \n"
                                    , iotypToStr(db->type)
                                    , db->idx
                                    , db->name.c_str()
                                    , db->offset
                                    );
                        }
                        else
                        {
                            FPS_ERROR_PRINT(" ****** Error no db item idx [%s.%d]  \n"
                                , iotypToStr(db->type)
                                , db->idx
                                );
                        }
                    }
                }
            }
            if (debug > 0)
                FPS_ERROR_PRINT(" %s DbVars<=== \n\n", __FUNCTION__);
        }

        // add all the vars referenced by this uri
        void addVarsToVec(std::vector<DbVar*>& dbs, const char* uri)
        {
           if(uri != NULL)
            {
                dburi_map::iterator it = dburiMap.find(uri);
                if (it != dburiMap.end())
                {
                    // it.first is the uri
                    if(debug)
                        FPS_DEBUG_PRINT(" %s uris checking [%s] uri [%s] \n ", __FUNCTION__, it->first.c_str(), uri);
                    // dbvar_map
                    auto dvar = it->second;
                    auto dbm = dvar->dbmap;
                    dbvar_map::iterator itd;
                    //int flag = 0;
                    for (itd = dbm.begin() ; itd != dbm.end(); ++itd)
                    {
                        DbVar* db = itd->second;
                        dbs.push_back(db);
                        if(debug)
                            FPS_ERROR_PRINT("added to Vector name: %s => Type: %d offset : %d\n", itd->first.c_str(), db->type, db->offset);
                    }
                }
            }
        }
        void addUrisToVec(dbs_type& dbs)
        {
            dburi_map::iterator itd;
            for (itd = dburiMap.begin();itd != dburiMap.end();++itd)
            {
                addVarsToVec(dbs, (const char*) itd->first.c_str(), PRINT_URI);
            }
        }
        
        void addVarsToVec(dbs_type& dbs, const char* uri, int flag=0)
        {
            if(uri != NULL)
            {
                dburi_map::iterator it = dburiMap.find(uri);
                //int flag = 0;

                if (it != dburiMap.end())
                {
                    // it.first is the uri
                    if(debug)
                        FPS_DEBUG_PRINT(" %s uris checking [%s] uri [%s] \n ", __FUNCTION__, it->first.c_str(), uri);
                    // dbvar_map
                    auto dvar = it->second;
                    auto dbm = dvar->dbmap;
                    dbvar_map::iterator itd;
                    //int flag = 0;
                    for (itd = dbm.begin() ; itd != dbm.end(); ++itd)
                    {
                        DbVar* db = itd->second;
                        dbs.push_back(std::make_pair(db, flag));
                    }
                }
            }
        }

        //typedef std::map<std::string, varList*> dburi_map;
        //typedef std::map<std::string, DbVar_t*> dbvar_map;
        void showNewUris()
        {
            FPS_ERROR_PRINT(" %s uris===> \n\n", __FUNCTION__);

            dburi_map::iterator it;
            dbvar_map::iterator itd;
            for (it = dburiMap.begin(); it != dburiMap.end(); ++it)
            {
                FPS_ERROR_PRINT(" .. uri [%s] num vars %d\n", it->first.c_str(), static_cast<int32_t>(it->second->dbmap.size()));

                // dbvar_map
                auto dvar = it->second;
                auto dbm = dvar->dbmap;
                for (itd = dbm.begin(); itd != dbm.end(); ++itd)
                {
                    FPS_ERROR_PRINT(" ..                            [%s] \n", itd->first.c_str());
                }
            }
            FPS_ERROR_PRINT(" %s<=== New uris \n\n", __FUNCTION__);
        }

        // returns the start of the untangled uri
        // sets flags to describe how to handle the uri 
        char* confirmUri(DbVar* &db, const char*uri, int who, char* &name, int& flags)
        {
            int old_debug = debug;
            //debug = 1;
            // first limit the uri 
            DbVar* dbf = NULL;
            if(debug>2)
                FPS_DEBUG_PRINT(" %s confirmUri===> \n", __FUNCTION__);
            char* turi = (char *)uri;
            char* nuri;
            char* tmp = NULL;
            if(strstr(uri, "/_system") != NULL) 
            {
                flags |= URI_FLAG_SYSTEM;
                flags |= URI_FLAG_URIOK;
                //if(debug>2)
                    FPS_ERROR_PRINT(" %s confirmUri SYSTEM [_system] found in  [%s], flags set 0x%0x\n",__FUNCTION__, uri, flags);
               debug=old_debug;
               return (char*)uri;
            }
            // seek reply format
            int rc = asprintf(&tmp, "%s/reply",base_uri);
            if(debug>2)
                FPS_ERROR_PRINT(" %s confirmUri seeking REPLY [%s] in [%s] rc [%d] \n",__FUNCTION__, tmp, uri, rc);

            if (strncmp(uri, tmp, strlen(tmp) )== 0)
            {

                flags |= URI_FLAG_REPLY;
                flags |= URI_FLAG_SET;
                turi = (char*)uri + strlen(tmp);
                if(debug>1)
                    FPS_ERROR_PRINT(" %s confirmUri REPLY found uri now [%s]\n",__FUNCTION__, turi);
            }
            else
            {
                int rc = 0;
                turi = (char *)uri;
                // seek extended format
                if(local_uri != NULL)
                {
                    if(tmp) free((void *)tmp);
                    rc = asprintf(&tmp, "%s/%s", local_uri, id);
                    if (strncmp(uri, tmp, strlen(tmp) )== 0)
                    {
                        flags |= URI_FLAG_DUMP;
                        flags |= URI_FLAG_URIOK;

                        turi = (char *)uri + strlen(tmp);
                        if(debug>2)
                            FPS_ERROR_PRINT(" %s confirmUri LOCAL [%s] DUMP [%s]\n",__FUNCTION__, tmp, turi);
                        if(tmp) free(tmp);
                        return turi;
                    }

                    if(tmp) free(tmp);
                    rc = asprintf(&tmp, "%s", local_uri);
                    if (strncmp(uri, tmp, strlen(tmp) )== 0)
                    {
                        flags |= URI_FLAG_GET;
                        flags |= URI_FLAG_SET;
                        turi = (char *)uri + strlen(local_uri);
                        if(debug>2)
                            FPS_ERROR_PRINT(" %s confirmUri LOCAL [%s] found uri now [%s] rc [%d] \n",__FUNCTION__, tmp, turi, rc);
                    }

                }
            }

            if (tmp)
            {
                free((void *)tmp);
            }
            // now look for  matching uri
            // look for "/interfaces/<id>/<someuri>"
            dburi_map::iterator it = dburiMap.find(turi);
            //bool match = false;
            nuri = NULL;
            for  (it = dburiMap.begin() ; it != dburiMap.end(); it++)
            {
                if(debug>2)
                    FPS_ERROR_PRINT(" %s uris checking [%s] uri [%s] turi [%s]  value %d\n"
                            , __FUNCTION__, it->first.c_str(), uri, turi
                            , (int) strncmp(turi, it->first.c_str(), strlen(it->first.c_str()))
                                );

                if (strncmp(turi, it->first.c_str(), strlen(it->first.c_str())) == 0)
                {
                    nuri = turi + strlen(it->first.c_str());
                    if (nuri[0] == '/') nuri++;
                    flags |= URI_FLAG_URIOK;
                    if(debug>2)
                        FPS_ERROR_PRINT("    URI FOUND [%s] checking name nuri  [%s] (len) %ld \n", it->first.c_str(), nuri, (long int)strlen(nuri));
                    if(strlen(nuri)> 0)
                    {
                        auto dvar = it->second;
                        auto dbm = dvar->dbmap;
                        dbvar_map::iterator itd = dbm.find(nuri);

                        if (itd != dbm.end())
                        {
                            dbf = itd->second;
                            if(debug>2)
                                FPS_ERROR_PRINT(" URI Match                [%s] %d %d\n"
                                                    , dbf->name.c_str() 
                                                    , dbf->type
                                                    , dbf->idx
                                                    );
                            name = nuri;
                            flags |= URI_FLAG_NAMEOK;
                            flags |= URI_FLAG_SINGLE;
                            db = dbf;
                            debug = old_debug;
                            return turi;
                        }
                    }
                    else
                    {
                        db = NULL;
                        debug = old_debug;

                        return turi;
                    }                   
                }
            }
            if(debug>2)
                FPS_ERROR_PRINT(" %s<=== uris \n\n", __FUNCTION__);
            debug = old_debug;
            return NULL;
        }
    
        int addBits(DbVar *db, cJSON *bits)
        {
            int asiz = cJSON_GetArraySize(bits);
            for (int i = 0  ; i < asiz; i++ )
            {
                cJSON* cji = cJSON_GetArrayItem(bits, i);
                // just use one copy of valuestring clean up will fix it.
                const char* bs = strdup(cji->valuestring);
                db->addBit(bs);
                bitsMap[bs] == std::make_pair(db, i);
            }
            return asiz;
        }
        // see if we have any uninitialized vars
        bool checkUris(int who)
        {
            dburi_map::iterator it;
            dbvar_map::iterator itd;
            bool ret = true;
            for (it = dburiMap.begin(); it != dburiMap.end(); ++it)
            {
                auto dvar = it->second;
                auto dbm = dvar->dbmap;
                for (itd = dbm.begin(); itd != dbm.end(); ++itd)
                {
                    DbVar* db = itd->second;
                    if (db->initSet == 0)
                    {
                        if(who == DNP3_OUTSTATION)
                        {
                            if ((db->type == Type_Analog) || (db->type == Type_Binary))
                            {
                                if(debug > 1)
                                    FPS_ERROR_PRINT(" %s init missing for [%s]/[%s]\n", __FUNCTION__, db->uri, db->name.c_str());
                                ret = false;
                            }
                        }
                        else // all others are on master
                        {
                            if (
                                (db->type == AnIn16) 
                                || (db->type == AnIn32)
                                || (db->type == AnF32 )
                                || (db->type == Type_Crob)
                                )
                            { 
                                FPS_ERROR_PRINT(" %s init missing for %s/%s\n", __FUNCTION__, db->uri, db->name.c_str());
                                ret = false;
                            }
                        }
                    }
                }
            }
            return ret;
        };

        // get the list of subs
        int getSubs(const char**subs, int num, int who)
        {
            if (num < static_cast<int32_t>(dburiMap.size()))
            {
                return dburiMap.size();
            }
            int idx = 0;
            if (subs)
            {
                dburi_map::iterator it;
                it = dburiMap.find(base_uri);
                if (it == dburiMap.end())
                {
                    subs[idx++] = base_uri;
                }
                if(local_uri)subs[idx++] = local_uri;

                for (it = dburiMap.begin(); it != dburiMap.end(); ++it)
                {
                    subs[idx++]=it->first.c_str();
                }
                if(1||debug)
                {
                    //FPS_ERROR_PRINT(" %s sub uris===>[       ]<=== \n", __FUNCTION__);
                    for (int i = 0 ; i < idx; i++)
                    {
                        FPS_ERROR_PRINT("      ===>[%s]<=== \n", subs[i]);
                    }
                }
            }
            else
            {
                FPS_ERROR_PRINT(" %s No Subs \n", __FUNCTION__);
            }
            
            return idx;
        }
        // put out the start up request from fims_echo
        // only get the ones for vars applied to this application (outstation or master)
        int getUris(int who)
        {
            if(debug)
                FPS_ERROR_PRINT(" %s uris===>%d<=== \n\n", __FUNCTION__, who);

            dburi_map::iterator it;
            for (it = dburiMap.begin(); it != dburiMap.end(); ++it)
            {
                char replyto[1024];
                char getUri[1024];
                //sprintf(replyto, "%s/reply%s", server_map->base_uri, it->first);
                //server_map->p_fims->Send("get", it->first, replyto, NULL);
                if (it->first[0] == '/') 
                {
                    snprintf(replyto, sizeof(replyto),"%s/reply%s",  base_uri, it->first.c_str());
                    snprintf(getUri,sizeof(getUri),"%s", it->first.c_str());
                } 
                else
                {
                    snprintf(replyto, sizeof(replyto),"%s/reply%s",  base_uri, it->first.c_str());
                    snprintf(getUri,sizeof(getUri),"%s", it->first.c_str());
                }
                if(debug)
                    FPS_ERROR_PRINT(" uri : [%s] replyto: [%s]\n"
                        , getUri
                        , replyto
                        );
        
                p_fims->Send("get", getUri, replyto, NULL);
            }
            if(debug)
                FPS_ERROR_PRINT(" %s<=== get uris DONE\n\n", __FUNCTION__);
            return 0;
        }

        // new way of doing this
        // we may not need this structure in the end
        // new structure varList  uri name, map of dbVars
        // struct varList {
        //    const char* uri;
        //    dbvar_map dbmap;
        //};
        //typedef std::map<std::string, varList*> dburi_map;
        //typedef std::map<std::string, DbVar_t*> dbvar_map;
//      typedef struct varList_t {
//           varList(const char* iuri){
//           uri = iuri;
//         };
//     // TODO delete dbmap
//        ~varList(){};
//        const char* uri;
//        dbvar_map dbmap;
//      } varList;

        void addDbUri(const char *uri, DbVar*db)
        {
            std::string mapUri = uri;
            // this is a pointer to the uri 
            // if there is not one in the map then create a new one and then add it
            //duri_map::iterator it_uris;
            if(dburiMap.find(uri) == dburiMap.end())
            {
                if(debug > 0)
                    FPS_ERROR_PRINT("     %s  ==> ADDED varlist [%s]  dburi size %d \n", __FUNCTION__, uri, (int) dburiMap.size());
                //dbvar_map dvar;
                dburiMap[mapUri] = new varList_t(uri); 
            }
            else
            {
                if(debug > 0)
                    FPS_ERROR_PRINT("     %s  ==> FOUND varlist [%s]  dburi size %d \n", __FUNCTION__, uri, (int) dburiMap.size());
            }
            //mymap.insert ( std::pair<char,int>('a',100) );
            varList_t *vl = dburiMap[mapUri];
            vl->addDb(db);

            //dbvar_map dbm = dburiMap[mapUri]->dbmap;
            //dbm.insert(std::pair<std::string,DbVar*>(db->name, db));

            if(debug > 0)
                FPS_ERROR_PRINT(" %s  ==> added var [%s]  dbm size %d \n", __FUNCTION__, db->name.c_str(), (int) vl->size());
        }

        char*getDefUri(int who)
        {
            int rc = 0;
            if(defUri == NULL)
            {
                if(base_uri == NULL)
                {
                    if(who == DNP3_MASTER)
                    {
                        rc=asprintf(&defUri,"/components/%s",id);
                    }
                    else
                    {
                        rc=asprintf(&defUri,"/interfaces/%s",id);
                    }
                }
                else
                {
                    rc=asprintf(&defUri,"%s", base_uri);
                }
                if(rc > 0)
                {
                    rc = 0;
                }
                
            }
            return defUri;
        }   
        
        char* name;
        char* protocol;
        char* id;
        char* ip_address;
        char *defUri;
        //char* pub;
        int version;
        int port;
        //int local_address;
        //int remote_address;
        int master_address;
        int station_address;
        int frequency;
        int freq1;
        int freq2;
        int freq3;
        bool unsolUpdate;
        bool useOffset;
        int rangeAStart;
        int rangeAStop;
        int rangeBStart;
        int rangeBStop;
        int rangeFreq;
        int idx;
        char* uri;
        bool useVindex;
        int who;   // master or outstation
        double deadband;

        // new way of doing this
        //dbvar_map dbMap;
        dbvec dbVec[Type_of_Var::NumTypes];
        // TODO remove the need for +1 we now use idx to position a variable
        dbix_map dbMapIxs[Type_of_Var::NumTypes+1];
        int maxIdx[Type_of_Var::NumTypes+1];
        //duri_map uriMap;
        bits_map bitsMap;
        dburi_map dburiMap;

        int numObjs[Type_of_Var::NumTypes];
        //bool useReadb[Type_of_Var::NumTypes]; // set true if writes to this type should be diverted to readb if setup

        fims* p_fims;
        cJSON* cj;  
        cJSON* cjPub;
        int cjloaded;
        int debug;
        int scanreq;     //used to request a class 1, 2 or 3 scan 
        int unsol;       // set to 1 to allow unsol in oustation
        cJSON* cjclass;  // used to change class of a var in outstation
        char* base_uri;
        char* local_uri;
        uint16_t pref;
        int num_subs;
        void *stackConfig;

        char* conn_type; // TCP or TLS (error out on anything else)

        // TLS stuff (might need another key):
        char* peerCertificate;
        char* privateKey;
        char* certificateChain;
        char* caCertificate;
        char* tls_folder_path;

        // RTU stuff:
        int baud;
        int dataBits;
        double stopBits;
        char* parity;
        char* flowType;
        int asyncOpenDelay;
        char* deviceName; // this might just be name really?
        char* valuestring;

} sysCfg;

DbVar* getDbVar(sysCfg* sysdb, const char* name);
void emit_event(sysCfg *sys, const char* source, const char* message, int severity);
cJSON* get_config_json(int argc, char* argv[], int num);

// new mapping
bool parse_system(cJSON* object, sysCfg* sys, int who);
bool parse_variables(cJSON* object, sysCfg* sys, int who);
cJSON* parseJSONConfig(char* file_path);

void pubWithTimeStamp(cJSON* cj, sysCfg* sys, const char* ev);

//void sysdbAddtoRecord(sysCfg* sysdb, const char* field, const AnalogOutputInt16& cmd, uint16_t index);
//void sysdbAddtoRecord(sysCfg* sysdb, const char* field, const AnalogOutputInt32& cmd, uint16_t index);
//void sysdbAddtoRecord(sysCfg* sysdb, const char* field, const AnalogOutputFloat32& cmd, uint16_t index);
//void sysdbAddtoRecord(sysCfg* sysdb, const char* field, const char* cmd, uint16_t index);
//const char* cfgGetSOEName(sysCfg* sysdb, const char* fname);

int addVarToCj(cJSON* cj, DbVar* db);
int addVarToCj(sysCfg* sys, cJSON* cj, const char* dname);

cJSON* parseBody(dbs_type& dbs, sysCfg* sys, fims_message* msg, int who);
int addValueToVec(dbs_type& dbs, sysCfg* sys, fims_message* msg, const char* name, cJSON* cjvalue, int flag);
int addValueToVec(dbs_type& dbs, sysCfg* sys, char* curi, const char* name, cJSON* cjvalue, int flag);

bool checkWho(sysCfg* sys, const char* uri, const char* name, int who);
bool checkWho(sysCfg* sys, DbVar* db, int who);
int getSysUris(sysCfg **sys, int who, const char** &subs, bool* &bpubs, int n);

const char *getVersion();
int getConfigs(int argc, char *argv[], sysCfg**sys, int who, fims *p_fims);

#endif
