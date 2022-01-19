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
#ifndef ASIODNP3_fpsSOEHandler_H 
#define ASIODNP3_fpsSOEHandler_H 
#include <cjson/cJSON.h>
#include <fims/libfims.h>
#include <opendnp3/master/ISOEHandler.h> 
#include <iostream> 
#include <memory> 
#include <sstream>
#include "dnp3_utils.h"

namespace asiodnp3 
{ 

/**
 *	ISOEHandler singleton that prints to the console.
 */ 
class fpsSOEHandler final : public opendnp3::ISOEHandler 
{ 
public:
    fpsSOEHandler(sysCfg* fpsDB) { sysdb = fpsDB;};
    static std::shared_ptr<ISOEHandler> Create(sysCfg* fpsDB)
    {
        return std::make_shared<fpsSOEHandler>(fpsDB);
    }
    void setDB(sysCfg* fpsDB){sysdb = fpsDB;};

    virtual void Process(const opendnp3::HeaderInfo& info,
                         const opendnp3::ICollection<opendnp3::Indexed<opendnp3::Binary>>& values) override;
    virtual void Process(const opendnp3::HeaderInfo& info,
                         const opendnp3::ICollection<opendnp3::Indexed<opendnp3::DoubleBitBinary>>& values) override;
    virtual void Process(const opendnp3::HeaderInfo& info,
                         const opendnp3::ICollection<opendnp3::Indexed<opendnp3::Analog>>& values) override;
    virtual void Process(const opendnp3::HeaderInfo& info,
                         const opendnp3::ICollection<opendnp3::Indexed<opendnp3::Counter>>& values) override;
    virtual void Process(const opendnp3::HeaderInfo& info,
                         const opendnp3::ICollection<opendnp3::Indexed<opendnp3::FrozenCounter>>& values) override;
    virtual void Process(const opendnp3::HeaderInfo& info,
                         const opendnp3::ICollection<opendnp3::Indexed<opendnp3::BinaryOutputStatus>>& values) override;
    virtual void Process(const opendnp3::HeaderInfo& info,
                         const opendnp3::ICollection<opendnp3::Indexed<opendnp3::AnalogOutputStatus>>& values) override;
    virtual void Process(const opendnp3::HeaderInfo& info,
                         const opendnp3::ICollection<opendnp3::Indexed<opendnp3::OctetString>>& values) override;
    virtual void Process(const opendnp3::HeaderInfo& info,
                         const opendnp3::ICollection<opendnp3::Indexed<opendnp3::TimeAndInterval>>& values) override;
    virtual void Process(const opendnp3::HeaderInfo& info,
                         const opendnp3::ICollection<opendnp3::Indexed<opendnp3::BinaryCommandEvent>>& values) override;
    virtual void Process(const opendnp3::HeaderInfo& info,
                         const opendnp3::ICollection<opendnp3::Indexed<opendnp3::AnalogCommandEvent>>& values) override;
    virtual void Process(const opendnp3::HeaderInfo& info,
                         const opendnp3::ICollection<opendnp3::Indexed<opendnp3::SecurityStat>>& values) override;
    virtual void Process(const opendnp3::HeaderInfo& info,
                         const opendnp3::ICollection<opendnp3::DNPTime>& values) override; protected:
    void Start() final {}
    void End() final {} private:
    template<class T>
    static void PrintAll(const opendnp3::HeaderInfo& info, const opendnp3::ICollection<opendnp3::Indexed<T>>& values)
    {
        auto print = [&](const opendnp3::Indexed<T>& pair) { Print<T>(info, pair.value, pair.index); };
        values.ForeachItem(print);
    }
    template<class T> static void Print(const opendnp3::HeaderInfo& info, const T& value, uint16_t index)
    {
        std::cout << "[" << index << "] : " << ValueToString(value) << " : " << static_cast<int>(value.flags.value)
                  << " : " << value.time << std::endl;
    }
    template<class T>
    static void myPrintAll(const opendnp3::HeaderInfo& info, const opendnp3::ICollection<opendnp3::Indexed<T>>& values)
    {
        auto print = [&](const opendnp3::Indexed<T>& pair) { myPrint<T>(info, pair.value, pair.index); };
        values.ForeachItem(print);
    }
    template<class T> static void myPrint(const opendnp3::HeaderInfo& info, const T& value, uint16_t index)
    {
        std::cout << "mystuff [" << index << "] : " << ValueToString(value) << " : " << static_cast<int>(value.flags.value)
                  << " : " << value.time << std::endl;
    }
    template<class T> static std::string ValueToString(const T& meas)
    {
        std::ostringstream oss;
        oss << meas.value;
        return oss.str();
    }
    static std::string GetTimeString(opendnp3::TimestampMode tsmode)
    {
        std::ostringstream oss;
        switch (tsmode)
        {
        case (opendnp3::TimestampMode::SYNCHRONIZED):
            return "synchronized";
            break;
        case (opendnp3::TimestampMode::UNSYNCHRONIZED):
            oss << "unsynchronized";
            break;
        default:
            oss << "no timestamp";
            break;
        }
        return oss.str();
    }
    static std::string ValueToString(const opendnp3::DoubleBitBinary& meas)
    {
        return opendnp3::DoubleBitToString(meas.value);
    }
    sysCfg* sysdb;
};
} // namespace asiodnp3
#endif
