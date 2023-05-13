#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <SPI.h>
#include <SD.h>
#include <EthernetServer.h>
#include <Ethernet.h>
#include <EthernetClient.h>

// --- CONFIG NETWORK ---

byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x50, 0x8E };
byte ip[] = { 192, 168, 1, 40 };

const char* SSID_WL = "meishi";
const char* PASS_WL = "s1250039";

// --- CONFIG SD ---

const int chipSelect = 4;

WebServer server(80);
const char* serverName = "meishi";
HTTPClient client;

void get();
void post();
void beginNetwk();
void beginServ();

String r0 = "0";
String g0 = "0";
String b0 = "0";
String r1 = "0";
String g1 = "0";
String b1 = "0";

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  Serial.print("");
  Serial.print("[BOOT] INIT SD");
  beginNetwk();
  beginServ();
  server.on("/", HTTP_GET, get);
  server.on("/", HTTP_POST, post);
  server.begin();
}

void loop() {

}

void beginNetwk() {
  Serial.println("[BOOT] BEGIN ETH");
  Ethernet.begin(mac, ip);
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("[ERRR] ETH SHIELD NOT FOUND -- CHECK HARDWARE AND RESTART");
    while (true) {
      delay(1);  // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("[WARN] ETH CABLE NOT CONN");
  }
  Serial.print("[INFO] IP ADDR: ");
  Serial.println(Ethernet.localIP());
}

void beginServ() {
  Serial.println("[BOOT] BEGIN SERVER");
  delay(10);  //内部レジスタ待ち

  if (!MDNS.begin(serverName)) {
    //Serial.println("mDNS Failed");
    while (1)
      ;
  }
  MDNS.addService("http", "tcp", 80);  //Webサーバーを開始
  Serial.print("[SERV] MY NAME IS ");
  Serial.println(serverName);
}

void get() {
  String HTML = "<!DOCTYPE html> \n<html lang=\"ja\">";
  HTML += "<HTML><HEAD><meta charset ='UTF-8'><TITLE>名刺</TITLE><style>*{text-align:center;font-size:30pt;}</style></HEAD>";
  HTML += "<BODY><p><B>colour</B></p>";
  HTML += "<p><form method=\"POST\" target=_self>";
  HTML += "R1<br><input type=\"range\" name=\"r0\" value=\"";
  HTML += r0;
  HTML += "\" min=\"0\" max=\"255\"/><br><br>";
  HTML += "G1<br><input type=\"range\" name=\"g0\" value=\"";
  HTML += g0;
  HTML += "\" min=\"0\" max=\"255\"/><br><br>";
  HTML += "B1<br><input type=\"range\" name=\"b0\" value=\"";
  HTML += b0;
  HTML += "\" min=\"0\" max=\"255\"/><br><br>";
  HTML += "R2<br><input type=\"range\" name=\"r1\" value=\"";
  HTML += r1;
  HTML += "\" min=\"0\" max=\"255\"/><br><br>";
  HTML += "G2<br><input type=\"range\" name=\"g1\" value=\"";
  HTML += g1;
  HTML += "\" min=\"0\" max=\"255\"/><br><br>";
  HTML += "B2<br><input type=\"range\" name=\"b1\" value=\"";
  HTML += b1;
  HTML += "\" min=\"0\" max=\"255\"/><br><br>";
  HTML += "<input type=\"submit\" value=\"set\"></p></form>";
  HTML += "</BODY></HTML>";
  server.send(200, "text/html", HTML);
}

void post() {
  r0 = server.arg("r0");
  g0 = server.arg("g0");
  b0 = server.arg("b0");
  r1 = server.arg("r1");
  g1 = server.arg("g1");
  b1 = server.arg("b1");
  light();
  //Serial.println("[DATA] R" + r0 + " G" + g0 + " B" + b0);
  get();
}
