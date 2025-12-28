#include <WiFi.h>
#include <WiFiAP.h>
#include <WebServer.h>
#include <Preferences.h>
#include <NetworkUdp.h>

WebServer server(80);
Preferences preferences;

// you can customize how initial AP is going to appear
const char* apSSID = "ProtectMe-net";
const char* apPassword = "protectme";

String staSSID = "";
String staPassword = "";

String scannedSSIDs[10];
int numScanned = 0;

const char* blockedDomains[] = {
  "doubleclick.net", "googlesyndication.com", "gvt2.com", "googletagmanager.com",
  "googletagservices.com", "google-analytics.com", "analytics.google.com",
  "adservice.google.com", "ads.google.com", "admob.com", "adsense.com",
  "googleadservices.com", "2mdn.net",
  "clients1.google.com", "clients2.google.com", "clients3.google.com",
  "clients4.google.com", "clients5.google.com", "clients6.google.com",
  "clients.google.com", "safebrowsing.google.com", "safebrowsing-cache.google.com",
  "connectivitycheck.gstatic.com", "connectivitycheck.android.com",
  "play.googleapis.com", "firebase.google.com", "crashlytics.com",
  "optimizationguide-pa.googleapis.com", "mtalk.google.com",
  "alt1-mtalk.google.com", "alt2-mtalk.google.com", "alt3-mtalk.google.com",
  "alt4-mtalk.google.com", "alt5-mtalk.google.com", "alt6-mtalk.google.com",
  "alt7-mtalk.google.com", "alt8-mtalk.google.com",
  "update.googleapis.com", "android.clients.google.com",
  "readaloud.googleapis.com", "fcm.googleapis.com", "notification.google.com",
  "cdn.ampproject.org",
  "z-m-gateway.facebook.com", "graph.facebook.com", "pixel.facebook.com",
  "connect.facebook.net",
  "tiktokv.com", "ttwstatic.com", "byteoversea.com", "snssdk.com",
  "log.tiktokv.com", "mon.tiktokv.com", "analytics.tiktok.com",
  "mcs-va.tiktokv.com", "mcs.tiktokv.com", "libraweb-va.tiktok.com",
  "sf16-website-login.neutral.ttwstatic.com",
  "telemetry.microsoft.com", "watson.telemetry.microsoft.com",
  "vortex.data.microsoft.com", "vortex-win.data.microsoft.com",
  "settings-win.data.microsoft.com", "oca.telemetry.microsoft.com",
  "guzzoni.apple.com", "xp.apple.com",
  "amazon-adsystem.com", "aaxads.com",
  "al-array.com", "tracking.al-array.com", "api.al-array.com", "apks.rp.al-array.com",
  "sensic.net", "clickblitzo.com",
  "pubmatic.com", "openx.net", "casalemedia.com", "rubiconproject.com",
  "appnexus.com", "outbrain.com", "taboola.com", "revcontent.com",
  "criteo.com", "bidswitch.net", "3lift.com", "contextweb.com",
  "sonobi.com", "lijit.com", "smartadserver.com", "serving-sys.com",
  "atdmt.com", "advertising.com", "adnxs.com", "yieldmanager.com",
  "scorecardresearch.com", "quantcast.com", "quantserve.com",
  "media.net", "adsafeprotected.com", "sharethrough.com",
  "districtm.io", "teads.tv", "spotxchange.com", "indexexchange.com",
  "gumgum.com", "triplelift.com", "sovrn.com", "nativo.com", "mixpanel.com"
};

const int numBlocked = sizeof(blockedDomains) / sizeof(blockedDomains[0]);

const int DNS_PORT = 53;
WiFiUDP udp;

const int LED_PIN = 2;
unsigned long ledMillis = 0;
int ledBrightness = 100;
int fadeAmount = 5;
bool protectionActive = false;
unsigned long blockFlashEnd = 0;
int blinkInterval = 0;
unsigned long previousMillis = 0;

#define LOG_LINES 50
String serialLog[LOG_LINES];
int logIndex = 0;

void addLog(const String& line) {
  serialLog[logIndex] = line;
  logIndex = (logIndex + 1) % LOG_LINES;
  Serial.println(line);
}

const String htmlStyle = 
  "<style>"
  "body{font-family:Arial,Helvetica,sans-serif;background:#fff;color:#000;margin:0;padding:20px;}"
  "h1{font-size:1.6rem;margin:0 0 30px;text-align:center;}"
  ".container{max-width:480px;margin:0 auto;background:#fff;padding:30px;}"
  "p{margin:15px 0;font-size:1rem;text-align:center;}"
  "strong{color:#000;}"
  "hr{border:0;border-top:1px solid #ddd;margin:30px 0;}"
  "button,input[type=submit]{width:100%;padding:12px;font-size:1rem;background:#eee;color:#000;border:1px solid #ccc;border-radius:4px;margin:10px 0;cursor:pointer;}"
  "input,select{width:100%;padding:12px;margin:10px 0;border:1px solid #ccc;border-radius:4px;font-size:1rem;box-sizing:border-box;}"
  ".footer{font-size:0.85rem;color:#555;text-align:center;margin-top:40px;}"
  ".terminal{font-family:'Courier New',monospace;font-size:0.9rem;background:#000;color:#0f0;padding:20px;border-radius:4px;overflow-x:auto;max-height:70vh;overflow-y:auto;white-space:pre-wrap;word-wrap:break-word;line-height:1.4;}"
  ".modal{display:none;position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(0,0,0,0.5);align-items:center;justify-content:center;}"
  ".modal-content{background:#fff;padding:30px;border-radius:8px;text-align:center;max-width:400px;}"
  ".modal button{margin:10px;padding:12px 24px;}"
  "</style>"
  "<meta name='viewport' content='width=device-width, initial-scale=1'>";

const String modalHTML = 
  "<div id='confirmModal' class='modal'>"
  "<div class='modal-content'>"
  "<h2>Confirm Action</h2>"
  "<p id='modalMessage'></p>"
  "<button onclick=\"window.location.href=document.getElementById('confirmLink').value\">Yes</button>"
  "<button onclick=\"document.getElementById('confirmModal').style.display='none'\">Cancel</button>"
  "<input type='hidden' id='confirmLink'>"
  "</div>"
  "</div>"
  "<script>"
  "function confirmAction(message, url) {"
  "  document.getElementById('modalMessage').innerText = message;"
  "  document.getElementById('confirmLink').value = url;"
  "  document.getElementById('confirmModal').style.display = 'flex';"
  "  return false;"
  "}"
  "</script>";

void handleConfig() {
  String html = "<!DOCTYPE html><html><head><title>ESP32 DNS Tool</title>" + htmlStyle + "</head>"
                "<body><div class='container'>"
                "<h1>ESP32 DNS Tool</h1>"
                "<p>Status: <strong>Active</strong></p>"
                "<p>Network: <strong>" + WiFi.SSID() + "</strong></p>"
                "<p>ESP32 IP: <strong>" + WiFi.localIP().toString() + "</strong></p>"
                "<p>Blocking: <strong>" + String(numBlocked) + " domains</strong></p>"
                "<hr>"
                "<a href='/resetwifi' onclick=\"return confirmAction('Are you sure you want to change WiFi network? This will clear saved credentials.', '/resetwifi')\"><button>Change WiFi Network</button></a>"
                "<a href='/reboot' onclick=\"return confirmAction('Are you sure you want to reboot the device?', '/reboot')\"><button>Reboot Device</button></a>"
                "<a href='/log'><button>View Live Log</button></a>"
                "<p class='footer'>They think they own your traffic.<br>They are wrong.</p>"
                + modalHTML +
                "</div></body></html>";
  server.send(200, "text/html", html);
}

void handleResetWiFi() {
  preferences.begin("wifi", false);
  preferences.clear();
  preferences.end();
  String html = "<!DOCTYPE html><html><head><title>Resetting...</title>" + htmlStyle + "</head>"
                "<body><div class='container'>"
                "<h1>Resetting...</h1>"
                "<p>WiFi credentials cleared.</p>"
                "<p>Rebooting into configuration mode...</p>"
                "<p class='footer'>Back in a moment.</p>"
                "</div></body></html>";
  server.send(200, "text/html", html);
  delay(2000);
  ESP.restart();
}

void handleReboot() {
  String html = "<!DOCTYPE html><html><head><title>Rebooting...</title>" + htmlStyle + "</head>"
                "<body><div class='container'>"
                "<h1>Rebooting...</h1>"
                "<p>The device is restarting.</p>"
                "<p class='footer'>See you soon.</p>"
                "</div></body></html>";
  server.send(200, "text/html", html);
  delay(1000);
  ESP.restart();
}

void handleRoot() {
  String options = "<option value=''>-- Select Network --</option>";
  for (int i = 0; i < numScanned; i++) {
    options += "<option value='" + scannedSSIDs[i] + "'>" + scannedSSIDs[i] + "</option>";
  }
  String html = "<!DOCTYPE html><html><head><title>Setup</title>" + htmlStyle + "</head>"
                "<body><div class='container'>"
                "<h1>ESP32 DNS Tool</h1>"
                "<p>Connect to a WiFi network to activate protection</p>"
                "<form action='/save'>"
                "<select name='ssid'>" + options + "</select>"
                "<input name='manualssid' placeholder='Or enter SSID manually'>"
                "<input name='pass' type='password' placeholder='Password' required>"
                "<input type='submit' value='CONNECT'>"
                "</form>"
                "<p class='footer'>Your traffic. Your rules.</p>"
                "</div></body></html>";
  server.send(200, "text/html", html);
}

void handleSave() {
  staSSID = server.arg("manualssid");
  if (staSSID == "") staSSID = server.arg("ssid");
  staPassword = server.arg("pass");

  preferences.begin("wifi", false);
  preferences.putString("ssid", staSSID);
  preferences.putString("pass", staPassword);
  preferences.end();

  String html = "<!DOCTYPE html><html><head><title>Saving...</title>" + htmlStyle + "</head>"
                "<body><div class='container'>"
                "<h1>Saving...</h1>"
                "<p>Credentials saved.</p>"
                "<p>Rebooting to connect...</p>"
                "<p class='footer'>Protection loading.</p>"
                "</div></body></html>";
  server.send(200, "text/html", html);
  delay(2000);
  ESP.restart();
}

void handleLog() {
  String logHtml = "<!DOCTYPE html><html><head><title>Live Log</title>" + htmlStyle + "<meta http-equiv='refresh' content='3'></head>"
                   "<body><div class='container'>"
                   "<h1>Live Log</h1>"
                   "<div class='terminal'>";
  for (int i = LOG_LINES - 1; i >= 0; i--) {
    int idx = (logIndex + i) % LOG_LINES;
    if (serialLog[idx] != "") logHtml += serialLog[idx] + "\n";
  }
  logHtml += "</div>"
             "<p><a href='/config'><button>Back to Panel</button></a></p>"
             "<p class='footer'>Updates every 3 seconds</p>"
             "</div></body></html>";
  server.send(200, "text/html", logHtml);
}

void updateLED() {
  unsigned long current = millis();

  if (blockFlashEnd > current) {
    analogWrite(LED_PIN, 255);
    return;
  }

  if (protectionActive) {
    if (current - ledMillis >= 20) {
      ledMillis = current;
      ledBrightness += fadeAmount;
      if (ledBrightness <= 50 || ledBrightness >= 255) fadeAmount = -fadeAmount;
      analogWrite(LED_PIN, ledBrightness);
    }
  } else if (blinkInterval > 0) {
    if (current - previousMillis >= blinkInterval) {
      previousMillis = current;
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }
  } else {
    analogWrite(LED_PIN, 0);
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, 255);
  delay(100);
  ledBrightness = 100;

  preferences.begin("wifi", true);
  staSSID = preferences.getString("ssid", "");
  staPassword = preferences.getString("pass", "");
  preferences.end();

  bool connectSuccess = false;
  if (staSSID != "") {
    Serial.print("Trying saved WiFi: ");
    Serial.println(staSSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(staSSID.c_str(), staPassword.c_str());
    unsigned long start = millis();
    while (millis() - start < 15000) {
      if (WiFi.status() == WL_CONNECTED) {
        connectSuccess = true;
        break;
      }
      delay(500);
      Serial.print(".");
    }
  }

  if (connectSuccess) {
    Serial.println("\nConnected! ESP32 IP: " + WiFi.localIP().toString());
    Serial.println("Ultra-Aggressive DNS Proxy started");
    Serial.printf("Blocking %d domains\n", numBlocked);

    protectionActive = true;
    blinkInterval = 0;

    IPAddress dns1(1, 1, 1, 1);
    IPAddress dns2(1, 0, 0, 1);
    WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), dns1, dns2);

    udp.begin(DNS_PORT);

    server.on("/generate_204", []() { 
      server.sendHeader("Location", "http://" + WiFi.localIP().toString() + "/config");
      server.send(302, "text/plain", "");
    });
    server.on("/fwlink", []() { 
      server.sendHeader("Location", "http://" + WiFi.localIP().toString() + "/config");
      server.send(302, "text/plain", "");
    });
    server.on("/hotspot-detect.html", []() { 
      server.sendHeader("Location", "http://" + WiFi.localIP().toString() + "/config");
      server.send(302, "text/plain", "");
    });
    server.on("/success.html", []() { server.send(200, "text/plain", "success"); });

    server.on("/config", HTTP_GET, handleConfig);
    server.on("/resetwifi", HTTP_GET, handleResetWiFi);
    server.on("/reboot", HTTP_GET, handleReboot);
    server.on("/log", HTTP_GET, handleLog);

    server.onNotFound([]() {
      server.sendHeader("Location", "http://" + WiFi.localIP().toString() + "/config");
      server.send(302, "text/plain", "");
    });

    server.begin();

    Serial.println("Control panel: http://" + WiFi.localIP().toString() + "/config");
  } else {
    Serial.println("Connection failed → AP mode");
    protectionActive = false;
    blinkInterval = 2000;

    WiFi.mode(WIFI_AP_STA);
    numScanned = WiFi.scanNetworks();
    if (numScanned > 10) numScanned = 10;
    for (int i = 0; i < numScanned; i++) {
      scannedSSIDs[i] = WiFi.SSID(i);
    }
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSSID, apPassword);

    Serial.print("AP SSID: ");
    Serial.println(apSSID);
    Serial.print("Open http://");
    Serial.println(WiFi.softAPIP());

    server.on("/", handleRoot);
    server.on("/save", handleSave);
    server.on("/generate_204", handleRoot);
    server.on("/fwlink", handleRoot);
    server.begin();

    while (true) {
      server.handleClient();
      updateLED();
      delay(10);
    }
  }
}

void loop() {
  server.handleClient();
  updateLED();

  int packetSize = udp.parsePacket();
  if (packetSize) {
    uint8_t packetBuffer[512];
    int len = udp.read(packetBuffer, 512);

    IPAddress clientIP = udp.remoteIP();

    String domain = "";
    int pos = 12;
    while (pos < len && packetBuffer[pos] != 0) {
      int labelLen = packetBuffer[pos++];
      for (int i = 0; i < labelLen; i++) domain += (char)packetBuffer[pos++];
      if (packetBuffer[pos] != 0) domain += ".";
    }
    domain.toLowerCase();
    if (domain.endsWith(".")) domain.remove(domain.length() - 1);

    String queryLine = "DNS query from " + clientIP.toString() + ": " + domain;
    addLog(queryLine);

    bool isBlocked = false;
    for (int i = 0; i < numBlocked; i++) {
      if (domain.endsWith(blockedDomains[i])) {
        isBlocked = true;
        break;
      }
    }

    IPAddress resolvedIP(0, 0, 0, 0);
    if (isBlocked) {
      addLog("-> BLOCKED (0.0.0.0)");
      blockFlashEnd = millis() + 100;
    } else {
      if (WiFi.hostByName(domain.c_str(), resolvedIP)) {
        String resLine = "-> Resolved: " + resolvedIP.toString();
        addLog(resLine);
      } else {
        addLog("-> Upstream resolve failed (fallback 0.0.0.0)");
      }
    }

    uint8_t response[512];
    memcpy(response, packetBuffer, len);
    response[2] |= 0x80;
    response[3] |= 0x80;
    response[7] = 1;
    int respLen = len;
    response[respLen++] = 0xC0; response[respLen++] = 0x0C;
    response[respLen++] = 0x00; response[respLen++] = 0x01;
    response[respLen++] = 0x00; response[respLen++] = 0x01;
    response[respLen++] = 0x00; response[respLen++] = 0x00;
    response[respLen++] = 0x00; response[respLen++] = 0x3C;
    response[respLen++] = 0x00; response[respLen++] = 0x04;
    response[respLen++] = resolvedIP[0];
    response[respLen++] = resolvedIP[1];
    response[respLen++] = resolvedIP[2];
    response[respLen++] = resolvedIP[3];

    udp.beginPacket(clientIP, udp.remotePort());
    udp.write(response, respLen);
    udp.endPacket();
  }
}
