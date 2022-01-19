/*
    * dnp3_server (outstation)
    * pwilshire 
    *   july 29, 2020
    * 
*/

#include <fims/libfims.h>
#include <string.h>
#include <stdint.h>

#include <fims/fps_utils.h>
#include <cjson/cJSON.h>
#include <unistd.h>
#include "dnp3_utils.h"

#include <iostream>
#include <string>
#include <thread>
#include <stdio.h>

#include <opendnp3/LogLevels.h>
#include <opendnp3/outstation/IUpdateHandler.h>

#include <asiodnp3/DNP3Manager.h>
#include <asiodnp3/UpdateBuilder.h>
#include "fpsCommandHandler.h"
#include "fpsLogger.h"
#include "fpsChannelListener.h"
#include "fpsOutstationApplication.h"


using namespace std;
using namespace opendnp3;
using namespace openpal;
using namespace asiopal;
using namespace asiodnp3;
#define MICROSECOND_TO_MILLISECOND 1000
#define NANOSECOND_TO_MILLISECOND  1000000
#define MAX_FIMS_CONNECT 5

fims *p_fims;
int num_configs = 0;

#define MAX_CONFIGS 16
sysCfg *sys_cfg[MAX_CONFIGS];

//TODO fill out from system config
void ConfigureDatabase(DatabaseConfig& config, sysCfg* sys)
{
    
    // just deal with analog vars and Group30Var5, this allows floating point numbers through the system
    //auto dsize = sys->dbVec[Type_Analog].size();
    auto dsize = sys->getTypeSize(Type_Analog);
    for (int i = 0; i < static_cast<int32_t>(dsize); i++)
    {
        DbVar* db = sys->getDbVarId(Type_Analog, i);
        if(db != NULL)
        {
            if(db->variation == Group30Var5)
            {
                config.analog[i].svariation = StaticAnalogVariation::Group30Var5;
            }
            else if(db->variation == Group30Var2)
            {
                config.analog[i].svariation = StaticAnalogVariation::Group30Var2;
            }
            if(db->evariation == Group32Var0)
            {
                //TODO config.analog[i].evariation = EventAnalogVariation::Group32Var0;
            }
            else if(db->evariation == Group32Var1)
            {
                config.analog[i].evariation = EventAnalogVariation::Group32Var1;
            }
            else if(db->evariation == Group32Var2)
            {
                config.analog[i].evariation = EventAnalogVariation::Group32Var2;
            }
            else if(db->evariation == Group32Var3)
            {
                config.analog[i].evariation = EventAnalogVariation::Group32Var3;
            }
            else if(db->evariation == Group32Var4)
            {
                config.analog[i].evariation = EventAnalogVariation::Group32Var4;
            }
            else if(db->evariation == Group32Var5)
            {
                config.analog[i].evariation = EventAnalogVariation::Group32Var5;
            }
            else if(db->evariation == Group32Var6)
            {
                config.analog[i].evariation = EventAnalogVariation::Group32Var6;
            }
            else if(db->evariation == Group32Var7)
            {
                config.analog[i].evariation = EventAnalogVariation::Group32Var7;
            }
            if (db->deadband > 0.0) {
                config.analog[i].deadband = db->deadband;
            }
            else
            {
                config.analog[i].deadband = sys->deadband;
            }
            
    
            if(db->clazz != 0)
            {
                switch (db->clazz ) 
                {
                    case 1:
                    {
                        config.analog[i].clazz = PointClass::Class1;
                        break;
                    }
                    case 2:
                    {
                        config.analog[i].clazz = PointClass::Class2;
                        break;
                    }
                    case 3:
                    {
                        config.analog[i].clazz = PointClass::Class3;
                        break;
                    }
                    default:
                        break;
                }
                if (sys->useVindex)
                    config.analog[i].vIndex = db->idx;
            }

            // else if(db->variation == Group30Var2)
            // {
            //     config.analog[i].svariation = StaticAnalogVariation::Group30Var2;
            // }
        }
    }
    auto bsize = sys->getTypeSize(Type_Binary);
    for (int i = 0; i < static_cast<int32_t>(bsize); i++)
    {
        DbVar* db = sys->getDbVarId(Type_Binary , i);
        if(db != NULL)
        {
            if(db->variation == Group1Var1)
            {
                config.binary[i].svariation = StaticBinaryVariation::Group1Var1;
            }
            if (sys->useVindex)
                config.binary[i].vIndex = db->idx;
        }
    }

    // example of configuring analog index 0 for Class2 with floating point variations by default
    //config.analog[0].clazz = PointClass::Class2;
    //config.analog[0].svariation = StaticAnalogVariation::Group30Var5;
    //config.analog[0].evariation = EventAnalogVariation::Group32Var7;
    //config.analog[0].deadband = 1.0; ///EventAnalogVariation::Group32Var7;   
}

DNP3Manager* setupDNP3Manager(sysCfg* sys)
{
    auto manager = new DNP3Manager(1, fpsLogger::Create(sys));
    return manager;
}

std::shared_ptr<IChannel> setupDNP3channel(DNP3Manager* manager, const char* cname, sysCfg* sys) {
    // Specify what log levels to use. NORMAL is warning and above
    // You can add all the comms logging by uncommenting below.
    const uint32_t FILTERS = levels::NORMAL | levels::ALL_COMMS;
    std::error_code ec;
    std::shared_ptr<IChannel> channel;
    const char* tls_folder_path = 
        "/usr/local/etc/config/dnp3_interface"; 
    if (sys->conn_type && strcmp(sys->conn_type, "TLS") == 0) // TLS client type
    {
        if (sys->tls_folder_path) // if they have a custom path to tls certificates then use that
        {
            tls_folder_path = sys->tls_folder_path;
        }
        if (!sys->caCertificate )
        {
            std::cout << "tls is missing file name caCertificate please provide it\n";
            return nullptr;
        }
        if (!sys->certificateChain )
        {
            std::cout << "tls is missing file name for certificateChain please provide it\n";
            return nullptr;
        }
        if (!sys->privateKey)
        {
            std::cout << "tls is missing file names for privateKey please provide it\n";
            return nullptr;
        }
        std::string caCertificate = std::string{tls_folder_path} + '/' + std::string{sys->caCertificate};
        std::string certificateChain = std::string{tls_folder_path} + '/' + std::string{sys->certificateChain};
        std::string privateKey = std::string{tls_folder_path} + '/' + std::string{sys->privateKey};

        // // check for files existences:
        // if (!std::filesystem::exists(caCertificate))
        // {
        //     std::cout << "caCertificate: " << caCertificate << " doesn't exist. Please provide one\n";
        //     return nullptr;
        // }
        // if (!std::filesystem::exists(certificateChain))
        // {
        //     std::cout << "certificateChain: " << certificateChain << " doesn't exist. Please provide one\n";
        //     return nullptr;
        // }
        // if (!std::filesystem::exists(privateKey))
        // {
        //     std::cout << "privateKey: " << privateKey << " doesn't exist. Please provide one\n";
        //     return nullptr;
        // }

        // Connect via a TCPClient socket to a outstation
        channel = manager->AddTLSServer(cname, 
                                        FILTERS, 
                                        ServerAcceptMode::CloseExisting, 
                                        sys->ip_address, sys->port, 
                                        TLSConfig(caCertificate, certificateChain, privateKey, 2),
                                        fpsChannelListener::Create(sys), 
                                        ec);

        if (ec)
        {
            std::cout << "Unable to create tls server: " << ec.message() << std::endl;
            return nullptr;
        }
    }
    else // tcp server type for default strcmp fail:
    {
        // default file path where we put our certificates
    // Create a TCP server (listener)
    //auto 
        channel = manager->AddTCPServer(cname, 
                                        FILTERS, 
                                        ServerAcceptMode::CloseExisting, 
                                        sys->ip_address, 
                                        sys->port,
                                        fpsChannelListener::Create(sys)
                                        );
    }
    return channel;
}

void setConfigUnsol(sysCfg* sys, OutstationStackConfig *config)
{
    if(sys->unsol == 0)
        config->outstation.params.allowUnsolicited = false;
    else
        config->outstation.params.allowUnsolicited = true;
}

std::shared_ptr<IOutstation> setupDNP3outstation (std::shared_ptr<IChannel> channel, const char* mname,
                                 sysCfg* sys, OutstationStackConfig *config)
{
    // The main object for a outstation. The defaults are useable,
    // but understanding the options are important.

    // Specify the maximum size of the event buffers. Defaults to 0
    config->outstation.eventBufferConfig = EventBufferConfig::AllTypes(10);
    //config.outstation.eventBufferConfig.maxBinaryEvents = fpsDB->dbVec[Type_Binary].size(),
    //config.outstation.eventBufferConfig.maxAnalogEvents = fpsDB->dbVec[Type_Analog].size(),
    // Specify the maximum size of the event buffers
    //config.outstation.eventBufferConfig = EventBufferConfig::AllTypes(10);

    // you can override an default outstation parameters here
    // in this example, we've enabled the oustation to use unsolicted reporting
    // if the master enables it
    //config.outstation.params.allowUnsolicited = true;
    if (sys->useVindex)
        config->outstation.params.indexMode = IndexMode::Discontiguous;

    // if(sys->unsol == 0)
    //     config.outstation.params.allowUnsolicited = false;
    // else
    //     config.outstation.params.allowUnsolicited = true;
    // allow sys to over rule
    setConfigUnsol(sys, config);

    //config.outstation.params.allowUnsolicited = false;

    // You can override the default link layer settings here
    // in this example we've changed the default link layer addressing
    // 
    config->link.LocalAddr = sys->station_address;   // was 10
    config->link.RemoteAddr = sys->master_address;//  was 1;

    config->link.KeepAliveTimeout = openpal::TimeDuration::Max();

    // You can optionally change the default reporting variations or class assignment prior to enabling the outstation
    ConfigureDatabase(config->dbConfig, sys);

    // Create a new outstation with a log level, command handler, and
    // config info this	returns a thread-safe interface used for
    // updating the outstation's database.
    // TODO fpsOutStationApplication
    char tmp [1024];
    snprintf(tmp,1024,"outstation_%s", sys->id);
    auto outstation = channel->AddOutstation(tmp 
                                            , fpsCommandHandler::Create(sys)
                                            , fpsOutstationApplication::Create(sys)
                                            , *config);

    // Enable the outstation and start communications
    outstation->Enable();
    sys->outstation = outstation;
    return outstation;
}
// scaled onto the wire
void addVarToBuilder (UpdateBuilder& builder, DbVar* db, int debug, bool useVindex)
{
    int idx = db->idx;
    if(useVindex)
        idx = db->vindex;

    switch (db->type) 
    {
        case Type_Analog:
        {
            double dval = db->valuedouble;
            if(debug>0)
            {
                FPS_ERROR_PRINT("*****  %s  var name [%s] db->idx [%d] db->vindex [%d] idx [%d] value [%f]\n"
                ,__FUNCTION__, db->name.c_str(), db->idx, db->vindex, idx, dval);
            }
            if((db->scale > 0.0) || (db->scale < 0.0))
            {
                dval *= db->scale;
                if(debug>0)
                {
                    FPS_ERROR_PRINT("*****  %s  var name [%s] idx [%d] vindex [%d] scaled value [%f]\n"
                    ,__FUNCTION__, db->name.c_str(), db->idx, db->vindex, dval);
                }
            }
            builder.Update(Analog(dval), idx);
            break;
        }
        case Type_AnalogOS:
        {
            builder.Update(AnalogOutputStatus(db->valuedouble), idx);
            break;
        }
        case Type_Binary:
        {
            int32_t vint = static_cast<int32_t>(db->valuedouble);
            if(db->scale < 0.0) 
            {
                if(vint > 0)
                    vint = 0;
                else
                    vint = 1;
            }

            builder.Update(Binary(vint), idx);
            break;
        }
        case Type_BinaryOS:
        {
            int32_t vint = static_cast<int32_t>(db->valuedouble);
            builder.Update(BinaryOutputStatus(vint), idx);
            break;
        }
        default:
            break;
    }
}

int main(int argc, char* argv[])
{
    //sysCfg sys_cfg;
    int rc = 0;
    int fims_connect = 0;
    int ttick = 0; // timeout tick

    if (strcmp(DNP3_UTILS_VERSION, getVersion())!=0)
    {
        FPS_ERROR_PRINT("Error with installed DNP3_UTILS_VERSION\n");
        return 1;
    }

    p_fims = new fims();

    if (p_fims == NULL)
    {
        FPS_ERROR_PRINT("Failed to allocate connection to FIMS server.\n");
        rc = 1;
        return 1;
        //goto cleanup;
    }
    
    bool running = true;
    num_configs = getConfigs(argc, argv, (sysCfg**)&sys_cfg, DNP3_OUTSTATION, p_fims);

    const char **subs = NULL;
    bool *bpubs = NULL;
    int num = getSysUris((sysCfg**)&sys_cfg, DNP3_OUTSTATION, subs, bpubs, num_configs);

    if(num < 0)
    {
        FPS_ERROR_PRINT("Failed to create subs array.\n");
        return 1;
    }

    FPS_ERROR_PRINT(">>>>>> Num Uris found %d .\n", num);
    for (int ix = 0; ix < num; ix++ )
    {
        FPS_ERROR_PRINT(">>>>>>>>>> Uri [%d] [%s] \n", ix, subs[ix]);    
    }

    // use the id for fims connect but also add outsttion designation 
    {
        char tmp[1024];
        snprintf(tmp, sizeof(tmp),"DNP3_O_%s", sys_cfg[0]->id);
        while(fims_connect < MAX_FIMS_CONNECT && p_fims->Connect(tmp) == false)
        {
            fims_connect++;
            sleep(1);
        }
    }

    if(fims_connect >= MAX_FIMS_CONNECT)
    {
        FPS_ERROR_PRINT("Failed to establish connection to FIMS server.\n");
        rc = 1;
        return 1;
        //goto cleanup;
    } 

    if(p_fims->Subscribe(subs, num, bpubs) == false)
    {
        FPS_ERROR_PRINT("Subscription failed.\n");
        p_fims->Close();
        return 1;
    }

    free((void *)bpubs);
    free((void *)subs);

    // Main point of interaction with the stack. 1 thread in the pool for 1 outstation
    // Manager must be in main scope

    auto manager = setupDNP3Manager(sys_cfg[0]);
    if (!manager)
    {
        FPS_ERROR_PRINT( "DNP3 Manger setup failed.\n");
        return 1;
    }

    auto channel = setupDNP3channel(manager, "server", sys_cfg[0]);

    if (!channel){
        FPS_ERROR_PRINT( "DNP3 Channel setup failed.\n");
        return 1;
    }
    // repeat for each config
    // put returned outstation into the config context
    for (int ixs = 0 ; ixs < num_configs; ixs++ )
    {
        sysCfg *sys = sys_cfg[ixs];
        if(sys->debug)
        {
            cout<<"*** Binaries: dbVec: "<<sys->dbVec[Type_Binary].size()
                << " indx :" << sys->getTypeSize(Type_Binary) << endl;

            cout<<"*** Analogs: dbVec: "<<sys->dbVec[Type_Analog].size()
                <<" indx :" << sys->getTypeSize(Type_Analog)+1 << endl;
        }
        sys->OSconfig = new OutstationStackConfig(DatabaseSizes( 
                                                    sys->getTypeSize(Type_Binary)+1,
                                                    0,                               // no double binaries
                                                    sys->getTypeSize(Type_Analog)+1,
                                                    0,                               // no counters
                                                    0,                               // no frozen counters
                                                    sys->getTypeSize(Type_BinaryOS),
                                                    sys->getTypeSize(Type_AnalogOS),
                                                    0,                               // no timers
                                                    0                                // no octet streams
                                                    ));

        auto outstation = setupDNP3outstation(channel, "outstation", sys, sys->OSconfig);
        if (!outstation){
            FPS_ERROR_PRINT( "Outstation setup failed id [%s] master %d station %d\n"
                            , sys->id, sys->master_address, sys->station_address);
            return 1;
        }
        FPS_ERROR_PRINT( "Outstation setup OK id [%s] master %d station %d\n"
                                    , sys->id, sys->master_address, sys->station_address);

        //sys->outstation = outstation;
        sys->getUris(DNP3_OUTSTATION);
    }

    FPS_ERROR_PRINT( "DNP3 Server Setup complete: Entering main loop.\n");

    while(running && p_fims->Connected())
    {
        // use a time out to detect init failure 
        fims_message* msg = p_fims->Receive_Timeout(1000000);
        if(msg == NULL)
        { 
            // TODO check for all the getURI resposes
            FPS_DEBUG_PRINT("Timeout tick %d\n", ttick);
            ttick++;
            for (int ixs = 0 ; ixs < num_configs; ixs++ )
            {
                sysCfg *sys = sys_cfg[ixs];

                bool ok = sys->checkUris(DNP3_OUTSTATION);
                if(ok == false)
                {
                    if (ttick > MAX_SETUP_TICKS)
                    {
                        // just quit here
                        FPS_DEBUG_PRINT("QUITTING TIME Timeout tick %d\n", ttick);
                    }
                }
            }
        }
        else
        {
            for (int ixs = 0 ; ixs < num_configs; ixs++ )
            {
                sysCfg *sys = sys_cfg[ixs];
                if (sys->debug)
                    FPS_ERROR_PRINT("****** Hey %s got a message uri [%s] \n", __FUNCTION__, msg->uri);
                dbs_type dbs; // collect all the parsed vars here

                cJSON* cjb = parseBody(dbs, sys, msg, DNP3_OUTSTATION);
                if(dbs.size() > 0)
                {
                    cJSON* cj = NULL;                
                    if((msg->replyto != NULL) && (strcmp(msg->method, "pub") != 0))
                        cj = cJSON_CreateObject();
                    
                    UpdateBuilder builder;
                    int varcount = 0;
                    while (!dbs.empty())
                    {
                        std::pair<DbVar*,int>dbp = dbs.back();
                        DbVar* db = dbp.first;
                        // only do this on sets pubs or  posts
                        if (
                            (strcmp(msg->method, "set") == 0) || 
                            (strcmp(msg->method, "post") == 0) || 
                            (strcmp(msg->method, "pub") == 0)
                            )
                            {
                                varcount++;
                                addVarToBuilder(builder, db, sys->debug, sys->useVindex);
                            }
                        addVarToCj(cj, dbp);  // include flag
                        addCjTimestamp(cj, "Timestamp");
                        dbs.pop_back();
                    }

                    //finalize set of updates
                    if(varcount > 0) 
                    {
                        auto updates = builder.Build();
                        sys->outstation->Apply(updates);
                    }
                    if(cj)
                    {
                        if(msg->replyto)
                        {
                            const char* reply = cJSON_PrintUnformatted(cj);
                            p_fims->Send("set", msg->replyto, NULL, reply);
                            free((void* )reply);
                        }
                        cJSON_Delete(cj);
                        cj = NULL;
                    }
                }
            
                if (sys->scanreq > 0)
                {
                    FPS_ERROR_PRINT("****** outstation scanreq %d ignored \n", sys->scanreq);
                    sys->scanreq = 0;
                }

                if (sys->unsolUpdate)
                {
                    FPS_ERROR_PRINT("****** outstation unsol %d handled \n", sys->unsol);
                    setConfigUnsol(sys, sys->OSconfig);
                    sys->unsolUpdate = false;
                }

                if (sys->cjclass != NULL)
                {
                    const char*tmp = cJSON_PrintUnformatted(sys->cjclass);
                    if(tmp != NULL)
                    {                
                        FPS_ERROR_PRINT("****** outstation class change [%s] handled \n", tmp);
                        free((void*)tmp);
                        sys->cjclass = NULL;
                    }
                }

                if (cjb != NULL)
                {
                    cJSON_Delete(cjb);
                    cjb = NULL;
                }
            }
            p_fims->free_message(msg);
        }
    }

    //TODO 
    //cleanup:

    if (manager) delete manager;
    if (p_fims) delete p_fims;
    for (int ixs = 0 ; ixs < num_configs; ixs++ )
    {
        sysCfg *sys = sys_cfg[ixs];
        if (sys->OSconfig) delete sys->OSconfig;
        //if (sys->outstation) delete sys->outstation;
        delete sys;
    }
    // if(sys_cfg[0]->ip_address    != NULL) free(sys_cfg[0]->ip_address);
    // if(sys_cfg[0]->name          != NULL) free(sys_cfg[0]->name);
    return rc;
}