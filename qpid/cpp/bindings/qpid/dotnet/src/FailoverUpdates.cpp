/*
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
*/

#include <windows.h>
#include <msclr\lock.h>
#include <oletx2xa.h>
#include <string>
#include <limits>

#include "qpid/messaging/FailoverUpdates.h"

#include "Connection.h"
#include "FailoverUpdates.h"
#include "QpidException.h"

namespace Org {
namespace Apache {
namespace Qpid {
namespace Messaging {

    /// <summary>
    /// FailoverUpdates is a managed wrapper for a qpid::messaging::FailoverUpdates
    /// </summary>

    // constructors

    FailoverUpdates::FailoverUpdates(Connection ^ connection)
    {
        System::Exception ^ newException = nullptr;

        try 
		{
            failoverupdatesp = new ::qpid::messaging::FailoverUpdates(*(connection->NativeConnection));
        } 
        catch (const ::qpid::types::Exception & error) 
		{
            String ^ errmsg = gcnew String(error.what());
            newException    = gcnew QpidException(errmsg);
        }

		if (newException != nullptr) 
		{
	        throw newException;
		}
    }


    // Destructor
    FailoverUpdates::~FailoverUpdates()
    {
        Cleanup();
    }


    // Finalizer
    FailoverUpdates::!FailoverUpdates()
    {
        Cleanup();
    }


    // Destroys kept object
    // TODO: add lock
    void FailoverUpdates::Cleanup()
    {
        if (NULL != failoverupdatesp)
        {
            delete failoverupdatesp;
            failoverupdatesp = NULL;
        }
    }
}}}}
