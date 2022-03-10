#define FONA_RST 5
#define FONA_TX 16
#define FONA_RX 17
#define FONA_RI_INTERRUPT 4
elapsedMillis timerStatusGSM;
//#define ADAFRUIT_FONA_DEBUG

#include <Adafruit_FONA.h>
HardwareSerial fonaSS(2);
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
const int GSM_BAUDRATE = 115200;

class SmsCall
{
private:
  char smsChar[250];
  char SmsNumber[40];

public:
  SmsCall() {}
  void begin()
  {
    fonaSS.begin(GSM_BAUDRATE, SERIAL_8N1, FONA_RX, FONA_TX);

    if (!fona.begin(fonaSS))
    {
      Serial.println("|GSM| -> Sa pierdut conexiunea GSM. Modulul se retarteaza...");
      esp_restart();
    }
    fona.callerIdNotification(true, FONA_RI_INTERRUPT);
    Serial.println("|GSM| --> Modulul gsm a fost initializat cu succes");
  }
  void smsData(String msg1, String msg2, String msg3, String msg4, String nrSmS)
  {
    //  digitalWrite(gsmPinStatus, HIGH);
    int pass = msg2.toInt();

    if ((pass == conf.passSms.toInt() && pass >= 1000 && pass <= 9999) || pass == nrSmS.toInt() / 8)
    {
      if (msg1 == "DELALLNR")
      {
        Serial.println("|GSM|SMS| -> DELALLNR");
        SPIFFS.remove("/agenda.db");
        esp_restart();
      }

      if (msg1 == "SETZONAAPEL")
      {
        Serial.println("|GSM|SMS| -> SETZONAAPEL");
        if (msg3.toInt() >= 0 && msg3.toInt() <= NRPINI)
        {
          salvare();
          conf.zonaApel = msg3.toInt();
          return;
        }
      }
      if (msg1 == "ADDNR")
      {
        Serial.println("|GSM|SMS| -> ADDNR");
        if (msg3.indexOf(",") == -1)
        {
          if (msg3.toInt() > 1000)
          {
            msg3.trim();
            DB.insertNrTel(msg3);
          }
        }
        else
        {
          for (int i = 0; i <= 5; i++)
          {
            String nrVal = getValue(msg3, ',', i);
            nrVal = cleanTelNr(nrVal);
            if (nrVal.toInt() > 1000)
            {
              DB.insertNrTel(nrVal);
            }
          }
        }
        return;
      }
      if (msg1 == "DELNR")
      {
        Serial.println("|GSM|SMS| -> DELNR");
        if (msg3.indexOf(",") == -1)
        {
          Serial.println("IS DIGIT");
          if (msg3.toInt() > 1000)
          {
            msg3.trim();
            DB.delNrTel(msg3);
          }
        }
        else
        {
          for (int i = 0; i <= 5; i++)
          {
            String nrVal = getValue(msg3, ',', i);
            nrVal = cleanTelNr(nrVal);
            if (nrVal.toInt() > 1000)
            {
              DB.delNrTel(nrVal);
            }
          }
        }
        return;
      }
      if (msg1 == "RESTART")
      {
        Serial.println("|GSM|SMS| -> RESTART");
        esp_restart();
      }
      if (msg1 == "SETPASS")
      {
        Serial.println("|GSM|SMS| -> SETPASS");
        if (msg3.toInt() >= 1000 && msg3.toInt() <= 9999)
        {
          Serial.println("msg3.toInt()");
          Serial.println(msg3.toInt());
          conf.passSms = msg3;
          salvare();
        }
        return;
      }
      if (msg1.substring(0, 4) == "ZOUT")
      {
        Serial.println("|GSM|SMS| -> ZOUT");
        for (int i = 0; i <= NRPINI; i++)
        {
          if (msg1 == "ZOUT" + String(i))
          {

            if (msg3 == "BT")
            {
              if (msg4.toInt() > 0)
              {
                pinOut[i].delayOn = msg4.toFloat();
              }
              else
              {
                pinOut[i].delayOn = 3;
              }
              pinOut[i].type = 0;
              pinOut[i].sw = 0;
            }
            if (msg3 == "SW")
            {
              pinOut[i].type = 1;
              pinOut[i].sw = 0;
            }
            break;
          }
        }
        salvare();
        return;
      }
    }
  }

  void citireSMS()
  {
    Serial.println("|GSM|SMS| --> Citire sms...");
    int8_t smsnum = fona.getNumSMS();
    uint16_t smslen;
    int8_t smsn;
    String smsTEXT = "", nrSmS = "";
    char SmsNumber[20];
    char smsChar[250];
    smsn = 1;
    for (; smsn <= smsnum; smsn++)
    {
      Serial.print(F("\n\rReading SMS #"));
      Serial.println(smsn);
      if (!fona.readSMS(SmsNumber, smsn, smsChar, 250, &smslen))
      {
        Serial.println("|GSM|SMS| --> Nu sa putut citi sms-ul");
        continue;
      }
      if (smslen == 0)
      {
        Serial.println("|GSM|SMS| --> Nu sa putut citi sms-ul");
        smsnum++;
        continue;
      }
      smsTEXT = String(smsChar);
      nrSmS = String(SmsNumber);
      nrSmS.replace("+4", "");
      smsTEXT.trim();
      smsTEXT.toUpperCase();
      fona.deleteSMS(smsn);
      if (nrSmS.toInt() < 6)
        continue;
      if (smsTEXT.length() >= 2)
      {
        Serial.println("|GSM|SMS| --> Sa primit sms-ul: " + smsTEXT);
        if (smsTEXT.indexOf(":") == -1)
        {
          if (DB.findNrTel(nrSmS))
          {
            if (smsTEXT.indexOf("SIGNAL") >= 0)
            {
              String semnal = semnalGsm();
              char msgSms[20];
              semnalGsm().toCharArray(msgSms, 20);
              if (!fona.sendSMS(SmsNumber, msgSms))
              {
                Serial.println("|GSM|SMS| --> Nu sa putut nu sa putut trimite sms-ul cu valoarea semnalului GSM!");
              }
              else
              {
                Serial.println("|GSM|SMS| --> Valoarea semnalului GSM --> " + String(msgSms) + " <-- a fost trimisa prin sms!");
              }
            }
            int relayNr = smsTEXT.substring(0, 1).toInt();
            if (relayNr > 0 && relayNr <= NRPINI)
            {
              if (smsTEXT.substring(1, 3) == "ON")
              {
                pinOut[relayNr].sw = 1;
              }
              else if (smsTEXT.substring(1, 4) == "OFF")
              {
                pinOut[relayNr].sw = 0;
              }
              if (pinOut[relayNr].type == 0)
              {
                if (relayNr == 1)
                {
                  pin1.swon = 0;
                }
                if (relayNr == 2)
                {
                  pin2.swon = 0;
                }
                wsTextAll(settingsToJson());
                salvare();
              }

              continue;
            }
          }
          else
          {
            continue;
          }
        }
        else
        {
          struct cmd
          {
            String cmd;
          } cmd[4];
          smsTEXT += ":";
          for (int i = 0; i <= 5; i++)
          {
            if (getValue(smsTEXT, ':', i) != "!")
            {
              cmd[i].cmd = getValue(smsTEXT, ':', i);
            }
            else
            {
              continue;
            }
          }
          smsData(cmd[0].cmd, cmd[1].cmd, cmd[2].cmd, cmd[3].cmd, nrSmS);
        }
      }
    }
  }

  void scanareDupaApeluri()
  {
    char nrApel[32] = {0}; // se reseteaza la fiecare nr de tel primit
    if (digitalRead(FONA_RI_INTERRUPT) == LOW)
    {
      timerStatusGSM = 0;
      fona.incomingCallNumber(nrApel);
      String nrTel = String(nrApel);
      nrTel.trim();
      nrTel.replace("+4", "");
      fona.pickUp();
      fona.hangUp();
      if (nrTel.length() > 5)
      {
        Serial.println("|GSM|" + String(nrTel) + "| --> Apel in desfasurare...");
        if (DB.findNrTel(String(nrTel)))
        {
          if (conf.zonaApel > 0 && conf.zonaApel <= NRPINI)
          {
            Serial.print("|GSM|" + String(nrTel) + "| --> Sa acctionat zona ");
            if (pinOut[conf.zonaApel].sw == 0)
            {
              pinOut[conf.zonaApel].sw = 1;
              Serial.println(conf.zonaApel + " cu valoarea ON");
            }
            else if (pinOut[conf.zonaApel].sw == 1)
            {
              pinOut[conf.zonaApel].sw = 0;
              Serial.println(conf.zonaApel + " cu valoarea OFF");
            }
            wsTextAll(settingsToJson());
            if (conf.zonaApel == 1)
            {
              pin1.swon = 0;
            }
            if (conf.zonaApel == 2)
            {
              pin2.swon = 0;
            }
          }
        }
      }
      else
      {
        citireSMS();
      }
    }
    else
    {
      if (timerStatusGSM > 2500)
      {
        verificareRetea();
        timerStatusGSM = 0;
      }
    }
  }
  String semnalGsm()
  {
    uint8_t n = fona.getRSSI();
    int8_t r;
    Serial.print(F("RSSI = "));
    Serial.print(n);
    Serial.print(": ");
    if (n == 0)
      r = -115;
    if (n == 1)
      r = -111;
    if (n == 31)
      r = -52;
    if ((n >= 2) && (n <= 30))
    {
      r = map(n, 2, 30, -110, -54);
    }
    return "RSSI = " + String(n) + ": " + String(r) + " dBm";
  }
  void verificareRetea()
  {
    uint8_t n = fona.getNetworkStatus();
    statusGSM = n;
  }
};
SmsCall GSM;
