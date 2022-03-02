#include <Arduino.h>
#include <WiFi.h>

#include <elapsedMillis.h>

#include <ArduinoJson.h>

#include <WebsocketHandler.hpp>
#include <Bounce2.h>

const int BAUDRATE = 115200;

// primul nr din lista nu este luat in considerare
int PIN_SW[] = {0, 32, 33};
#define NRPINI 2

struct conf
{
  String defaultConf;
  String passSms;
  int zonaApel;
  String modAntena;

} conf;

struct pinOut
{
  boolean sw;
  boolean type;
  float delayOn;
} pinOut[5];

int statusGSM = 0;

// timeing
elapsedMillis timerMemSetariPini;

boolean wsSendConfOneTime = false, startMemorarePiniOut = false;

void wsTextAll(String msg);

// imports
#include "Global.h"
#include "DB.h"
#include "PinLoop.h"
#include "SmsCall.h"
#include "wificonf.h"
#include "Webserver.h"
#include "Websockets.h"

Bounce2::Button sw1 = Bounce2::Button();
Bounce2::Button sw2 = Bounce2::Button();

void setup()
{
  Serial.begin(BAUDRATE);
  Serial.println("|PROG| -> START SETUP");

  pinMode(2, OUTPUT);
  pinMode(PIN_SW[1], OUTPUT);
  pinMode(PIN_SW[2], OUTPUT);
  digitalWrite(PIN_SW[1], LOW);
  digitalWrite(PIN_SW[2], LOW);

  timerMemSetariPini = 10000;

  if (!SPIFFS.begin(false))
  {
    Serial.println("An Error has occurred while mounting SPIFFS ! Formatting in progress");
    return;
  }
  // file list
  File root = SPIFFS.open("/");
  if (!root)
  {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println(" - not a directory");
    return;
  }
  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
  // remove existing file
  // SPIFFS.remove("/setari.db");
  // SPIFFS.remove("/agenda.db");

  DB.begin();

  DB.settings();
  defaultConfig(false);

  GSM.begin();

  configWifi();


  webServer = new HTTPServer();

  ResourceNode *spiffsNode = new ResourceNode("", "", &handleSPIFFS);
  WebsocketNode *chatNode = new WebsocketNode("/ws", &ChatHandler::create);

  webServer->registerNode(chatNode);
  webServer->setDefaultNode(spiffsNode);
  webServer->addMiddleware(&middlewareAuthentication);
  webServer->addMiddleware(&middlewareAuthorization);
  webServer->start();

  Serial.println("[WEB] --> Webserver OK.");
  sw1.attach(26, INPUT);
  sw2.attach(27, INPUT);
  sw1.interval(5);
  sw1.setPressedState(HIGH);
  sw2.interval(5);
  sw2.setPressedState(HIGH);
}
void loop()
{
  sw1.update();
  sw2.update();

  if (sw1.pressed())
  {
    pinOut[1].sw = !pinOut[1].sw;
    wsTextAll(settingsToJson());
  }
  if (sw2.pressed())
  {
    pinOut[2].sw = !pinOut[2].sw;
    wsTextAll(settingsToJson());
  }
  statusLed(statusGSM);
  GSM.scanareDupaApeluri();

  if (startMemorarePiniOut == true)
  {
    if (timerMemSetariPini > 10000)
    {
      Serial.println("|DB| --> Settings are saved!");
      DB.settingsToDB(settingsToJson());
      startMemorarePiniOut = false;
    }
  }
  else
  {
    timerMemSetariPini = 0;
  }
  pin1.loopPin();
  pin2.loopPin();

  if (WiFi.status() == WL_CONNECTED)
  {
    if (webServer->isRunning())
    {

      if (wsSendConfOneTime == true)
      {
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
          if (activeClients[i] != nullptr)
          {
            activeClients[i]->send(settingsToJson().c_str(), ChatHandler::SEND_TYPE_TEXT);
          }
        }
        wsSendConfOneTime = false;
      }
      webServer->loop();
    }
  }
  else
  {
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
      if (activeClients[i] != nullptr)
      {
        activeClients[i]->onClose();
      }
    }
  }
}

void wsTextAll(String msg)
{
  for (int i = 0; i < MAX_CLIENTS; i++)
  {
    if (activeClients[i] != nullptr)
    {
      activeClients[i]->send(msg.c_str(), ChatHandler::SEND_TYPE_TEXT);
    }
  }
}
