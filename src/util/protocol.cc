/**
* Copyright 2015 Telefonica Investigación y Desarrollo, S.A.U
*
* This file is part of iotagent project.
*
* iotagent is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License as published
* by the Free Software Foundation, either version 3 of the License,
* or (at your option) any later version.
*
* iotagent is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU Affero General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with iotagent. If not, see http://www.gnu.org/licenses/.
*
* For those usages not covered by the GNU Affero General Public License
* please contact with iot_support at tid dot es
*/
#include "protocol.h"

iota::Protocol::Protocol() {

  _description = "";
  _name = "";
  _id = "";

}


iota::Protocol::Protocol(const std::string& name) {

  _description = "";
  _name = name;
  _id = "";

}


iota::Protocol::~Protocol(void) {
}

void iota::Protocol::add(resource_endpoint data) {
   _endpoints.push_back (data);
}

