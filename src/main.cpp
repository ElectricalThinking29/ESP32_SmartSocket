#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
// Thu vien thoi gian thuc: https://hocarm.org/dong-ho-thoi-gian-thuc-cho-esp8266/
// Fix loi khong lay duoc thong tin tu url, thu lam theo: https://arduino.stackexchange.com/questions/33101/use-server-arg-results-as-a-function-parameter

// SSID & Password
const char *ssid = "****";
const char *password = "****";

WebServer server(80);

const int controlPin = 14; // GPIO14 (D5)

String datetime = "YYYY-MM-DDTHH:MM";
String timezone = "UTC";
String action = "on";

struct tm timeinfo;

// Nội dung HTML & CSS hiển thị trên web server
String HTML = R"rawliteral(
<!DOCTYPE html>
<html>
  <body>
    <h1> ESP32 Web Server </h1>   
    <p> Relay State: %PINSTATE% </p>
    <button onclick="window.location.href='/relayOn'"> Turn On Socket </button>
    <button onclick="window.location.href='/relayOff'"> Turn Off Socket </button>

    <h2>Set Relay On/Off Time</h2>
    <button onclick="window.location.href='/setDateTime'"> Set Timer </button>
    <button onclick="window.location.href='/cancelDateTime'"> Cancel Timer </button>
    <p> Scheduled Time: SCHEDULED </p>
    <form action="/" method="get">
      <label for="datetime"> Date and time: </label>
      <input type="datetime-local" id="datetime" name="datetime">

      <label for="timezone"> Timezone: </label>
      <select id="timezone" name="timezone">
        <option value="UTC">UTC</option>
        <option value="Europe/London">London</option>
        <option value="America/New_York">New York</option>
        <option value="Asia/Tokyo">Tokyo</option>
        <option value="Asia/HochiminhCity">Ho Chi Minh City</option>
        <!-- other timezones -->
      </select>

      <label for="action">Action:</label>
      <select id="action" name="action">
        <option value="on">Turn On</option>
        <option value="off">Turn Off</option>
      </select>
    </form>
    
    <h2>Timer History</h2>
    <table>
    <tr> <th>Datetime</th> <th>Timezone</th> <th>Action</th> <th>Set</th> </tr>
    <tr> <td>DATETIME</td> <td>TIMEZONE</td> <td>ACTION</td> <td>SET?</td></tr>
    </table>
  </body>
</html>
)rawliteral";

// Xử lý url gốc (/)
void handle_root()
{
  datetime = server.arg("datetime");
  timezone = server.arg("timezone");
  action = server.arg("action");
  Serial.print("Datetime: " + datetime);
  Serial.print(", Timezone: " + timezone);
  Serial.println(", Action: " + action);

  server.send(200, "text/html", HTML);
}

// Turn relay on
void relayOn()
{
  digitalWrite(controlPin, HIGH);
  HTML.replace("%PINSTATE%", "ON");
  HTML.replace("OFF", "ON");
  server.send(200, "text/html", HTML);
}

// Turn relay off
void relayOff()
{
  digitalWrite(controlPin, LOW);
  HTML.replace("%PINSTATE%", "OFF");
  HTML.replace("ON", "OFF");
  server.send(200, "text/html", HTML);
}

// Set relay on/off time
void setDateTime()
{
  // Get datetime from url
  handle_root();

  HTML.replace("NOT SCHEDULED", "SCHEDULED");
  HTML.replace("<tr> <td>DATETIME</td> <td>TIMEZONE</td> <td>ACTION</td> <td>SET?</td></tr>", "<tr> <td>"+ datetime +"</td> <td>"+ timezone +"</td> <td>" + action + "</td> <td>" + "SET" + "</td> </tr> <tr> <td>DATETIME</td> <td>TIMEZONE</td> <td>ACTION</td> <td>SET?</td></tr>");
  server.send(200, "text/html", HTML);
}

int getSlectedTimezone(String timezone)
{
  if (timezone == "UTC")
  {
    return 0;
  }
  else if (timezone == "Europe/London")
  {
    return 3600;
  }
  else if (timezone == "America/New_York")
  {
    return -14400;
  }
  else if (timezone == "Asia/Tokyo")
  {
    return 32400;
  }
  else if (timezone == "Asia/HochiminhCity")
  {
    return 25200;
  }
  else
  {
    return 0;
  }
}

// Check datetime
void checkDateTime()
{
  // Compare datetime with current time
  if(!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  String nowdatetime = String(timeinfo.tm_year + 1900) + "-" + String(timeinfo.tm_mon + 1) + "-" + String(timeinfo.tm_mday) + "T" + String(timeinfo.tm_hour) + ":" + String(timeinfo.tm_min);
  if(action == "on")
  {
    if(nowdatetime == datetime)
    {
      relayOn();
    }
  } else if(action == "off")
  {
    if(nowdatetime == datetime)
    {
      relayOff();
    }
  }
}

// Cancel relay on/off time
void cancelDateTime()
{
  datetime = "YYYY-MM-DDTHH:MM";
  timezone = "UTC";
  action = "on";

  HTML.replace("SCHEDULED", "NOT SCHEDULED");
  HTML.replace("<tr> <td>DATETIME</td> <td>TIMEZONE</td> <td>ACTION</td> <td>SET?</td></tr>", "<tr> <td>"+ datetime +"</td> <td>"+ timezone +"</td> <td>" + action + "</td> <td>" + "RESET" + "</td> </tr> <tr> <td>DATETIME</td> <td>TIMEZONE</td> <td>ACTION</td> <td>SET?</td></tr>");
  server.send(200, "text/html", HTML);
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
  server.on("/setDateTime", setDateTime);
  server.on("/cancelDateTime", cancelDateTime);

  server.begin();
  Serial.println("HTTP server started");
  delay(100);
}

void loop()
{
  server.handleClient();
  //checkDateTime();
}
