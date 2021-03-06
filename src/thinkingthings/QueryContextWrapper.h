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
#ifndef SRC_THINKINGTHINGS_QUERYCONTEXTWRAPPER_H_
#define SRC_THINKINGTHINGS_QUERYCONTEXTWRAPPER_H_

#include <ngsi/QueryContext.h>
#include <ngsi/ContextResponses.h>
#include <rest/iot_cb_comm.h>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <util/common.h>
#include <string>
#include <pion/logger.hpp>

namespace iota {
namespace esp {
namespace tt {
class QueryContextWrapper {
  public:

    QueryContextWrapper(boost::property_tree::ptree*);

    ::iota::ContextResponses queryContext(::iota::QueryContext&
                                          qContext); //maybe something else is needed.

    virtual ~QueryContextWrapper();
  protected:
  private:
    pion::logger m_logger;
    std::string cb_url;
    std::string cb_response;
    boost::property_tree::ptree* pt_cb;

    iota::ContextBrokerCommunicator cb_communicator;


    virtual ::iota::ContextResponses doQueryContext(::iota::QueryContext& qContext);


};
}
}
}
#endif // QUERYCONTEXTWRAPPER_H
