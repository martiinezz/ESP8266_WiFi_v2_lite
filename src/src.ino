#include <Arduino.h>
#include <ArduinoOTA.h>
#include <time.h>

#include "emonesp.h"
#include "app_config.h"
#include "wifi.h"
#include "web_server.h"
#include "openevse.h"
#include "input.h"
#include "mqtt.h"
#include "ota.h"
#include "lcd.h"
#include "espal.h"
#include "event.h"

#include "RapiSender.h"

RapiSender rapiSender(&RAPI_PORT);

unsigned long Timer1; // Timer for events once every 30 seconds
unsigned long Timer3; // Timer for events once every 2 seconds

boolean rapi_read = 0; //flag to indicate first read of RAPI status

static uint32_t start_mem = 0;
static uint32_t last_mem = 0;

static void hardware_setup();
void scheduler_loop();

void setup()
{
  hardware_setup();
  ESPAL.begin();

  DEBUG.println();
  DEBUG.printf("OpenEVSE WiFI %s\n", ESPAL.getShortId().c_str());
  DEBUG.printf("Firmware: %s\n", currentfirmware.c_str());
  DEBUG.printf("Free: %d\n", ESPAL.getFreeHeap());

  config_load_settings();
  wifi_setup();
  web_server_setup();

#ifdef ENABLE_OTA
  ota_setup();
#endif

  input_setup();
  start_mem = last_mem = ESPAL.getFreeHeap();
}

void loop() {
  lcd_loop();
  web_server_loop();
  wifi_loop();
#ifdef ENABLE_OTA
  ota_loop();
#endif
  rapiSender.loop();

  if(OpenEVSE.isConnected())
  {
    if(OPENEVSE_STATE_STARTING != state && OPENEVSE_STATE_INVALID != state)
    {
      if (rapi_read == 0)
      {
        handleRapiRead();
        rapi_read=1;
      }

      if ((millis() - Timer3) >= 2000) {
        update_rapi_values();
        Timer3 = millis();
      }
    }
  }
  else
  {
    if ((millis() - Timer3) >= 1000)
    {
      OpenEVSE.begin(rapiSender, [](bool connected)
      {
        if(connected)
        {
          OpenEVSE.getStatus([](int ret, uint8_t evse_state, uint32_t session_time, uint8_t pilot_state, uint32_t vflags) {
            state = evse_state;
          });
        }
      });
      Timer3 = millis();
    }
  }

  if(wifi_client_connected())
  {
    mqtt_loop();

    if ((millis() - Timer1) >= 30000) {
      if(!Update.isRunning())
      {
        DynamicJsonDocument data(4096);
        create_rapi_json(data);
        event_send(data);
        scheduler_loop();
      }
      Timer1 = millis();
    }
  }
}

void scheduler_loop()
{
  if (scheduler_timers == "" || scheduler_timers == "[]") return;

  time_t now = time(NULL);
  struct tm *tm_now = localtime(&now);
  int now_min = tm_now->tm_hour * 60 + tm_now->tm_min;

  DynamicJsonDocument doc(2048);
  deserializeJson(doc, scheduler_timers);
  JsonArray array = doc.as<JsonArray>();

  bool should_charge = false;
  for (JsonObject t : array) {
    String start = t["start"];
    String end = t["end"];
    int s_h = start.substring(0, 2).toInt();
    int s_m = start.substring(3, 5).toInt();
    int e_h = end.substring(0, 2).toInt();
    int e_m = end.substring(3, 5).toInt();

    int start_min = s_h * 60 + s_m;
    int end_min = e_h * 60 + e_m;

    if (start_min < end_min) {
      if (now_min >= start_min && now_min < end_min) should_charge = true;
    } else {
      // Over midnight
      if (now_min >= start_min || now_min < end_min) should_charge = true;
    }
    if (should_charge) break;
  }

  static bool last_should_charge = false;
  static bool first_run = true;

  if (first_run || should_charge != last_should_charge) {
    if (should_charge) {
      DBUGLN("Scheduler: Enable charging");
      rapiSender.sendCmd("$FE");
    } else {
      DBUGLN("Scheduler: Pause charging");
      rapiSender.sendCmd("$FS");
    }
    last_should_charge = should_charge;
    first_run = false;
  }
}

void event_send(String &json)
{
  StaticJsonDocument<512> event;
  deserializeJson(event, json);
  event_send(event);
}

void event_send(JsonDocument &event)
{
  web_server_event(event);
  mqtt_publish(event);
}

void hardware_setup()
{
  Serial.begin(115200);
  DEBUG_BEGIN(115200);
  pinMode(0, INPUT);
}
