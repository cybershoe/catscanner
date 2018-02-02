#define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINT(x)  Serial.println(x);
#else
  #define DEBUG_PRINT(x)
#endif

#include <SoftwareSerial.h>
#include <QueueList.h>

SoftwareSerial rfid(2,4);

struct packet {
  long ts;
  byte mType;
  byte rId;
  byte sType;
  String data;  
};

unsigned long addr;
packet xbeeBuf;
byte xbeeIdx = 0;
byte xbeeLen = 0;
String rfidBuf;

QueueList <packet> sendQ;
QueueList <packet> recvQ;

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

  // Main loop
  checkXbee();
  checkRfid();
  sendMessages();
}

void checkXbee() {
  if (Serial.available()) {
    byte b = Serial.read();
    switch (xbeeIdx) {
      case 0:
        xbeeIdx = (b == 0x7A) ? (xbeeIdx + 1) : 0; // Valid magic number
        break;
      case 1:
        xbeeIdx = (b == 0x69) ? (xbeeIdx + 1) : 0;
        break;
      case 2:
        xbeeIdx = (b == 0x01) ? (xbeeIdx + 1) : 0; // Correct version
        break;
      case 3:
        if (b == ((addr >> 24) & 0xFF)) {
          xbeeIdx++;
        } else {
          xbeeIdx = 0;
        }
        break;
      case 4:
        if (b == ((addr >> 16) & 0xFF)) {
          xbeeIdx++;
        } else {
          xbeeIdx = 0;
        }
        break;
      case 5:
        if (b == ((addr >> 8) & 0xFF)) {
          xbeeIdx++;
        } else {
          xbeeIdx = 0;
        }
        break;
      case 6:
        if (b == (addr & 0xFF)) {
          xbeeIdx++;
        } else {
          xbeeIdx = 0;
        }
        break;
      case 7:
        xbeeBuf.ts += b;
        xbeeIdx++;
        break;
      case 8:
        xbeeBuf.ts = (xbeeBuf.ts << 8) + b;
        xbeeIdx++;
        break;
      case 9:
        xbeeBuf.ts = (xbeeBuf.ts << 8) + b;
        xbeeIdx++;
        break;
      case 10:
        xbeeBuf.ts = (xbeeBuf.ts << 8) + b;
        xbeeIdx++;
        break;
      case 11:
        xbeeBuf.mType = b;
        xbeeIdx++;
        break;
      case 12:
        xbeeBuf.rId = b;
        xbeeIdx++;
        break;
      case 13:
        xbeeBuf.sType = b;
        xbeeIdx++;
        break;
      case 14:
        if (b <= 64) {
          xbeeLen = b;
          xbeeBuf.data = "";
          xbeeIdx++;
        } else { // Invalid data length
          DEBUG_PRINT("Data length too long: " + String(b,HEX));
          xbeeIdx = 0;
        }
        break;
      default:
        if (b == 0x0) { // End of data
          handleXbee(xbeeBuf);
          xbeeIdx = 0;
        } else if (xbeeIdx >= (xbeeLen + 15)) { // Past end of data length
          DEBUG_PRINT("Past len");
          xbeeIdx = 0;
        } else {
          xbeeBuf.data.concat((char)b);
          xbeeIdx++;
        }
    }
  }
}

void checkRfid() {
  if (rfid.available()) {
    char b = rfid.read();
    if (b == 0xd || rfidBuf.length() == 64) {
      sendQ.push(packet{1517332839, 0x3, 0x0, 0x1, rfidBuf});
      rfidBuf = "";
    } else {
      rfidBuf.concat(b);
    }
  }
}

void handleXbee(packet &pkt) {
  DEBUG_PRINT("mType: " + String(pkt.mType, HEX) + " rId: " + String(pkt.rId, HEX) + " sType: " + String(pkt.sType, HEX) + " data: " + pkt.data);
}

void sendMessages() {  // Trasmit 
  if (!sendQ.isEmpty()) { // Only send one packet per iteration to prevent buffer blocking
    packet pkt = sendQ.pop();
    byte buf[pkt.data.length() + 16];
    buf[0] = 0x7A;  // Magic
    buf[1] = 0x69;  // number
    buf[2] = 0x1;  // Version 1
    buf[3] = (addr >> 24) & 0xFF;  // Station ID
    buf[4] = (addr >> 16) & 0xFF;
    buf[5] = (addr >> 8) & 0xFF;
    buf[6] = addr & 0xFF;
    buf[7] = (pkt.ts >> 24) & 0xFF;  // Timestamp
    buf[8] = (pkt.ts >> 16) & 0xFF;
    buf[9] = (pkt.ts >> 8) & 0xFF;
    buf[10] = pkt.ts & 0xFF;
    buf[11] = pkt.mType;  // Message type
    buf[12] = pkt.rId;  // Request ID
    buf[13] = pkt.sType;  // Message subtype
    buf[14] = (uint8_t)pkt.data.length();  // Data length
    pkt.data.getBytes(buf + 15, pkt.data.length()+1);  // Data
    
    //buf[data.length() + 1] = 0x0;
    Serial.write(buf, pkt.data.length()+16);
  }
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
  rfid.setTimeout(5000);
  String result = rfid.readStringUntil("\r");
  rfid.setTimeout(50);
  result.trim();
  DEBUG_PRINT("Result from RFID: " + result);
  return result;
}

