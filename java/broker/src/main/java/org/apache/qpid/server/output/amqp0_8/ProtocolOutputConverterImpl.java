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

/*
 * This file is auto-generated by Qpid Gentools v.0.1 - do not modify.
 * Supported AMQP versions:
 *   8-0
 */
package org.apache.qpid.server.output.amqp0_8;

import org.apache.qpid.server.protocol.AMQProtocolSession;
import org.apache.qpid.server.queue.AMQMessage;
import org.apache.qpid.server.queue.AMQMessageHandle;
import org.apache.qpid.server.store.StoreContext;
import org.apache.qpid.server.output.ProtocolOutputConverter;
import org.apache.qpid.framing.*;
import org.apache.qpid.framing.abstraction.ContentChunk;
import org.apache.qpid.framing.abstraction.MessagePublishInfo;
import org.apache.qpid.AMQException;

import org.apache.mina.common.ByteBuffer;

import java.util.Iterator;

public class ProtocolOutputConverterImpl implements ProtocolOutputConverter
{


    public static Factory getInstanceFactory()
    {
        return new Factory()
        {
    
            public ProtocolOutputConverter newInstance(AMQProtocolSession session)
            {
                return new ProtocolOutputConverterImpl(session);
            }
        };
    }

    private final AMQProtocolSession _protocolSession;

    private ProtocolOutputConverterImpl(AMQProtocolSession session)
    {
        _protocolSession = session;
    }


    public AMQProtocolSession getProtocolSession()
    {
        return _protocolSession;
    }

    public void writeDeliver(AMQMessage message, int channelId, long deliveryTag, AMQShortString consumerTag)
            throws AMQException
    {
        AMQDataBlock deliver = createEncodedDeliverFrame(message, channelId, deliveryTag, consumerTag);
        AMQDataBlock contentHeader = ContentHeaderBody.createAMQFrame(channelId,
                                                                      message.getContentHeaderBody());

        final AMQMessageHandle messageHandle = message.getMessageHandle();
        final StoreContext storeContext = message.getStoreContext();


        final int bodyCount = messageHandle.getBodyCount(storeContext);

        if(bodyCount == 0)
        {
            SmallCompositeAMQDataBlock compositeBlock = new SmallCompositeAMQDataBlock(deliver,
                                                                             contentHeader);

            writeFrame(compositeBlock);
        }
        else
        {


            //
            // Optimise the case where we have a single content body. In that case we create a composite block
            // so that we can writeDeliver out the deliver, header and body with a single network writeDeliver.
            //
            ContentChunk cb = messageHandle.getContentChunk(storeContext, 0);

            AMQDataBlock firstContentBody = new AMQFrame(channelId, getProtocolSession().getMethodRegistry().getProtocolVersionMethodConverter().convertToBody(cb));
            AMQDataBlock[] blocks = new AMQDataBlock[]{deliver, contentHeader, firstContentBody};
            CompositeAMQDataBlock compositeBlock = new CompositeAMQDataBlock(blocks);
            writeFrame(compositeBlock);

            //
            // Now start writing out the other content bodies
            //
            for(int i = 1; i < bodyCount; i++)
            {
                cb = messageHandle.getContentChunk(storeContext, i);
                writeFrame(new AMQFrame(channelId, getProtocolSession().getMethodRegistry().getProtocolVersionMethodConverter().convertToBody(cb)));
            }


        }


    }


    public void writeGetOk(AMQMessage message, int channelId, long deliveryTag, int queueSize) throws AMQException
    {

        final AMQMessageHandle messageHandle = message.getMessageHandle();
        final StoreContext storeContext = message.getStoreContext();

        AMQDataBlock deliver = createEncodedGetOkFrame(message, channelId, deliveryTag, queueSize);


        AMQDataBlock contentHeader = ContentHeaderBody.createAMQFrame(channelId,
                                                                      message.getContentHeaderBody());

        final int bodyCount = messageHandle.getBodyCount(storeContext);
        if(bodyCount == 0)
        {
            SmallCompositeAMQDataBlock compositeBlock = new SmallCompositeAMQDataBlock(deliver,
                                                                             contentHeader);
            writeFrame(compositeBlock);
        }
        else
        {


            //
            // Optimise the case where we have a single content body. In that case we create a composite block
            // so that we can writeDeliver out the deliver, header and body with a single network writeDeliver.
            //
            ContentChunk cb = messageHandle.getContentChunk(storeContext, 0);

            AMQDataBlock firstContentBody = new AMQFrame(channelId, getProtocolSession().getMethodRegistry().getProtocolVersionMethodConverter().convertToBody(cb));
            AMQDataBlock[] blocks = new AMQDataBlock[]{deliver, contentHeader, firstContentBody};
            CompositeAMQDataBlock compositeBlock = new CompositeAMQDataBlock(blocks);
            writeFrame(compositeBlock);

            //
            // Now start writing out the other content bodies
            //
            for(int i = 1; i < bodyCount; i++)
            {
                cb = messageHandle.getContentChunk(storeContext, i);
                writeFrame(new AMQFrame(channelId, getProtocolSession().getMethodRegistry().getProtocolVersionMethodConverter().convertToBody(cb)));
            }


        }


    }


    private AMQDataBlock createEncodedDeliverFrame(AMQMessage message, int channelId, long deliveryTag, AMQShortString consumerTag)
            throws AMQException
    {
        final MessagePublishInfo pb = message.getMessagePublishInfo();
        final AMQMessageHandle messageHandle = message.getMessageHandle();

        MethodRegistry methodRegistry = MethodRegistry.getMethodRegistry(ProtocolVersion.v8_0);
        BasicDeliverBody deliverBody =
                methodRegistry.createBasicDeliverBody(consumerTag,
                                                      deliveryTag,
                                                      messageHandle.isRedelivered(),
                                                      pb.getExchange(),
                                                      pb.getRoutingKey());
        AMQFrame deliverFrame = deliverBody.generateFrame(channelId);


        return deliverFrame;
    }

    private AMQDataBlock createEncodedGetOkFrame(AMQMessage message, int channelId, long deliveryTag, int queueSize)
            throws AMQException
    {
        final MessagePublishInfo pb = message.getMessagePublishInfo();
        final AMQMessageHandle messageHandle = message.getMessageHandle();

        MethodRegistry methodRegistry = MethodRegistry.getMethodRegistry(ProtocolVersion.v8_0);
        BasicGetOkBody getOkBody =
                methodRegistry.createBasicGetOkBody(deliveryTag,
                                                    messageHandle.isRedelivered(),
                                                    pb.getExchange(),
                                                    pb.getRoutingKey(),
                                                    queueSize);
        AMQFrame getOkFrame = getOkBody.generateFrame(channelId);

        return getOkFrame;
    }

    public byte getProtocolMinorVersion()
    {
        return getProtocolSession().getProtocolMinorVersion();
    }

    public byte getProtocolMajorVersion()
    {
        return getProtocolSession().getProtocolMajorVersion();
    }

    private AMQDataBlock createEncodedReturnFrame(AMQMessage message, int channelId, int replyCode, AMQShortString replyText) throws AMQException
    {
        MethodRegistry methodRegistry = MethodRegistry.getMethodRegistry(ProtocolVersion.v8_0);
        BasicReturnBody basicReturnBody =
                methodRegistry.createBasicReturnBody(replyCode,
                                                     replyText,
                                                     message.getMessagePublishInfo().getExchange(),
                                                     message.getMessagePublishInfo().getRoutingKey());
        AMQFrame returnFrame = basicReturnBody.generateFrame(channelId);

        return returnFrame;
    }

    public void writeReturn(AMQMessage message, int channelId, int replyCode, AMQShortString replyText)
            throws AMQException
    {
        AMQDataBlock returnFrame = createEncodedReturnFrame(message, channelId, replyCode, replyText);

        AMQDataBlock contentHeader = ContentHeaderBody.createAMQFrame(channelId,
                                                                      message.getContentHeaderBody());

        Iterator<AMQDataBlock> bodyFrameIterator = message.getBodyFrameIterator(getProtocolSession(), channelId);
        //
        // Optimise the case where we have a single content body. In that case we create a composite block
        // so that we can writeDeliver out the deliver, header and body with a single network writeDeliver.
        //
        if (bodyFrameIterator.hasNext())
        {
            AMQDataBlock firstContentBody = bodyFrameIterator.next();
            AMQDataBlock[] blocks = new AMQDataBlock[]{returnFrame, contentHeader, firstContentBody};
            CompositeAMQDataBlock compositeBlock = new CompositeAMQDataBlock(blocks);
            writeFrame(compositeBlock);
        }
        else
        {
            CompositeAMQDataBlock compositeBlock = new CompositeAMQDataBlock(new AMQDataBlock[]{returnFrame, contentHeader});

            writeFrame(compositeBlock);
        }

        //
        // Now start writing out the other content bodies
        // TODO: MINA needs to be fixed so the the pending writes buffer is not unbounded
        //
        while (bodyFrameIterator.hasNext())
        {
            writeFrame(bodyFrameIterator.next());
        }
    }


    public void writeFrame(AMQDataBlock block)
    {
        getProtocolSession().writeFrame(block);
    }


    public void confirmConsumerAutoClose(int channelId, AMQShortString consumerTag)
    {
        MethodRegistry methodRegistry = MethodRegistry.getMethodRegistry(ProtocolVersion.v8_0);
        BasicCancelOkBody basicCancelOkBody = methodRegistry.createBasicCancelOkBody(consumerTag);
        writeFrame(basicCancelOkBody.generateFrame(channelId));

    }
}