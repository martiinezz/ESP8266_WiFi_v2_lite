#include "emonesp.h"
#include "mqtt.h"
#include "app_config.h"
#include "input.h"
#include "espal.h"
#include "openevse.h"

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>

WiFiClient espClient;
PubSubClient mqttclient(espClient);

static long nextMqttReconnectAttempt = 0;
static unsigned long mqttRestartTime = 0;

#ifndef MQTT_CONNECT_TIMEOUT
#define MQTT_CONNECT_TIMEOUT (5 * 1000)
#endif

// -------------------------------------------------------------------
// MQTT msg Received callback function:
// -------------------------------------------------------------------
void mqttmsg_callback(char *topic, byte * payload, unsigned int length) {
  String topic_string = String(topic);
  String payload_str = "";
  for (unsigned int i = 0; i < length; i++) {
    payload_str += (char) payload[i];
  }

  DBUGF("MQTT Topic: %s, Payload: %s", topic, payload_str.c_str());

  if (topic_string == mqtt_topic + "/cmd/start") {
    rapiSender.sendCmd("$FE", [](int ret) { DBUGF("Start CMD: %d", ret); });
  }
  else if (topic_string == mqtt_topic + "/cmd/pause") {
    rapiSender.sendCmd("$FS", [](int ret) { DBUGF("Pause CMD: %d", ret); });
  }
  else if (topic_string == mqtt_topic + "/cmd/current") {
    String cmd = "$SC " + payload_str;
    rapiSender.sendCmd(cmd, [](int ret) { DBUGF("Current CMD: %d", ret); });
  }
  else if (topic_string == mqtt_topic + "/rapi/in") {
    rapiSender.sendCmd(payload_str, [](int ret) {
      if (RAPI_RESPONSE_OK == ret || RAPI_RESPONSE_NK == ret) {
        String out_topic = mqtt_topic + "/rapi/out";
        mqttclient.publish(out_topic.c_str(), rapiSender.getResponse());
      }
    });
  }
}

// -------------------------------------------------------------------
// MQTT Connect
// -------------------------------------------------------------------
boolean mqtt_connect() {
  if (mqtt_server == "") return false;

  mqttclient.setServer(mqtt_server.c_str(), mqtt_port);
  mqttclient.setCallback(mqttmsg_callback);

  String strID = String(ESP.getChipId());
  if (mqttclient.connect(strID.c_str(), mqtt_user.c_str(), mqtt_pass.c_str(), (mqtt_topic + "/status/mqtt_connected").c_str(), 1, 1, "0")) {
    DEBUG.println("MQTT connected");
    mqttclient.publish((mqtt_topic + "/status/mqtt_connected").c_str(), "1", true);

    mqttclient.subscribe((mqtt_topic + "/cmd/start").c_str());
    mqttclient.subscribe((mqtt_topic + "/cmd/pause").c_str());
    mqttclient.subscribe((mqtt_topic + "/cmd/current").c_str());
    mqttclient.subscribe((mqtt_topic + "/rapi/in").c_str());

    return true;
  }
  return false;
}

// -------------------------------------------------------------------
// Publish status to MQTT
// -------------------------------------------------------------------
void mqtt_publish(JsonDocument &data) {
  if(!config_mqtt_enabled() || !mqttclient.connected()) return;

  String base = mqtt_topic + "/status/";

  // Specific status topics
  if (data.containsKey("state")) {
    mqttclient.publish((base + "state").c_str(), data["state"].as<String>().c_str(), true);
  }
  if (data.containsKey("amp")) {
    mqttclient.publish((base + "amp").c_str(), data["amp"].as<String>().c_str(), true);
  }
  if (data.containsKey("pilot")) {
    mqttclient.publish((base + "pilot").c_str(), data["pilot"].as<String>().c_str(), true);
  }
  if (data.containsKey("wh")) {
    mqttclient.publish((base + "wh").c_str(), data["wh"].as<String>().c_str(), true);
  }
  if (data.containsKey("srssi")) {
    mqttclient.publish((base + "rssi").c_str(), data["srssi"].as<String>().c_str(), true);
  }

  // Uptime and others
  mqttclient.publish((base + "uptime").c_str(), String(millis() / 1000).c_str(), true);
  mqttclient.publish((base + "wifi_connected").c_str(), String(WiFi.status() == WL_CONNECTED ? 1 : 0).c_str(), true);
}

// -------------------------------------------------------------------
// MQTT state management
// -------------------------------------------------------------------
void mqtt_loop() {
  if(mqttRestartTime > 0 && millis() > mqttRestartTime) {
    mqttRestartTime = 0;
    if (mqttclient.connected()) mqttclient.disconnect();
    nextMqttReconnectAttempt = 0;
  }

  if(config_mqtt_enabled()) {
    if (!mqttclient.connected()) {
      long now = millis();
      if (now > nextMqttReconnectAttempt) {
        nextMqttReconnectAttempt = now + MQTT_CONNECT_TIMEOUT;
        mqtt_connect();
      }
    } else {
      mqttclient.loop();
    }
  }
}

void mqtt_restart() {
  mqttRestartTime = millis();
}

boolean mqtt_connected() {
  return mqttclient.connected();
}
