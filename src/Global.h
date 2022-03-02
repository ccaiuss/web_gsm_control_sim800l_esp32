elapsedMillis timerLedStatusGSM;

void salvare()
{
  timerMemSetariPini = 0;
  startMemorarePiniOut = true;
}
void defaultConfig(boolean val)
{
  if (conf.defaultConf != "OK" || val)
  {
    conf.defaultConf = "OK";
    conf.passSms = "1234";
    conf.modAntena = "wifi";
    conf.zonaApel = 1;

    for (int i = 0; i <= NRPINI; i++)
    {
      pinOut[i].sw = 0;
      pinOut[i].type = 0;
      pinOut[i].delayOn = 3;
    }
    salvare();
  }
}
// String  var = getValue( StringVar, ',', 2); // if  a,4,D,r  would return D
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length();

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "!";
} // END

String cleanTelNr(String data)
{
  String tel = "";

  for (int i = 0; i <= data.length(); i++)
  {
    if (isAlphaNumeric(data.charAt(i)))
    {
      tel += String(data.charAt(i));
    }
  }
  return tel;
} // END

void statusLed(int n)
{
  switch (n)
  {
  case 0:
    digitalWrite(2, LOW);
    break;
  case 1:
    if (timerLedStatusGSM > 2000 && timerLedStatusGSM < 2200)
    {
      digitalWrite(2, HIGH);
    }
    else if (timerLedStatusGSM < 2000)
    {
      digitalWrite(2, LOW);
    }
    else if (timerLedStatusGSM > 2200)
    {
      timerLedStatusGSM = 0;
    }
    break;
  case 2:
    if (timerLedStatusGSM > 200 && timerLedStatusGSM < 400)
    {
      digitalWrite(2, HIGH);
    }
    else if (timerLedStatusGSM < 200)
    {
      digitalWrite(2, LOW);
    }
    else if (timerLedStatusGSM > 400)
    {
      timerLedStatusGSM = 0;
    }
    break;
  case 3:
    digitalWrite(2, HIGH);
    break;
  case 4:
    digitalWrite(2, HIGH);
    break;
  case 5:
    digitalWrite(2, HIGH);
    break;
  }
}

String send_connection_state_values_html()
{
  String state = "N/A";
  String Networks = "";
  DynamicJsonBuffer jsonBuffer;
  JsonObject &jsonStr = jsonBuffer.createObject();
  if (WiFi.status() == 0)
    state = "Idle";
  else if (WiFi.status() == 1)
    state = "NO SSID AVAILBLE";
  else if (WiFi.status() == 2)
    state = "SCAN COMPLETED";
  else if (WiFi.status() == 3)
    state = "CONNECTED";
  else if (WiFi.status() == 4)
    state = "CONNECT FAILED";
  else if (WiFi.status() == 5)
    state = "CONNECTION LOST";
  else if (WiFi.status() == 6)
    state = "DISCONNECTED";
  int n = WiFi.scanNetworks();

  if (n == 0)
  {
    Networks = "<div class=\"well\" role=\"alert\">No networks found!</div>";
  }
  else
  {
    Networks += "<table class=\"table\">";
    Networks += "<thead><tr><th>Name</th><th>Quality</th><tr></thead><tbody>";
    for (int i = 0; i < n; ++i)
    {
      int quality = 0;
      if (WiFi.RSSI(i) <= -100)
      {
        quality = 0;
      }
      else if (WiFi.RSSI(i) >= -50)
      {
        quality = 100;
      }
      else
      {
        quality = 2 * (WiFi.RSSI(i) + 100);
      }
      Networks += "<tr><td>" + String(WiFi.SSID(i)) + " </td><td>" + String(quality) + "% (" + String(WiFi.RSSI(i)) + "dBm)</td></tr>";
    }
    Networks += "</tbody></table>";
  }
  jsonStr["wifiStatus"] = Networks;
  jsonStr["conf"] = 4;
  String buff;
  jsonStr.printTo(buff);
  return buff;
}

boolean jsonToSettings(String val)
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject &jsonStr = jsonBuffer.parse(val);
  if (jsonStr.success())
  {
    conf.modAntena = jsonStr["modAntena"].as<String>();
    conf.defaultConf = jsonStr["defaultConf"].as<String>();
    conf.zonaApel = jsonStr["zonaApel"];
    conf.passSms = jsonStr["passSms"].as<String>();

    for (int i = 1; i <= NRPINI; i++)
    {
      pinOut[i].sw = jsonStr["sw" + String(i)];
      pinOut[i].type = jsonStr["type" + String(i)];
      pinOut[i].delayOn = jsonStr["delayOn" + String(i)];
    }
    return true;
    
  }
  return false;
}
String settingsToJson()
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject &jsonStr = jsonBuffer.createObject();
  jsonStr["defaultConf"] = "OK";
  jsonStr["cmd"] = "client";
  jsonStr["zonaApel"] = conf.zonaApel;
  jsonStr["passSms"] = conf.passSms;
  jsonStr["modAntena"] = conf.modAntena;
  for (int i = 1; i <= NRPINI; i++)
  {
    jsonStr["sw" + String(i)] = pinOut[i].sw;
    jsonStr["type" + String(i)] = pinOut[i].type;
    jsonStr["delayOn" + String(i)] = pinOut[i].delayOn;
  }

  String buff;
  jsonStr.printTo(buff);
  return buff;
}
