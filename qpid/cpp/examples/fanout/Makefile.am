#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#
examplesdir=$(pkgdatadir)/examples/fanout

MAKELDFLAGS=$(CLIENTFLAGS)
include $(top_srcdir)/examples/makedist.mk

noinst_PROGRAMS=fanout_producer listener
fanout_producer_SOURCES=fanout_producer.cpp
fanout_producer_LDADD=$(CLIENT_LIB)

listener_SOURCES=listener.cpp
listener_LDADD=$(CLIENT_LIB)

examples_DATA=               \
	fanout_producer.cpp  \
	listener.cpp         \
	$(MAKEDIST)

EXTRA_DIST=                  \
	$(examples_DATA)     \
	CMakeLists.txt       \
	verify               \
	verify.in            \
	fanout_fanout_producer.vcproj \
	fanout_listener.vcproj
