/*
 * This is the code to control Xmas Bauble lights for Home Assistant from http://www.instructables.com/id/Illuminated-Christmas-Tree-Ornament-WiFi-Controlle/
 *
 * You can upload the code using the Arduino IDE.
 * Additional code from https://github.com/corbanmailloux/esp-mqtt-rgb-led
 *
 * See https://github.com/lolouk44/xmas-baubles for more info
 *
 */
 
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Adafruit_NeoPixel.h>
#include <user_interface.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
    
#define NUM_PIXELS 20
Adafruit_NeoPixel pixels(NUM_PIXELS, D2, NEO_GRB | NEO_KHZ800);

// Set configuration options for pins, WiFi, and MQTT in the following file:
#include "config.h"

MDNSResponder mdns;

os_timer_t osTimer;

// network credentials
const char* ssid     = CONFIG_WIFI_SSID;
const char* password = CONFIG_WIFI_PASS;

//18 colours
int R[18] = {255,0,255,0,255,0,255,0,160,80,80,160,0,255,0,255,0,255};
int G[18] = {0,255,80,160,160,80,255,0,255,0,255,0,255,0,255,0,255,0};
int B[18] = {0,255,0,255,0,255,0,255,0,255,0,255,0,255,80,160,160,80};

int countColors = 0;
int numColors = 18;
int FlashMode = 1;
String FlashModeText = "Sparkle";
bool WemosStatus = true;

// Buttons text colors and initial values
String buttonColor [2] = {"white", "black"};
IPAddress WemosIP;
boolean ColorState [18] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}; // initial colors
boolean buttonSparkle  = 1; 
boolean buttonBrighter = 1; 
boolean buttonFaster   = 1;
boolean buttonDarker   = 1;
boolean buttonSlower   = 1; 
boolean buttonReset    = 1; 

int brightValue = 4;
int brightMax   = 16; // brightness (1..16)

int waitTimes [16]= {25, 50, 100, 150, 200, 250, 500, 750, 1000, 1500, 2000, 2500, 3000, 5000, 10000, 30000}; // millis()
int waitTime     = 1; // default values
int lastColor    = 0;
int foundColor   = 0;
int currentColor = 0;
int beginSparkle = 0;
int SparklePos = 0;
int SparkleLastPos = 0;
int currentLed   = 0;


//MQTT Config
const char* mqtt_server = CONFIG_MQTT_HOST;
const char* mqtt_username = CONFIG_MQTT_USER;
const char* mqtt_password = CONFIG_MQTT_PASS;
const char* client_id = CONFIG_MQTT_CLIENT_ID;

const char* light_state_topic = CONFIG_MQTT_TOPIC_STATE;
const char* light_set_topic = CONFIG_MQTT_TOPIC_SET;

const char* on_cmd = CONFIG_MQTT_PAYLOAD_ON;
const char* off_cmd = CONFIG_MQTT_PAYLOAD_OFF;

const int BUFFER_SIZE = JSON_OBJECT_SIZE(15);
bool stateOn = true;

WiFiClient espClient;
PubSubClient client(espClient);



ESP8266WebServer server(80);

String webPage = "";

void setColor(int Rs, int Gs, int Bs) 
{
   for (int i = 0; i < NUM_PIXELS; i++) 
   {
      pixels.setPixelColor (i, (Rs * brightValue)/brightMax, (Gs * brightValue)/brightMax, (Bs * brightValue)/brightMax);
   }
   pixels.show();
}

int nextColor (int lastColor)
{
  foundColor = numColors; // nothing found return value
  countColors = 0;        // count number of searches inside the loop

  do
  {
    currentColor += 1;
    countColors  += 1;
    if (currentColor>numColors)   {currentColor=0;}
    if (ColorState[currentColor]) {foundColor=currentColor;}
  }
  while (currentColor != lastColor && foundColor == numColors && countColors < numColors+1);
  return (foundColor);
  }

// interrupt os-timer
void timerCallback(void *pArg) 
{
  if (WemosStatus)
  {
    switch (FlashMode)
    {
      case 1:
        //Sparkle
        FlashModeText = "Sparkle";
        SparkleLastPos = SparklePos;
        SparklePos = random(0,20);                            // Startposition for first color
        pixels.setPixelColor (SparkleLastPos, (0 * brightValue)/brightMax, (0 * brightValue)/brightMax, (255 * brightValue)/brightMax);
        pixels.setPixelColor (SparklePos, (127 * brightValue)/brightMax, (255 * brightValue)/brightMax, (212 * brightValue)/brightMax);

        pixels.show();
        break;

      case 2:
        //Rainbow
        FlashModeText = "Rainbow";
        beginSparkle += 1;                            // Startposition for first color
        if (beginSparkle>numColors-1) {beginSparkle=0;}

        currentColor = 0;                             // Begin with first color in array
        for (int x = 0; x < NUM_PIXELS; x++) 
        {
          currentColor = nextColor (lastColor);
          lastColor = currentColor;
          currentLed = beginSparkle + x;
          if (currentLed >= NUM_PIXELS) 
          {
            currentLed = currentLed - NUM_PIXELS;
          }
          pixels.setPixelColor (currentLed, (R[currentColor] * brightValue)/brightMax, (G[currentColor] * brightValue)/brightMax, (B[currentColor] * brightValue)/brightMax);
        }
        pixels.show();
        break;

      case 3:
        //Uniglow
        FlashModeText = "Uniglow";
        SparklePos = random(0,18);
        for (int i = 0; i < NUM_PIXELS; i++) 
        {
          pixels.setPixelColor (i, (R[SparklePos] * brightValue)/brightMax, (G[SparklePos] * brightValue)/brightMax, (B[SparklePos] * brightValue)/brightMax);
        }
        pixels.show();
        break;

    }
  }
  else
  {
    for (int x = 0; x < NUM_PIXELS; x++) 
    {
      pixels.setPixelColor (x, (0,0,0));
    }
    pixels.show();
  }
}

void setup(void){

  // neopixel
  pixels.begin();
  pixels.show();
  setColor(4,4,4); // wait for IP

  // preparing GPIOs
  delay(1000);
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  WemosIP = WiFi.localIP();
  client.setServer(mqtt_server, 16306);
  client.setCallback(callback);  




  // timer
  os_timer_setfn (&osTimer, timerCallback, NULL);
  os_timer_arm   (&osTimer, waitTimes[waitTime], true);

  server.on ("/", [](){showPage(); });
  server.on ("/socketBri", [](){brightValue += 1; 
                 buttonDarker = 1; 
                 if (brightValue>=16){brightValue=16; buttonBrighter=0;}
                 showPage();  });

  server.on ("/socketFas", [](){waitTime -= 1;
                 buttonSlower = 1; 
                 if (waitTime<=0){waitTime=0; buttonFaster=0;} 
                 os_timer_arm (&osTimer, waitTimes[waitTime], true);
                 showPage(); });

  server.on ("/socketDar", [](){brightValue -= 1; 
                 buttonBrighter = 1;
                 if (brightValue<=1){brightValue=1; buttonDarker = 0;}
                 showPage(); });

  server.on ("/socketSlo", [](){waitTime += 1;
                 buttonFaster=1;
                 if (waitTime>=15){waitTime=15; buttonSlower=0;} 
                 os_timer_arm (&osTimer, waitTimes[waitTime], true);
                 showPage(); });

  server.on ("/socketRes", [](){brightValue = 4;
                 waitTime = 0;
                 os_timer_arm(&osTimer, waitTimes[waitTime], true);

                 buttonSparkle  = 1;
                 buttonBrighter = 1;
                 buttonFaster   = 1;
                 buttonDarker   = 1;
                 buttonSlower   = 1; 

                 showPage (); });

  server.on ("/off", [](){
                WemosStatus = false;    
                 showPage (); });

  server.on ("/on", [](){
                WemosStatus = true;    
                 showPage (); });

  server.on ("/sparkle", []()
  {
    WemosStatus = true;
    FlashMode = 1;
    waitTime = 1;
    os_timer_arm(&osTimer, waitTimes[waitTime], true);
    stateOn = true;
    sendState();
    showPage ();
  }
  );

  server.on ("/rainbow", []()
  {
    WemosStatus = true;
    FlashMode = 2;
    waitTime = 5;
    os_timer_arm(&osTimer, waitTimes[waitTime], true);
    stateOn = true;
    sendState();
    showPage ();
  }
  );

  server.on ("/uniglow", []()
  {
    WemosStatus = true;
    FlashMode = 3;
    waitTime = 5;
    os_timer_arm(&osTimer, waitTimes[waitTime], true);
    stateOn = true;
    sendState();
    showPage ();
  }
  );

  server.begin();
  Serial.println("HTTP server started");
  if (!client.connected()) {
    reconnect();
  }
  sendState();
  }

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  Serial.println(message);

  if (!processJson(message)) {
    return;
  }

  if (stateOn) {
    // Update lights
    WemosStatus = true;    
    showPage ();
  }
  else {
    WemosStatus = false;    
    showPage ();
  }


  sendState();
}


bool processJson(char* message) {
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject(message);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return false;
  }

  if (root.containsKey("state")) {
    if (strcmp(root["state"], on_cmd) == 0) {
      stateOn = true;
    }
    else if (strcmp(root["state"], off_cmd) == 0) {
      stateOn = false;
    }
  }

  // If "flash" is included, treat RGB and brightness differently

  if ((root.containsKey("effect") && strcmp(root["effect"], "sparkle") == 0)) {
    WemosStatus = true;
    FlashMode = 1;
    waitTime = 1;
    os_timer_arm(&osTimer, waitTimes[waitTime], true);
    showPage ();
   }
  else if ((root.containsKey("effect") && strcmp(root["effect"], "rainbow") == 0)) {
    WemosStatus = true;
    FlashMode = 2;
    waitTime = 5;
    os_timer_arm(&osTimer, waitTimes[waitTime], true);
    showPage ();
   }
  else if ((root.containsKey("effect") && strcmp(root["effect"], "uniglow") == 0)) {
    WemosStatus = true;
    FlashMode = 3;
    waitTime = 5;
    os_timer_arm(&osTimer, waitTimes[waitTime], true);
    showPage ();
   }

  return true;
}

void sendState() {
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();

  root["state"] = (stateOn) ? on_cmd : off_cmd;

  switch (FlashMode)
  {
    case 1:
      //Sparkle
      root["effect"] = "sparkle";
      break;

    case 2:
      //Rainbow
      root["effect"] = "rainbow";
      break;

    case 3:
      //Uniglow
      root["effect"] = "uniglow";
      break;
  }

  char buffer[root.measureLength() + 1];
  root.printTo(buffer, sizeof(buffer));

  client.publish(light_state_topic, buffer, true);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(client_id, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(light_set_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void showPage() 
{
   webPage  = "<h2><center>Wemo Xmas Lights Controller (" + String(WemosIP[0])+"."+String(WemosIP[1])+"."+String(WemosIP[2])+"."+String(WemosIP[3]) + ")</center></h2>";
   webPage += "<p>Flash Mode: " + FlashModeText + "<br>";
   webPage += "<p>Speed: " +  String(waitTimes[waitTime]) + " ms<br>";
   webPage += "<p>Brightness: " +  String(brightValue * 6) + "%<br></p>";
   webPage += "   <a href=\"socketBri\"><button style=\"background:#7F7F7F;font-size:200%;width:33%;padding:3%;color:" + buttonColor [buttonBrighter] + "\">brighter  </button></a>";
   webPage += "   <a href=\"socketFas\"><button style=\"background:#7F7F7F;font-size:200%;width:33%;padding:3%;color:" + buttonColor [buttonFaster]   + "\">faster    </button></a>";
   webPage += "   <a href=\"socketSpa\"><button style=\"background:#7F7F7F;font-size:200%;width:33%;padding:3%;color:" + buttonColor [buttonSparkle]  + "\">sparkle   </button></a>";
   webPage += "   <a href=\"socketDar\"><button style=\"background:#7F7F7F;font-size:200%;width:33%;padding:3%;color:" + buttonColor [buttonDarker]   + "\">darker    </button></a>";
   webPage += "   <a href=\"socketSlo\"><button style=\"background:#7F7F7F;font-size:200%;width:33%;padding:3%;color:" + buttonColor [buttonSlower]   + "\">slower    </button></a>";
   webPage += "   <a href=\"socketRes\"><button style=\"background:#7F7F7F;font-size:200%;width:33%;padding:3%;color:" + buttonColor [buttonReset]    + "\">reset     </button></a></p>";

   server.send(200, "text/html", webPage);
   yield (); 
}

void loop(void)
{
  server.handleClient();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

