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
package org.apache.qpid.qman.debug;

import javax.management.ObjectName;

import org.apache.muse.util.xml.XmlUtils;
import org.apache.qpid.transport.util.Logger;
import org.w3c.dom.Node;

/**
 * Utility class used for debbugging WSDL documents
 * 
 * @author Andrea Gazzarini
 */
public class WsdlDebugger {
	public final static Logger LOGGER = Logger.get(WsdlDebugger.class);
	
	/**
	 * Prints out to log the given node.
	 * 
	 * @param node the xml node to be printed out.
	 */
	public static void debug(ObjectName resourceId, Node node) 
	{
		if (LOGGER.isDebugEnabled())
		{
			LOGGER.debug(resourceId+" : "+XmlUtils.toString(node, false,true));
		}
	}
}