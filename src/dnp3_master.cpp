
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
 * dnp3_master.cpp
 *
 *  Created on: Sep 28, 2020
 *      Author: jcalcagni - modbus
 *              pwilshire - dnp3 version
 */

#include <stdio.h>
#include <string>
#include <string.h>
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
#include <asiopal/UTCTimeSource.h> 
#include <opendnp3/LogLevels.h> 
#include <asiodnp3/ConsoleLogger.h> 

//#include <asiodnp3/DefaultMasterApplication.h> 
#include <asiodnp3/PrintingChannelListener.h> 
//#include <asiodnp3/PrintingCommandCallback.h> 
#include <asiopal/ChannelRetry.h>


#include "fpsSOEHandler.h"
#include "fpsMasterApplication.h"
#include "fpsLogger.h"
#include "fpsChannelListener.h"
#include "fpsCommandCallback.h"
#include "dnp3_utils.h"

using namespace std; 
using namespace openpal; 
using namespace asiopal; 
using namespace asiodnp3; 
using namespace opendnp3; 
#define MICROSECOND_TO_MILLISECOND 1000
#define NANOSECOND_TO_MILLISECOND  1000000
#define MAX_FIMS_CONNECT 5

volatile bool running = true;
int num_configs = 0;
#define MAX_CONFIGS 16
sysCfg *sys_cfg[MAX_CONFIGS];

void signal_handler (int sig)
{
    running = false;
    FPS_ERROR_PRINT("signal of type %d caught.\n", sig);
    signal(sig, SIG_DFL);
}

DNP3Manager* setupDNP3Manager(sysCfg* ourDB)
{
    auto manager = new DNP3Manager(1, fpsLogger::Create(ourDB));
    return manager;
}

// TODO(WALKER): Get TLS working here and make sure that keys are in place:
// during the config stage put all the info into sysCfg so we know if it will be TLS or not
// possibly along with the filepaths for our encryption

//auto channel = setupDNP3channel(manager, sys_cfg.id, &sys_cfg, sys_cfg.ip_address, sys_cfg.port);
// I think we can have several channels under one manager
std::shared_ptr<IChannel> setupDNP3channel(DNP3Manager* manager, sysCfg* sys)
{

     // Specify what log levels to use. NORMAL is warning and above
    // You can add all the comms logging by uncommenting below
    uint32_t FILTERS = levels::NORMAL;
    if (sys->debug > 2) 
        FILTERS |= levels::ALL_APP_COMMS;
    // This is the main point of interaction with the stack
    // Connect via a TCPClient socket to a outstation
    // repeat for each outstation
    std::error_code ec;

    std::shared_ptr<IChannel> channel;
    const char* tls_folder_path = 
        "/usr/local/etc/config/dnp3_interface"; // default file path where we put our certificates
    // TODO setup our own PrintingChannelListener
    if (sys->conn_type && strcmp(sys->conn_type, "TLS") == 0) // TLS client type
    {
        if (sys->tls_folder_path) // if they have a custom path to tls certificates then use that
        {
            tls_folder_path = sys->tls_folder_path;
        }
        if (!sys->peerCertificate)
        {
            std::cout << "tls is missing file name for  peerCertificate  please provide it\n";
            return nullptr;
        }
        if (!sys->privateKey)
        {
            std::cout << "tls is missing file name forr  privateKey please provide it\n";
            return nullptr;
        }
        std::string peerCertificate = std::string{tls_folder_path} + '/' + std::string{sys->peerCertificate};
        std::string privateKey = std::string{tls_folder_path} + '/' + std::string{sys->privateKey};

        // check for files existences:
        // if (!std::filesystem::exists(peerCertificate))
        // {
        //     std::cout << "peerCertificate: " << peerCertificate << " doesn't exist. Please provide one\n";
        //     return nullptr;
        // }
        // if (!std::filesystem::exists(privateKey))
        // {
        //     std::cout << "privateKey: " << privateKey << " doesn't exist. Please provide one\n";
        //     return nullptr;
        // }

        // TLSConfig cfg1(cert2, cert1, key1);
        // TLSConfig cfg2(cert1, cert2, key2);
        
        // Connect via a TCPClient socket to a outstation
        channel = manager->AddTLSClient(sys->id, 
                                        FILTERS, 
                                        ChannelRetry::Default(), 
                                        {IPEndpoint(sys->ip_address, sys->port)}, 
                                        "0.0.0.0",
                                        TLSConfig(peerCertificate, privateKey, privateKey),
                                        // fpsChannelListener::Create(sys), 
                                        PrintingChannelListener::Create(),
                                        ec);

        if (ec)
        {
            std::cout << "Unable to create tls client: " << ec.message() << std::endl;
            return nullptr;
        }
    }
    else if (sys->conn_type && strcmp(sys->conn_type, "RTU") == 0) // serial/RTU channel
    {
        SerialSettings rtu_settings;
        if (sys->baud != 0)
        {
            rtu_settings.baud = sys->baud; // do error checking here (how?)
        }
        if (sys->dataBits != 0)
        {
            rtu_settings.dataBits = sys->dataBits; // do error checking here (how?)
        }
        if (sys->stopBits >= 0) // double/float
        {
            if (sys->stopBits == 0)
            {
                rtu_settings.stopBits = opendnp3::StopBits::None;
            }
            else if (sys->stopBits == 1)
            {
                rtu_settings.stopBits = opendnp3::StopBits::One;
            }
            else if (sys->stopBits == 1.5)
            {
                rtu_settings.stopBits = opendnp3::StopBits::OnePointFive;
            }
            else if (sys->stopBits == 2)
            {
                rtu_settings.stopBits = opendnp3::StopBits::Two;
            }
            else
            {
                std::cout << "a stopBits number of: " << sys->stopBits << " is not accepted for serial RTU communications\n";
                return nullptr;
            }
        }
        if (sys->parity)
        {
            if (strcmp(sys->parity, "none") == 0)
            {
                rtu_settings.parity = opendnp3::Parity::None;
            }
            else if (strcmp(sys->parity, "even") == 0)
            {
                rtu_settings.parity = opendnp3::Parity::Even;
            }
            else if (strcmp(sys->parity, "odd") == 0)
            {
                rtu_settings.parity = opendnp3::Parity::Odd;
            }
            else
            {
                std::cout << "parity: " << sys->parity << " is not accepted for serial RTU communications\n";
                return nullptr;
            }
        }
        if (sys->flowType)
        {
            if (strcmp(sys->flowType, "none") == 0)
            {
                rtu_settings.flowType = opendnp3::FlowControl::None;
            }
            else if (strcmp(sys->flowType, "hardware") == 0)
            {
                rtu_settings.flowType = opendnp3::FlowControl::Hardware;
            }
            else if (strcmp(sys->flowType, "xonxoff") == 0)
            {
                rtu_settings.flowType = opendnp3::FlowControl::XONXOFF;
            }
            else
            {
                std::cout << "flowtype: " << sys->flowType << " is not accepted for serial RTU communications\n";
                return nullptr;
            }
        }
        if (sys->asyncOpenDelay != 0) // I don't know what this is supposed to be
        {
            if (sys->asyncOpenDelay < 0)
            {
                std::cout << "asyncOpenDelay: " << sys->asyncOpenDelay << " is negative, must be a postitive value (this is in milliseconds) for serial RTU communications\n";
                return nullptr;
            }
            rtu_settings.asyncOpenDelay.milliseconds = sys->asyncOpenDelay;
        }
        if (sys->deviceName)
        {
            rtu_settings.deviceName = sys->deviceName; 
        }
        // default values if they do not specify in the config (deviceName is "" by default I believe):
        //   baud(9600),
        //   dataBits(8),
        //   stopBits(opendnp3::StopBits::One),
        //   parity(opendnp3::Parity::None),
        //   flowType(opendnp3::FlowControl::None),
        //   asyncOpenDelay(openpal::TimeDuration::Milliseconds(500))
        channel = manager->AddSerial(sys->id
                                        , FILTERS
                                        , ChannelRetry::Default()
                                        , rtu_settings
                                        , PrintingChannelListener::Create());
    }
    else // tcp client type for default strcmp fail (or no conn_type):
    {
    //std::shared_ptr<IChannel> 
        channel = manager->AddTCPClient(sys->id 
                                        , FILTERS 
                                        , ChannelRetry::Default() 
                                        , {IPEndpoint(sys->ip_address, sys->port)}
                                        ,"0.0.0.0" 
                                        , fpsChannelListener::Create(sys));
    // The master config object for a master. The default are
    // useable, but understanding the options are important.
    }
    return channel;
}

//  this my need to be out of the scope of the function.
// TODO add this to sys.
//MasterStackConfig stackConfig;

std::shared_ptr<IMaster> setupDNP3master (std::shared_ptr<IChannel> channel, const char* mname, sysCfg* sys)
{
    //sys->stackConfig = &stackConfig;
    sys->stackConfig = new MasterStackConfig();
    // you can override application layer settings for the master here
    // in this example, we've change the application layer timeout to 2 seconds
    sys->stackConfig->master.responseTimeout = TimeDuration::Seconds(2);
    if(sys->unsol)
        sys->stackConfig->master.disableUnsolOnStartup = false;
    else
        sys->stackConfig->master.disableUnsolOnStartup = true;
    // You can override the default link layer settings here
    // in this example we've changed the default link layer addressing
    sys->stackConfig->link.LocalAddr = sys->master_address;//localAddr; // 1
    sys->stackConfig->link.RemoteAddr = sys->station_address;//remoteAddr; //10;
    // Create a new master on a previously declared port, with a
    // name, log level, command acceptor, and config info. This
    // returns a thread-safe interface used for sending commands.
    char tmp [1024];
    snprintf(tmp,1024,"master_%s", sys->id);
    auto master = channel->AddMaster(/*"master"*/ tmp, // id for logging
                                     fpsSOEHandler::Create(sys), // callback for data processing  this generates the pub elements when we get data
                                     fpsMasterApplication::Create(sys), // master application instance this manages the collection of all the pub elements 
                                     *sys->stackConfig // stack configuration
                                    );

    if(master == NULL)
    {
        FPS_ERROR_PRINT(" Error creating master [%s] master %d station %d\n"
            ,tmp, sys->master_address, sys->station_address);
        return NULL;
    }
    FPS_ERROR_PRINT(" OK created master [%s] master %d station %d \n"
            , tmp, sys->master_address, sys->station_address);

    sys->master = master;

    // do an integrity poll (Class 3/2/1/0) once per minute
    //auto integrityScan = master->AddClassScan(ClassField::AllClasses(), TimeDuration::Minutes(1));
    if (sys->frequency > 0)
    //auto integrityScan = 
        sys->master->AddClassScan(ClassField::AllClasses(), TimeDuration::Milliseconds(sys->frequency));
    //void Scan(const HeaderBuilderT& builder, TaskConfig config = TaskConfig::Default());

    //void ScanClasses(const opendnp3::ClassField& field, const opendnp3::TaskConfig& config) override;
    // do a exception poll scans as requested
    if (sys->freq1 > 0)
    //auto exceptionScan = 
        sys->master->AddClassScan(ClassField(ClassField::CLASS_1), TimeDuration::Milliseconds(sys->freq1));
    if (sys->freq2 > 0)
    //auto exceptionScan = 
        sys->master->AddClassScan(ClassField(ClassField::CLASS_2), TimeDuration::Milliseconds(sys->freq2));
    if (sys->freq3 > 0)
    //auto exceptionScan = 
        sys->master->AddClassScan(ClassField(ClassField::CLASS_3), TimeDuration::Milliseconds(sys->freq3));
    // virtual std::shared_ptr<IMasterScan> AddRangeScan(opendnp3::GroupVariationID gvId,
    //                                                   uint16_t start,
    //                                                   uint16_t stop,
    //                                                   openpal::TimeDuration period,
    //                                                   const opendnp3::TaskConfig& config
    //                                                   = opendnp3::TaskConfig::Default())
    //     = 0; (edited) 
    if(sys->rangeFreq > 0)
    {
        //auto rangeScan = 
        if(sys->rangeAStop > 0)
        {
            sys->master->AddRangeScan(GroupVariationID(30,0),
                                    sys->rangeAStart,
                                    sys->rangeAStop,
                                    TimeDuration::Milliseconds(sys->rangeFreq));
        }
        //TODO binaries
        if(sys->rangeBStop > 0)
        {
            sys->master->AddRangeScan(GroupVariationID(1,0),
                                    sys->rangeBStart,
                                    sys->rangeBStop,
                                    TimeDuration::Milliseconds(sys->rangeFreq));
        }
    }
    //auto exceptionScan = master->Scan(ClassField(ClassField::CLASS_1);
    
    //auto binscan = master->AddAllObjectsScan(GroupVariationID(1,1),
    //                                                               TimeDuration::Seconds(5));
    //auto objscan = master->AddAllObjectsScan(GroupVariationID(30,1),
    //                                                               TimeDuration::Seconds(5));
    if(0)
        auto objscan2 = sys->master->AddAllObjectsScan(GroupVariationID(32,7),
                                                                   TimeDuration::Seconds(10));
    // Enable the master. This will start communications.
    sys->master->Enable();
    bool channelCommsLoggingEnabled = true;
    bool masterCommsLoggingEnabled = true;
    //bool channelCommsLoggingEnabled = false;
    //bool masterCommsLoggingEnabled = false;

    auto levels = channelCommsLoggingEnabled ? levels::ALL_COMMS : levels::NORMAL;
    channel->SetLogFilters(levels);
    levels = masterCommsLoggingEnabled ? levels::ALL_COMMS : levels::NORMAL;
    sys->master->SetLogFilters(levels);
    // other master controls are :
    //
    return master;
}

// this runs whn we get a "set" on a defined value
// the data is immediately send to the outstation.
// these are all scaled and signed if neded onto the wire

void addVarToCommands (CommandSet & commands, std::pair<DbVar*,int>dbp)
{
    DbVar* db = dbp.first;
    //int flag = dbp.second;

    switch (db->type) 
    {
        case Type_Crob:
        {
            commands.Add<ControlRelayOutputBlock>({WithIndex(ControlRelayOutputBlock(ControlCodeFromType(db->crob)), db->idx)});
            break;
        }
        case AnIn16:
        {
            int16_t anInt16  = getInt16Val(db);
            commands.Add<AnalogOutputInt16>({WithIndex(AnalogOutputInt16(anInt16), db->idx)});
            break;
        }
        case AnIn32:
        {
            int32_t anInt32  = getInt32Val(db);
            commands.Add<AnalogOutputInt32>({WithIndex(AnalogOutputInt32(anInt32), db->idx)});
            //commands.Add<AnalogOutputInt32>({WithIndex(AnalogOutputInt32(ival, db->idx)});
            break;
        }
        case AnF32:
        {
            double anF32  = getF32Val(db);
            commands.Add<AnalogOutputFloat32>({WithIndex(AnalogOutputFloat32(anF32), db->idx)});
            break;
        }
        default:
            break;
    }
}

int main(int argc, char *argv[])
{
    if (strcmp(DNP3_UTILS_VERSION, getVersion())!=0)
    {
        FPS_ERROR_PRINT("Error with installed DNP3_UTILS_VERSION\n");
        return 1;
    }

    fims* p_fims;
    p_fims = new fims();
    
    bool running = true;
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    int fims_connect = 0;
    num_configs = getConfigs(argc, argv, (sysCfg**)&sys_cfg, DNP3_MASTER, p_fims);

    
    const char **subs = NULL;
    bool *bpubs = NULL;
    
     // TODO make this ppen to subs. Perhaps we need a vector
    // components is pulled by uri  repeat for exch config
    int num = getSysUris((sysCfg **)&sys_cfg, DNP3_MASTER, subs, bpubs, num_configs);
    if(num < 0)
    {
        FPS_ERROR_PRINT("Failed to create subs array.\n");
        return 1;
    }

    if (p_fims == NULL)
    {
        FPS_ERROR_PRINT("Failed to allocate connection to FIMS server.\n");
        //rc = 1;
        return 1;//goto cleanup;
    }
    // use the id for fims connect bt also add master designation 
    {
        char tmp[1024];
        snprintf(tmp, sizeof(tmp),"DNP3_M_%s", sys_cfg[0]->id);
        while(fims_connect < MAX_FIMS_CONNECT && p_fims->Connect(tmp) == false)
        {
            fims_connect++;
            sleep(1);
        }
    }

    if(fims_connect >= MAX_FIMS_CONNECT)
    {
        FPS_ERROR_PRINT("Failed to establish connection to FIMS server.\n");
        return 1;
    }

    FPS_DEBUG_PRINT("Map configured: Initializing data.\n");
    //if(p_fims->Subscribe((const char**)sub_array, 3, (bool *)publish_only) == false)
    if(p_fims->Subscribe((const char**)subs, num, (bool *)bpubs) == false)
    {
        FPS_ERROR_PRINT("Subscription failed.\n");
        p_fims->Close();
        return 1;
    }
    FPS_ERROR_PRINT("Number of subscriptions %d.\n", num);
    for (int is = 0 ; is < num ; is++)
    {
        FPS_ERROR_PRINT(" subscriptions %d is [%s]\n", is, subs[is]);
    }

    free((void *)bpubs);
    free((void *)subs);
// first sysconfig sets up Manager
    auto manager = setupDNP3Manager(sys_cfg[0]);
    if (!manager){
        FPS_ERROR_PRINT("Error in setupDNP3Manager.\n");
        return 1;
    }
    // now we use data from the config file
    //std::shared_ptr<IChannel> 
    //auto channel = setupDNP3channel(manager, "tcpclient1", "127.0.0.1", 20001);
    auto channel = setupDNP3channel(manager, sys_cfg[0]); //, sys_cfg.ip_address, sys_cfg.port);
    if (!channel){
        FPS_ERROR_PRINT("Error in setupDNP3channel.\n");
        delete manager;
        return 1;
    }

    // repeat for each config
    // put returned master into the config context
        //std::shared_ptr<IChannel> 
    //int cfg_num = 0;
    for (int ixs = 0 ; ixs < num_configs; ixs++ )
    {
        sysCfg *sys = sys_cfg[ixs];

        auto master = setupDNP3master (channel, DNP3_MASTER, sys);

        if (!master){
            FPS_ERROR_PRINT("Error in setupDNP3master.\n");
            delete manager;
            //delete channel;
            return 1;
        }
        sys->master = master;

    }
    //we cant do this
    //auto master2 = setupDNP3master (channel2, "master2", ourDB , 2 /*localAddr*/ , 10 /*RemoteAddr*/);

    FPS_DEBUG_PRINT("DNP3 Setup complete: Entering main loop.\n");
    
    // send out intial gets
    // set max ticks
    
    // no need to do this for the master
    //sys_cfg.getUris(DNP3_MASTER);

    // set all values to inval  done at the start
    // start time to complete gets
    // TODO set for all the getURI responses as todo
    // done only get outstation vars 

    while(running && p_fims->Connected())
    {
        // once a second
        fims_message* msg = p_fims->Receive_Timeout(1000000);
        //sysCfg *sys = sys_cfg[0];
        if(msg != NULL)
        {
            for (int ixs = 0 ; ixs < num_configs; ixs++ )
            {
                sysCfg *sys = sys_cfg[ixs];
            
                dbs_type dbs; // collect all the parsed vars here
                // We can use a single dbs 
                cJSON* cjb = parseBody(dbs, sys, msg, DNP3_MASTER);

                if(dbs.size() > 0)
                {
                    CommandSet commands;
                    int numCmds = 0;
                    cJSON*cj = NULL;                
                    if((msg->replyto != NULL) && (strcmp(msg->method,"pub") != 0))
                        cj = cJSON_CreateObject();

                    while (!dbs.empty())
                    {
                        std::pair<DbVar*,int>dbp = dbs.back();
                        // only do this on sets or posts
                        if ((strcmp(msg->method,"set") == 0) || (strcmp(msg->method,"post") == 0))
                        {
                            addVarToCommands(commands, dbp);
                            numCmds++;
                        }
                        if(cj) addVarToCj(cj, dbp);
                        dbs.pop_back();
                    }

                    if(cj)
                    {
                        const char* reply = cJSON_PrintUnformatted(cj);
                        cJSON_Delete(cj);
                        cj = NULL;
                        // TODO check that SET is the correct thing to do in MODBUS_CLIENT
                        // probably OK since this is a reply
                        if(msg->replyto)
                            p_fims->Send("set", msg->replyto, NULL, reply);

                        free((void* )reply);
                    }
                    if(numCmds > 0)
                    {
                        FPS_DEBUG_PRINT("      *****Running Direct Outputs \n");
                        sys->master->DirectOperate(std::move(commands), fpsCommandCallback::Get());
                    }
                }
            
                // TODO master has to be assigned per config
                if (sys->scanreq > 0)
                {
                    switch (sys->scanreq)
                    {
                        case 1:
                        {
                            sys->master->ScanClasses(ClassField(ClassField::CLASS_1));
                            break;
                        }
                        case 2:
                        {
                            sys->master->ScanClasses(ClassField(ClassField::CLASS_2));
                            break;
                        }
                        case 3:
                        {
                            sys->master->ScanClasses(ClassField(ClassField::CLASS_3));
                            break;
                        }
                        default:
                            break;
                    }
                    FPS_ERROR_PRINT("****** master scanreq %d serviced\n", sys->scanreq);
                    sys->scanreq = 0;
                }

                if (cjb != NULL)
                {
                    cJSON_Delete(cjb);
                }
            }
            p_fims->free_message(msg);
        }
    }

    //cleanup:
    if (manager){
        delete manager;
        //delete channel;
    }
    if (p_fims) delete p_fims;
    // sys_cfg should clean itself up
 
    //if(sys_cfg.eth_dev       != NULL) free(sys_cfg.eth_dev);
    //if(sys_cfg.ip_address    != NULL) free(sys_cfg.ip_address);
    //if(sys_cfg.name          != NULL) free(sys_cfg.name);
    //if(sys_cfg.serial_device != NULL) free(sys_cfg.serial_device);
    //if(sys_cfg.mb != NULL)             modbus_free(sys_cfg.mb);
    // for(int fd = 0; fd < fd_max; fd++)
    //     if(FD_ISSET(fd, &all_connections))
    //         close(fd);
    for (int ix = 0; ix < num_configs; ix++)
    {
        FPS_ERROR_PRINT("****** client cleanup sys %d start\n", ix);

        sysCfg *sys = sys_cfg[ix];
        //if (sys->master)  delete sys->master;
        if (sys->stackConfig)  delete sys->stackConfig;
        delete sys;
        FPS_ERROR_PRINT("****** client cleanup sys %d done\n", ix);

    }

    return 0;
}


