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
package org.apache.qpid.client.handler;

import org.apache.qpid.AMQException;
import org.apache.qpid.client.protocol.AMQProtocolSession;
import org.apache.qpid.client.state.StateAwareMethodListener;
import org.apache.qpid.framing.QueueDeleteOkBody;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * @author Apache Software Foundation
 */
public class QueueDeleteOkMethodHandler implements StateAwareMethodListener<QueueDeleteOkBody>
{
    private static final Logger _logger = LoggerFactory.getLogger(QueueDeleteOkMethodHandler.class);
    private static final QueueDeleteOkMethodHandler _instance = new QueueDeleteOkMethodHandler();

    public static QueueDeleteOkMethodHandler getInstance()
    {
        return _instance;
    }

    private QueueDeleteOkMethodHandler()
    { }

    public void methodReceived(AMQProtocolSession session, QueueDeleteOkBody body, int channelId)
            throws AMQException
    {        
        if (_logger.isDebugEnabled())
        {
            _logger.debug("Received Queue.Delete-Ok message, message count: " + body.getMessageCount());
        }
    }


}
