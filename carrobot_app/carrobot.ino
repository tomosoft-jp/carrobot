//#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>

#define USE_SERIAL Serial

IPAddress ip(192, 168, 10, xx); //ip
IPAddress gateway(192, 168, 10, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress DNS(192, 168, 10, 1);

ESP8266WiFiMulti WiFiMulti;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

unsigned long last_1sec = 0;
int socketid;

const char *ssid = "xxxxx";
const char *pass = "xxxxx";

////////////////////////
#define LED_RED     0
#define LED_GREEN   16

#define AIN1 13  // Right Motor Blue
#define AIN2 15  // Right Motor Yellow
#define PWMA 2
#define BIN1 14  // Left  Motor Yellow
#define BIN2 5   // Left  Motor Blue
#define PWMB 4
#define STBY 12

// PWN
#define PWN_FREQ 1000 // PWM frequency: 1000Hz(1kHz)
#define PWN_RANGE 100 // PWN range: 100

int helthcnt = 0;


///////////////////
void carsetting(uint8_t * payload) {

  if (payload[0] == '#') {
    // we get move data
    // decode move data
    char dst1[10];
    strncpy(dst1, (const char *)&payload[1], 6);
    dst1[6] = '\x0';

    uint32_t mov = (uint32_t) strtol((const char *) dst1, NULL, 16);
    USE_SERIAL.printf("payload[1]: %s dst1: %s move:%x \n", &payload[1], dst1, mov);
    USE_SERIAL.printf("AIN1: %d AIN2: %d PWMA: %x  \n",
                      ((mov >> 16) & 0xFF),  ((mov >> 8) & 0xFF), ((mov >> 0) & 0xFF));

    digitalWrite(AIN1, ((mov >> 16) & 0xFF));
    digitalWrite(AIN2, ((mov >> 8) & 0xFF));
    analogWrite(PWMA, ((mov >> 0) & 0xFF));

    char dst2[10];
    strncpy(dst2, (const char *)&payload[7], 6);
    dst2[6] = '\x0';

    mov = (uint32_t) strtol((const char *) dst2, NULL, 16);
    USE_SERIAL.printf("payload[1]: %s dst2: %s move:%x \n", &payload[1], dst2, mov);
    USE_SERIAL.printf("BIN1: %d BIN2: %d PWMB: %x \n",
                      ((mov >> 16) & 0xFF),  ((mov >> 8) & 0xFF), ((mov >> 0) & 0xFF));
    
    digitalWrite(BIN1, ((mov >> 16) & 0xFF));
    digitalWrite(BIN2, ((mov >> 8) & 0xFF));
    analogWrite(PWMB, ((mov >> 0) & 0xFF));
    digitalWrite(STBY, LOW);
  }
  else if (payload[0] == '%') {
    // we get helth check req
    helthcnt = 5;
  }
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      USE_SERIAL.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        // send message to client
        webSocket.sendTXT(num, "Connected Car Robot");
        socketid = num;
      }
      break;
    case WStype_TEXT:
      USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);
      carsetting(payload);
      break;
  }
}

void setup() {
  //USE_SERIAL.begin(921600);
  USE_SERIAL.begin(115200);

  //USE_SERIAL.setDebugOutput(true);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] BOOT01 WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  digitalWrite(LED_RED, 1);
  digitalWrite(LED_GREEN, 0);

  WiFi.config(ip, gateway, subnet, DNS); //static_ip
  delay(100);

  WiFiMulti.addAP( ssid, pass);

  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  USE_SERIAL.printf("Module IP: ");
  //USE_SERIAL.printf(WiFi.localIP()[0]);
  ip = WiFi.localIP();
  USE_SERIAL.println(ip);

  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  if (MDNS.begin("esp8266")) {
    USE_SERIAL.println("MDNS responder started");
  }

  // handle index
  server.on("/", []() {
    // send index.html
    //server.send(200, "text/html", "<html><head><script>var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);connection.onopen = function () {  connection.send('Connect ' + new Date()); }; connection.onerror = function (error) {    console.log('WebSocket Error ', error);};connection.onmessage = function (e) {  console.log('Server: ', e.data);};function sendRGB() {  var r = parseInt(document.getElementById('r').value).toString(16);  var g = parseInt(document.getElementById('g').value).toString(16);  var b = parseInt(document.getElementById('b').value).toString(16);  if(r.length < 2) { r = '0' + r; }   if(g.length < 2) { g = '0' + g; }   if(b.length < 2) { b = '0' + b; }   var rgb = '#'+r+g+b;    console.log('RGB: ' + rgb); connection.send(rgb); }</script></head><body>LED Control:<br/><br/>R: <input id=\"r\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" oninput=\"sendRGB();\" /><br/>G: <input id=\"g\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" oninput=\"sendRGB();\" /><br/>B: <input id=\"b\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" oninput=\"sendRGB();\" /><br/></body></html>");
  });

  server.begin();

  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);

  digitalWrite(LED_RED, 0);
  digitalWrite(LED_GREEN, 1);

  // Initialize GPIO mode
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);

  // Initialize PWN
  analogWriteFreq(PWN_FREQ);
  analogWriteRange(PWN_RANGE);

  digitalWrite(STBY, HIGH);
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  analogWrite(PWMA, 0);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMB, 0);

}

void loop() {
  unsigned long t = millis();
  webSocket.loop();
  server.handleClient();

  if ((t - last_1sec) > 1 * 1000) {
    if (helthcnt != 0) {
      digitalWrite(LED_GREEN,  helthcnt % 2);
      helthcnt--;
      webSocket.sendTXT(socketid, "ping Car Robot");
      last_1sec = millis();
    }
  }
}
