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
package org.apache.qpid.server.txn;

import org.apache.qpid.server.message.EnqueableMessage;
import org.apache.qpid.server.queue.BaseQueue;
import org.apache.qpid.server.queue.QueueEntry;

import java.util.Collection;
import java.util.List;

public interface ServerTransaction
{

    void addPostCommitAction(Action postCommitAction);




    public static interface Action
    {
        public void postCommit();

        public void onRollback();
    }

    void dequeue(BaseQueue queue, EnqueableMessage message, Action postCommitAction);

    void dequeue(Collection<QueueEntry> ackedMessages, Action postCommitAction);

    void enqueue(BaseQueue queue, EnqueableMessage message, Action postCommitAction);

    void enqueue(List<? extends BaseQueue> queues, EnqueableMessage message, Action postCommitAction);


    void commit();

    void rollback();
}
