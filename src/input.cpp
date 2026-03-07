#if defined(ENABLE_DEBUG) && !defined(ENABLE_DEBUG_INPUT)
#undef ENABLE_DEBUG
#endif

#include <Arduino.h>
#include <ArduinoJson.h>
#include <espal.h>
#include <ESP8266WiFi.h>

#include <time.h>
#include <sys/time.h>

#include "emonesp.h"
#include "input.h"
#include "app_config.h"
#include "event.h"
#include "wifi.h"
#include "openevse.h"

#include "RapiSender.h"

int rapi_command = 1;

double amp = 0;
double voltage = DEFAULT_VOLTAGE;
double temp1 = 0;
bool temp1_valid = false;
double temp2 = 0;
bool temp2_valid = false;
double temp3 = 0;
bool temp3_valid = false;
long pilot = 0;
long state = OPENEVSE_STATE_STARTING;
long elapsed = 0;

// Defaults OpenEVSE Settings
byte rgb_lcd = 1;
byte serial_dbg = 0;
byte auto_service = 1;
int service = 1;

long current_scale = 0;
long current_offset = 0;

// Default OpenEVSE Safety Configuration
byte diode_ck = 1;
byte gfci_test = 1;
byte ground_ck = 1;
byte stuck_relay = 1;
byte vent_ck = 1;
byte temp_ck = 1;
byte auto_start = 1;
String firmware = "-";
String protocol = "-";

// Default OpenEVSE Fault Counters
long gfci_count = 0;
long nognd_count = 0;
long stuck_count = 0;

// OpenEVSE Usage Statistics
long wattsec = 0;
long watthour_total = 0;

unsigned long comm_sent = 0;
unsigned long comm_success = 0;

void create_rapi_json(JsonDocument &doc)
{
  doc["amp"] = amp * AMPS_SCALE_FACTOR;
  doc["voltage"] = voltage * VOLTS_SCALE_FACTOR;
  doc["pilot"] = pilot;
  doc["wh"] = watthour_total;
  if(temp1_valid) {
    doc["temp1"] = temp1 * TEMP_SCALE_FACTOR;
  } else {
    doc["temp1"] = false;
  }
  if(temp2_valid) {
    doc["temp2"] = temp2 * TEMP_SCALE_FACTOR;
  } else {
    doc["temp2"] = false;
  }
  if(temp3_valid) {
    doc["temp3"] = temp3 * TEMP_SCALE_FACTOR;
  } else {
    doc["temp3"] = false;
  }
  doc["state"] = state;
  doc["freeram"] = ESPAL.getFreeHeap();
  doc["srssi"] = WiFi.RSSI();
}

void update_rapi_values() {
  switch(rapi_command)
  {
    case 1:
      rapiSender.sendCmd("$GE", [](int ret)
      {
        if(RAPI_RESPONSE_OK == ret && rapiSender.getTokenCnt() >= 3) {
          pilot = strtol(rapiSender.getToken(1), NULL, 10);
        }
      });
      break;
    case 2:
      OpenEVSE.getStatus([](int ret, uint8_t evse_state, uint32_t session_time, uint8_t pilot_state, uint32_t vflags)
      {
        if(RAPI_RESPONSE_OK == ret) {
          state = evse_state;
          elapsed = session_time;
        }
      });
      break;
    case 3:
      OpenEVSE.getChargeCurrentAndVoltage([](int ret, double a, double volts)
      {
        if(RAPI_RESPONSE_OK == ret) {
          amp = a;
          if(volts >= 0) voltage = volts;
        }
      });
      break;
    case 4:
      OpenEVSE.getTemperature([](int ret, double t1, bool t1_valid, double t2, bool t2_valid, double t3, bool t3_valid)
      {
        if(RAPI_RESPONSE_OK == ret) {
          temp1 = t1; temp1_valid = t1_valid;
          temp2 = t2; temp2_valid = t2_valid;
          temp3 = t3; temp3_valid = t3_valid;
        }
      });
      break;
    case 5:
      rapiSender.sendCmd("$GU", [](int ret)
      {
        if(RAPI_RESPONSE_OK == ret && rapiSender.getTokenCnt() >= 3) {
          wattsec = strtol(rapiSender.getToken(1), NULL, 10);
          watthour_total = strtol(rapiSender.getToken(2), NULL, 10);
        }
      });
      break;
    case 6:
      rapiSender.sendCmd("$GF", [](int ret)
      {
        if(RAPI_RESPONSE_OK == ret && rapiSender.getTokenCnt() >= 4) {
          gfci_count = strtol(rapiSender.getToken(1), NULL, 16);
          nognd_count = strtol(rapiSender.getToken(2), NULL, 16);
          stuck_count = strtol(rapiSender.getToken(3), NULL, 16);
        }
      });
      rapi_command = 0;
      break;
  }
  rapi_command++;
}

void handleRapiRead()
{
  OpenEVSE.getVersion([](int ret, const char *returned_firmware, const char *returned_protocol) {
    if(RAPI_RESPONSE_OK == ret) {
      firmware = returned_firmware;
      protocol = returned_protocol;
    }
  });

  OpenEVSE.getTime([](int ret, time_t evse_time)
  {
    if(RAPI_RESPONSE_OK == ret) {
      struct timeval set_time = { evse_time, 0 };
      settimeofday(&set_time, NULL);
    }
  });

  rapiSender.sendCmd("$GA", [](int ret)
  {
    if(RAPI_RESPONSE_OK == ret && rapiSender.getTokenCnt() >= 3) {
      current_scale = strtol(rapiSender.getToken(1), NULL, 10);
      current_offset = strtol(rapiSender.getToken(2), NULL, 10);
    }
  });

  rapiSender.sendCmd("$GE", [](int ret)
  {
    if(RAPI_RESPONSE_OK == ret && rapiSender.getTokenCnt() >= 3) {
      pilot = strtol(rapiSender.getToken(1), NULL, 10);
      long flags = strtol(rapiSender.getToken(2), NULL, 16);
      service = bitRead(flags, 0) + 1;
      diode_ck = bitRead(flags, 1);
      vent_ck = bitRead(flags, 2);
      ground_ck = bitRead(flags, 3);
      stuck_relay = bitRead(flags, 4);
      auto_service = bitRead(flags, 5);
      auto_start = bitRead(flags, 6);
      serial_dbg = bitRead(flags, 7);
      rgb_lcd = bitRead(flags, 8);
      gfci_test = bitRead(flags, 9);
      temp_ck = bitRead(flags, 10);
    }
  });
}

void input_setup()
{
  OpenEVSE.onState([](uint8_t evse_state, uint8_t pilot_state, uint32_t current_capacity, uint32_t vflags)
  {
    state = evse_state;
    StaticJsonDocument<32> event;
    event["state"] = state;
    event_send(event);
  });

  OpenEVSE.onWiFi([](uint8_t wifiMode)
  {
    switch(wifiMode)
    {
      case OPENEVSE_WIFI_MODE_AP:
      case OPENEVSE_WIFI_MODE_AP_DEFAULT:
        wifi_turn_on_ap();
        break;
      case OPENEVSE_WIFI_MODE_CLIENT:
        wifi_turn_off_ap();
        break;
    }
  });
}
