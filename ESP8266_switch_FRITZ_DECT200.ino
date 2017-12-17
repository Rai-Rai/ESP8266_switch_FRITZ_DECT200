/*  ESP8266_switch_Fritz_DECT200
    Copyright (C) 2017 Eike Benedikt Lotz

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

    By courtesy of Sven Bursch-Osewold:
    <http://www.open4me.de/index.php/2016/10/fritzbox-esp8266-laesst-telefon-klingeln/>
    Copyright (C) 2016 Sven Bursch-Osewold
*/
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
// Wi-Fi network details
const char* ssid = "Your_Wi-Fi_SSID";
const char* password = "Your_Wi-Fi-password";   // Leave empty in case it's a public network

// FritzBox address and user details
// I'd suggest to add a user to your FritzBox that has only access to
// smart home devices.
const char* fb_address = "fritz.box";   // IP addresses will also work
const char* fb_user = "user";
const char* fb_pwd = "password";

// FritzBox actor AIN (only one can be switched with this sketch)
// Can be obtained from the details page from the DECT200 socket
// you want to switch, don't use spaces!
const char* fb_ain = "012345678901";

// Should the actor be switched on or off
const boolean switch_on = true;

// Set the hostname for your Arduino device
const char* WiFi_hostname = "ESP8266";

// Use DHCP or a static setup. Otherwise you can also tell your
// FritzBox to always assign the same IP.
// Useful if you want to deny internet access to this device.
const boolean use_dhcp = true;
const int static_ip[4] = {192, 168, 1, 10};
const int static_gateway[4] = {192, 168, 1, 1};
const int static_subnet[4] = {255, 255, 255, 0};
const int static_dns[4] = {192, 168, 1, 1};

// Set how often the device should be tried to be switched
// Just in case... There are certainly better solutions like
// checking if the DECT200 change state etc.
// Not yet implemented.
const int iterations = 3;

// Define global variables
char* task;

void setup() {
  // Open serial connection
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.persistent(true); // Memorise WiFi connection settings instead of always writing them to flash also when there was no change

  // Set hostname of this device
  WiFi.hostname(WiFi_hostname);

  // Set static IP, gateway and subnet if enabled
  if (use_dhcp == false) {
    IPAddress ip(static_ip[0], static_ip[1], static_ip[2], static_ip[3]);
    IPAddress dns(static_dns[0], static_dns[1], static_dns[2], static_dns[3]);
    IPAddress gateway(static_gateway[0], static_gateway[1], static_gateway[2], static_gateway[3]);
    IPAddress subnet(static_subnet[0], static_subnet[1], static_subnet[2], static_subnet[3]);
    WiFi.config(ip, gateway, subnet, dns);
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);

  // Print connection status
  Serial.print("\nWaiting for Wi-Fi connection");
  // Wait for the Wi-Fi connection completion
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  // Print connection details when the connection is up
  Serial.println("\n");
  Serial.println("Wi-Fi connected");
  Serial.println();
  Serial.println("SSID:     " + WiFi.SSID());
  Serial.println("Hostname: " + WiFi.hostname());
  Serial.println("IP:       " + String(WiFi.localIP()[0]) + "." + WiFi.localIP()[1] + "." + WiFi.localIP()[2] + "." + WiFi.localIP()[3]);
  Serial.println("Gateway:  " + String(WiFi.gatewayIP()[0]) + "." + WiFi.gatewayIP()[1] + "." + WiFi.gatewayIP()[2] + "." + WiFi.gatewayIP()[3]);
  Serial.println("Subnet:   " + String(WiFi.subnetMask()[0]) + "." + WiFi.subnetMask()[1] + "." + WiFi.subnetMask()[2] + "." + WiFi.subnetMask()[3]);
  Serial.println("DNS:      " + String(static_dns[0]) + "." + String(static_dns[1]) + "." + String(static_dns[2]) + "." + String(static_dns[3]));
  Serial.println();

  // Print if the actor is switched on or of
  if (switch_on == true) {
    Serial.println("Switching actor on\n");
  }
  if (switch_on == false) {
    Serial.println("Switching actor off\n");
  }
}

int counter = 0;
void loop() {
  if (counter < iterations) {
    counter ++;

    if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
      HTTPClient http;

      // Get Challenge
      http.begin("http://" + String(fb_address) + "/login_sid.lua");
      int retCode = http.GET();
      if (retCode != 200) {
        Serial.println("Get challenge failed!");
        Serial.println("Error code: " + String(retCode) + "\n");
      }
      String result = http.getString();
      String challenge = result.substring(result.indexOf("<Challenge>") + 11, result.indexOf("<Challenge>") + 19);

      // Calculate Response
      String reponseASCII = challenge + "-" + String(fb_pwd);
      String responseHEX = "";
      for (unsigned int i = 0; i  < reponseASCII.length(); i++) {
        responseHEX = responseHEX + String(reponseASCII.charAt(i), HEX) + "00";
      }

      MD5Builder md5;
      md5.begin();
      md5.addHexString(responseHEX);
      md5.calculate();

      String response = challenge + "-" + md5.toString();

      // Login and get SID
      http.begin("http://" + String(fb_address) + "/login_sid.lua?user=" + String(fb_user) + "&response=" + response);
      retCode = http.GET();
      if (retCode != 200) {
        Serial.println("Get SID failed!");
        Serial.println("Error code: " + String(retCode) + "\n");
      }
      result = http.getString();
      String sid = result.substring(result.indexOf("<SID>") + 5,  result.indexOf("<SID>") + 21);

      // Wait 2 sec
      delay(2000);

      // Switch DECT200 on
      if (switch_on == true) {
        http.begin("http://" + String(fb_address) + "/webservices/homeautoswitch.lua?switchcmd=setswitchon&sid=" + sid + "&ain=" + String(fb_ain));
      }
      // Switch DECT200 off
      if (switch_on == false) {
        http.begin("http://" + String(fb_address) + "/webservices/homeautoswitch.lua?switchcmd=setswitchoff&sid=" + sid + "&ain=" + String(fb_ain));
      }
      // If the reutrn code is != 200 forward the error to the serial monitor
      retCode = http.GET();
      if (retCode != 200) {
        Serial.println("Switching on DECT200 failed!");
        Serial.println("Error code: " + String(retCode) + "\n");
      }

      // Wait 2 sec
      delay(2000);

      // Logout
      http.begin("http://" + String(fb_address) + "/login_sid.lua?logout=1&sid=" + sid);
      http.GET();
      result = http.getString();
      http.end();
    }
    if (switch_on == true) {
      task = "on";
    }
    if (switch_on == false) {
      task = "off";
    }

    if (counter == 1) {
      Serial.println("Tried switching " + String(task) + " once");
    }
    else {
      Serial.println("Tried switching " + String(task) + " " + String(counter) + " times");
    }
    if (counter == iterations) {
      WiFi.disconnect();
    }
  }
}
