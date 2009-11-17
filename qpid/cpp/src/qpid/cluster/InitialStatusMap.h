#ifndef QPID_CLUSTER_INITIALSTATUSMAP_H
#define QPID_CLUSTER_INITIALSTATUSMAP_H

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

#include "types.h"
#include <qpid/framing/ClusterInitialStatusBody.h>
#include <boost/optional.hpp>

namespace qpid {
namespace cluster {

/**
 * Track status of cluster members during initialization.
 */
class InitialStatusMap
{
  public:
    typedef framing::ClusterInitialStatusBody Status;

    InitialStatusMap(const MemberId& self);
    /** Process a config change. @return true if we need to re-send our status */
    void configChange(const MemberSet& newConfig);
    /** @return true if we need to re-send status */
    bool isResendNeeded();

    /** Process received status */
    void received(const MemberId&, const Status& is);

    /**@return true if the map is complete. */
    bool isComplete();
    /**@pre isComplete. @return this node's elders */
    MemberSet getElders();
    /**@pre isComplete. @return True if we need an update. */
    bool isUpdateNeeded();

  private:
    typedef std::map<MemberId, boost::optional<Status> > Map;
    static bool notInitialized(const Map::value_type&);
    static bool isActive(const Map::value_type&);
    void check();
    Map map;
    MemberSet firstConfig;
    MemberId self;
    bool complete, updateNeeded, resendNeeded;
};
}} // namespace qpid::cluster

#endif  /*!QPID_CLUSTER_INITIALSTATUSMAP_H*/