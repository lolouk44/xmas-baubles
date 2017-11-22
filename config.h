/*
 * This is a sample configuration file for the "Xmas Bauble" light from http://www.instructables.com/id/Illuminated-Christmas-Tree-Ornament-WiFi-Controlle/
 *
 * Change the settings below and save the file as "config.h"
 * You can then upload the code using the Arduino IDE.
 * Original code from https://github.com/corbanmailloux/esp-mqtt-rgb-led
 *
 * See https://github.com/lolouk44/xmas-baubles for more info
 *
 */



// WiFi
#define CONFIG_WIFI_SSID "SSID_GOES_HERE"
#define CONFIG_WIFI_PASS "PASSWORD_GOES_HERE"

// MQTT
#define CONFIG_MQTT_HOST "MQTT_SERVER_ADDRESS_GOES_HERE"
#define CONFIG_MQTT_USER "USERNAME_GOES_HERE"
#define CONFIG_MQTT_PASS "PASSWORD_GOES_HERE"

#define CONFIG_MQTT_CLIENT_ID "xmas-bauble-1" // Must be unique on the MQTT network

// MQTT Topics
#define CONFIG_MQTT_TOPIC_STATE "Baubles/Bauble1"
#define CONFIG_MQTT_TOPIC_SET "Baubles/Bauble1/set"

#define CONFIG_MQTT_PAYLOAD_ON "ON"
#define CONFIG_MQTT_PAYLOAD_OFF "OFF"


