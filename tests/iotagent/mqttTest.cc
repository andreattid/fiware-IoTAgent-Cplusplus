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
#include "mqttTest.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string/trim.hpp>


#include <cppunit/extensions/HelperMacros.h>

#include <cmath>
#include <ctime>

#include "esp/plugins/input_mqtt/ESP_Plugin_Input_Mqtt.h"

#include "mqtt/MqttService.h"
#include "util/iota_exception.h"


using ::testing::Return;
using ::testing::NotNull;
using ::testing::StrEq;
using ::testing::_;
using ::testing::Invoke;


//iota::AdminService* AdminService_ptr = new iota::AdminService();

//HttpMock cb_mock(1026, "/NGSI10/updateContext");



CPPUNIT_TEST_SUITE_REGISTRATION(MqttTest);

MqttTest::MqttTest() {
  mqttMsg.topic = NULL;
  iota::Configurator::initialize("../../tests/iotagent/config.json");
}

MqttTest::~MqttTest() {
  // cb_mock.stop();
}

void MqttTest::setUp() {



  std::cout << "SETUP MqttTest " << std::endl;
  cbPublish = new iota::esp::ngsi::IotaMqttServiceImpl("/iot/mqtt");
  mqttService = new iota::esp::MqttService();

  mqttService->set_resource("/iot/mqtt");
  cbPublish->set_resthandle(mqttService);
  cbPublish->set_command_service(mqttService);

}

void MqttTest::tearDown() {
  // delete cbPublish; //Already deleted inside MqttService.

  if (mqttMsg.topic != NULL) {
    free(mqttMsg.topic);
  }
  delete mqttService; //
}


void MqttTest::testCBPublisher() {
  HttpMock cb_mock(1026, "/NGSI10/updateContext");
  cb_mock.init();
  std::cout << "Test CBPublisher starting ... " << std::endl;

  std::string entity;



  std::string expected = "";

  const iota::JsonValue& service_object =
    iota::Configurator::instance()->getServicebyApiKey("/iot/mqtt", "1234");

  if (service_object.HasMember("entity_type")) {
    entity.assign(service_object["entity_type"].GetString());
  }


  expected.append("{\"updateAction\":\"APPEND\",\"contextElements\":");
  expected.append("[{\"id\":\"thing:dev01\",\"type\":\"");
  expected.append(entity);
  expected.append("\",\"isPattern\":\"false\",\"attributes\"");
  expected.append(":[{\"name\":\"te\",\"type\":\"string\",\"value\":\"234\",\"metadatas\"");



  std::string actual = "";
  std::string apikey = "1234";
  std::string device = "dev01";

  std::string jsonMqtt = "";
  jsonMqtt.append(std::string("{\"name\" : \""));
  jsonMqtt.append("te");
  jsonMqtt.append("\",\"type\":\"string\",");
  jsonMqtt.append(std::string("\"value\" : \""));
  jsonMqtt.append("234");
  jsonMqtt.append(std::string("\"}"));


  actual.assign(cbPublish->publishContextBroker(jsonMqtt, apikey, device));
  boost::trim(actual);
  boost::erase_all(actual, "\n");
  boost::erase_all(actual, "\r");

  std::cout << "EXPECTED:" << expected <<  std::endl;
  std::cout << "TEST RESULT:" << actual << std::endl;



  CPPUNIT_ASSERT(actual.find(expected) != std::string::npos);
  CPPUNIT_ASSERT(
    actual.find("\"metadatas\":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":")
    != std::string::npos);
  CPPUNIT_ASSERT(
    actual.find("{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":") !=
    std::string::npos);
  cb_mock.stop();

  std::cout << "Test CBPublisher done!" << std::endl;
}

void MqttTest::testCBPublisherMissingApikey() {
  std::string actual;
  std::string device = "dev01";
  std::string apikey;
  std::string jsonMqtt = "";




  CPPUNIT_ASSERT_THROW(actual.assign(cbPublish->publishContextBroker(jsonMqtt,
                                     apikey, device)), std::runtime_error);
  std::cout << "Test MqttService missing apikey" <<  std::endl;
}

void MqttTest::testCBPublisherMissingIDdevice() {
  std::string actual;
  std::string apikey = "1234";
  std::string device;
  std::string jsonMqtt = "";



  CPPUNIT_ASSERT_THROW(actual.assign(cbPublish->publishContextBroker(jsonMqtt,
                                     apikey, device)), std::runtime_error);
  std::cout << "Test MqttService missing ID Device" << std::endl;
}

void MqttTest::testReceivedMqtt() {
  std::cout << "Test Receive Mqtt Message starting .... " << std::endl;
  HttpMock cb_mock(1026, "/NGSI10/updateContext");
  cb_mock.init();
  std::string jsonMqtt;
  //Test mqtt message is built at stubLoopToOnMessage method
  mqtt_alias.assign("te");
  mqtt_payload.assign("23");
  mqtt_apikey.assign("1234");
  mqtt_device.assign("dev01");


  //Expected
  jsonMqtt.append(std::string("{\"name\" : \""));
  jsonMqtt.append(mqtt_alias);
  jsonMqtt.append("\",\"type\":\"string\",");
  jsonMqtt.append(std::string("\"value\" : \""));
  jsonMqtt.append(mqtt_payload);
  jsonMqtt.append(std::string("\"}"));


  mockMosquitto = new MockMosquitto();
  mockPublisher = new MockIotaMqttService();



  defineExpectationsMqttt();


  EXPECT_CALL(*mockPublisher, doPublishCB(StrEq(mqtt_apikey), StrEq(mqtt_device),
                                          StrEq(jsonMqtt))).WillOnce(Return(std::string("OK")));

  std::string sensorfile("../../tests/iotagent/sensormqtt-json.xml");
  std::string logPath("./");


  //TEST
  mqttService->initESPLib(logPath, sensorfile);
  delete(cbPublish);  //I'm going to use the mock.
  mqttService->setIotaMqttService(mockPublisher);

  mqttService->startESP();

  std::cout << "Sensor Started" << std::endl;
  int idsensor = mqttService->idsensor;


  //Finishing
  SLEEP(100);
  iota::esp::MqttService::getESPLib()->stopSensor(idsensor);
  std::cout << "Sensor Stopping... " << std::endl;

  CPPUNIT_ASSERT(idsensor > 0);
  std::cout << "Test Receive MQTT message completed!" << std::endl;
  cb_mock.stop();
  //delete mqttService;


}


void MqttTest::testBadEntityType() {
  //entity type should be taken from Configuration.
  //cb_mock.init();
  HttpMock cb_mock(1026, "/NGSI10/updateContext");
  cb_mock.init();


  std::string entity;

  std::string expected = "";

  const iota::JsonValue& service_object =
    iota::Configurator::instance()->getServicebyApiKey("/iot/mqtt", "1234");

  if (service_object.HasMember("entity_type")) {
    entity.assign(service_object["entity_type"].GetString());
  }


  expected.append("{\"updateAction\":\"APPEND\",\"contextElements\":");
  expected.append("[{\"id\":\"thing:dev01\",\"type\":\"");
  expected.append(entity);
  expected.append("\",\"isPattern\":\"false\",\"attributes\"");
  expected.append(":[{\"name\":\"te\",\"type\":\"string\",\"value\":\"234\",\"metadatas\"");



  std::string actual = "";
  std::string apikey = "1234";
  std::string device = "dev01";

  std::string jsonMqtt = "";
  jsonMqtt.append(std::string("{\"name\" : \""));
  jsonMqtt.append("te");
  jsonMqtt.append("\",\"type\":\"string\",");
  jsonMqtt.append(std::string("\"value\" : \""));
  jsonMqtt.append("234");
  jsonMqtt.append(std::string("\"}"));




  actual.assign(cbPublish->publishContextBroker(jsonMqtt, apikey, device));
  boost::trim(actual);
  boost::erase_all(actual, "\n");
  boost::erase_all(actual, "\r");

  std::cout << "EXPECTED:" << expected <<  std::endl;
  std::cout << "TEST RESULT:" << actual << std::endl;



  CPPUNIT_ASSERT(actual.find(expected) != std::string::npos);
  CPPUNIT_ASSERT(
    actual.find("\"metadatas\":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":")
    != std::string::npos);
  CPPUNIT_ASSERT(
    actual.find("{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":") !=
    std::string::npos);
  cb_mock.stop();

}

void MqttTest::testNotInitCBPublisher() {

  std::cout << "Test NotInitCBPublisher starting... " << std::endl;
  std::string jsonMqtt;
  //Test mqtt message is built at stubLoopToOnMessage method
  mqtt_alias.assign("te");
  mqtt_payload.assign("23");
  mqtt_apikey.assign("1234");
  mqtt_device.assign("dev01");


  //Expected
  jsonMqtt.append(std::string("{\"name\" : \""));
  jsonMqtt.append(mqtt_alias);
  jsonMqtt.append("\",\"type\":\"string\",");
  jsonMqtt.append(std::string("\"value\" : \""));
  jsonMqtt.append(mqtt_payload);
  jsonMqtt.append(std::string("\"}"));

  mockMosquitto = new MockMosquitto();


  MockPluginOutput* mockOutput = new MockPluginOutput();


  //Test will fail if this output is not called.
  EXPECT_CALL(*mockOutput, execute(_, _, _)).WillOnce(Invoke(this,
      &MqttTest::stubExecute)).WillRepeatedly(Return(true));


  defineExpectationsMqttt();





  std::string config("ConfigFile");
  std::string value("../../tests/iotagent/MqttService.xml");
  mqttService->set_option(config, value);


  std::map<int, ESP_Sensor*>::iterator it =
    mqttService->getESPLib()->sensors.find(mqttService->idsensor);
  ESP_Sensor* mySensor;

  if (it != mqttService->getESPLib()->sensors.end()) {
    std::cout << "Found a Sensor..." << std::endl;
    mySensor = (ESP_Sensor*) it->second;


    //Hotswapping of output plugin
    //but before getting rid of it, I get all the postprocessors it has.
    for (int i = 0; i < mySensor->outputs.size(); i++) {
      for (std::vector<ESP_Postprocessor_Base* >::iterator it =
             mySensor->outputs[i]->postprocessors.begin();
           it != mySensor->outputs[i]->postprocessors.end(); it++) {
        mockOutput->postprocessors.push_back((*it));

      }

      delete mySensor->outputs[i];
    }
    //Delete existing...
    mySensor->outputs.clear();
    //add my mock object
    mySensor->outputs.push_back(mockOutput);

  }


  //First check, is ESP lib properly initialized?
  CPPUNIT_ASSERT(mqttService->idsensor > 0);


  std::cout << "Test NotInitCBPublisher ESP Starting ... " << std::endl;


  //MqttService plugin can now start.
  mqttService->start();
  //Expected calls should be made here.
  std::cout << "Test NotInitCBPublisher Waiting for Sensors to terminate ... " <<
            std::endl;
  SLEEP(1000);//Allow some time to process request
  std::cout << " Test NoInitCBPublisher completed" << std::endl;


 // delete mqttService;




}

void MqttTest::testBadXMLConfigOutputIoT() {

  HttpMock cb_mock(1026, "/NGSI10/updateContext");
  cb_mock.init();
  std::string jsonMqtt;
  //Test mqtt message is built at stubLoopToOnMessage method
  std::cout << "Test BAD XML Config Output IoT " << std::endl;
  mqtt_alias.assign("te");
  mqtt_payload.assign("23");
  mqtt_apikey.assign("1234");
  mqtt_device.assign("dev01");


  //Expected
  jsonMqtt.append(std::string("{\"name\" : \""));
  jsonMqtt.append(mqtt_alias);
  jsonMqtt.append("\",\"type\":\"string\",");
  jsonMqtt.append(std::string("\"value\" : \""));
  jsonMqtt.append(mqtt_payload);
  jsonMqtt.append(std::string("\"}"));

  mockMosquitto = new MockMosquitto();
  mockPublisher = new MockIotaMqttService();





  defineExpectationsMqttt();

  //This will fail if no call is made.
  EXPECT_CALL(*mockPublisher, doPublishCB( StrEq(mqtt_apikey),
                                          StrEq(mqtt_device),StrEq(jsonMqtt))).WillOnce(Return(std::string("OK")));



  std::string sensorfile("../../tests/iotagent/sensormqtt-bad-output.xml");
  std::string logPath("./");


  //TEST

  mqttService->initESPLib(logPath, sensorfile);
  std::cout << "initESPLib, adding mockPublisher" << std::endl;
  mqttService->setIotaMqttService(mockPublisher);
  delete cbPublish;
  mqttService->startESP();

  std::cout << "Sensor Started" << std::endl;
  int idsensor = mqttService->idsensor;

  //Finishing
  SLEEP(100);
  iota::esp::MqttService::getESPLib()->stopSensor(idsensor);
  std::cout << "Sensor Stopping... " << std::endl;

  CPPUNIT_ASSERT(idsensor > 0);
  std::cout << "Test BAD XML Config Output IoT completed!" << std::endl;
  //delete mqttService;
  cb_mock.stop();

}

void MqttTest::testLongAliasesNotWorking() {

  HttpMock cb_mock(1026, "/NGSI10/updateContext");
  cb_mock.init();
  std::string jsonMqtt;
  //Test mqtt message is built at stubLoopToOnMessage method
  std::cout << "Test LongAliases not Working Starting..." << std::endl;
  mqtt_alias.assign("temperature");
  mqtt_payload.assign("23");
  mqtt_apikey.assign("1234");
  mqtt_device.assign("dev01");


  //Expected
  jsonMqtt.append(std::string("{\"name\" : \""));
  jsonMqtt.append(mqtt_alias);
  jsonMqtt.append("\",\"type\":\"string\",");
  jsonMqtt.append(std::string("\"value\" : \""));
  jsonMqtt.append(mqtt_payload);
  jsonMqtt.append(std::string("\"}"));


  mockPublisher = new MockIotaMqttService();
  mockMosquitto = new MockMosquitto();

  defineExpectationsMqttt();

  //This will fail if no call is made.
  EXPECT_CALL(*mockPublisher, doPublishCB(StrEq(mqtt_apikey),
                                          StrEq(mqtt_device),StrEq(jsonMqtt))).WillOnce(Return(std::string("OK")));


  std::string sensorfile("../../tests/iotagent/sensormqtt-bad-long-alias.xml");
  std::string logPath("./");


  //TEST
  mqttService->initESPLib(logPath, sensorfile);
  mqttService->setIotaMqttService(mockPublisher);
  delete cbPublish;

  mqttService->startESP();
  std::cout << "Sensor Started" << std::endl;
  int idsensor = mqttService->idsensor;

  //Finishing
  SLEEP(100);
  iota::esp::MqttService::getESPLib()->stopSensor(idsensor);
  std::cout << "Sensor Stopping... " << std::endl;

  CPPUNIT_ASSERT(idsensor > 0);
  std::cout << "Test Long Aliases not Working  completed!" << std::endl;
  cb_mock.stop();

  //delete mqttService;

}

void MqttTest::testMultipleMeasures() {
  /*
  This test is for a bug that was raised about multi-measures being sent as
  one single measure with an non-existing alias (mul20), so the thing is that
  the expected behaviour is:
  - multi-measures will be decomposed into different and individual CB publications,
  - Timestamp will be generated per publication, so it may be different on each one.
  - NOTE: CB doesn't accept same attribute more than once per publication. Hence
  */
  HttpMock cb_mock(1026, "/NGSI10/updateContext");
  cb_mock.init();
  std::cout << "TEST: testMultipleMeasures starting... " << std::endl;
  mqtt_alias.assign("mul20");
  mqtt_payload.assign("t|23#t|34#pres|23.5#h|55");
  mqtt_apikey.assign("1234");
  mqtt_device.assign("dev01");

  mockMosquitto = new MockMosquitto();
  mockPublisher = new MockIotaMqttService();



  defineExpectationsMqttt();

  std::string jsonMqtt1, jsonMqtt2, jsonMqtt3, jsonMqtt4;
  //Expected
  jsonMqtt1.append(std::string("{\"name\" : \"t\""));
  jsonMqtt1.append(",\"type\":\"string\",");
  jsonMqtt1.append(std::string("\"value\" : \"23\""));
  jsonMqtt1.append(std::string("}"));

  jsonMqtt2.append(std::string("{\"name\" : \"t\""));
  jsonMqtt2.append(",\"type\":\"string\",");
  jsonMqtt2.append(std::string("\"value\" : \"34\""));
  jsonMqtt2.append(std::string("}"));

  jsonMqtt3.append(std::string("{\"name\" : \"pres\""));
  jsonMqtt3.append(",\"type\":\"string\",");
  jsonMqtt3.append(std::string("\"value\" : \"23.5\""));
  jsonMqtt3.append(std::string("}"));


  jsonMqtt4.append(std::string("{\"name\" : \"h\""));
  jsonMqtt4.append(",\"type\":\"string\",");
  jsonMqtt4.append(std::string("\"value\" : \"55\""));
  jsonMqtt4.append(std::string("}"));

  //4 calls need to be made to this service.
  EXPECT_CALL(*mockPublisher, doPublishCB(StrEq(mqtt_apikey),StrEq(mqtt_device),StrEq(jsonMqtt1))).WillOnce(Return(std::string("OK")));
  EXPECT_CALL(*mockPublisher, doPublishCB(StrEq(mqtt_apikey),StrEq(mqtt_device),StrEq(jsonMqtt2))).WillOnce(Return(std::string("OK")));
  EXPECT_CALL(*mockPublisher, doPublishCB(StrEq(mqtt_apikey),StrEq(mqtt_device),StrEq(jsonMqtt3))).WillOnce(Return(std::string("OK")));
  EXPECT_CALL(*mockPublisher, doPublishCB(StrEq(mqtt_apikey),StrEq(mqtt_device),StrEq(jsonMqtt4))).WillOnce(Return(std::string("OK")));


  std::string sensorfile("../../tests/iotagent/sensormqtt-json.xml");
  std::string logPath("./");


  //TEST

  mqttService->initESPLib(logPath, sensorfile);

  std::cout << "TEST: testMultipleMeasures  SENSORFILE LOADED " << std::endl;
  mqttService->setIotaMqttService(mockPublisher);
  delete cbPublish;

  mqttService->startESP();
  std::cout << "Sensor Started" << std::endl;
  int idsensor = mqttService->idsensor;

  //Finishing
  SLEEP(100);
  iota::esp::MqttService::getESPLib()->stopSensor(idsensor);
  std::cout << "Sensor Stopping... " << std::endl;


  CPPUNIT_ASSERT(idsensor > 0);
  std::cout << "TEST: testMultipleMeasures DONE " << std::endl;
  //delete mqttService;
  cb_mock.stop();
}

/**
 @brief this test will check that the call to doRequestCommands is done
 at IotaMqttService level.
*/
void MqttTest::testGetAllCommandsPolling() {

  std::cout << "TEST: testGetAllCommandsPolling starting... " << std::endl;
  mqtt_alias.assign("cmdget");
  mqtt_payload.assign("");
  mqtt_apikey.assign("1234");
  mqtt_device.assign("dev01");

  mockMosquitto = new MockMosquitto();
  mockPublisher = new MockIotaMqttService();

  //Simulates that a mqtt message comes with fields defined above.
  defineExpectationsMqttt();

  std::string apikeyExpected = "1234";
  std::string deviceExpected = "dev01";



  //Check input parameters. It will fail the test if not called
  EXPECT_CALL(*mockPublisher, doRequestCommands(StrEq(apikeyExpected),
              StrEq(deviceExpected))).WillRepeatedly(Return());
  //as there are two post-processors, it will called more than once.

  std::string sensorfile("../../tests/iotagent/sensormqtt-json.xml");
  std::string logPath("./");


  //TEST

  mqttService->initESPLib(logPath, sensorfile);

  std::cout << "TEST: testGetAllCommandsPolling  SENSORFILE LOADED " << std::endl;
  mqttService->setIotaMqttService(mockPublisher);
  delete cbPublish;
  mqttService->startESP();
  std::cout << "Sensor Started" << std::endl;
  int idsensor = mqttService->idsensor;

  //Finishing
  SLEEP(100);
  iota::esp::MqttService::getESPLib()->stopSensor(idsensor);
  std::cout << "Sensor Stopping... " << std::endl;


  CPPUNIT_ASSERT(idsensor > 0);
  std::cout << "TEST: testGetAllCommandsPolling DONE " << std::endl;

  //delete mqttService;

}

/**
 @brief This test will insert a command for a particular device (provisioned on device.json for MQTT)
 and later, the mock will simulate the arrival of a MQTT message requesting all commands, so MqttService
 will have to return it and later on, it will execute it. The call to publish on the mock will prove
 that the end-to-end flow has been completed.
*/
void MqttTest::testPollingOneMqttCommand() {

  std::cout << "TEST: testPollingOneMqttCommand Starting...  " << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);

  //Command to get processed.
  std::string querySTR = "";
  std::string bodySTR = "{\"updateAction\":\"UPDATE\",";
  bodySTR.append("\"contextElements\":[{\"id\":\"dev_mqtt\",\"type\":\"type2\",\"isPattern\":\"false\",");
  bodySTR.append("\"attributes\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"dev_mqtt@ping6|22\",");
  bodySTR.append("\"metadatas\":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"2014-11-23T17:33:36.341305Z\"}]}");
  bodySTR.append("]} ]}");

  pion::http::request_ptr http_request(new
                                       pion::http::request("/iot/ngsi/mqtt/updateContext"));
  http_request->set_method("POST");
  http_request->add_header(iota::types::FIWARE_SERVICE, "service2");
  http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/ssrv2");
  http_request->set_query_string(querySTR);
  http_request->set_content(bodySTR);

  std::map<std::string, std::string> url_args;
  std::multimap<std::string, std::string> query_parameters;
  pion::http::response http_response;
  std::string response;

  std::cout <<
            "TEST: testPollingOneMqttCommand Inserting command into Iotagent MqttService...  "
            << std::endl;
  //Command insertion
  mqttService->op_ngsi(http_request, url_args, query_parameters,
                       http_response, response);

  //we could check the response first.
  std::cout <<
            "TEST: testPollingOneMqttCommand Inserting command into Iotagent MqttService...  DONE"
            << std::endl;
  std::cout << "TEST: testPollingOneMqttCommand Inserting command RESPONSE:   " <<
            response << std::endl;

  //MQTT Message to request commands.
  mqtt_alias.assign("cmdget");
  mqtt_payload.assign("");
  mqtt_apikey.assign("1234");
  mqtt_device.assign("dev_mqtt");

  mockMosquitto = new MockMosquitto();

  //Simulates that a mqtt message comes with fields defined above.
  defineExpectationsMqttt();


  std::string sensorfile("../../tests/iotagent/sensormqtt-mqttwriter.xml");
  std::string logPath("./");


  mockMosquittoPub = new MockMosquitto();
  //Definition of expectations for Mqtt publisher mock
  defineExpectationsMqttPublisher();



  //Expect call for publishing the command.

  EXPECT_CALL(*mockMosquittoPub, mqttPublish(_, _, _, _, _,
              _)).WillOnce(Invoke(this,
                                  &MqttTest::stubPublish)); //ASSERT is within stubPublish.

  EXPECT_CALL(*mockMosquittoPub, mqttDisconnect()).WillOnce(Return(0));

  std::cout << "TEST: testPollingOneMqttCommand loading ESP...  " << std::endl;
  mqttService->initESPLib(logPath, sensorfile);

  std::cout << "TEST: testPollingOneMqttCommand ESP Loaded  " << std::endl;
  mqttService->setIotaMqttService(cbPublish);

  mqttService->startESP();
  std::cout << "TEST: testPollingOneMqttCommand Sensor started  " << std::endl;


  int idsensor = mqttService->idsensor;
  SLEEP(1000);
  std::cout << "TEST: testPollingOneMqttCommand Stopping sensor...  " <<
            std::endl;
  iota::esp::MqttService::getESPLib()->stopSensor(idsensor);
  std::cout << "Sensor Stopping... " << std::endl;


  CPPUNIT_ASSERT(idsensor > 0);

  cb_mock->stop();

  //delete mqttService;
  std::cout << "TEST: testPollingOneMqttCommand DONE  " << std::endl;
}



/**
This test will simulate the execution of a command on a MQTT device and the later
response sent back to IotAgent. The entry point is the "execute_command".
*/
void MqttTest::testPollingCommandExecution() {

  std::cout << "TEST: testPollingCommandExecution Starting...  " << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);

  //Command to get processed.
  std::string querySTR = "";
  std::string bodySTR = "{\"updateAction\":\"UPDATE\",";
  bodySTR.append("\"contextElements\":[{\"id\":\"dev_mqtt\",\"type\":\"type2\",\"isPattern\":\"false\",");
  bodySTR.append("\"attributes\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"dev_mqtt@ping6|22\",");
  bodySTR.append("\"metadatas\":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"2014-11-23T17:33:36.341305Z\"}]}");
  bodySTR.append("]} ]}");

  pion::http::request_ptr http_request(new
                                       pion::http::request("/iot/ngsi/mqtt/updateContext"));
  http_request->set_method("POST");
  http_request->add_header(iota::types::FIWARE_SERVICE, "service2");
  http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/ssrv2");
  http_request->set_query_string(querySTR);
  http_request->set_content(bodySTR);

  std::map<std::string, std::string> url_args;
  std::multimap<std::string, std::string> query_parameters;
  pion::http::response http_response;
  std::string response;

  std::cout <<
            "TEST: testPollingCommandExecution Inserting command into Iotagent MqttService...  "
            << std::endl;
  //Command insertion
  mqttService->op_ngsi(http_request, url_args, query_parameters,
                       http_response, response);

  //we could check the response first.

  std::cout <<
            "TEST: testPollingCommandExecution Inserting command into Iotagent MqttService...  DONE"
            << std::endl;
  std::cout << "TEST: testPollingCommandExecution Inserting command RESPONSE:   "
            << response << std::endl;

  cb_mock->get_last(); //This will delete the updateContext on the mock

  //I need to know the cmdid.

  boost::property_tree::ptree service_ptree;
  std::string apikey("1234");
  std::string device("dev_mqtt");

  mqttService->get_service_by_apiKey(service_ptree, apikey);


  std::string srv = service_ptree.get<std::string>(iota::store::types::SERVICE, "");
  std::string srv_path = service_ptree.get<std::string>(iota::store::types::SERVICE_PATH
                         , "");

  boost::shared_ptr<iota::Device> dev = mqttService->get_device(device, srv, srv_path);

  iota::CommandVect all_commands =  mqttService->get_all_command(dev, service_ptree);

  CPPUNIT_ASSERT(all_commands.size() == 1);

  std::string cmd_id = all_commands[0]->get_id();

  //MQTT Message for command execution.
  mqtt_alias.assign("cmdexe");
  mqtt_payload.assign("cmdid|");
  mqtt_payload.append(cmd_id);
  mqtt_payload.append("#result|ok");
  mqtt_apikey.assign("1234");
  mqtt_device.assign("dev_mqtt");

  mockMosquitto = new MockMosquitto();

  //Simulates that a mqtt message comes with fields defined above.
  defineExpectationsMqttt();


  std::string sensorfile("../../tests/iotagent/sensormqtt-json.xml");
  std::string logPath("./");

  std::cout << "TEST: testPollingOneMqttCommand loading ESP...  " << std::endl;
  mqttService->initESPLib(logPath, sensorfile);

  std::cout << "TEST: testPollingOneMqttCommand ESP Loaded  " << std::endl;
  mqttService->setIotaMqttService(cbPublish);

  mqttService->startESP();
  std::cout << "TEST: testPollingOneMqttCommand Sensor started  " << std::endl;


  int idsensor = mqttService->idsensor;
  SLEEP(1000);
  std::cout << "TEST: testPollingOneMqttCommand Stopping sensor...  " <<
            std::endl;
  iota::esp::MqttService::getESPLib()->stopSensor(idsensor);
  std::cout << "Sensor Stopping... " << std::endl;


  CPPUNIT_ASSERT(idsensor > 0);



  all_commands =  mqttService->get_all_command(dev, service_ptree);
  std::cout << "TEST: testPollingOneMqttCommand Checking commands ...  # " <<
            all_commands.size() << std::endl;
  CPPUNIT_ASSERT(all_commands.size() == 0);
  //cb_mock->get_last(); //for checking status

  cb_mock->stop();

  //delete mqttService;
  std::cout << "TEST: testPollingOneMqttCommand DONE  " << std::endl;


}

void MqttTest::testExtractingCmdId() {

  std::string expected_id = "234as329890sfs";
  std::string expected_payload = "dev@ping#other|value#";

  std::string cmd_beginning("cmdid|");
  cmd_beginning.append(expected_id);
  cmd_beginning.append("#dev@ping#other|value#");

  std::string cmd_ending("dev@ping#other|value#cmdid|");
  cmd_ending.append(expected_id);
  cmd_ending.append("#");

  std::string cmd_missing("dev@ping#other|value");

  std::cout << "TEST: testExtractingCmdId Starting..." << std::endl;

  iota::esp::ngsi::IotaMqttServiceImpl* iotaService = new
  iota::esp::ngsi::IotaMqttServiceImpl("");

  std::string actual_id;
  std::string actual_payload;

  iotaService->extract_command_id(cmd_beginning, actual_payload, actual_id);

  CPPUNIT_ASSERT_EQUAL(expected_id, actual_id);
  CPPUNIT_ASSERT_EQUAL(expected_payload, actual_payload);

  std::cout << "TEST: testExtractingCmdId cmd id at the start... DONE" <<
            std::endl;

  iotaService->extract_command_id(cmd_ending, actual_payload, actual_id);

  CPPUNIT_ASSERT_EQUAL(expected_id, actual_id);
  CPPUNIT_ASSERT_EQUAL(expected_payload, actual_payload);

  std::cout << "TEST: testExtractingCmdId cmd id at the end... DONE" << std::endl;


  //CPPUNIT_ASSERT_THROW(iotaService->extract_command_id(cmd_missing,actual_payload,actual_id),iota::IotaException);
  //std::cout << "TEST: testExtractingCmdId missing cmd id... DONE" << std::endl;

}

void MqttTest::testPushCommandExecution() {
  std::cout << "TEST: testPushCommandExecution Starting...  " << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);

  //Command to get processed.
  std::string querySTR = "";
  std::string bodySTR = "{\"updateAction\":\"UPDATE\",";
  bodySTR.append("\"contextElements\":[{\"id\":\"dev_mqtt_push\",\"type\":\"type2\",\"isPattern\":\"false\",");
  bodySTR.append("\"attributes\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"dev_mqtt_push@ping6|22\",");
  bodySTR.append("\"metadatas\":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"2014-11-23T17:33:36.341305Z\"}]}");
  bodySTR.append("]} ]}");

  pion::http::request_ptr http_request(new
                                       pion::http::request("/iot/ngsi/mqtt/updateContext"));
  http_request->set_method("POST");
  http_request->add_header(iota::types::FIWARE_SERVICE, "service2");
  http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/ssrv2");
  http_request->set_query_string(querySTR);
  http_request->set_content(bodySTR);

  std::map<std::string, std::string> url_args;
  std::multimap<std::string, std::string> query_parameters;
  pion::http::response http_response;
  std::string response;


  std::string sensorfile("../../tests/iotagent/sensormqtt-mqttwriter.xml");
  std::string logPath("./");


  std::cout << "TEST: testPushCommandExecution loading ESP...  " << std::endl;
  mqttService->initESPLib(logPath, sensorfile);

  mockMosquitto = new MockMosquitto();

  //Simulates that a mqtt message comes with fields defined above.
  defineExpectationsMqttNoIncomingMsg();

  //Definition of expectations for Mqtt publisher mock
   mockMosquittoPub = new MockMosquitto();
  defineExpectationsMqttPublisher();

  //Expect call for publishing the command.

  EXPECT_CALL(*mockMosquittoPub, mqttPublish(_, _, _, _, _,
              _)).WillOnce(Invoke(this,
                                  &MqttTest::stubPublishPush)); //ASSERT is within stubPublishPub.

  EXPECT_CALL(*mockMosquittoPub, mqttDisconnect()).WillOnce(Return(0));


  std::cout << "TEST: testPushCommandExecution ESP Loaded  " << std::endl;
  mqttService->setIotaMqttService(cbPublish);

  mqttService->startESP();
  std::cout << "TEST: testPushCommandExecution Sensor started  " << std::endl;

  std::cout <<
            "TEST: testPushCommandExecution Inserting command into Iotagent MqttService...  "
            << std::endl;
  //Command insertion
  mqttService->op_ngsi(http_request, url_args, query_parameters,
                       http_response, response);

  //Checking command just inserted

  boost::property_tree::ptree service_ptree;
  std::string apikey("1234");
  std::string device("dev_mqtt_push");

  mqttService->get_service_by_apiKey(service_ptree, apikey);


  std::string srv = service_ptree.get<std::string>(iota::store::types::SERVICE, "");
  std::string srv_path = service_ptree.get<std::string>(iota::store::types::SERVICE_PATH
                         , "");

  boost::shared_ptr<iota::Device> dev = mqttService->get_device(device, srv, srv_path);
  iota::CommandVect all_commands =  mqttService->get_all_command(dev, service_ptree);
  std::cout << "TEST: testPushCommandExecution Checking commands ...  # " <<
            all_commands.size() << std::endl;
  //no hay comandos en la cache, porque el comando se ha pasado a DELIVERED
  //ya que ha recibido un 202
  CPPUNIT_ASSERT(all_commands.size() == 0);


  std::cout <<
            "TEST: testPushCommandExecution Inserting command into Iotagent MqttService...  DONE"
            << std::endl;
  std::cout << "TEST: testPushCommandExecution Inserting command RESPONSE:   " <<
            response << std::endl;



  int idsensor = mqttService->idsensor;
  SLEEP(1000);
  std::cout << "TEST: testPushCommandExecution Stopping sensor...  " << std::endl;

  std::cout << "Sensor Stopping... " << std::endl;

  CPPUNIT_ASSERT(idsensor > 0);
  cb_mock->stop();

  //delete mqttService;

  std::cout << "TEST: testPushCommandExecution DONE  " << std::endl;
}

void MqttTest::testPushCommandResponse() {
  /**
  This is the response to a previously executed command.
  */

  std::cout << "TEST: testPushCommandResponse Starting...  " << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);


  //Let's create and store a command like CommandHandle would do it
  std::string apikey("1234");
  std::string device("dev_mqtt_push");

  boost::property_tree::ptree service_ptree;


  mqttService->get_service_by_apiKey(service_ptree, apikey);


  std::string srv = service_ptree.get<std::string>(iota::store::types::SERVICE, "");
  std::string srv_path = service_ptree.get<std::string>(iota::store::types::SERVICE_PATH
                         , "");

  boost::shared_ptr<iota::Device> dev = mqttService->get_device(device, srv, srv_path);

  boost::property_tree::ptree command_to_send;
  std::string cmdname("PING");
  std::string cmd_id;
  std::string cmd_value("dev_mqtt_push@ping6|22");
  std::string sequence;

  mqttService->getCommandLine(cmdname, cmd_value, sequence, dev, service_ptree,
                              cmd_id, command_to_send);


  //Now, let's assume the command was inserted and sent to the MQTT broker.
  std::string entity(dev->_entity_type);
  mqttService->save_command(cmdname, cmd_id, 20, command_to_send, dev, entity,
                            dev->_endpoint, service_ptree, sequence, iota::types::READY_FOR_READ);

  //Maybe this is redundant,
  iota::CommandVect all_commands =  mqttService->get_all_command(dev, service_ptree);

  CPPUNIT_ASSERT(all_commands.size() == 1);


  //MQTT Message for command execution.
  mqtt_alias.assign("cmdexe/PING");
  mqtt_payload.assign("cmdid|");
  mqtt_payload.append(cmd_id);
  mqtt_payload.append("#result|ok");
  mqtt_apikey.assign("1234");
  mqtt_device.assign("dev_mqtt_push");

  mockMosquitto = new MockMosquitto();

  //Simulates that a mqtt message comes with fields defined above.
  defineExpectationsMqttt();


  std::string sensorfile("../../tests/iotagent/sensormqtt-json.xml");
  std::string logPath("./");

  std::cout << "TEST: testPushCommandResponse loading ESP...  " << std::endl;
  mqttService->initESPLib(logPath, sensorfile);

  std::cout << "TEST: testPushCommandResponse ESP Loaded  " << std::endl;
  mqttService->setIotaMqttService(cbPublish);

  mqttService->startESP();
  std::cout << "TEST: testPushCommandResponse Sensor started  " << std::endl;

  int idsensor = mqttService->idsensor;

  CPPUNIT_ASSERT(idsensor > 0);


  all_commands =  mqttService->get_all_command(dev, service_ptree);
  std::cout << "TEST: testPushCommandResponse Checking commands ...  # " <<
            all_commands.size() << std::endl;
  CPPUNIT_ASSERT(all_commands.size() == 0);
  //cb_mock->get_last(); //for checking status

  cb_mock->stop();
  SLEEP(1000);
  std::cout << "TEST: testPushCommandResponse Stopping sensor...  " << std::endl;
  //delete mqttService;
  std::cout << "TEST: testPushCommandResponse DONE  " << std::endl;
}


/**
This test will simulate the arrival of a cmd, which is never intended for the iotagent but for the device, and it can be
seen as an echo of the MQTT Broker (because we are subscribed to ALL topics). Expected result is ignore it.
*/
void MqttTest::testPostprocessorJSON_IoTOutput_cmd(){

std::cout << "TEST: testPostprocessorJSON_IoTOutput_cmd starting... " << std::endl;
  mqtt_alias.assign("cmd/PING");
  mqtt_payload.assign("cmdid|a234i923sfj2342fsfs");
  mqtt_apikey.assign("1234");
  mqtt_device.assign("dev01");

  mockMosquitto = new MockMosquitto();
  mockPublisher = new MockIotaMqttService();

  //Simulates that a mqtt message comes with fields defined above.
  defineExpectationsMqttt();

  std::string apikeyExpected = "1234";
  std::string deviceExpected = "dev01";



  //MAKE sure NO METHOD is called.
  EXPECT_CALL(*mockPublisher, doRequestCommands(_,_)).Times(0);
  EXPECT_CALL(*mockPublisher, doPublishCB(_,_,_)).Times(0);
  EXPECT_CALL(*mockPublisher, processCommandResponse(_,_,_)).Times(0);

  //as there are two post-processors, it will called more than once.

  std::string sensorfile("../../tests/iotagent/sensormqtt-json.xml");
  std::string logPath("./");


  //TEST

  mqttService->initESPLib(logPath, sensorfile);

  std::cout << "TEST: testPostprocessorJSON_IoTOutput_cmd  SENSORFILE LOADED " << std::endl;
  mqttService->setIotaMqttService(mockPublisher);
  delete cbPublish;
  mqttService->startESP();
  std::cout << "Sensor Started" << std::endl;
  int idsensor = mqttService->idsensor;

  //Finishing
  SLEEP(100);

  std::cout << "Sensor Stopping... " << std::endl;


  CPPUNIT_ASSERT(idsensor > 0);
  std::cout << "TEST: testPostprocessorJSON_IoTOutput_cmd DONE " << std::endl;

  //delete mqttService;


}

void MqttTest::testLocationContextBroker(){
  std::cout << "testLocationContextBroker  starting .... " << std::endl;

  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);

  std::string entity;



  std::string expected = "";

  const iota::JsonValue& service_object =
    iota::Configurator::instance()->getServicebyApiKey("/iot/mqtt", "1234");

  if (service_object.HasMember("entity_type")) {
    entity.assign(service_object["entity_type"].GetString());
  }


  expected.append("{\"updateAction\":\"APPEND\",\"contextElements\":");
  expected.append("[{\"id\":\"room_uttest\",\"type\":\"");
  expected.append("thing");
  expected.append("\",\"isPattern\":\"false\",\"attributes\"");
  expected.append(":[{\"name\":\"position\",\"type\":\"coords\",\"value\":\"23.2234,33.23424243\",\"metadatas\"");
  expected.append(":[{\"name\":\"location\",\"type\":\"string\",\"value\":\"WGS84\"}");




  std::string actual = "";
  std::string apikey = "1234";
  std::string device = "unitTest_mqtt_location";

  std::string jsonMqtt = "";
  jsonMqtt.append(std::string("{\"name\" : \""));
  jsonMqtt.append("l");
  jsonMqtt.append("\",\"type\":\"string\",");
  jsonMqtt.append(std::string("\"value\" : \""));
  jsonMqtt.append("23.2234/33.23424243");
  jsonMqtt.append(std::string("\"}"));


  actual.assign(cbPublish->publishContextBroker(jsonMqtt, apikey, device));
  boost::trim(actual);
  boost::erase_all(actual, "\n");
  boost::erase_all(actual, "\r");

  std::cout << "EXPECTED:" << expected <<  std::endl;
  std::cout << "TEST RESULT:" << actual << std::endl;



  CPPUNIT_ASSERT(actual.find(expected) != std::string::npos);
  CPPUNIT_ASSERT(
    actual.find(expected) != std::string::npos);



  cb_mock->stop();
  std::cout << "Test testLocationContextBroker done!" << std::endl;

}


int MqttTest::stubReadClient(int id, char* buffer, int len) {
  if (buffer == NULL) {
    return 0;
  }
  else {
    //COPY

    memcpy(buffer, marshalled_Mqtt.c_str(), marshalled_Mqtt.length());
    return marshalled_Mqtt.length();
  }
}

//Just checking that CBPublisher is not null as of now
bool MqttTest::stubExecute(CC_AttributesType* attributes,
                           ESP_Postprocessor_Base* postprocessor, std::map<std::string, void*> userData) {

  std::map<std::string, void*>::iterator it =
    userData.find("contextBrokerPublisher");

  CPPUNIT_ASSERT(it != userData.end());

  if (it != userData.end()) {

    std::cout << "Test Execute: checking CBPublisher" << std::endl;
    CPPUNIT_ASSERT(it->second != NULL);
  }

  return true;
}

//Callback needed for ESP to connect to MQTT
int MqttTest::stubConnect(const char* host, int port, int keepalive) {
  mockMosquitto->callBack->on_connect(1);
  return 0;
}

int MqttTest::stubConnectPub(const char* host, int port, int keepalive) {
  mockMosquittoPub->callBack->on_connect(1);
  return 0;
}

/**
It defines mqtt message
*/
int MqttTest::stubLoopToOnMessage(int, int) {
  std::string topic;
  topic.append(mqtt_apikey);
  topic.append("/");
  topic.append(mqtt_device);
  topic.append("/");
  topic.append(mqtt_alias);

  mqttMsg.mid = 2;
  mqttMsg.qos = 0;
  mqttMsg.retain = false;

  mqttMsg.topic = (char*) malloc(topic.length() + 1);
  strcpy(mqttMsg.topic, topic.c_str());

  mqttMsg.payload = (void*) mqtt_payload.c_str();
  mqttMsg.payloadlen = mqtt_payload.length();

  mockMosquitto->callBack->on_message(&mqttMsg);
  return 0;
}

void MqttTest::defineExpectationsMqttNoIncomingMsg(){
  std::string user = "admin";
  std::string passwd = "1234";


  //>> EXPECTATIONS
  EXPECT_CALL(*mockMosquitto, mqttSetPassword(StrEq(user.c_str()),
              StrEq(passwd.c_str()))).WillOnce(Return(0));


  EXPECT_CALL(*mockMosquitto, mqttConnect(_, _, _)).WillOnce(Invoke(this,
      &MqttTest::stubConnect));

  EXPECT_CALL(*mockMosquitto, mqttLoop(100, 1)).WillRepeatedly(Return(0));
  EXPECT_CALL(*mockMosquitto, mqttLoop(0, 1)).WillRepeatedly(Return(0));

  EXPECT_CALL(*mockMosquitto, mqttSubscribe(_, _, _)).WillOnce(Return(0));


  EXPECT_CALL(*mockMosquitto, mqttUnsubscribe(_, _)).WillOnce(Return(0));
  EXPECT_CALL(*mockMosquitto, mqttDisconnect()).WillOnce(Return(0));

  ESP_Plugin_Input_Mqtt::getInstance()->setMosquitto(mockMosquitto, "mqttrunner");

}

void MqttTest::defineExpectationsMqttt() {


  std::string user = "admin";
  std::string passwd = "1234";


  //>> EXPECTATIONS
  EXPECT_CALL(*mockMosquitto, mqttSetPassword(StrEq(user.c_str()),
              StrEq(passwd.c_str()))).WillOnce(Return(0));


  EXPECT_CALL(*mockMosquitto, mqttConnect(_, _, _)).WillOnce(Invoke(this,
      &MqttTest::stubConnect));

  EXPECT_CALL(*mockMosquitto, mqttLoop(100, 1)).WillRepeatedly(Return(0));
  EXPECT_CALL(*mockMosquitto, mqttLoop(0, 1)).WillOnce(Invoke(this,
      &MqttTest::stubLoopToOnMessage)).WillRepeatedly(Return(0));


  EXPECT_CALL(*mockMosquitto, mqttSubscribe(_, _, _)).WillOnce(Return(0));


  EXPECT_CALL(*mockMosquitto, mqttUnsubscribe(_, _)).WillOnce(Return(0));
  EXPECT_CALL(*mockMosquitto, mqttDisconnect()).WillOnce(Return(0));


  ESP_Plugin_Input_Mqtt::getInstance()->setMosquitto(mockMosquitto, "mqttrunner");


  mqttMsg.topic = NULL;
  mqttMsg.payload = NULL;
  mqttMsg.payloadlen = 0;

}

void MqttTest::defineExpectationsMqttPublisher() {

  std::string user = "admin";
  std::string passwd = "1234";

  //Order matters.

  EXPECT_CALL(*mockMosquittoPub, mqttSetPassword(StrEq(user.c_str()),
              StrEq(passwd.c_str()))).WillRepeatedly(Return(0));

  //Basic and common expected calls.
  EXPECT_CALL(*mockMosquittoPub, mqttConnect(_, _, _)).WillRepeatedly(Invoke(this,
      &MqttTest::stubConnectPub));
  EXPECT_CALL(*mockMosquittoPub, mqttLoop(_, _)).WillRepeatedly(Return(0));

  ESP_Plugin_Input_Mqtt::getInstance()->setMosquitto(mockMosquittoPub, "mqttwriter");


}

//Asserting the topic.
int MqttTest::stubPublish(int* mid, const char* topic, int payloadLen,
                          const void* payload, int qos, bool retain) {



  std::string strTopic(topic);
  std::string expected("1234/dev_mqtt/cmd/PING");

  CPPUNIT_ASSERT_EQUAL(expected, strTopic);

  return 0;
}

int MqttTest::stubPublishPush(int* mid, const char* topic, int payloadLen,
                              const void* payload, int qos, bool retain) {



  std::string strTopic(topic);
  std::string expected("1234/dev_mqtt_push/cmd/PING");

  CPPUNIT_ASSERT_EQUAL(expected, strTopic);

  return 0;
}

void MqttTest::start_cbmock(boost::shared_ptr<HttpMock>& cb_mock,
                            const std::string& type) {
  cb_mock->init();
  std::string mock_port = boost::lexical_cast<std::string>(cb_mock->get_port());
  std::cout << "mock with port:" << mock_port << std::endl;

  iota::Configurator::release();
  iota::Configurator* my_instance = iota::Configurator::instance();

  std::stringstream ss;
  ss << "{ \"ngsi_url\": {"
     <<   "     \"updateContext\": \"/NGSI10/updateContext\","
     <<   "     \"registerContext\": \"/NGSI9/registerContext\","
     <<   "     \"queryContext\": \"/NGSI10/queryContext\""
     <<   "},"
     <<   "\"timeout\": 10,"
     <<   "\"dir_log\": \"/tmp/\","
     <<   "\"timezones\": \"/etc/iot/date_time_zonespec.csv\","
     <<   "\"storage\": {"
     <<   "\"host\": \"127.0.0.1\","
     <<   "\"type\": \"" <<  type << "\","
     <<   "\"port\": \"27017\","
     <<   "\"dbname\": \"iotest\","
     <<   "\"file\": \"../../tests/iotagent/devices.json\""
     << "},"
     << "\"resources\":[{\"resource\": \"/iot/mqtt\","
     << "  \"options\": {\"FileName\": \"MqttService\" },"
     <<    " \"services\":[ {"
     <<   "\"apikey\": \"1234\","
     <<   "\"service\": \"service2\","
     <<   "\"service_path\": \"/ssrv2\","
     <<   "\"token\": \"token2\","
     <<   "\"cbroker\": \"http://127.0.0.1:" << mock_port << "/mock\", "
     <<   "\"entity_type\": \"thing\""
     << "} ] } ] }";
  //my_instance->update_conf(ss);
  std::string err = my_instance->read_file(ss);
  std::cout << "GET CONF " << my_instance->getAll() << std::endl;
  if (!err.empty()) {
    std::cout << "start_cbmock:" << err << std::endl;
    std::cout << "start_cbmock_data:" << ss.str() << std::endl;
  }


}
