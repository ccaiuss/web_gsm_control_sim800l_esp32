const char *wifi_ssid = "Mihai";
const char *wifi_pass = "admin?55";

const char *ap_ssid = "ABCTools";
const char *ap_pass = "abctools123456";

bool startAP(const char *ssid, const char *password = NULL)
{
    WiFi.mode(WIFI_AP);
    bool success;
    success = WiFi.softAP(ssid, password, 3, false, 4);
    delay(500);

    if (!success)
    {
        ESP.restart();
    }
    return success;
}

bool connectSTA(const char *ssid, const char *password)
{
    IPAddress clientip;
    IPAddress subnet;
    IPAddress gateway;
    IPAddress dns;

    clientip.fromString("192.168.1.155");
    subnet.fromString("255.255.255.0");
    gateway.fromString("192.168.1.1");
    dns.fromString("8.8.8.8");
    WiFi.config(clientip, gateway, subnet, dns);
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid, password);

    if (WiFi.waitForConnectResult())
    {
        // Assume time is out first and check
        Serial.println();
        Serial.print(F("[INFO] Adresa IP: "));
        Serial.println(WiFi.localIP());
        Serial.print("[INFO] Adresa MAC: ");
        Serial.println(WiFi.macAddress()); // This will read MAC Address of ESP

        String data = ssid;
        data += " " + WiFi.localIP().toString();
        // writeEvent("INFO", "wifi", "WiFi is connected", data);
        return true;
    }
    else
    {
        // We couln't connect, time is out, inform
        Serial.println();
        Serial.println(F("[WIFI] Nu sa putut conecta in timpul alocat !"));

        return false;
    }
}

void configWifi()
{
    if (conf.modAntena == "wifi")
    {
        if (!connectSTA(wifi_ssid, wifi_pass))
        {
            startAP(ap_ssid, ap_pass);
        }
    }
    else
    {
        startAP(ap_ssid, ap_pass);
    }
}
