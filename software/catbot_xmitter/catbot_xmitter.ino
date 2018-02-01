#define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINT(x)  Serial.println(x);
#else
  #define DEBUG_PRINT(x)
#endif

#include <SoftwareSerial.h>

SoftwareSerial rfid(2,4);

unsigned long addr;
String xbeeBuf;
String rfidBuf;

void setup() {

  // If debugging is enabled, attempt to initialize the serial port.
  #ifdef DEBUG
  //Initialize serial
  Serial.begin(9600);
  delay(5000);
  DEBUG_PRINT("Serial initialized");
  #endif

  // Get XBee HW id (i.e.: low half of MAC)
  char addrs[9];
  xbeeCmd("ATSL").toCharArray(addrs, 9);
  addr = strtoul(addrs, NULL, 16);
  DEBUG_PRINT("XBee address " + String(addr, HEX));

  // Check in with controller
  Serial.println("Reader " + String(addr, HEX) + " checking in");
  DEBUG_PRINT("Sent checkin");
  
  // Init RFID reader
  rfid.begin(9600);
  while (!rfid) {}
  rfid.setTimeout(50);
  DEBUG_PRINT("RFID reader initialized");

  DEBUG_PRINT("Setup complete");
}

void loop() {
  // put your main code here, to run repeatedly:

  if (rfid.available()) {
    //Serial.println(rfid.readStringUntil("\r"));
    String data = rfid.readStringUntil("\r");
    data.trim();
    sendMessage(0x3, 0x0, 0x1, data);
  }

  if (Serial.available()) {
    DEBUG_PRINT("Xbee has bytes");
    checkXbee();
  }
  /*xbee.println("Operating frequency: " + rfidCmd("MOF"));
  delay(5000);
  */
}

void checkXbee() {
  while (Serial.available()) {
    char b = Serial.read();
    DEBUG_PRINT("Got a character: " + String(b, HEX));
    if (b == '\r') {
      DEBUG_PRINT(xbeeBuf);
      xbeeBuf = "";
    } else {
      xbeeBuf.concat(b);
    }
  }
  DEBUG_PRINT("XBee buffer: " + xbeeBuf);
}

void sendMessage(byte mType, byte rId, byte sType, String data) {
  long ts = 1517332839;
  byte buf[data.length() + 16] = "abcdefghijklmnopqrst";
  buf[0] = 0x7A;  // Magic
  buf[1] = 0x69;  // number
  buf[2] = 0x1;  // Version 1
  buf[3] = (addr >> 24) & 0xFF;  // Station ID
  buf[4] = (addr >> 16) & 0xFF;
  buf[5] = (addr >> 8) & 0xFF;
  buf[6] = addr & 0xFF;
  //addr.getBytes(buf + 3, addr.length()+1); // Station ID
  buf[7] = (ts >> 24) & 0xFF;  // Timestamp
  buf[8] = (ts >> 16) & 0xFF;
  buf[9] = (ts >> 8) & 0xFF;
  buf[10] = ts & 0xFF;
  buf[11] = mType;  // Message type
  buf[12] = rId;  // Request ID
  buf[13] = sType;  // Message subtype
  buf[14] = (uint8_t)data.length();  // Data length
  data.getBytes(buf + 15, data.length()+1);  // Data
  
  //buf[data.length() + 1] = 0x0;
  Serial.write(buf, data.length()+20);
  DEBUG_PRINT("Last byte in buf value: " + String(buf[data.length()+19], HEX));


}

String xbeeCmd(String cmd){
  // Enter command mode
  DEBUG_PRINT("Entering command mode for command: " + cmd);
  delay(2000);
  Serial.print("+++");
  Serial.readStringUntil("\r");
  Serial.println(cmd);
  String result = Serial.readStringUntil("\r");
  result.trim();
  Serial.println("ATCN");
  delay(5000);
  while (Serial.available()) {
    Serial.read();
  }
  DEBUG_PRINT("Result from XBee: " + result);
  return result;
}

String rfidCmd(String cmd) {
  rfid.println(cmd);
  DEBUG_PRINT("Sent command to RFID: " + cmd);
  String result = rfid.readStringUntil("\r");
  result.trim();
  DEBUG_PRINT("Result from RFID: " + result);
  return result;
}

