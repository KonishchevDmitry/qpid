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
#include <qpid/framing/AMQContentBody.h>
#include <iostream>

qpid::framing::AMQContentBody::AMQContentBody(){
}

qpid::framing::AMQContentBody::AMQContentBody(const string& _data) : data(_data){
}

u_int32_t qpid::framing::AMQContentBody::size() const{
    return data.size();
}
void qpid::framing::AMQContentBody::encode(Buffer& buffer) const{
    buffer.putRawData(data);
}
void qpid::framing::AMQContentBody::decode(Buffer& buffer, u_int32_t _size){
    buffer.getRawData(data, _size);
}

void qpid::framing::AMQContentBody::print(std::ostream& out) const
{
    out << "content (" << size() << " bytes)";
}
