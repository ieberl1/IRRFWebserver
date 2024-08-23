# IRRFWebserver
This repository will enable a nodemcu (ESP8266) to interact with an IR devices as well as RF devices and be controlled via HTTP.
I use it to control a projector via an infrared blaster as well as a light via it's RF.

# Dependencies
All dependencies can be installed via the Arduino Library Manager
* WiFiManager: https://github.com/tzapu/WiFiManager by tzapu
* Arduino-IRremote: https://github.com/Arduino-IRremote/Arduino-IRremote by Armin Joachimsmeyer
* ...

# Use
* Compile & push to your esp8266
* Connect to the device over WiFi using the configured SSID (currently set to 'ConfigureLCRemote')
* Use the page that pops up to configure the SSID/password of the WiFi network it should connect to (generally your home WiFi network)

# Commands
* The webserver will be available at http://lcRemote.local
* Send RF commands -> http://lcRemote.local/rf/<code\>/\<length\>
* Send IR commands -> http://lcRemote.local/ir/NEC/<address\>/<command\>/<numRepeats\>
