#include <SPI.h>
#include <SD.h>
#include <EthernetServer.h>
#include <Ethernet.h>
#include <EthernetClient.h>

// --- CONFIG NETWORK ---

byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x50, 0x8E };
IPAddress ip(192, 168, 1, 40);
IPAddress dns(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
EthernetServer server(80);

// --- CONFIG SD ---

const int chipSelect = 4;
File myFile;
char fileName[13];

// --- FUNCTION ---

void beginNetwk();
void beginSD();
void badReq(EthernetClient);
void notFound(EthernetClient);
bool readHead(EthernetClient);

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("");
  Serial.println("[BOOT] WELCOME");
  beginSD();
  beginNetwk();
  server.begin();
}

void loop() {
  bool flag = false;
  EthernetClient client = server.available();
  if (client) {
    flag = false;
    Serial.println("new client");
    // an HTTP request ends with a blank line
    while (client.connected()) {
      if (client.available()) {
        // if (client.read() != 'G' || client.read() != 'E' || client.read() != 'T' || client.read() != ' ') {
        //   client.println("HTTP/1.1 400 BAD REQUEST");
        //   client.stop();
        // }
        if (flag == false) {
          flag = readHead(client);
        } else {
          // if you've gotten to the end of the line (received a newline
          // character) and the line is blank, the HTTP request has ended,
          // so you can send a reply
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}


void beginNetwk() {
  Serial.println("[BOOT] BEGIN ETH");
  Ethernet.begin(mac, ip, dns, gateway, subnet);
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("[ERRR] ETH SHIELD CONN ERR -- CHECK HARDWARE AND RESTART");
    while (true) {
      delay(1);  // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("[WARN] ETH CABLE NOT CONN");
  }
  Serial.print("[INFO] IP ADDR: ");
  Serial.println(Ethernet.localIP());
  byte macBuffer[6];               // create a buffer to hold the MAC address
  Ethernet.MACAddress(macBuffer);  // fill the buffer
  Serial.print("[INFO] MAC ADDR: ");
  for (byte octet = 0; octet < 6; octet++) {
    if (macBuffer[octet] < 0x10) {
      Serial.print('0');
    }
    Serial.print(macBuffer[octet], HEX);
    if (octet < 5) {
      Serial.print(':');
    }
  }
  Serial.println("");
  Serial.println("[BOOT] ETH UP");
}

void beginSD() {
  Serial.println("[BOOT] SD INIT");
  if (!SD.begin(4)) {
    Serial.println("[ERRR] SD INIT ERR -- CHECK HARDWARE AND RESTART");
    while (1)
      ;
  }
  Serial.println("[BOOT] SD UP");
}

void notFound(EthernetClient client) {
  client.println("HTTP/1.1 404 NOT_FOUND");
  client.println("Connection: close");
  client.stop();
  Serial.println("[INFO] HTTP 404");
}

void badReq(EthernetClient client) {
  client.println("HTTP/1.1 400 BAD_REQUEST");
  client.println("Connection: close");
  client.stop();
  Serial.println("[INFO] HTTP 400");
}

bool readHead(EthernetClient client) {
  bool isCurrentLineBlank = true;
  char c;
  int cur;
  c = client.read();
  Serial.write(c);
  while (c != ' ') {
    c = client.read();
    Serial.write(c);
  }
  while (c != ' ') {
    c = client.read();
    Serial.write(c);
  }
  c = client.read();
  Serial.write(c);
  if (c != '/') {
    badReq(client);
    return false;
  }
  for (cur = 0; cur < 12; cur++) {
    fileName[cur] = client.read();
    if (fileName[cur] == ' ') {
      fileName[cur] == '\0';
      break;
    }
  }
  if(fileName[0] == '\0'){
    fileName[0] = "index.html";
  }
  while (c != ' ') {
    c = client.read();
    Serial.write(c);
  }
  Serial.print("\n[INFO] FILE ");
  Serial.print(fileName);
  Serial.println(" REQD");
  myFile = SD.open(fileName);
  if (!myFile) {
    notFound(client);
    Serial.println("client disconnected");
    return false;
  }
  while (1) {
    c = client.read();
    Serial.write(c);
    if (c == '\n' && isCurrentLineBlank == true) {
      // send a standard HTTP response header
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("Connection: close");  // the connection will be closed after completion of the response
      client.println();
      client.println("<!DOCTYPE HTML>");
      client.println("<html>");
      // output the value of each analog input pin
      for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
        int sensorReading = analogRead(analogChannel);
        client.print("analog input ");
        client.print(analogChannel);
        client.print(" is ");
        client.print(sensorReading);
        client.println("<br />");
      }
      client.println("</html>");
      break;
    }
    if (c == '\n') {
      // you're starting a new line
      isCurrentLineBlank = true;
    } else if (c != '\r') {
      // you've gotten a character on the current line
      isCurrentLineBlank = false;
    }
  }
  return true;
}