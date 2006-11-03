/*
 *
 * Copyright (c) 2006 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifndef _LFAcceptor_
#define _LFAcceptor_

#include "apr-1/apr_network_io.h"
#include "apr-1/apr_poll.h"
#include "apr-1/apr_time.h"

#include "qpid/io/Acceptor.h"
#include "qpid/concurrent/Monitor.h"
#include "qpid/io/LFProcessor.h"
#include "qpid/io/LFSessionContext.h"
#include "qpid/concurrent/Runnable.h"
#include "qpid/io/SessionContext.h"
#include "qpid/io/SessionHandlerFactory.h"
#include "qpid/concurrent/Thread.h"
#include <qpid/SharedObject.h>

namespace qpid {
namespace io {

/** APR Acceptor. */
class Acceptor : public qpid::SharedObject<Acceptor>
{
  public:
    Acceptor(int16_t port, int backlog, int threads);
    virtual int16_t getPort() const;
    virtual void run(SessionHandlerFactory* factory);
    virtual void shutdown();

  private:
    int16_t port;
    LFProcessor processor;
    apr_socket_t* socket;
    volatile bool running;
};

}
}


#endif
