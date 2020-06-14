/*
   Julian Krieger, KriegersBlog
   14.06.2020, v1.0
   A webserver that holds a site to control the connected RGB-strip
   My configuration: 60LEDs/m * 5m = 300LEDs | NodeMCU ESP8266 v1.0 (D2 -> GPIO4)
*/

/*************************************** Libraries ***************************************/
//Webserver Librarys
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

//RGB-Strip library
#include <WS2812FX.h>

//RGB-Strip Definitions
#ifdef __AVR__
#endif
#define LED_PIN    4
#define LED_COUNT 300

/*************************************** Objects ***************************************/
//Webserver Objects
ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server(80);

//RGB-Strip Object
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

//Handler Prototypes
void handleRoot();
void handleNotFound();
void handleSettings();
void handleAnimations();

//HTML Code
char htmlCode[] = R"(
<html>
  <head>
    <meta charset="utf-8"/>
    <title>RGB - Steuerung</title>
  </head>
  
  <body>
    <center><font face="Trebuchet MS">
    <h1>Modi</h1>
    <h3>Animationen</h3>
      <form action="/Animations" method="POST">
        <button style="width: 200px; height: 50px;" type="submit" name="effect" value="12">Regenbogen</button>
        <button style="width: 200px; height: 50px;" type="submit" name="effect" value="11">Farbwelle</button>
        <button style="width: 200px; height: 50px;" type="submit" name="effect" value="3">ColorWipe</button>
        <br><br>
        <button style="width: 200px; height: 50px;" type="submit" name="effect" value="13">Scan (einfarbig)</button>
        <button style="width: 200px; height: 50px;" type="submit" name="effect" value="14">Doppelscan (einfarbig)</button>
        <button style="width: 200px; height: 50px;" type="submit" name="effect" value="43">MultiScan (einfarbig)</button>
        <br><br>
        <button style="width: 200px; height: 50px;" type="submit" name="effect" value="15">Atmen (einfarbig)</button>
        <button style="width: 200px; height: 50px;" type="submit" name="effect" value="18">Atmen verteilt (einfarbig)</button>
        <button style="width: 200px; height: 50px;" type="submit" name="effect" value="46">Feuerwerk</button>
        
        <h1>Statisch</h1>
        
        <button style="width: 300px; height: 50px;" type="submit" name="setting" value="static_colorwipe">ColorWipe</button>
        <button style="width: 300px; height: 50px;" type="submit" name="setting" value="0">Einfarbig</button>
      </form>
      
      <h1>Settings</h1>
      
      <form action="/Settings" method="POST">
        <button style="width: 300px; height: 50px;" type="submit" name="setting" value="1">STARTEN</button>
        <button style="width: 300px; height: 50px;" type="submit" name="setting" value="0">STOPPEN</button>
        <br><br>
        <input style="width: 300px; height: 50px; font-size: 40px;" type="number" id="speed" name="speed" placeholder="10-65000">
        <button style="width: 300px; height: 50px;" type="submit" name="setting" value="10">Geschwindigkeit einstellen</button>
        <br><br>
        <input style="width: 300px; height: 50px; font-size: 40px;" type="number" id="brightness" name="brightness" placeholder="0-255">
        <button style="width: 300px; height: 50px;" type="submit" name="setting" value="20">Helligkeit einstellen</button>
        <br><br>
        <button style="width: 50px; height: 50px; background-color: red;" type="submit" name="setting" value="101"></button>
        <button style="width: 50px; height: 50px; background-color: green;" type="submit" name="setting" value="102"></button>
        <button style="width: 50px; height: 50px; background-color: blue;" type="submit" name="setting" value="103"></button>
        <button style="width: 50px; height: 50px; background-color: black;" type="submit" name="setting" value="1337"></button>
      </form>
    </center> 
  </body>
</html>
  )";
    
/*************************************** Standard Methods ***************************************/
void setup(void) {

  //WiFi setup
  wifiMulti.addAP("SSID", "PW");

  //HTTP server setup
  server.on("/", HTTP_GET, handleRoot);
  server.on("/Animations", HTTP_POST, handleAnimations);
  server.on("/Settings", HTTP_POST, handleSettings);
  server.onNotFound(handleNotFound);
  server.begin();

  //RGB-Strip setup
  ws2812fx.init();
  ws2812fx.setBrightness(255);
  ws2812fx.start();
  ws2812fx.setColor(BLUE);

  //MDNS Responder setup
  boolean mdns_online = MDNS.begin("rgb");

  //Serial Output
  Serial.begin(115200);
  delay(500);
  
  Serial.print("\n \n \n");

  //Server-Status
  Serial.println("SERVER STATUS");
  Serial.println("HTTP server started");
  if (mdns_online) {
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  //WiFi-Status
  Serial.println("\nWIFI STATUS");
  Serial.println("Connected to '" + WiFi.SSID() + "'");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("mDNS Name: 'rgb'");

  //RGB-Settings
  Serial.println("\nRGB STATUS");
  Serial.print("Standard-Brightness: ");
  Serial.println(ws2812fx.getBrightness());
  Serial.print("Standard-Speed: ");
  Serial.println(ws2812fx.getSpeed());
  Serial.print("Standard-Color: ");
  Serial.println(ws2812fx.getColor());
  
  Serial.println("\n\n");
}

void loop(void) {
  ws2812fx.service();
  server.handleClient();
  MDNS.update();
}



/*************************************** HTTP-Handles ***************************************/

//200 (Main-Page)
void handleRoot() {
  server.send(200, "text/html", htmlCode);
  Serial.println("\n200: Main-Page loaded \n");
}

//303 (After Input/Interaction)
void handleInput(String input) {
  server.sendHeader("Location", "/");
  server.send(303);
  Serial.println("303: Input handled (" + input +")");
}

//404 (Page not found)
void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
  Serial.println("404: Page not found");
}

/*************************************** Input-Handles ***************************************/

void handleAnimations() {
  handleInput("Animation");
  ws2812fx.setMode(getFromHtml("effect"));
  Serial.print(getFromHtml("effect"));
  Serial.print(": Mode set to '");
  Serial.print(ws2812fx.getModeName(getFromHtml("effect")));
  Serial.println("'");
}

void handleSettings() {
  handleInput("Setting");

  switch (getFromHtml("setting")) {
    case 0:
      ws2812fx.stop();
      Serial.println("RGB0: RGB-Strip stopped");
      break;
    case 1:
      ws2812fx.start();
      Serial.println("RGB1: RGB-Strip started");
      break;
    case 10:
      ws2812fx.setSpeed(getFromHtml("speed"));
      Serial.print("RGB10: Speed set to '");
      Serial.print(getFromHtml("speed"));
      Serial.println("'");
      break;
    case 20:
      ws2812fx.setBrightness(getFromHtml("brightness"));
      Serial.print("RGB20: Brightness set to '");
      Serial.print(getFromHtml("brightness"));
      Serial.println("'");
      break;
    case 101:
      ws2812fx.setColor(RED);
      Serial.println("RGB101: Color set to 'RED'");
      break;
    case 102:
      ws2812fx.setColor(GREEN);
      Serial.println("RGB102: Color set to 'GREEN'");
      break;
    case 103:
      ws2812fx.setColor(BLUE);
      Serial.println("RGB103: Color set to 'BLUE'");
      break;
    case 1337:
      printInformation();
      break;
  }
}

void printInformation(){
  Serial.println("\n---------------");
  Serial.print("Speed: ");
  Serial.println(ws2812fx.getSpeed());
  Serial.print("Brightness: ");
  Serial.println(ws2812fx.getBrightness());
  Serial.print("Mode: ");
  Serial.println(ws2812fx.getModeName(getFromHtml("effect")));
  Serial.println("---------------\n");
}

int getFromHtml(String html_name) {
  return server.arg(html_name).toInt();
}
