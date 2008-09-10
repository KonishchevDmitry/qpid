#ifndef _broker_Queue_h
#define _broker_Queue_h

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
#include "OwnershipToken.h"
#include "Consumer.h"
#include "Message.h"
#include "PersistableQueue.h"
#include "QueuePolicy.h"
#include "QueueBindings.h"

#include "qpid/framing/FieldTable.h"
#include "qpid/sys/Monitor.h"
#include "qpid/management/Manageable.h"
#include "qpid/management/Queue.h"
#include "qpid/framing/amqp_types.h"

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/intrusive_ptr.hpp>

#include <list>
#include <vector>
#include <memory>
#include <deque>
#include <algorithm>

namespace qpid {
    namespace broker {
        class Broker;
        class MessageStore;
        class QueueRegistry;
        class TransactionContext;
        class Exchange;

        using std::string;

        /**
         * The brokers representation of an amqp queue. Messages are
         * delivered to a queue from where they can be dispatched to
         * registered consumers or be stored until dequeued or until one
         * or more consumers registers.
         */
        class Queue : public boost::enable_shared_from_this<Queue>,
            public PersistableQueue, public management::Manageable {

            typedef std::list<Consumer::shared_ptr> Listeners;
            typedef std::deque<QueuedMessage> Messages;

            const string name;
            const bool autodelete;
            MessageStore* store;
            const OwnershipToken* owner;
            uint32_t consumerCount;
            OwnershipToken* exclusive;
            bool noLocal;
            std::string traceId;
            std::vector<std::string> traceExclude;
            Listeners listeners;
            Messages messages;
            mutable qpid::sys::Mutex consumerLock;
            mutable qpid::sys::Mutex messageLock;
            mutable qpid::sys::Mutex ownershipLock;
            mutable uint64_t persistenceId;
            framing::FieldTable settings;
            std::auto_ptr<QueuePolicy> policy;
            bool policyExceeded;
            QueueBindings bindings;
            boost::shared_ptr<Exchange> alternateExchange;
            framing::SequenceNumber sequence;
            management::Queue* mgmtObject;

            void push(boost::intrusive_ptr<Message>& msg);
            void setPolicy(std::auto_ptr<QueuePolicy> policy);
            bool seek(QueuedMessage& msg, Consumer::shared_ptr position);
            bool getNextMessage(QueuedMessage& msg, Consumer::shared_ptr c);
            bool consumeNextMessage(QueuedMessage& msg, Consumer::shared_ptr c);
            bool browseNextMessage(QueuedMessage& msg, Consumer::shared_ptr c);

            void removeListener(Consumer::shared_ptr);
            void addListener(Consumer::shared_ptr);

            bool isExcluded(boost::intrusive_ptr<Message>& msg);

            void dequeued(boost::intrusive_ptr<Message>& msg);
            void popAndDequeue();

        public:

            virtual void notifyDurableIOComplete();
            typedef boost::shared_ptr<Queue> shared_ptr;

            typedef std::vector<shared_ptr> vector;

            Queue(const string& name, bool autodelete = false, 
                  MessageStore* const store = 0, 
                  const OwnershipToken* const owner = 0,
                  management::Manageable* parent = 0);
            ~Queue();

            bool dispatch(Consumer::shared_ptr);
            /**
             * Check whether there would be a message available for
             * dispatch to this consumer. If not, the consumer will be
             * notified of events that may have changed this
             * situation.
             */
            bool checkForMessages(Consumer::shared_ptr);

            void create(const qpid::framing::FieldTable& settings);
            void configure(const qpid::framing::FieldTable& settings);
            void destroy();
            void bound(const string& exchange, const string& key, const qpid::framing::FieldTable& args);
            void unbind(ExchangeRegistry& exchanges, Queue::shared_ptr shared_ref);

            bool acquire(const QueuedMessage& msg);

            /**
             * Delivers a message to the queue. Will record it as
             * enqueued if persistent then process it.
             */
            void deliver(boost::intrusive_ptr<Message>& msg);
            /**
             * Dispatches the messages immediately to a consumer if
             * one is available or stores it for later if not.
             */
            void process(boost::intrusive_ptr<Message>& msg);
            /**
             * Returns a message to the in-memory queue (due to lack
             * of acknowledegement from a receiver). If a consumer is
             * available it will be dispatched immediately, else it
             * will be returned to the front of the queue.
             */
            void requeue(const QueuedMessage& msg);
            /**
             * Used during recovery to add stored messages back to the queue
             */
            void recover(boost::intrusive_ptr<Message>& msg);

            void consume(Consumer::shared_ptr c, bool exclusive = false);
            void cancel(Consumer::shared_ptr c);

            uint32_t purge(const uint32_t purge_request = 0); //defaults to all messages 

            uint32_t getMessageCount() const;
            uint32_t getConsumerCount() const;
            inline const string& getName() const { return name; }
            bool isExclusiveOwner(const OwnershipToken* const o) const;
            void releaseExclusiveOwnership();
            bool setExclusiveOwner(const OwnershipToken* const o);
            bool hasExclusiveConsumer() const;
            bool hasExclusiveOwner() const;
            inline bool isDurable() const { return store != 0; }
            inline const framing::FieldTable& getSettings() const { return settings; }
            inline bool isAutoDelete() const { return autodelete; }
            bool canAutoDelete() const;
            const QueueBindings& getBindings() const { return bindings; }


            bool enqueue(TransactionContext* ctxt, boost::intrusive_ptr<Message> msg);
            /**
             * dequeue from store (only done once messages is acknowledged)
             */
            bool dequeue(TransactionContext* ctxt, boost::intrusive_ptr<Message> msg);

            /**
             * Gets the next available message 
             */
            QueuedMessage get();

            const QueuePolicy* getPolicy();

            void setAlternateExchange(boost::shared_ptr<Exchange> exchange);
            boost::shared_ptr<Exchange> getAlternateExchange();
            bool isLocal(boost::intrusive_ptr<Message>& msg);

            //PersistableQueue support:
            uint64_t getPersistenceId() const;
            void setPersistenceId(uint64_t persistenceId) const;
            void encode(framing::Buffer& buffer) const;
            uint32_t encodedSize() const;

            static Queue::shared_ptr decode(QueueRegistry& queues, framing::Buffer& buffer);
            static void tryAutoDelete(Broker& broker, Queue::shared_ptr);

            virtual void setExternalQueueStore(ExternalQueueStore* inst);

            // Manageable entry points
            management::ManagementObject* GetManagementObject (void) const;
            management::Manageable::status_t
            ManagementMethod (uint32_t methodId, management::Args& args);

            /** Apply f to each Message on the queue. */
            template <class F> void eachMessage(const F& f) const {
                sys::Mutex::ScopedLock l(messageLock);
                std::for_each(messages.begin(), messages.end(), f);
            }

            /** Apply f to each QueueBinding on the queue */
            template <class F> void eachBinding(const F& f) {
                bindings.eachBinding(f);
            }
        };
    }
}


#endif  /*!_broker_Queue_h*/
