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
package org.apache.qpid.agent;

import java.util.HashMap;

import org.apache.qpid.agent.annotations.QMFType;

@QMFType(className = "Pikelet", packageName = "org.apache.test")
public class Pikelet extends Crumpet
{
    protected String shape;
    HashMap<String, Crumpet> crumpets = new HashMap<String, Crumpet>();

    public String getShape()
    {
        return shape;
    }

    public void setShape(String shape)
    {
        this.shape = shape;
    }

    public HashMap<String, Crumpet> getCrumpets()
    {
        return crumpets;
    }

    public void setCrumpets(HashMap<String, Crumpet> crumpets)
    {
        this.crumpets = crumpets;
    }
}
