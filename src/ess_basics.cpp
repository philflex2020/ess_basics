
/* this is the outline of a master dnp3 controller
* the system can communicate with a number outstations
* The confg file will determine the address info on each outstation.
* each outsation will have a number of data points.
* a feature of dnp3 is the need to povide a shadow  stack to hold the user defined data points
* iniitally a copy of the modbus stuff
* note that modbus_client only responds to gets and sets on the base_url
* it pubs the result of the query_registers
*base_uri is set by /components + sys_cfg.name
*/
/*
 * ess_basics.cpp
 *
 *  Created on: jan 16, 2022
 *      Author: pwilshire - new version no dnp3 stuff in it
 */

#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>
#include <map>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <signal.h>
#include <climits>

#include <arpa/inet.h>
#include <stdint.h>
#include <cjson/cJSON.h>
#include <fims/libfims.h>

#include <iostream>
#include <sstream>
#include "simdjson.h" // for json parsing


using namespace std; 

#define MICROSECOND_TO_MILLISECOND 1000
#define NANOSECOND_TO_MILLISECOND  1000000
#define MAX_FIMS_CONNECT 5

volatile bool running = true;
int num_configs = 0;
#define MAX_CONFIGS 16
//sysCfg *sys_cfg[MAX_CONFIGS];

#ifndef FPS_ERROR_PRINT 
#define FPS_ERROR_PRINT printf
#define FPS_DEBUG_PRINT printf
#endif

void signal_handler (int sig)
{
    running = false;
    FPS_ERROR_PRINT("signal of type %d caught.\n", sig);
    signal(sig, SIG_DFL);
}
// baseUtils
int split_delim(std::vector<std::string>&v, const std::string& s, char c);


// class dbSj;

// static std::map<dbSj*,dbSj*> dbSjMap;
// attempt to load  an updateable  dbs stricture directly from simdjson

#include "dbVal.h"
#include "dbSj.h"

// // todo 
// //    track owners only delete when owners are all gone
// //    add set/get val with locks
// class dbVal {

// public:
//     dbVal(){};
//     ~dbVal(){};
//     enum dtype { DB_BASE, DB_NONE, DB_OBJ,DB_ARRAY,DB_STRING,DB_DOUBLE, DB_INT,DB_BOOL,DB_NULL,DB_END};
//     dtype dbtype;
//     std::vector<void*> ownvec;
//     std::string valuestring;
// union {
//     double valuedouble;
//     double valueint;
//     bool valuebool;
//     };
// };

void doExit()
{
    cout << "on exit map size :" << dbSjMap.size()<< std::endl;
    for (auto xx : dbSjMap)
    {
        dbSj *db = xx.second;
        if(db)
        {
            db->getroot();
            cout << " root ["<<db->root<<"] name ["<< db->name<<"] depth :[" << db->depth << "] dbtype :["<<db->dbtype<<"]"<<std::endl; 
            //delete db;
            dbSjMap[xx.first] = nullptr;
        }
    }
}
// assume simple strings terminated by \"  followed by space or ":"
int find_end_of_string(std::string &skey, const char*sp);
// {
//     int cnt = 0;
//     int csnap = 0;
//     bool done = false;
//     bool term = false;
//     const char* origsp=sp;
//     while (!done && *sp)
//     {
//         if(!term && *sp == '"')
//         {
//             term = true;
//             csnap = cnt;
//         }
//         else if(term && *sp == '"')
//         {
//             term = false;
//         }
//         if(term)
//         {
//             if(*sp == ':')
//             {
//                 skey.append(origsp,csnap);
//                 return csnap;
//             }
//             else if(*sp == ',')
//             {
//                 skey.append(origsp,csnap);
//                 return csnap;
//             }
//         }
//         cnt++;
//         sp++;
//     }
//     skey.append(origsp,csnap);

//     return csnap;
// }

void r_print_dbSj(std::stringstream& out, int level, dbSj* db)
{
    //cout << "Running" << std::endl;
    bool add_comma= false;

    if(db->dbtype == dbSj::DB_BASE)
    {
        out << "{";// <<std::endl;
        add_comma = false;
        if(!db->name.empty())out << "\"" << db->name<< "\" :"; 
 
        int xx = db->dvec.size();
        for ( int i = 0; i < xx ; i++)
        {
            if (add_comma) {
                out << ",";//<< std::endl;
            }
            r_print_dbSj(out, level+1, db->dvec[i]);
            add_comma = true;
        }
       out << "}"<< std::endl;
    }
    else if(db->dbtype == dbSj::DB_OBJ)
    {
        if(!db->name.empty()){out << "\"" << db->name<< "\" :";}
        out << "{";//<<std::endl;
        add_comma = false;
        for ( auto  xx : db->dvec)
        {
            if (add_comma) {
                out << ",";//<< std::endl;
            }
            r_print_dbSj(out, level+1, xx);
            add_comma = true;
        }
        out << "}";
    }
    else if(db->dbtype == dbSj::DB_ARRAY)
    {
        if(!db->name.empty()) {out << "\"" << db->name<< "\" :";} 
        out << "[";// <<std::endl; 
        add_comma =  false;
        for ( auto xx : db->dvec)
        {
            if (add_comma) {
                out << ",";//<< std::endl;
            }
            r_print_dbSj(out, 1, xx);
            add_comma = true;
        }
        out << "]"; 
    }
    else if(db->dbtype == dbSj::DB_STRING)
    {
        if(!db->name.empty())out << "\"" << db->name<< "\" :"; 

        out << "\""<<db->dbval->valuestring<<"\"" ;
    }
    else if(db->dbtype == dbSj::DB_BOOL)
    {
        if(!db->name.empty())out << "\"" << db->name<< "\" :"; 
        auto val = db->dbval->valuebool?"true":"false";
        out << val;

    }
    else if(db->dbtype == dbSj::DB_DOUBLE)
    {
        if(!db->name.empty())out << "\"" << db->name<< "\" :"; 

        out <<db->dbval->valuedouble;

    }
    else
    {
        if(!db->name.empty())out << "\"" << db->name<< "\" :"; 

        out << db->get_dtype();
    }
}

void r_print_dbSj2(std::stringstream& out, int level, dbSj* db)
{
    cout << " running " <<__func__ << std::endl;
    r_print_dbSj(out, level, db);
}

void recursive_print_dbSj(int level, dbSj* db)
{
    if(db->dbtype == dbSj::DB_BASE)
    {
        cout << "[" << level <<"] type ["<< db->get_dtype()<<"] name -->  ["<< db->name<< "]" <<std::endl; 
        for ( auto xx : db->dvec)
        {
            recursive_print_dbSj(level+1, xx);
        }
    }
    else if(db->dbtype == dbSj::DB_OBJ)
    {
        cout << "obj [" << level <<"] type ["<< db->get_dtype()<<"] name -->  ["<< db->name<< "]" <<std::endl; 
        for ( auto xx : db->dvec)
        {
            recursive_print_dbSj(level+1, xx);
        }
    }
    else if(db->dbtype == dbSj::DB_ARRAY)
    {
        cout << " array [" << level <<"] type ["<< db->get_dtype()<<"] name -->  ["<< db->name<< "]" <<std::endl; 
        for ( auto xx : db->dvec)
        {
            recursive_print_dbSj(level+1, xx);
        }
    }
    else if(db->dbtype == dbSj::DB_STRING)
    {
        cout << "[" << level <<"] type ["<< db->get_dtype()<<"] name -->  ["<< db->name<< "] value [" <<db->dbval->valuestring<<"]" <<std::endl; 

    }
    else if(db->dbtype == dbSj::DB_DOUBLE)
    {
        cout << "[" << level <<"] type ["<< db->get_dtype()<<"] name -->  ["<< db->name<< "] value [" <<db->dbval->valuedouble<<"]" <<std::endl; 
    }
    else
    {
        cout << "[" << level <<"] type ["<< db->get_dtype()<<"] name -->  ["<< db->name<< "]" <<std::endl; 
    }
}
// // keep track of depth
// // if depth == 1 and we have a value then update the "value" param
// // if the depth > 1 and the name = value then flag the base that the value has been updated
dbSj* recursive_load_json(dbSj*base, int depth, simdjson::ondemand::value element )
{
    //int eos = 0;
    dbSj*db =  nullptr;
    std::string skey = "";
    if(!base)
    {
        base = new dbSj;        
        base->dbtype=dbSj::DB_BASE;
        //base->name = "__base";
        base->parent = nullptr;
        db =  recursive_load_json(base, depth+1, element);
        return db;
    }
    else
    {
        switch (element.type()) {
          case simdjson::ondemand::json_type::object:
            //cout << "["<<depth<<"] OBJECT---base name>"<< base->name<<"<-- base type ["<< base->get_dtype()<<"]"<<std::endl;
            if(base->dbtype == dbSj::DB_NONE) 
                base->dbtype = dbSj::DB_OBJ;
            //add_comma = false;
            for (auto field : element.get_object()) {
                skey = "";
                find_end_of_string(skey, field.key().raw());
                if(!skey.empty())
                {
                    db = base->find_key(skey.c_str());  // creates a key if one is not found
                    if(!db->parent) db->parent  = base;
                    //db->depth = depth;
                }
                else
                {
                    db=base;
                }

                recursive_load_json(db, depth+1, field.value());
                if(db->update)
                {
                    cout << "["<<depth<<"] OBJECT--->"<< db->name<<"<--["<< db->get_dtype()<<"] UPDATED"<<std::endl;
                    db->update = false;

                }
                //add_comma = true;
            }
            break;
            case simdjson::ondemand::json_type::array:
                base->dbtype = dbSj::DB_ARRAY;
                cout << "["<<depth<<"] ARRAY---base name >"<< base->name<<"<-- base type ["<< base->get_dtype()<<"]"<<std::endl;
                for (auto field : element.get_array()) {
                    // We need the call to value() to get
                    // an ondemand::value type.
                    skey = "";
                    db = new dbSj;
                    db->dbtype = dbSj::DB_OBJ;
                    base->dvec.push_back(db);
                    recursive_load_json(db, depth+1,field.value());
                }
            break;
            case simdjson::ondemand::json_type::number:
                // assume it fits in a double
                base->dbval->valuedouble = element.get_double();
                base->dbtype=dbSj::DB_DOUBLE;
                if(base->parent)
                {
                    if (base->name == "value")
                    {
                        std::cout << "updating value " << std::endl;
                        if(base->parent)
                        {
                            base->parent->update = true;
                            if(base->parent->dbval != base->dbval)
                            {
                                //delete base->parent->dbval;
                                base->parent->dbval = base->dbval;
                            }
                        }
                    }
                }
                //base->update = true;
            break;
            case simdjson::ondemand::json_type::string:
                skey = "";
                //eos = 
                find_end_of_string(skey, element.get_raw_json_string().raw());
                base->dbval->valuestring = skey;
                base->dbtype=dbSj::DB_STRING;
                if(base->parent)
                {
                    if (base->name == "value")
                    {
                        std::cout << "updating value " << std::endl;
                        if(base->parent)
                        {
                            base->parent->update = true;
                            if(base->parent->dbval != base->dbval)
                            {
                                //delete base->parent->dbval;
                                base->parent->dbval = base->dbval;
                            }
                        }
                    }
                }
            break;
            case simdjson::ondemand::json_type::boolean:
                base->dbval->valuebool = element.get_bool();
                base->dbtype=dbSj::DB_BOOL;
                if(base->parent)
                {
                    if (base->name == "value")
                    {
                        std::cout << "updating value " << std::endl;
                        if(base->parent)
                        {
                            base->parent->update = true;
                            if(base->parent->dbval != base->dbval)
                            {
                                //delete base->parent->dbval;
                                base->parent->dbval = base->dbval;
                            }
                        }
                    }
                }
            break;
            case simdjson::ondemand::json_type::null:
                base->dbtype=dbSj::DB_NULL;
                if(base->parent)
                {
                    if (base->name == "value")
                    {
                        std::cout << "updating value " << std::endl;
                        if(base->parent)
                        {
                            base->parent->update = true;
                            if(base->parent->dbval != base->dbval)
                            {
                                //delete base->parent->dbval;
                                base->parent->dbval = base->dbval;
                            }
                        }
                    }
                }
            break;
        }
    }
    return base;
}

void recursive_print_json(simdjson::ondemand::value element)
{
  bool add_comma;
  switch (element.type()) {
  case simdjson::ondemand::json_type::array:
    cout << "[";
    add_comma = false;
    for (auto child : element.get_array()) {
      if (add_comma) {
        cout << ",";
      }
      // We need the call to value() to get
      // an ondemand::value type.
      recursive_print_json(child.value());
      add_comma = true;
    }
    cout << "]";
    break;
  case simdjson::ondemand::json_type::object:
    cout << "{";
    add_comma = false;
    for (auto field : element.get_object()) {
      if (add_comma) {
        cout << ",";
      }
      // key() returns the key as it appears in the raw
      // JSON document, if we want the unescaped key,
      // we should do field.unescaped_key().
      cout << "\"" << field.key() << "\": ";
      recursive_print_json(field.value());
      add_comma = true;
    }
    cout << "}\n";
    break;
  case simdjson::ondemand::json_type::number:
    // assume it fits in a double
    cout << element.get_double();
    break;
  case simdjson::ondemand::json_type::string:
    // get_string() would return escaped string, but
    // we are happy with unescaped string.
    cout << "\"" << element.get_raw_json_string() << "\"";
    break;
  case simdjson::ondemand::json_type::boolean:
    cout << element.get_bool();
    break;
  case simdjson::ondemand::json_type::null:
    cout << "null";
    break;
  }
}

// // padded_string s(len);
// //std::fread(s.data(), 1, len, fp);
// //simdjson_result<padded_string>

// // dbSj* parse_input(dbSj*base, const char *json, const char* fname )
// // {
// //    const size_t json_len = std::strlen(json);
// //    simdjson::ondemand::parser parser;
// //    *   simdjson::dom::element element = parser.parse(json, json_len);

dbSj* parse_input(dbSj*base, const simdjson::simdjson_result<simdjson::padded_string> &json, const char* fname )
{
    simdjson::ondemand::parser parser;
    simdjson::ondemand::document doc;


    //const simdjson::simdjson_result<simdjson::padded_string> json = simdjson::padded_string::load(fname);
    //const auto json = simdjson::padded_string::load(fname);

    doc = parser.iterate(json); 

    simdjson::ondemand::value val = doc;

    base = recursive_load_json(base, 0, val);
    cout<< " after file >" << fname << "<"<< std::endl; 
    recursive_print_dbSj(0, base);
    return base;
}

dbSj* parse_input(dbSj* base, const char* fname )
{
    simdjson::ondemand::parser parser;
    simdjson::ondemand::document doc;

    const simdjson::simdjson_result<simdjson::padded_string> json = simdjson::padded_string::load(fname);
    auto err = json.error();
    if (err)
    {
        FPS_ERROR_PRINT("Can't open json file %s, err = ??. Please check your config path\n", fname);//, simdjson::error_message(err));
        return nullptr;
    }
    return parse_input(base, json, fname);
}

// //https://raw.githubusercontent.com/simdjson/simdjson/master/singleheader/simdjson.h

dbSj* parse_input_str(dbSj* base, const char* data , const char * uri)
{
    simdjson::ondemand::parser parser;
    simdjson::ondemand::document doc;
     const size_t data_len = std::strlen(data);


    const simdjson::simdjson_result<simdjson::padded_string> json = simdjson::padded_string(data, data_len);
    //const auto json = simdjson::padded_string::load(fname);

    doc = parser.iterate(json); 

    simdjson::ondemand::value val = doc;

    base = recursive_load_json(base, 0, val);
    cout<< " after input_str >" << uri << "<"<< std::endl; 
    recursive_print_dbSj(0, base);
    return base;
}

void parse_print_input(const simdjson::simdjson_result<simdjson::padded_string> &json, const char* fname )
{
    simdjson::ondemand::parser parser;
    simdjson::ondemand::document doc;


    //const simdjson::simdjson_result<simdjson::padded_string> json = simdjson::padded_string::load(fname);
    //const auto json = simdjson::padded_string::load(fname);

    doc = parser.iterate(json); 

    simdjson::ondemand::value val = doc;

    recursive_print_json(val);
}

void parse_print_input(const char* fname)
{
    const simdjson::simdjson_result<simdjson::padded_string> json = simdjson::padded_string::load(fname);
    auto err = json.error();
    if (err)
    {
        FPS_ERROR_PRINT("Can't open json file %s, err = ??. Please check your config path\n", fname);//, simdjson::error_message(err));
        return;
    }
    parse_print_input(json, fname);
}

// // split up the string /a:/b: into the subs** that we need
// // get the list of subs
char** getSubs(int &idx, char* in);


int main(int argc, char *argv[])
{
    // dbSj* base2 = new dbSj;  // 232
    // if(base2)
    // {
    //     delete base2;
    //     return 0;
    // }

    fims* p_fims;
    p_fims = new fims();
    dbSj* base = nullptr;

    running = true;
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    int fims_connect = 0;
    if(argc < 2 )
    {
        cout << "Using default config" << std::endl;
        base = parse_input(base, "config/default.json");
    }
    else
    {
        int idx = 1;
        while (idx < argc)
        {
            base = parse_input(base, argv[idx]);
            idx++;
        }
    }
    //base = parse_input(base, argv[2]);
    //parse_print_input(argv[2]);
    auto dsubs = base->find("subs");
    char** ssubs = nullptr;
    int idx = 0;

    if(dsubs)
    {
        cout << " Found subs [" << dsubs->dbval->valuestring <<"]";
        ssubs = 
        getSubs(idx, (char*)dsubs->dbval->valuestring.c_str());
        cout << " idx  [" << idx <<"]"<< std::endl;
    }
    FPS_ERROR_PRINT("Number of subscriptions %d.\n", idx);
    for (int is = 0 ; is < idx ; is++)
    {
        FPS_ERROR_PRINT(" subscription %d is [%s]\n", is, ssubs[is]);
    }

    {
        char tmp[1024];
        snprintf(tmp, sizeof(tmp),"DNP3_M_%s", "test");
        while(fims_connect < MAX_FIMS_CONNECT && p_fims->Connect(tmp) == false)
        {
            fims_connect++;
            sleep(1);
        }
        FPS_ERROR_PRINT(" Fims Connected %d\n", fims_connect);
    }
    if(p_fims->Subscribe((const char**)ssubs, idx /*, (bool *)bpubs*/) == false)
    {
        FPS_ERROR_PRINT("Subscription failed.\n");
        p_fims->Close();
        return 1;
    }
    base->setVal(base,"/components/base:test:val:data",1234.2);

    std::stringstream out;
    r_print_dbSj(out, 0, base);
    cout<<out.str()<<std::endl;

    while(running && p_fims->Connected())
    {
        // once a second
        fims_message* msg = p_fims->Receive_Timeout(1000);
        if(msg)
        {
            bool runrep = true;
            if ((strcmp(msg->method,"set")== 0) || (strcmp(msg->method,"pub")== 0))
            {
                if(msg->body)
                {
                    if (strcmp(msg->method,"pub")== 0)
                    {
                        runrep=false;
                    }

                    auto dreps = base->find(msg->uri);
                    if(!dreps)
                    {
                        cout << " Adding uri -->" << msg->uri << "<--" << std::endl;
                        dreps =  base->adddbSj(msg->uri);
                    }
                    else
                    {
                        cout << " found uri -->" << msg->uri << "<--" << std::endl;
                    }
                    parse_input_str(dreps, msg->body, msg->uri);
                    if(msg->replyto&& runrep)
                    {
                        p_fims->Send("set",msg->replyto,nullptr, msg->body);
                    }
                }
            }
            else if (strcmp(msg->method,"get")== 0)
            {
                std::stringstream out;
                out << "{";
                auto dreps = base->find(msg->uri);
                if(dreps)
                {
                    r_print_dbSj2(out, 0, dreps);
                }
                else
                {
                    r_print_dbSj2(out, 0, base);
                    //out << " Error :: uri [" <<msg->uri<<"]  Not found";
                }
                
                out << "}";
                if(msg->replyto)
                {

                    p_fims->Send("set",msg->replyto,nullptr, out.str().c_str());
                }
            }
            else
            {
                FPS_ERROR_PRINT(" got a fims message type [%s]", msg->method);
                FPS_ERROR_PRINT(" uri [%s]", msg->uri);
                if(msg->body) FPS_ERROR_PRINT(" body [%s]", msg->body);
                FPS_ERROR_PRINT("\n");

            }
            p_fims->free_message(msg);
        }
    }
    FPS_ERROR_PRINT(" System completed OK\n");
    if(p_fims)p_fims->Close();
    doExit();
    return 0;
    

    // num_configs = getConfigs(argc, argv, (sysCfg**)&sys_cfg, 0, p_fims);

    
    // const char **subs = NULL;
    // bool *bpubs = NULL;
    
    //  // TODO make this ppen to subs. Perhaps we need a vector
    // // components is pulled by uri  repeat for exch config
    // int num = getSysUris((sysCfg **)&sys_cfg, 0, subs, bpubs, num_configs);
    // if(num < 0)
    // {
    //     FPS_ERROR_PRINT("Failed to create subs array.\n");
    //     return 1;
    // }

    // if (p_fims == NULL)
    // {
    //     FPS_ERROR_PRINT("Failed to allocate connection to FIMS server.\n");
    //     //rc = 1;
    //     return 1;//goto cleanup;
    // }
    // // use the id for fims connect bt also add master designation 
    // {
    //     char tmp[1024];
    //     snprintf(tmp, sizeof(tmp),"DNP3_M_%s", sys_cfg[0]->id);
    //     while(fims_connect < MAX_FIMS_CONNECT && p_fims->Connect(tmp) == false)
    //     {
    //         fims_connect++;
    //         sleep(1);
    //     }
    // }

    // if(fims_connect >= MAX_FIMS_CONNECT)
    // {
    //     FPS_ERROR_PRINT("Failed to establish connection to FIMS server.\n");
    //     return 1;
    // }

    // FPS_DEBUG_PRINT("Map configured: Initializing data.\n");
    // //if(p_fims->Subscribe((const char**)sub_array, 3, (bool *)publish_only) == false)
    // if(p_fims->Subscribe((const char**)subs, num, (bool *)bpubs) == false)
    // {
    //     FPS_ERROR_PRINT("Subscription failed.\n");
    //     p_fims->Close();
    //     return 1;
    // }
    // FPS_ERROR_PRINT("Number of subscriptions %d.\n", num);
    // for (int is = 0 ; is < num ; is++)
    // {
    //     FPS_ERROR_PRINT(" subscriptions %d is [%s]\n", is, subs[is]);
    // }

    // free((void *)bpubs);
    // free((void *)subs);
    // while(running && p_fims->Connected())
    // {
    //     // once a second
    //     fims_message* msg = p_fims->Receive_Timeout(1000);
    //     //sysCfg *sys = sys_cfg[0];
    //     if(msg != NULL)
    //     {
    //         for (int ixs = 0 ; ixs < num_configs; ixs++ )
    //         {
    //             sysCfg *sys = sys_cfg[ixs];
            
    //             dbs_type dbs; // collect all the parsed vars here
    //             // We can use a single dbs 
    //             cJSON* cjb = parseBody(dbs, sys, msg, DNP3_MASTER);

    //             if(dbs.size() > 0)
    //             {
    //                 int commands;
    //                 int numCmds = 0;
    //                 cJSON*cj = NULL;                
    //                 if((msg->replyto != NULL) && (strcmp(msg->method,"pub") != 0))
    //                     cj = cJSON_CreateObject();

    //                 while (!dbs.empty())
    //                 {
    //                     std::pair<DbVar*,int>dbp = dbs.back();
    //                     // only do this on sets or posts
    //                     if ((strcmp(msg->method,"set") == 0) || (strcmp(msg->method,"post") == 0))
    //                     {
    //                         addVarToCommands(&commands, dbp);
    //                         numCmds++;
    //                     }
    //                     if(cj) addVarToCj(cj, dbp);
    //                     dbs.pop_back();
    //                 }

    //                 if(cj)
    //                 {
    //                     const char* reply = cJSON_PrintUnformatted(cj);
    //                     cJSON_Delete(cj);
    //                     cj = NULL;
    //                     // TODO check that SET is the correct thing to do in MODBUS_CLIENT
    //                     // probably OK since this is a reply
    //                     if(msg->replyto)
    //                         p_fims->Send("set", msg->replyto, NULL, reply);

    //                     free((void* )reply);
    //                 }
    //                 if(numCmds > 0)
    //                 {
    //                     FPS_DEBUG_PRINT("      *****Running Direct Outputs \n");
    //                     //sys->master->DirectOperate(std::move(commands), fpsCommandCallback::Get());
    //                 }
    //             }
            
    //             // TODO master has to be assigned per config
    //             if (sys->scanreq > 0)
    //             {
    //                 switch (sys->scanreq)
    //                 {
    //                     case 1:
    //                     {
    //                         //sys->master->ScanClasses(ClassField(ClassField::CLASS_1));
    //                         break;
    //                     }
    //                     case 2:
    //                     {
    //                         //sys->master->ScanClasses(ClassField(ClassField::CLASS_2));
    //                         break;
    //                     }
    //                     case 3:
    //                     {
    //                         //sys->master->ScanClasses(ClassField(ClassField::CLASS_3));
    //                         break;
    //                     }
    //                     default:
    //                         break;
    //                 }
    //                 FPS_ERROR_PRINT("****** master scanreq %d serviced\n", sys->scanreq);
    //                 sys->scanreq = 0;
    //             }

    //             if (cjb != NULL)
    //             {
    //                 cJSON_Delete(cjb);
    //             }
    //         }
    //         p_fims->free_message(msg);
    //     }
    // }

    // //cleanup:
    // // if (manager){
    // //     delete manager;
    // //     //delete channel;
    // // }
    // if (p_fims) delete p_fims;
    // // sys_cfg should clean itself up
 
    // //if(sys_cfg.eth_dev       != NULL) free(sys_cfg.eth_dev);
    // //if(sys_cfg.ip_address    != NULL) free(sys_cfg.ip_address);
    // //if(sys_cfg.name          != NULL) free(sys_cfg.name);
    // //if(sys_cfg.serial_device != NULL) free(sys_cfg.serial_device);
    // //if(sys_cfg.mb != NULL)             modbus_free(sys_cfg.mb);
    // // for(int fd = 0; fd < fd_max; fd++)
    // //     if(FD_ISSET(fd, &all_connections))
    // //         close(fd);
    // for (int ix = 0; ix < num_configs; ix++)
    // {
    //     FPS_ERROR_PRINT("****** client cleanup sys %d start\n", ix);

    //     sysCfg *sys = sys_cfg[ix];
    //     //if (sys->master)  delete sys->master;
    //     //if (sys->stackConfig)  delete sys->stackConfig;
    //     delete sys;
    //     FPS_ERROR_PRINT("****** client cleanup sys %d done\n", ix);

    // }

    return 0;
}


