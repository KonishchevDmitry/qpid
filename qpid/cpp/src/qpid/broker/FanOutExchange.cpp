/*
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 */
#include "qpid/broker/FanOutExchange.h"
#include "qpid/broker/FedOps.h"
#include <algorithm>

using namespace qpid::broker;
using namespace qpid::framing;
using namespace qpid::sys;
namespace _qmf = qmf::org::apache::qpid::broker;

FanOutExchange::FanOutExchange(const std::string& _name, Manageable* _parent, Broker* b) :
    Exchange(_name, _parent, b)
{
    if (mgmtExchange != 0)
        mgmtExchange->set_type (typeName);
}

FanOutExchange::FanOutExchange(const std::string& _name, bool _durable,
                               const FieldTable& _args, Manageable* _parent, Broker* b) :
    Exchange(_name, _durable, _args, _parent, b)
{
    if (mgmtExchange != 0)
        mgmtExchange->set_type (typeName);
}

bool FanOutExchange::bind(Queue::shared_ptr queue, const string& /*key*/, const FieldTable* args)
{
    string fedOp(args ? args->getAsString(qpidFedOp) : fedOpBind);
    string fedTags(args ? args->getAsString(qpidFedTags) : "");
    string fedOrigin(args ? args->getAsString(qpidFedOrigin) : "");
    bool propagate = false;

    if (args == 0 || fedOp.empty() || fedOp == fedOpBind) {
        Binding::shared_ptr binding (new Binding ("", queue, this, FieldTable(), fedOrigin));
        if (bindings.add_unless(binding, MatchQueue(queue))) {
            binding->startManagement();
            propagate = fedBinding.addOrigin(fedOrigin);
            if (mgmtExchange != 0) {
                mgmtExchange->inc_bindingCount();
            }
        } else {
            // queue already present - still need to track fedOrigin
            fedBinding.addOrigin(fedOrigin);
            return false;
        }
    } else if (fedOp == fedOpUnbind) {
        propagate = fedBinding.delOrigin(fedOrigin);
        if (fedBinding.count() == 0)
            unbind(queue, "", 0);
    } else if (fedOp == fedOpReorigin) {
        if (fedBinding.hasLocal()) {
            propagateFedOp(string(), string(), fedOpBind, string());
        }
    }

    routeIVE();
    if (propagate)
        propagateFedOp(string(), fedTags, fedOp, fedOrigin);
    return true;
}

bool FanOutExchange::unbind(Queue::shared_ptr queue, const string& /*key*/, const FieldTable* /*args*/)
{
    bool propagate = false;

    if (bindings.remove_if(MatchQueue(queue))) {
        propagate = fedBinding.delOrigin();
        if (mgmtExchange != 0) {
            mgmtExchange->dec_bindingCount();
        }
    } else {
        return false;
    }

    if (propagate)
        propagateFedOp(string(), string(), fedOpUnbind, string());
    return true;
}

void FanOutExchange::route(Deliverable& msg, const string& /*routingKey*/, const FieldTable* /*args*/)
{
    PreRoute pr(msg, this);
    doRoute(msg, bindings.snapshot());
}
    
bool FanOutExchange::isBound(Queue::shared_ptr queue, const string* const, const FieldTable* const)
{
    BindingsArray::ConstPtr ptr = bindings.snapshot();
    return ptr && std::find_if(ptr->begin(), ptr->end(), MatchQueue(queue)) != ptr->end();
}


FanOutExchange::~FanOutExchange() {}

const std::string FanOutExchange::typeName("fanout");
