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
#include "unit_test.h"
#include "test_tools.h"
#include "qpid/Exception.h"
#include "qpid/broker/Broker.h"
#include "qpid/broker/Queue.h"
#include "qpid/broker/Deliverable.h"
#include "qpid/broker/ExchangeRegistry.h"
#include "qpid/broker/QueueRegistry.h"
#include "qpid/broker/NullMessageStore.h"
#include "qpid/broker/ExpiryPolicy.h"
#include "qpid/framing/MessageTransferBody.h"
#include "qpid/client/QueueOptions.h"
#include <iostream>
#include "boost/format.hpp"

using boost::intrusive_ptr;
using namespace qpid;
using namespace qpid::broker;
using namespace qpid::framing;
using namespace qpid::sys;

class TestConsumer : public virtual Consumer{
public:
    typedef boost::shared_ptr<TestConsumer> shared_ptr;            

    intrusive_ptr<Message> last;
    bool received;
    TestConsumer(bool acquire = true):Consumer(acquire), received(false) {};

    virtual bool deliver(QueuedMessage& msg){
        last = msg.payload;
        received = true;
        return true;
    };
    void notify() {}
    OwnershipToken* getSession() { return 0; }
};

class FailOnDeliver : public Deliverable
{
    Message msg;
public:
    void deliverTo(const boost::shared_ptr<Queue>& queue)
    {
        throw Exception(QPID_MSG("Invalid delivery to " << queue->getName()));
    }
    Message& getMessage() { return msg; }
};

intrusive_ptr<Message> create_message(std::string exchange, std::string routingKey) {
    intrusive_ptr<Message> msg(new Message());
    AMQFrame method((MessageTransferBody(ProtocolVersion(), exchange, 0, 0)));
    AMQFrame header((AMQHeaderBody()));
    msg->getFrames().append(method);
    msg->getFrames().append(header);
    msg->getFrames().getHeaders()->get<DeliveryProperties>(true)->setRoutingKey(routingKey);
    return msg;
}

QPID_AUTO_TEST_SUITE(QueueTestSuite)

QPID_AUTO_TEST_CASE(testAsyncMessage) {
    Queue::shared_ptr queue(new Queue("my_test_queue", true));
    intrusive_ptr<Message> received;
    
    TestConsumer::shared_ptr c1(new TestConsumer());
    queue->consume(c1);
    
    
    //Test basic delivery:
    intrusive_ptr<Message> msg1 = create_message("e", "A");
    msg1->enqueueAsync(queue, 0);//this is done on enqueue which is not called from process
    queue->process(msg1);
    sleep(2);
    
    BOOST_CHECK(!c1->received);
    msg1->enqueueComplete();
    
    received = queue->get().payload;
    BOOST_CHECK_EQUAL(msg1.get(), received.get());    
}
    
    
QPID_AUTO_TEST_CASE(testAsyncMessageCount){
    Queue::shared_ptr queue(new Queue("my_test_queue", true));
    intrusive_ptr<Message> msg1 = create_message("e", "A");
    msg1->enqueueAsync(queue, 0);//this is done on enqueue which is not called from process
    
    queue->process(msg1);
    sleep(2);
    uint32_t compval=0;
    BOOST_CHECK_EQUAL(compval, queue->getMessageCount());
    msg1->enqueueComplete();
    compval=1;
    BOOST_CHECK_EQUAL(compval, queue->getMessageCount());    
}

QPID_AUTO_TEST_CASE(testConsumers){
    Queue::shared_ptr queue(new Queue("my_queue", true));
    
    //Test adding consumers:
    TestConsumer::shared_ptr c1(new TestConsumer());
    TestConsumer::shared_ptr c2(new TestConsumer());
    queue->consume(c1);
    queue->consume(c2);
    
    BOOST_CHECK_EQUAL(uint32_t(2), queue->getConsumerCount());
    
    //Test basic delivery:
    intrusive_ptr<Message> msg1 = create_message("e", "A");
    intrusive_ptr<Message> msg2 = create_message("e", "B");
    intrusive_ptr<Message> msg3 = create_message("e", "C");
    
    queue->deliver(msg1);
    BOOST_CHECK(queue->dispatch(c1));
    BOOST_CHECK_EQUAL(msg1.get(), c1->last.get());
    
    queue->deliver(msg2);
    BOOST_CHECK(queue->dispatch(c2));
    BOOST_CHECK_EQUAL(msg2.get(), c2->last.get());
    
    c1->received = false;
    queue->deliver(msg3);
    BOOST_CHECK(queue->dispatch(c1));
    BOOST_CHECK_EQUAL(msg3.get(), c1->last.get());        
    
    //Test cancellation:
    queue->cancel(c1);
    BOOST_CHECK_EQUAL(uint32_t(1), queue->getConsumerCount());
    queue->cancel(c2);
    BOOST_CHECK_EQUAL(uint32_t(0), queue->getConsumerCount());
}

QPID_AUTO_TEST_CASE(testRegistry){
    //Test use of queues in registry:
    QueueRegistry registry;
    registry.declare("queue1", true, true);
    registry.declare("queue2", true, true);
    registry.declare("queue3", true, true);
    
    BOOST_CHECK(registry.find("queue1"));
    BOOST_CHECK(registry.find("queue2"));
    BOOST_CHECK(registry.find("queue3"));
    
    registry.destroy("queue1");
    registry.destroy("queue2");
    registry.destroy("queue3");
    
    BOOST_CHECK(!registry.find("queue1"));
    BOOST_CHECK(!registry.find("queue2"));
    BOOST_CHECK(!registry.find("queue3"));
}

QPID_AUTO_TEST_CASE(testDequeue){
    Queue::shared_ptr queue(new Queue("my_queue", true));
    intrusive_ptr<Message> msg1 = create_message("e", "A");
    intrusive_ptr<Message> msg2 = create_message("e", "B");
    intrusive_ptr<Message> msg3 = create_message("e", "C");
    intrusive_ptr<Message> received;
    
    queue->deliver(msg1);
    queue->deliver(msg2);
    queue->deliver(msg3);
    
    BOOST_CHECK_EQUAL(uint32_t(3), queue->getMessageCount());
    
    received = queue->get().payload;
    BOOST_CHECK_EQUAL(msg1.get(), received.get());
    BOOST_CHECK_EQUAL(uint32_t(2), queue->getMessageCount());

    received = queue->get().payload;
    BOOST_CHECK_EQUAL(msg2.get(), received.get());
    BOOST_CHECK_EQUAL(uint32_t(1), queue->getMessageCount());

    TestConsumer::shared_ptr consumer(new TestConsumer());
    queue->consume(consumer);
    queue->dispatch(consumer);
    if (!consumer->received)
        sleep(2);

    BOOST_CHECK_EQUAL(msg3.get(), consumer->last.get());
    BOOST_CHECK_EQUAL(uint32_t(0), queue->getMessageCount());

    received = queue->get().payload;
    BOOST_CHECK(!received);
    BOOST_CHECK_EQUAL(uint32_t(0), queue->getMessageCount());
        
}

QPID_AUTO_TEST_CASE(testBound)
{
    //test the recording of bindings, and use of those to allow a queue to be unbound
    string key("my-key");
    FieldTable args;

    Queue::shared_ptr queue(new Queue("my-queue", true));
    ExchangeRegistry exchanges;
    //establish bindings from exchange->queue and notify the queue as it is bound:
    Exchange::shared_ptr exchange1 = exchanges.declare("my-exchange-1", "direct").first;
    exchange1->bind(queue, key, &args);
    queue->bound(exchange1->getName(), key, args);

    Exchange::shared_ptr exchange2 = exchanges.declare("my-exchange-2", "fanout").first;
    exchange2->bind(queue, key, &args);
    queue->bound(exchange2->getName(), key, args);

    Exchange::shared_ptr exchange3 = exchanges.declare("my-exchange-3", "topic").first;
    exchange3->bind(queue, key, &args);
    queue->bound(exchange3->getName(), key, args);

    //delete one of the exchanges:
    exchanges.destroy(exchange2->getName());
    exchange2.reset();

    //unbind the queue from all exchanges it knows it has been bound to:
    queue->unbind(exchanges, queue);

    //ensure the remaining exchanges don't still have the queue bound to them:
    FailOnDeliver deliverable;        
    exchange1->route(deliverable, key, &args);
    exchange3->route(deliverable, key, &args);
}

QPID_AUTO_TEST_CASE(testPersistLastNodeStanding){

    client::QueueOptions args;
	args.setPersistLastNode();
	
	Queue::shared_ptr queue(new Queue("my-queue", true));
    queue->configure(args);
	
    intrusive_ptr<Message> msg1 = create_message("e", "A");
    intrusive_ptr<Message> msg2 = create_message("e", "B");
    intrusive_ptr<Message> msg3 = create_message("e", "C");

	//enqueue 2 messages
    queue->deliver(msg1);
    queue->deliver(msg2);
	
	//change mode
	queue->setLastNodeFailure();
	
	//enqueue 1 message
    queue->deliver(msg3);
	
	//check all have persistent ids.
    BOOST_CHECK(msg1->isPersistent());
    BOOST_CHECK(msg2->isPersistent());
    BOOST_CHECK(msg3->isPersistent());

}

class TestMessageStoreOC : public NullMessageStore
{
  public:

    uint enqCnt;
    uint deqCnt;
    bool error;
    
    virtual void dequeue(TransactionContext*,
                 const boost::intrusive_ptr<PersistableMessage>& /*msg*/,
                 const PersistableQueue& /*queue*/)
    {
        if (error) throw Exception("Dequeue error test");
        deqCnt++;
    }

    virtual void enqueue(TransactionContext*,
                 const boost::intrusive_ptr<PersistableMessage>& /*msg*/,
                 const PersistableQueue& /* queue */)
    {
        if (error) throw Exception("Enqueue error test");
        enqCnt++;
    }

    void createError()
    {
        error=true;
    }
    
    TestMessageStoreOC() : NullMessageStore(),enqCnt(0),deqCnt(0),error(false) {}
    ~TestMessageStoreOC(){}
};


QPID_AUTO_TEST_CASE(testLVQOrdering){

    client::QueueOptions args;
    // set queue mode
	args.setOrdering(client::LVQ);

	Queue::shared_ptr queue(new Queue("my-queue", true ));
	queue->configure(args);
	
    intrusive_ptr<Message> msg1 = create_message("e", "A");
    intrusive_ptr<Message> msg2 = create_message("e", "B");
    intrusive_ptr<Message> msg3 = create_message("e", "C");
    intrusive_ptr<Message> msg4 = create_message("e", "D");
    intrusive_ptr<Message> received;

    //set deliever match for LVQ a,b,c,a

    string key;
	args.getLVQKey(key);
    BOOST_CHECK_EQUAL(key, "qpid.LVQ_key");
	

	msg1->getProperties<MessageProperties>()->getApplicationHeaders().setString(key,"a");
	msg2->getProperties<MessageProperties>()->getApplicationHeaders().setString(key,"b");
	msg3->getProperties<MessageProperties>()->getApplicationHeaders().setString(key,"c");
	msg4->getProperties<MessageProperties>()->getApplicationHeaders().setString(key,"a");
	
	//enqueue 4 message
    queue->deliver(msg1);
    queue->deliver(msg2);
    queue->deliver(msg3);
    queue->deliver(msg4);
	
    BOOST_CHECK_EQUAL(queue->getMessageCount(), 3u);
	
    received = queue->get().payload;
    BOOST_CHECK_EQUAL(msg4.get(), received.get());

    received = queue->get().payload;
    BOOST_CHECK_EQUAL(msg2.get(), received.get());
	
    received = queue->get().payload;
    BOOST_CHECK_EQUAL(msg3.get(), received.get());

    intrusive_ptr<Message> msg5 = create_message("e", "A");
    intrusive_ptr<Message> msg6 = create_message("e", "B");
    intrusive_ptr<Message> msg7 = create_message("e", "C");
	msg5->getProperties<MessageProperties>()->getApplicationHeaders().setString(key,"a");
	msg6->getProperties<MessageProperties>()->getApplicationHeaders().setString(key,"b");
	msg7->getProperties<MessageProperties>()->getApplicationHeaders().setString(key,"c");
    queue->deliver(msg5);
    queue->deliver(msg6);
    queue->deliver(msg7);
	
    BOOST_CHECK_EQUAL(queue->getMessageCount(), 3u);
	
    received = queue->get().payload;
    BOOST_CHECK_EQUAL(msg5.get(), received.get());

    received = queue->get().payload;
    BOOST_CHECK_EQUAL(msg6.get(), received.get());
	
    received = queue->get().payload;
    BOOST_CHECK_EQUAL(msg7.get(), received.get());
	
}

QPID_AUTO_TEST_CASE(testLVQEmptyKey){

    client::QueueOptions args;
    // set queue mode
    args.setOrdering(client::LVQ);

    Queue::shared_ptr queue(new Queue("my-queue", true ));
    queue->configure(args);
	
    intrusive_ptr<Message> msg1 = create_message("e", "A");
    intrusive_ptr<Message> msg2 = create_message("e", "B");

    string key;
    args.getLVQKey(key);
    BOOST_CHECK_EQUAL(key, "qpid.LVQ_key");
	

    msg1->getProperties<MessageProperties>()->getApplicationHeaders().setString(key,"a");
    queue->deliver(msg1);
    queue->deliver(msg2);
    BOOST_CHECK_EQUAL(queue->getMessageCount(), 2u);
    
}

QPID_AUTO_TEST_CASE(testLVQAcquire){

    client::QueueOptions args;
    // set queue mode
    args.setOrdering(client::LVQ);

    Queue::shared_ptr queue(new Queue("my-queue", true ));
    queue->configure(args);
	
    intrusive_ptr<Message> msg1 = create_message("e", "A");
    intrusive_ptr<Message> msg2 = create_message("e", "B");
    intrusive_ptr<Message> msg3 = create_message("e", "C");
    intrusive_ptr<Message> msg4 = create_message("e", "D");
    intrusive_ptr<Message> msg5 = create_message("e", "F");
    intrusive_ptr<Message> msg6 = create_message("e", "G");

    //set deliever match for LVQ a,b,c,a

    string key;
    args.getLVQKey(key);
    BOOST_CHECK_EQUAL(key, "qpid.LVQ_key");
	

    msg1->getProperties<MessageProperties>()->getApplicationHeaders().setString(key,"a");
    msg2->getProperties<MessageProperties>()->getApplicationHeaders().setString(key,"b");
    msg3->getProperties<MessageProperties>()->getApplicationHeaders().setString(key,"c");
    msg4->getProperties<MessageProperties>()->getApplicationHeaders().setString(key,"a");
    msg5->getProperties<MessageProperties>()->getApplicationHeaders().setString(key,"b");
    msg6->getProperties<MessageProperties>()->getApplicationHeaders().setString(key,"c");
	
    //enqueue 4 message
    queue->deliver(msg1);
    queue->deliver(msg2);
    queue->deliver(msg3);
    queue->deliver(msg4);
    
    BOOST_CHECK_EQUAL(queue->getMessageCount(), 3u);

    framing::SequenceNumber sequence(1);
    QueuedMessage qmsg(queue.get(), msg1, sequence);
    QueuedMessage qmsg2(queue.get(), msg2, ++sequence);

    BOOST_CHECK(!queue->acquire(qmsg));
    BOOST_CHECK(queue->acquire(qmsg2));
    
    BOOST_CHECK_EQUAL(queue->getMessageCount(), 2u);
    
    queue->deliver(msg5);
    BOOST_CHECK_EQUAL(queue->getMessageCount(), 3u);

    // set mode to no browse and check
    args.setOrdering(client::LVQ_NO_BROWSE);
    queue->configure(args);
    TestConsumer::shared_ptr c1(new TestConsumer(false));
    
    queue->dispatch(c1);
    queue->dispatch(c1);
    queue->dispatch(c1);
    
    queue->deliver(msg6);
    BOOST_CHECK_EQUAL(queue->getMessageCount(), 3u);

    intrusive_ptr<Message> received;
    received = queue->get().payload;
    BOOST_CHECK_EQUAL(msg4.get(), received.get());

}

QPID_AUTO_TEST_CASE(testLVQMultiQueue){

    client::QueueOptions args;
    // set queue mode
    args.setOrdering(client::LVQ);

    Queue::shared_ptr queue1(new Queue("my-queue", true ));
    Queue::shared_ptr queue2(new Queue("my-queue", true ));
    intrusive_ptr<Message> received;
    queue1->configure(args);
    queue2->configure(args);
	
    intrusive_ptr<Message> msg1 = create_message("e", "A");
    intrusive_ptr<Message> msg2 = create_message("e", "A");

    string key;
    args.getLVQKey(key);
    BOOST_CHECK_EQUAL(key, "qpid.LVQ_key");

    msg1->getProperties<MessageProperties>()->getApplicationHeaders().setString(key,"a");
    msg2->getProperties<MessageProperties>()->getApplicationHeaders().setString(key,"a");
	
    queue1->deliver(msg1);
    queue2->deliver(msg1);
    queue1->deliver(msg2);
    
    received = queue1->get().payload;
    BOOST_CHECK_EQUAL(msg2.get(), received.get());

    received = queue2->get().payload;
    BOOST_CHECK_EQUAL(msg1.get(), received.get());
    
}

QPID_AUTO_TEST_CASE(testLVQRecover){

/* simulate this
  1. start 2 nodes
  2. create cluster durable lvq
  3. send a transient message to the queue
  4. kill one of the nodes (to trigger force persistent behaviour)...
  5. then restart it (to turn off force persistent behaviour)
  6. send another transient message with same lvq key as in 3
  7. kill the second node again (retrigger force persistent)
  8. stop and recover the first node
*/
    TestMessageStoreOC  testStore;
    client::QueueOptions args;
    // set queue mode
    args.setOrdering(client::LVQ);
    args.setPersistLastNode();

    Queue::shared_ptr queue1(new Queue("my-queue", true, &testStore));
    intrusive_ptr<Message> received;
    queue1->configure(args);
 	
    intrusive_ptr<Message> msg1 = create_message("e", "A");
    intrusive_ptr<Message> msg2 = create_message("e", "A");
    // 2
    string key;
    args.getLVQKey(key);
    BOOST_CHECK_EQUAL(key, "qpid.LVQ_key");

    msg1->getProperties<MessageProperties>()->getApplicationHeaders().setString(key,"a");
    msg2->getProperties<MessageProperties>()->getApplicationHeaders().setString(key,"a");
	// 3
    queue1->deliver(msg1);
    // 4
    queue1->setLastNodeFailure();
    BOOST_CHECK_EQUAL(testStore.enqCnt, 1u);
    // 5
    queue1->clearLastNodeFailure();
    BOOST_CHECK_EQUAL(testStore.enqCnt, 1u);
    // 6
    queue1->deliver(msg2);
    BOOST_CHECK_EQUAL(testStore.enqCnt, 1u);
    queue1->setLastNodeFailure();
    BOOST_CHECK_EQUAL(testStore.enqCnt, 2u);
    BOOST_CHECK_EQUAL(testStore.deqCnt, 1u);
}

void addMessagesToQueue(uint count, Queue& queue, uint oddTtl = 200, uint evenTtl = 0) 
{
    for (uint i = 0; i < count; i++) {
        intrusive_ptr<Message> m = create_message("exchange", "key");
        if (i % 2) {
            if (oddTtl) m->getProperties<DeliveryProperties>()->setTtl(oddTtl);
        } else {
            if (evenTtl) m->getProperties<DeliveryProperties>()->setTtl(evenTtl);
        }
        m->setTimestamp(new broker::ExpiryPolicy);
        queue.deliver(m);
    }
}

QPID_AUTO_TEST_CASE(testPurgeExpired) {
    Queue queue("my-queue");
    addMessagesToQueue(10, queue);
    BOOST_CHECK_EQUAL(queue.getMessageCount(), 10u);
    ::usleep(300*1000);
    queue.purgeExpired();
    BOOST_CHECK_EQUAL(queue.getMessageCount(), 5u);
}

QPID_AUTO_TEST_CASE(testQueueCleaner) {
    Timer timer;
    QueueRegistry queues;
    Queue::shared_ptr queue = queues.declare("my-queue").first;
    addMessagesToQueue(10, *queue, 200, 400);
    BOOST_CHECK_EQUAL(queue->getMessageCount(), 10u);

    QueueCleaner cleaner(queues, timer);
    cleaner.start(100 * qpid::sys::TIME_MSEC);
    ::usleep(300*1000);
    BOOST_CHECK_EQUAL(queue->getMessageCount(), 5u);
    ::usleep(300*1000);
    BOOST_CHECK_EQUAL(queue->getMessageCount(), 0u);
}

QPID_AUTO_TEST_CASE(testMultiQueueLastNode){

    TestMessageStoreOC  testStore;
    client::QueueOptions args;
    args.setPersistLastNode();

    Queue::shared_ptr queue1(new Queue("queue1", true, &testStore ));
    queue1->configure(args);
    Queue::shared_ptr queue2(new Queue("queue2", true, &testStore ));
    queue2->configure(args);
	
    intrusive_ptr<Message> msg1 = create_message("e", "A");

    queue1->deliver(msg1);
    queue2->deliver(msg1);

    //change mode
    queue1->setLastNodeFailure();
    BOOST_CHECK_EQUAL(testStore.enqCnt, 1u);
    queue2->setLastNodeFailure();
    BOOST_CHECK_EQUAL(testStore.enqCnt, 2u);

    // check they don't get stored twice
    queue1->setLastNodeFailure();
    queue2->setLastNodeFailure();
    BOOST_CHECK_EQUAL(testStore.enqCnt, 2u);

    intrusive_ptr<Message> msg2 = create_message("e", "B");
    queue1->deliver(msg2);
    queue2->deliver(msg2);

    queue1->clearLastNodeFailure();
    queue2->clearLastNodeFailure();
    // check only new messages get forced
    queue1->setLastNodeFailure();
    queue2->setLastNodeFailure();
    BOOST_CHECK_EQUAL(testStore.enqCnt, 4u);

    // check no failure messages are stored
    queue1->clearLastNodeFailure();
    queue2->clearLastNodeFailure();
    
    intrusive_ptr<Message> msg3 = create_message("e", "B");
    queue1->deliver(msg3);
    queue2->deliver(msg3);
    BOOST_CHECK_EQUAL(testStore.enqCnt, 4u);
    queue1->setLastNodeFailure();
    queue2->setLastNodeFailure();
    BOOST_CHECK_EQUAL(testStore.enqCnt, 6u);
    
    // check requeue 1
    intrusive_ptr<Message> msg4 = create_message("e", "C");
    intrusive_ptr<Message> msg5 = create_message("e", "D");

    framing::SequenceNumber sequence(1);
    QueuedMessage qmsg1(queue1.get(), msg4, sequence);
    QueuedMessage qmsg2(queue2.get(), msg5, ++sequence);
    
    queue1->requeue(qmsg1);
    BOOST_CHECK_EQUAL(testStore.enqCnt, 7u);
    
    // check requeue 2
    queue2->clearLastNodeFailure();
    queue2->requeue(qmsg2);
    BOOST_CHECK_EQUAL(testStore.enqCnt, 7u);
    queue2->setLastNodeFailure();
    BOOST_CHECK_EQUAL(testStore.enqCnt, 8u);
    
    queue2->clearLastNodeFailure();
    queue2->setLastNodeFailure();
    BOOST_CHECK_EQUAL(testStore.enqCnt, 8u);
}

QPID_AUTO_TEST_CASE(testLastNodeRecoverAndFail){
/*
simulate this:
  1. start two nodes
  2. create cluster durable queue and add some messages
  3. kill one node (trigger force-persistent behaviour)
  4. stop and recover remaining node
  5. add another node
  6. kill that new node again
make sure that an attempt to re-enqueue a message does not happen which will 
result in the last man standing exiting with an error. 

we need to make sure that recover is safe, i.e. messages are
not requeued to the store.
*/
    TestMessageStoreOC  testStore;
    client::QueueOptions args;
    // set queue mode
    args.setPersistLastNode();

    Queue::shared_ptr queue1(new Queue("my-queue", true, &testStore));
    intrusive_ptr<Message> received;
    queue1->configure(args);
 	
    // check requeue 1
    intrusive_ptr<Message> msg1 = create_message("e", "C");
    intrusive_ptr<Message> msg2 = create_message("e", "D");

    queue1->recover(msg1);

    queue1->setLastNodeFailure();
    BOOST_CHECK_EQUAL(testStore.enqCnt, 0u);

    queue1->clearLastNodeFailure();
    BOOST_CHECK_EQUAL(testStore.enqCnt, 0u);

    queue1->deliver(msg2);
    BOOST_CHECK_EQUAL(testStore.enqCnt, 0u);
    queue1->setLastNodeFailure();
    BOOST_CHECK_EQUAL(testStore.enqCnt, 1u);

}

QPID_AUTO_TEST_CASE(testLastNodeJournalError){
/*
simulate store excption going into last node standing

*/
    TestMessageStoreOC  testStore;
    client::QueueOptions args;
    // set queue mode
    args.setPersistLastNode();

    Queue::shared_ptr queue1(new Queue("my-queue", true, &testStore));
    intrusive_ptr<Message> received;
    queue1->configure(args);
 	
    // check requeue 1
    intrusive_ptr<Message> msg1 = create_message("e", "C");

    queue1->deliver(msg1);
    testStore.createError();
    
    ScopedSuppressLogging sl; // Suppress messages for expected errors.
    queue1->setLastNodeFailure();
    BOOST_CHECK_EQUAL(testStore.enqCnt, 0u);

}QPID_AUTO_TEST_SUITE_END()

