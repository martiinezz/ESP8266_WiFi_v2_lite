#include "emonesp.h"
#include "espal.h"
#include "mqtt.h"
#include "input.h"

#include "app_config.h"
#include "app_config_mode.h"

#include <Arduino.h>
#include <EEPROM.h>             // Save config settings
#include <ConfigJson.h>

#define EEPROM_SIZE     4096
#define CHECKSUM_SEED    128

// Wifi Network Strings
String esid;
String epass;

// Web server authentication (leave blank for none)
String www_username;
String www_password;

// Advanced settings
String esp_hostname;
String sntp_hostname;

// MQTT Settings
String mqtt_server;
uint32_t mqtt_port;
String mqtt_topic;
String mqtt_user;
String mqtt_pass;
String mqtt_solar;
String mqtt_grid_ie;
String mqtt_vrms;
String mqtt_announce_topic;

// Scheduler timers
String scheduler_timers;

// 24-bits of Flags
uint32_t flags;

String esp_hostname_default = "openevse";

void config_changed(String name);

ConfigOptDefenition<uint32_t> flagsOpt = ConfigOptDefenition<uint32_t>(flags, 0, "flags", "f");

ConfigOpt *opts[] = 
{
// Wifi Network Strings
  new ConfigOptDefenition<String>(esid, "", "ssid", "ws"),
  new ConfigOptSecret(epass, "", "pass", "wp"),

// Web server authentication (leave blank for none)
  new ConfigOptDefenition<String>(www_username, "", "www_username", "au"),
  new ConfigOptSecret(www_password, "", "www_password", "ap"),

// Advanced settings
  new ConfigOptDefenition<String>(esp_hostname, esp_hostname_default, "hostname", "hn"),

// MQTT Settings
  new ConfigOptDefenition<String>(mqtt_server, "emonpi", "mqtt_server", "ms"),
  new ConfigOptDefenition<uint32_t>(mqtt_port, 1883, "mqtt_port", "mpt"),
  new ConfigOptDefenition<String>(mqtt_topic, esp_hostname, "mqtt_topic", "mt"),
  new ConfigOptDefenition<String>(mqtt_user, "emonpi", "mqtt_user", "mu"),
  new ConfigOptSecret(mqtt_pass, "emonpimqtt2016", "mqtt_pass", "mp"),
  new ConfigOptDefenition<String>(mqtt_solar, "", "mqtt_solar", "mo"),
  new ConfigOptDefenition<String>(mqtt_grid_ie, "emon/emonpi/power1", "mqtt_grid_ie", "mg"),
  new ConfigOptDefenition<String>(mqtt_vrms, "emon/emonpi/vrms", "mqtt_vrms", "mv"),
  new ConfigOptDefenition<String>(mqtt_announce_topic, "openevse/announce/"+ESPAL.getShortId(), "mqtt_announce_topic", "ma"),

// Scheduler timers
  new ConfigOptDefenition<String>(scheduler_timers, "[]", "timers", "t"),

// Flags
  &flagsOpt,

// Virtual Options
  new ConfigOptVirtualBool(flagsOpt, CONFIG_SERVICE_MQTT, CONFIG_SERVICE_MQTT, "mqtt_enabled", "me"),
  new ConfigOptVirtualChargeMode(flagsOpt, "charge_mode", "chmd")
};

ConfigJson config(opts, sizeof(opts) / sizeof(opts[0]), EEPROM_SIZE);

// -------------------------------------------------------------------
// Reset EEPROM, wipes all settings
// -------------------------------------------------------------------
void
ResetEEPROM() {
  EEPROM.begin(EEPROM_SIZE);

  //DEBUG.println("Erasing EEPROM");
  for (int i = 0; i < EEPROM_SIZE; ++i) {
    EEPROM.write(i, 0xff);
    //DEBUG.print("#");
  }
  EEPROM.end();
}

// -------------------------------------------------------------------
// Load saved settings from EEPROM
// -------------------------------------------------------------------
void
config_load_settings() 
{
  config.onChanged(config_changed);

  if(!config.load()) {
    DBUGF("No JSON config found, trying v1 settings");
    config_load_v1_settings();
  }
}

void config_changed(String name)
{
  DBUGF("%s changed", name.c_str());

  if(name == "flags") {
    if(mqtt_connected() != config_mqtt_enabled()) {
      mqtt_restart();
    }
  } else if(name.startsWith("mqtt_")) {
    mqtt_restart();
  }
}

void config_commit()
{
  config.commit();
}

bool config_deserialize(String& json) {
  return config.deserialize(json.c_str());
}

bool config_deserialize(const char *json)
{
  return config.deserialize(json);
}

bool config_deserialize(DynamicJsonDocument &doc) 
{
  return config.deserialize(doc);
}

bool config_serialize(String& json, bool longNames, bool compactOutput, bool hideSecrets)
{
  return config.serialize(json, longNames, compactOutput, hideSecrets);
}

bool config_serialize(DynamicJsonDocument &doc, bool longNames, bool compactOutput, bool hideSecrets)
{
  return config.serialize(doc, longNames, compactOutput, hideSecrets);
}

void config_set(const char *name, uint32_t val) {
  config.set(name, val);
} 
void config_set(const char *name, String val) {
  config.set(name, val);
} 
void config_set(const char *name, bool val) {
  config.set(name, val);
} 
void config_set(const char *name, double val) {
  config.set(name, val);
} 

void
config_save_mqtt(bool enable, String server, uint16_t port, String topic, String user, String pass, String solar, String grid_ie)
{  uint32_t newflags = flags & ~CONFIG_SERVICE_MQTT;
  if(enable) {
    newflags |= CONFIG_SERVICE_MQTT;
  }

  config.set("mqtt_server", server);
  config.set("mqtt_port", port);
  config.set("mqtt_topic", topic);
  config.set("mqtt_user", user);
  config.set("mqtt_pass", pass);
  config.set("mqtt_solar", solar);
  config.set("mqtt_grid_ie", grid_ie);
  config.set("flags", newflags);
  config.commit();
}

void
config_save_admin(String user, String pass) {
  config.set("www_username", user);
  config.set("www_password", pass);
  config.commit();
}

void
config_save_advanced(String hostname) {
  config.set("hostname", hostname);
  config.commit();
}

void
config_save_wifi(String qsid, String qpass)
{
  config.set("ssid", qsid);
  config.set("pass", qpass);
  config.commit();
}

void
config_save_flags(uint32_t newFlags) {
  config.set("flags", newFlags);
  config.commit();
}

void
config_reset() {
  ResetEEPROM();
  config.reset();
}
