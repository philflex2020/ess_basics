/*
 * Copyright 2013-2019 Automatak, LLC
 *
 * Licensed to Green Energy Corp (www.greenenergycorp.com) and Automatak
 * LLC (www.automatak.com) under one or more contributor license agreements.
 * See the NOTICE file distributed with this work for additional information
 * regarding copyright ownership. Green Energy Corp and Automatak LLC license
 * this file to you under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "fpsLogger.h"

#include <opendnp3/LogLevels.h>

#include <cassert>
#include <chrono>
#include <iostream>
#include <sstream>
#include <stdio.h>

#include "dnp3_utils.h"

using namespace std;
using namespace opendnp3;
using namespace std::chrono;

namespace asiodnp3
{

void fpsLogger::Log(const openpal::LogEntry& entry)
{
    auto time = std::chrono::high_resolution_clock::now();
    auto num = duration_cast<milliseconds>(time.time_since_epoch()).count();

    ostringstream oss;
    string mstring = LogFlagToString(entry.filters.GetBitfield());
    char message [2048];
 
    oss << "fps Logger ms(" << num << ") " << LogFlagToString(entry.filters.GetBitfield());
    oss << " " << entry.loggerid;
    if (printLocation)
    {
        oss << " - " << entry.location;
    }
    oss << " - " << entry.message;
    snprintf(message, sizeof(message), "DNP3  %s message [%s] --[%s]\n"
                    ,sysdb->id 
                    ,mstring.c_str()
                    ,oss.str().c_str());
    //fprintf(stderr, "%s\n", message);

    emit_event(sysdb, "DNP3", message, 1);

    // unique_lock creates a temp lock that goes away >>> 
    std::unique_lock<std::mutex> lock(mutex);
    //FPS_DEBUG_PRINT("%s\n", oss.str().c_str());
    if(1)std::cout << oss.str() << std::endl;
    // <<< here
}

} // namespace asiodnp3