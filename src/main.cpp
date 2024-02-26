#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
//#include <NTPClient>

// SSID & Password
const char *ssid = "*****";
const char *password = "*****";

WebServer server(80);

const int controlPin = 14; // GPIO14 (D5)


// Nội dung HTML & CSS hiển thị trên web server
String HTML = R"rawliteral(
<!DOCTYPE html>
<html>
  <body>
    <h1>ESP32 Web Server</h1>   
    <p>Relay State: %PINSTATE%</p>
    <button onclick="window.location.href='/relayOn'">Turn On Socket</button>
    <button onclick="window.location.href='/relayOff'">Turn Off Socket</button>


    <h2>Set Relay On/Off Time</h2>
    <form>
      <label for="datetime">Date and time:</label>
      <input type="datetime-local" id="datetime" name="datetime">

      <label for="timezone">Timezone:</label>
      <select id="timezone" name="timezone">
        <option value="UTC">UTC</option>
        <option value="Europe/London">London</option>
        <option value="America/New_York">New York</option>
        <!-- other timezones -->
      </select>

      <label for="action">Action:</label>
      <select id="action" name="action">
        <option value="on">Turn On</option>
        <option value="off">Turn Off</option>
      </select>
    
      <input type="submit" value="Set" onclick="window.location.href='/setDateTime'">
    </form>

  </body>
</html>
)rawliteral";

// Xử lý url gốc (/)
void handle_root()
{
  server.send(200, "text/html", HTML);
}

// Turn relay on
void relayOn()
{
  digitalWrite(controlPin, HIGH);
  // server.send(200, "text/plain", "Relay turned on");
  HTML.replace("%PINSTATE%", "ON");
}

// Turn relay off
void relayOff()
{
  digitalWrite(controlPin, LOW);
  // server.send(200, "text/plain", "Relay turned off");
  HTML.replace("%PINSTATE%", "OFF");
}

// Set relay on/off time
void setDateTime(){
  String date = server.arg("datetime");
  String timezone = server.arg("timezone");
  String action = server.arg("action");
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Try Connecting to ");
  Serial.println(ssid);

  pinMode(controlPin, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected successfully");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handle_root);
  server.on("/relayOn", relayOn);
  server.on("/relayOff", relayOff);

  server.begin();
  Serial.println("HTTP server started");
  delay(100);
}

void loop()
{
  server.handleClient();
}
