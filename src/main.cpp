#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// SSID & Password
const char *ssid = "YOUR_SSID";
const char *password = "YOUR_PASSWORD";

// Set your Static IP address
IPAddress local_IP(192, 168, 1, 184);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

// WebServer at port 80
WebServer server(80);

// Relay control pin (GPIO14 - D14)
const int controlPin = 14; // GPIO14 (D14)

String datetime = "YYYY-MM-DDTHH:MM";
String timezone = "UTC";
String action = "on";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

bool TimerSet = false;

// HTML content
String HTML = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<body style="text-align: center;">

<h1> ESP32 Web Server </h1>   
<p> Relay State: %PINSTATE% </p>
<button onclick="window.location.href='/relayOn'"> Turn On Socket </button>
<button onclick="window.location.href='/relayOff'"> Turn Off Socket </button>
<button onclick="window.location.href='/'"> Reload </button>

<h2>Set Relay On/Off Time</h2>

<p> Scheduled Time: NOT SCHEDULED </p>

<form action="/setDateTime" method="get">

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

<input type="submit" value="Set">
<input type="button" value="Cancel" onclick="window.location.href='/cancelDateTime'">
</form>

<h2>Timer History</h2>
<table style="width:100%" border="1" cellspacing="0" cellpadding="3">
<tr> <th>Datetime</th> <th>Timezone</th> <th>Action</th> <th>Set</th> </tr>
<tr> <td>DATETIME</td> <td>TIMEZONE</td> <td>ACTION</td> <td>SET?</td> </tr> 
</table>

</body>
</html>

)rawliteral";

/*Function declaration*/
void handle_root();
void relayOn();
void relayOff();
void setDateTime();
int getSlectedTimezone();
//unsigned long convertToEpoch(String datetime, int timezoneOffset);

void checkDateTime();
void cancelDateTime();

// URI Root handle (/)
void handle_root()
{
  server.send(200, "text/html", HTML);
}

// Turn relay on
void relayOn()
{
  digitalWrite(controlPin, LOW);
  HTML.replace("%PINSTATE%", "ON");
  HTML.replace("OFF", "ON");
  server.send(200, "text/html", HTML);
  Serial.println("Relay On");
}

// Turn relay off
void relayOff()
{
  digitalWrite(controlPin, HIGH);
  HTML.replace("%PINSTATE%", "OFF");
  HTML.replace("ON", "OFF");
  server.send(200, "text/html", HTML);
  Serial.println("Relay Off");
}

// Set relay on/off time
void setDateTime()
{
  // Get datetime from uri
  datetime = server.arg("datetime");
  timezone = server.arg("timezone");
  action = server.arg("action");

  Serial.print("Datetime: " + datetime);
  Serial.print(", Timezone: " + timezone);
  Serial.println(", Action: " + action);

  HTML.replace("NOT SCHEDULED", "SCHEDULED");

  String newRow = "<tr> <td>" + datetime + "</td> <td>" + timezone + "</td> <td>" + action + "</td> <td>" + "SET" + "</td> </tr>" + "<tr> <td>DATETIME</td> <td>TIMEZONE</td> <td>ACTION</td> <td>SET?</td> </tr> ";

  HTML.replace("<tr> <td>DATETIME</td> <td>TIMEZONE</td> <td>ACTION</td> <td>SET?</td> </tr> ", newRow);

  server.send(200, "text/html", HTML);

  TimerSet = true;
}

// Convert timezone to offset seconds
int getSlectedTimezone(String &timezone)
{
  if (timezone == "UTC")
  {
    return 0;
  }
  else if (timezone == "Europe/London")
  {
    return -3600;
  }
  else if (timezone == "America/New_York")
  {
    return +14400;
  }
  else if (timezone == "Asia/Tokyo")
  {
    return -32400;
  }
  else if (timezone == "Asia/HochiminhCity")
  {
    return -25200;
  }
  else
  {
    return -1;
  }
}

// Convert datetime to epoch, seconds from 1st Jan 1970
unsigned long convertToEpoch(String &datetime, int timezoneOffset)
{
  struct tm tm;
  time_t epochTime;
  if (datetime.length() > 0)
  {
    strptime(datetime.c_str(), "%Y-%m-%dT%H:%M", &tm);
    epochTime = mktime(&tm);
  }
  return epochTime + timezoneOffset;
}

// Check datetime
void checkDateTime()
{
  timeClient.update();

  // Select timezone
  int timezoneOffset = getSlectedTimezone(timezone);
  Serial.println("Timezone: "+String(timezoneOffset));
  
  // Get current time
  unsigned long nowdatetimeEpoch = timeClient.getEpochTime();
  Serial.println("Now: "+ String(nowdatetimeEpoch));
  //unsigned long nowdatetime = 1638308400; // 2021-12-03 13:00:00
  
  // Convert datetime to epoch
  unsigned long SetdatetimeEpoch = 0;
  if(datetime.length() > 0){
    SetdatetimeEpoch = convertToEpoch(datetime, timezoneOffset);
  Serial.println("Set: "+ String(SetdatetimeEpoch));
  }

  // Compare datetime with current time
  if (SetdatetimeEpoch == 0)
  {
    Serial.println("Failed to convert time");
    return;
  } else if (nowdatetimeEpoch == SetdatetimeEpoch)
  {
    if (action == "on")
    {
      relayOn();
      TimerSet = false;
      cancelDateTime();
      // Reload page
      server.sendHeader("Location", "/");
      server.send(303);
    }
    else if (action == "off")
    {
      relayOff();
      TimerSet = false;
      cancelDateTime();
      // Reload page
      server.sendHeader("Location", "/");
      server.send(303);
    }
  }
  delay(1000);
}

// Cancel relay on/off time
void cancelDateTime()
{
  datetime = "YYYY-MM-DDTHH:MM";
  timezone = "UTC";
  action = "on";

  HTML.replace("SCHEDULED", "NOT SCHEDULED");
  HTML.replace("<tr> <td>DATETIME</td> <td>TIMEZONE</td> <td>ACTION</td> <td>SET?</td> </tr> ", "<tr> <td> Delete timer </td> <td>"+ timezone +"</td> <td>" + action + "</td> <td>" + "RESET" + "<tr> <td>DATETIME</td> <td>TIMEZONE</td> <td>ACTION</td> <td>SET?</td> </tr> ");
  server.send(200, "text/html", HTML);

  TimerSet = false;
}

// Setup
void setup()
{
  // Initialize serial port (USBCDC with baudrate 115200)
  Serial.begin(115200);
  Serial.println("Try Connecting to ");
  Serial.println(ssid);

  // Initialize relay pin
  pinMode(controlPin, OUTPUT);
  digitalWrite(controlPin, HIGH); // Default state is OFF

  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  // Initialize WiFi
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

  // Initialize WebServer and all /handlers
  server.on("/", handle_root);
  server.on("/relayOn", relayOn);
  server.on("/relayOff", relayOff);
  server.on("/setDateTime", setDateTime);
  server.on("/cancelDateTime", cancelDateTime);
  server.begin();
  Serial.println("HTTP server started");

  // Initialize NTPClient
  timeClient.begin();
  delay(100);
}

void loop()
{
  server.handleClient();
  if (TimerSet == true)
  {
    checkDateTime();
  }
}
