#ifndef BASE_CPP
#define BASE_CPP
#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <iostream>
#include <sstream>
#include "simdjson.h" // for json parsing
//
//#include "dbVal.h"
//#include "dbSj.h"

#ifndef FPS_ERROR_PRINT 
#define FPS_ERROR_PRINT printf
#define FPS_DEBUG_PRINT printf
#endif
//#include "formatters.hpp"
/******************************************************
 *              
 *                 baseUtils.cpp
 *  p.wilshire
 *  02/05/2022
 *   test with test_checkUri
 *
 *     /ess   remove and pass
  *     remove and flags |= 1<<0 
 *         /full   remove and flags |= 1<<1 
 *         /naked   remove and flags |= 1<<2 
 *    
 ******************************************************/

/***********************************************
 *                 baseUtils
 ***********************************************/

// baseUtils::baseUtils()
// {
// }

// baseUtils::~baseUtils()
// {
// }

int split_delim(std::vector<std::string>&v, const std::string& s, char c) 
{
    auto end = s.cend();
    auto start = end;
    int idx = 0;
    for( auto it = s.cbegin(); it != end; ++it ) {
        if( *it != c ) {
            if( start == end )
                start = it;
            continue;
        }
        if( start != end ) {
            idx++;
            v.emplace_back(start, it);
            start = end;
        }
    }
    if( start != end )
    {
        idx++;
        v.emplace_back(start, end);
    }
    return idx;
}

int find_end_of_string(std::string &skey, const char*sp)
{
    int cnt = 0;
    int csnap = 0;
    bool done = false;
    bool term = false;
    const char* origsp=sp;
    while (!done && *sp)
    {
        if(!term && *sp == '"')
        {
            term = true;
            csnap = cnt;
        }
        else if(term && *sp == '"')
        {
            term = false;
        }
        if(term)
        {
            if(*sp == ':')
            {
                skey.append(origsp,csnap);
                return csnap;
            }
            else if(*sp == ',')
            {
                skey.append(origsp,csnap);
                return csnap;
            }
        }
        cnt++;
        sp++;
    }
    skey.append(origsp,csnap);

    return csnap;
}


// void r_print_dbSj(std::stringstream& out, int level, dbSj* db)
// {
//     //std::cout << "Running" << std::endl;
//     bool add_comma= false;

//     if(db->dbtype == dbSj::DB_BASE)
//     {
//         out << "{";// <<std::endl;
//         add_comma = false;
//         if(!db->name.empty())out << "\"" << db->name<< "\" :"; 
 
//         int xx = db->dvec.size();
//         for ( int i = 0; i < xx ; i++)
//         {
//             if (add_comma) {
//                 out << ",";//<< std::endl;
//             }
//             r_print_dbSj(out, level+1, db->dvec[i]);
//             add_comma = true;
//         }
//        out << "}"<< std::endl;
//     }
//     else if(db->dbtype == dbSj::DB_OBJ)
//     {
//         if(!db->name.empty()){out << "\"" << db->name<< "\" :";}
//         out << "{";//<<std::endl;
//         add_comma = false;
//         for ( auto  xx : db->dvec)
//         {
//             if (add_comma) {
//                 out << ",";//<< std::endl;
//             }
//             r_print_dbSj(out, level+1, xx);
//             add_comma = true;
//         }
//         out << "}";
//     }
//     else if(db->dbtype == dbSj::DB_ARRAY)
//     {
//         if(!db->name.empty()) {out << "\"" << db->name<< "\" :";} 
//         out << "[";// <<std::endl; 
//         add_comma =  false;
//         for ( auto xx : db->dvec)
//         {
//             if (add_comma) {
//                 out << ",";//<< std::endl;
//             }
//             r_print_dbSj(out, 1, xx);
//             add_comma = true;
//         }
//         out << "]"; 
//     }
//     else if(db->dbtype == dbSj::DB_STRING)
//     {
//         if(!db->name.empty())out << "\"" << db->name<< "\" :"; 

//         out << "\""<<db->dbval->valuestring<<"\"" ;
//     }
//     else if(db->dbtype == dbSj::DB_BOOL)
//     {
//         if(!db->name.empty())out << "\"" << db->name<< "\" :"; 
//         auto val = db->dbval->valuebool?"true":"false";
//         out << val;

//     }
//     else if(db->dbtype == dbSj::DB_DOUBLE)
//     {
//         if(!db->name.empty())out << "\"" << db->name<< "\" :"; 

//         out <<db->dbval->valuedouble;

//     }
//     else
//     {
//         if(!db->name.empty())out << "\"" << db->name<< "\" :"; 

//         out << db->get_dtype();
//     }
// }

// void r_print_dbSj2(std::stringstream& out, int level, dbSj* db)
// {
//     std::cout << " running " <<__func__ << std::endl;
//     r_print_dbSj(out, level, db);
// }

// void recursive_print_dbSj(int level, dbSj* db)
// {
//     if(db->dbtype == dbSj::DB_BASE)
//     {
//         std::cout << "[" << level <<"] type ["<< db->get_dtype()<<"] name -->  ["<< db->name<< "]" <<std::endl; 
//         for ( auto xx : db->dvec)
//         {
//             recursive_print_dbSj(level+1, xx);
//         }
//     }
//     else if(db->dbtype == dbSj::DB_OBJ)
//     {
//         std::cout << "obj [" << level <<"] type ["<< db->get_dtype()<<"] name -->  ["<< db->name<< "]" <<std::endl; 
//         for ( auto xx : db->dvec)
//         {
//             recursive_print_dbSj(level+1, xx);
//         }
//     }
//     else if(db->dbtype == dbSj::DB_ARRAY)
//     {
//         std::cout << " array [" << level <<"] type ["<< db->get_dtype()<<"] name -->  ["<< db->name<< "]" <<std::endl; 
//         for ( auto xx : db->dvec)
//         {
//             recursive_print_dbSj(level+1, xx);
//         }
//     }
//     else if(db->dbtype == dbSj::DB_STRING)
//     {
//         std::cout << "[" << level <<"] type ["<< db->get_dtype()<<"] name -->  ["<< db->name<< "] value [" <<db->dbval->valuestring<<"]" <<std::endl; 

//     }
//     else if(db->dbtype == dbSj::DB_DOUBLE)
//     {
//         std::cout << "[" << level <<"] type ["<< db->get_dtype()<<"] name -->  ["<< db->name<< "] value [" <<db->dbval->valuedouble<<"]" <<std::endl; 
//     }
//     else
//     {
//         std::cout << "[" << level <<"] type ["<< db->get_dtype()<<"] name -->  ["<< db->name<< "]" <<std::endl; 
//     }


// }
// // keep track of depth
// // if depth == 1 and we have a value then update the "value" param
// // if the depth > 1 and the name = value then flag the base that the value has been updated
// dbSj* recursive_load_json(dbSj*base, int depth, simdjson::ondemand::value element )
// {
//     //int eos = 0;
//     dbSj*db =  nullptr;
//     std::string skey = "";
//     if(!base)
//     {
//         base = new dbSj;        
//         base->dbtype=dbSj::DB_BASE;
//         //base->name = "__base";
//         base->parent = nullptr;
//         db =  recursive_load_json(base, depth+1, element);
//         return db;
//     }
//     else
//     {
//         switch (element.type()) {
//           case simdjson::ondemand::json_type::object:
//             //std::cout << "["<<depth<<"] OBJECT---base name>"<< base->name<<"<-- base type ["<< base->get_dtype()<<"]"<<std::endl;
//             if(base->dbtype == dbSj::DB_NONE) 
//                 base->dbtype = dbSj::DB_OBJ;
//             //add_comma = false;
//             for (auto field : element.get_object()) {
//                 skey = "";
//                 find_end_of_string(skey, field.key().raw());
//                 if(!skey.empty())
//                 {
//                     db = base->find_key(skey.c_str());  // creates a key if one is not found
//                     if(!db->parent) db->parent  = base;
//                     //db->depth = depth;
//                 }
//                 else
//                 {
//                     db=base;
//                 }

//                 recursive_load_json(db, depth+1, field.value());
//                 if(db->update)
//                 {
//                     std::cout << "["<<depth<<"] OBJECT--->"<< db->name<<"<--["<< db->get_dtype()<<"] UPDATED"<<std::endl;
//                     db->update = false;

//                 }
//                 //add_comma = true;
//             }
//             break;
//             case simdjson::ondemand::json_type::array:
//                 base->dbtype = dbSj::DB_ARRAY;
//                 std::cout << "["<<depth<<"] ARRAY---base name >"<< base->name<<"<-- base type ["<< base->get_dtype()<<"]"<<std::endl;
//                 for (auto field : element.get_array()) {
//                     // We need the call to value() to get
//                     // an ondemand::value type.
//                     skey = "";
//                     db = new dbSj;
//                     db->dbtype = dbSj::DB_OBJ;
//                     base->dvec.push_back(db);
//                     recursive_load_json(db, depth+1,field.value());
//                 }
//             break;
//             case simdjson::ondemand::json_type::number:
//                 // assume it fits in a double
//                 base->dbval->valuedouble = element.get_double();
//                 base->dbtype=dbSj::DB_DOUBLE;
//                 if(base->parent)
//                 {
//                     if (base->name == "value")
//                     {
//                         std::cout << "updating value " << std::endl;
//                         if(base->parent)
//                             base->parent->update = true;
//                     }
//                 }
//                 //base->update = true;
//             break;
//             case simdjson::ondemand::json_type::string:
//                 skey = "";
//                 //eos = 
//                 find_end_of_string(skey, element.get_raw_json_string().raw());
//                 base->dbval->valuestring = skey;
//                 base->dbtype=dbSj::DB_STRING;
//                 if(base->parent)
//                 {
//                     if (base->name == "value")
//                     {
//                         std::cout << "updating value " << std::endl;
//                         if(base->parent)
//                             base->parent->update = true;
//                     }
//                 }
//             break;
//             case simdjson::ondemand::json_type::boolean:
//                 base->dbval->valuebool = element.get_bool();
//                 base->dbtype=dbSj::DB_BOOL;
//                 if(base->parent)
//                 {
//                     if (base->name == "value")
//                     {
//                         std::cout << "updating value " << std::endl;
//                         if(base->parent)
//                             base->parent->update = true;
//                     }
//                 }
//             break;
//             case simdjson::ondemand::json_type::null:
//                 base->dbtype=dbSj::DB_NULL;
//                 if(base->parent)
//                 {
//                     if (base->name == "value")
//                     {
//                         std::cout << "updating value " << std::endl;
//                         if(base->parent)
//                             base->parent->update = true;
//                     }
//                 }
//             break;
//         }
//     }
//     return base;
// }

// void recursive_print_json(simdjson::ondemand::value element) {
//   bool add_comma;
//   switch (element.type()) {
//   case simdjson::ondemand::json_type::array:
//     std::cout << "[";
//     add_comma = false;
//     for (auto child : element.get_array()) {
//       if (add_comma) {
//         std::cout << ",";
//       }
//       // We need the call to value() to get
//       // an ondemand::value type.
//       recursive_print_json(child.value());
//       add_comma = true;
//     }
//     std::cout << "]";
//     break;
//   case simdjson::ondemand::json_type::object:
//     std::cout << "{";
//     add_comma = false;
//     for (auto field : element.get_object()) {
//       if (add_comma) {
//         std::cout << ",";
//       }
//       // key() returns the key as it appears in the raw
//       // JSON document, if we want the unescaped key,
//       // we should do field.unescaped_key().
//       std::cout << "\"" << field.key() << "\": ";
//       recursive_print_json(field.value());
//       add_comma = true;
//     }
//     std::cout << "}\n";
//     break;
//   case simdjson::ondemand::json_type::number:
//     // assume it fits in a double
//     std::cout << element.get_double();
//     break;
//   case simdjson::ondemand::json_type::string:
//     // get_string() would return escaped string, but
//     // we are happy with unescaped string.
//     std::cout << "\"" << element.get_raw_json_string() << "\"";
//     break;
//   case simdjson::ondemand::json_type::boolean:
//     std::cout << element.get_bool();
//     break;
//   case simdjson::ondemand::json_type::null:
//     std::cout << "null";
//     break;
//   }
// }

// // padded_string s(len);
// //std::fread(s.data(), 1, len, fp);
// //simdjson_result<padded_string>

// // dbSj* parse_input(dbSj*base, const char *json, const char* fname )
// // {
// //    const size_t json_len = std::strlen(json);
// //    simdjson::ondemand::parser parser;
// //    *   simdjson::dom::element element = parser.parse(json, json_len);

// dbSj* parse_input(dbSj*base, const simdjson::simdjson_result<simdjson::padded_string> &json, const char* fname )
// {
//     simdjson::ondemand::parser parser;
//     simdjson::ondemand::document doc;


//     //const simdjson::simdjson_result<simdjson::padded_string> json = simdjson::padded_string::load(fname);
//     //const auto json = simdjson::padded_string::load(fname);

//     doc = parser.iterate(json); 

//     simdjson::ondemand::value val = doc;

//     base = recursive_load_json(base, 0, val);
//     std::cout<< " after file >" << fname << "<"<< std::endl; 
//     recursive_print_dbSj(0, base);
//     return base;
// }

// dbSj* parse_input(dbSj* base, const char* fname )
// {
//     simdjson::ondemand::parser parser;
//     simdjson::ondemand::document doc;

//     const simdjson::simdjson_result<simdjson::padded_string> json = simdjson::padded_string::load(fname);
//     auto err = json.error();
//     if (err)
//     {
//         FPS_ERROR_PRINT("Can't open json file %s, err = ??. Please check your config path\n", fname);//, simdjson::error_message(err));
//         return nullptr;
//     }
//     return parse_input(base, json, fname);
// }

// //https://raw.githubusercontent.com/simdjson/simdjson/master/singleheader/simdjson.h

// dbSj* parse_input_str(dbSj* base, const char* data , const char * uri)
// {
//     simdjson::ondemand::parser parser;
//     simdjson::ondemand::document doc;
//      const size_t data_len = std::strlen(data);


//     const simdjson::simdjson_result<simdjson::padded_string> json = simdjson::padded_string(data, data_len);
//     //const auto json = simdjson::padded_string::load(fname);

//     doc = parser.iterate(json); 

//     simdjson::ondemand::value val = doc;

//     base = recursive_load_json(base, 0, val);
//     std::cout<< " after input_str >" << uri << "<"<< std::endl; 
//     recursive_print_dbSj(0, base);
//     return base;
// }

// void parse_print_input(const simdjson::simdjson_result<simdjson::padded_string> &json, const char* fname )
// {
//     simdjson::ondemand::parser parser;
//     simdjson::ondemand::document doc;


//     //const simdjson::simdjson_result<simdjson::padded_string> json = simdjson::padded_string::load(fname);
//     //const auto json = simdjson::padded_string::load(fname);

//     doc = parser.iterate(json); 

//     simdjson::ondemand::value val = doc;

//     recursive_print_json(val);
// }

// void parse_print_input(const char* fname)
// {
//     const simdjson::simdjson_result<simdjson::padded_string> json = simdjson::padded_string::load(fname);
//     auto err = json.error();
//     if (err)
//     {
//         FPS_ERROR_PRINT("Can't open json file %s, err = ??. Please check your config path\n", fname);//, simdjson::error_message(err));
//         return;
//     }
//     parse_print_input(json, fname);
// }

// split up the string /a:/b: into the subs** that we need
// get the list of subs
char** getSubs(int &idx, char* in)
{
    idx = 1;
    char *spin = strdup(in) ;
    char*sp = spin;
    std::cout << "spin = [" << spin << "]" << std::endl;
    char ** subs = nullptr;
    while (*sp)
    {
        if(*sp == ':')
        {
            *sp = 0;
            idx++;
            sp++;
        }
        sp++;
    }
    subs = (char**)malloc(sizeof(char*) * idx);

    sp = spin;
    int i = 0;
    while (i < idx)
    {
        subs[i] =  sp;
        std::cout << "sp = [" << sp << "]" << std::endl;

        sp += strlen(sp);
        sp++;
        i++;
    }
    //free(spin);
    
    return subs;
}
#endif

