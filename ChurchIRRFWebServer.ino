#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
// https://github.com/tzapu/WiFiManager

#include <uri/UriBraces.h>
#include <uri/UriRegex.h>

// Needed to send RF messages
#include <RCSwitch.h>

// Needed to send IR messages
// See https://github.com/Arduino-IRremote/Arduino-IRremote by Armin Joachimsmeyer
#define DISABLE_CODE_FOR_RECEIVER
#include "PinDefinitionsAndMore.h"
#include <IRremote.hpp> // include the library

ESP8266WebServer server(80);

RCSwitch mySwitch = RCSwitch();

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);

  // Transmitter is connected to Arduino Pin #4 (NodeMCU Pin D2)
  mySwitch.enableTransmit(4);

  // Optional set pulse length.
  mySwitch.setPulseLength(276);

  Serial.print(F("Send IR signals at pin "));
  Serial.println(IR_SEND_PIN);
  IrSender.begin(DISABLE_LED_FEEDBACK); // Start with IR_SEND_PIN as send pin and disable feedback LED at default feedback LED pin


  WiFi.mode(WIFI_STA);
  
  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;
  // reset settings - wipe stored credentials for testing
  // these are stored by the esp library
  //wm.resetSettings();

  // Sets timeout until configuration portal gets turned off
  // Useful to make it all retry or go to sleep in seconds
  wm.setConfigPortalTimeout(180);  // Timeout after 3 minutes
  Serial.println("");

  // "ConfigureLCRemote" is the SSID when it cannot connect to a stored WiFi network
  if(!wm.autoConnect("ConfigureLCRemote")) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    // Reset and try again
    ESP.restart();
    delay(5000);
  }
  
  //if you get here you have connected to the WiFi    
  Serial.println("connected...yeey :)");

  if (MDNS.begin("lcRemote")) {
    Serial.println("MDNS responder started at http://lcRemote.local");
  }

  server.on(F("/"), []() {
    String resp = "Hello from lcRemote!\r\nCurrent Endpoints:\r\n/set/pulseLength/<length>\r\n/rf/<code>/<length>\r\n/ir/NEC/<address>/<command>/<numRepeats>";
    server.send(200, "text/plain", resp);
  });
//todo: Fix below!
  // Handle setting the pulseLength. Should include "/set/pulseLength/<length>"
  server.on(UriBraces("/set/pulseLength/{}"), []() {
    String length = server.pathArg(0);
    String resp = "Something went wrong handling pulseLength";
    int i_length = length.toInt();
    if (i_length > 0){  // Length should always be > zero
      mySwitch.setPulseLength(i_length);
      resp = "Called mySwitch.setPulseLength(" + length + ")";
    }
    server.send(200, "text/plain", resp);
    Serial.println(resp);
  });

  // Handle rf calls. Should include "/rf/<code>/<length>"
  server.on(UriBraces("/rf/{}/{}"), []() {
    unsigned long ul_code = 0;
    unsigned long ul_length = 0;
    unsigned int ui_length = 0;
    String resp = "Something went wrong";

    String code = server.pathArg(0);
    String length = server.pathArg(1);
    ul_code = strtoul(server.pathArg(0).c_str(), NULL, 10);  // Convert first arg to unsigned long
    ul_length = strtoul(server.pathArg(1).c_str(), NULL, 10);
    if (ul_length <= 65535)  // Check for overflow before assigning unsigned long to unsigned int
    {
      ui_length = (unsigned int)ul_length;
    }
    
    if(ul_code != 0 && ui_length != 0)
    {
      mySwitch.send(ul_code, ul_length);
      resp = "Called mySwitch.send(" + code + ", " + length + ")";

      server.send(200, "text/plain", resp);
      //Serial.println(resp);
    }
    else
    {
      resp = "Code '" + code + "' or Length '" + length + "' was not an acceptable value.";
      server.send(400, "text/plain", resp);
      //Serial.println(resp);
    }
  });

  // Handle ir calls. Should include "/ir/NEC/<address>/<command>/<numRepeats>"
  server.on(UriBraces("/ir/NEC/{}/{}/{}"), []() {
    String resp = "Something went wrong";
    String address = server.pathArg(0);
    String command = server.pathArg(1);
    String numRepeats = server.pathArg(2);
    unsigned long ul_address = 0;
    uint16_t ui16_address = 0;
    unsigned long ul_command = 0;
    uint8_t ui8_command = 0;

    long l_numRepeats = 0;
    int_fast8_t if8_numRepeats = 0;

    // Check for valid address and convert to uint16_t
    ul_address = strtoul(address.c_str(), NULL, 10);
    if (ul_address <= 65535)
    {
      ui16_address = (uint16_t)ul_address;
    }

    // Check for valid command and convert to uint8_t
    ul_command = strtoul(command.c_str(), NULL, 10);
    if (ul_command <= 255){
      ui8_command = (uint8_t)ul_command;
    }

    // Check for valid numRepeats and convert to int_fast8_t
    l_numRepeats = strtol(numRepeats.c_str(), NULL, 10);
    if (l_numRepeats <= 127 && l_numRepeats >= -128)
    {
      if8_numRepeats = (int_fast8_t)l_numRepeats;
    }

    // Vars only can be 0 when they are given as a string "0"
    if((if8_numRepeats != 0 || numRepeats == "0") && (ui16_address != 0 || address == "0") && (ui8_command != 0 || command == "0"))
    {
      //IrSender.sendNEC(ui16_address, ui8_command, if8_numRepeats);
      resp = "Called IrSender.sendNEC(" + address + ", " + command + ", " + numRepeats + ")";
      server.send(200, "text/plain", resp);
      //Serial.println(resp);
    }
    else
    {
      resp = "Address '" + address + "' or Command '" + command + "' or numRepeats '" + numRepeats + "' was not an acceptable value.";
      server.send(400, "text/plain", resp);
      //Serial.println(resp);
    }
  });

  server.begin();
  //Serial.println("HTTP server started");
  MDNS.addService("http", "tcp", 80);

  digitalWrite(LED_BUILTIN, HIGH); // Turns the LED Off
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}
