/*
  IoT-Based Home Automation via SMS (SIM800L + Arduino)
  Commands (case-insensitive):
    load1on, load1off
    load2on, load2off
    load3on, load3off
    load4on, load4off
    allon, alloff
    loadstatus
  NOTE: Relays are assumed ACTIVE-LOW (0 = ON, 1 = OFF)
*/

#include <EEPROM.h>
#include <SoftwareSerial.h>

// -------------------- GSM Serial --------------------
SoftwareSerial GSM(8, 9); // RX<-SIM800 TX on D8, TX->SIM800 RX on D9 (use level shift for SIM RX!)

// -------------------- Authorized Numbers --------------------
const String PHONE1 = "+919908233322"; // change with your number
const String PHONE2 = "+917780528459";

// -------------------- Relays --------------------
#define Relay1 2
#define Relay2 3
#define Relay3 4
#define Relay4 5

// 0 = ON, 1 = OFF (active-low relay boards)
int load1 = 1, load2 = 1, load3 = 1, load4 = 1;

// -------------------- RX buffers --------------------
String RxString = "";  // raw incoming from GSM
char   RxChar   = ' ';
int    Counter  = 0;

String GSM_Nr  = "";
String GSM_Msg = "";

// -------------------- Helpers --------------------
void relays() {
  digitalWrite(Relay1, load1);
  digitalWrite(Relay2, load2);
  digitalWrite(Relay3, load3);
  digitalWrite(Relay4, load4);
}

void eeprom_write() {
  EEPROM.write(1, load1);
  EEPROM.write(2, load2);
  EEPROM.write(3, load3);
  EEPROM.write(4, load4);
}

bool Received(const String& s) {
  return RxString.indexOf(s) >= 0;
}

void sendSMS(const String& number, const String& msg) {
  GSM.print(F("AT+CMGS=\""));
  GSM.print(number);
  GSM.println(F("\""));
  delay(500);
  GSM.println(msg);
  delay(500);
  GSM.write(byte(26)); // Ctrl+Z
  delay(5000);
}

void toLowerInPlace(String& s) {
  for (size_t i = 0; i < s.length(); i++) {
    s[i] = (char)tolower(s[i]);
  }
}

// Parse "+CMT:" delivery into GSM_Nr & GSM_Msg
void GetSMS() {
  // Number
  GSM_Nr = RxString;
  int t = GSM_Nr.indexOf('"');
  if (t >= 0) {
    GSM_Nr.remove(0, t + 1);
    t = GSM_Nr.indexOf('"');
    if (t >= 0) GSM_Nr.remove(t);
  }

  // Message (grab last line-ish)
  GSM_Msg = RxString;
  // Try to jump to content after the metadata quotes
  for (int i = 0; i < 4; i++) {
    int q = GSM_Msg.indexOf('"');
    if (q < 0) break;
    GSM_Msg.remove(0, q + 1);
  }
  // Remove leading quotes/newlines/spaces
  GSM_Msg.trim();
  // Keep only first line
  int nl = GSM_Msg.indexOf('\n');
  if (nl > 0) GSM_Msg = GSM_Msg.substring(0, nl);
  toLowerInPlace(GSM_Msg);

  Serial.print(F("Number: ")); Serial.println(GSM_Nr);
  Serial.print(F("SMS: "));    Serial.println(GSM_Msg);
}

// Robust module init loop
void initModule(const String& cmd, const char* expect, int waitMs) {
  while (true) {
    Serial.println(cmd);
    GSM.println(cmd);
    delay(100);

    unsigned long start = millis();
    String resp;
    while (millis() - start < (unsigned long)waitMs) {
      while (GSM.available()) {
        resp += (char)GSM.read();
      }
      if (resp.indexOf(expect) >= 0) {
        Serial.println(expect);
        delay(waitMs);
        return;
      }
    }
    Serial.println(F("Error"));
    delay(waitMs);
  }
}

void applyCommand(const String& cmd) {
  if (cmd == "load1on")  { load1 = 0; sendSMS(GSM_Nr, "Ok Load 1 is On"); }
  else if (cmd == "load1off") { load1 = 1; sendSMS(GSM_Nr, "Ok Load 1 is Off"); }

  else if (cmd == "load2on")  { load2 = 0; sendSMS(GSM_Nr, "Ok Load 2 is On"); }
  else if (cmd == "load2off") { load2 = 1; sendSMS(GSM_Nr, "Ok Load 2 is Off"); }

  else if (cmd == "load3on")  { load3 = 0; sendSMS(GSM_Nr, "Ok Load 3 is On"); }
  else if (cmd == "load3off") { load3 = 1; sendSMS(GSM_Nr, "Ok Load 3 is Off"); }

  else if (cmd == "load4on")  { load4 = 0; sendSMS(GSM_Nr, "Ok Load 4 is On"); }
  else if (cmd == "load4off") { load4 = 1; sendSMS(GSM_Nr, "Ok Load 4 is Off"); }

  else if (cmd == "allon")  { load1 = load2 = load3 = load4 = 0; sendSMS(GSM_Nr, "Ok All Load is On"); }
  else if (cmd == "alloff") { load1 = load2 = load3 = load4 = 1; sendSMS(GSM_Nr, "Ok All Load is Off"); }

  else if (cmd == "loadstatus") {
    String s;
    s  = (load1 == 0 ? "Load1 On\r\n" : "Load1 Off\r\n");
    s += (load2 == 0 ? "Load2 On\r\n" : "Load2 Off\r\n");
    s += (load3 == 0 ? "Load3 On\r\n" : "Load3 Off\r\n");
    s += (load4 == 0 ? "Load4 On"     : "Load4 Off");
    sendSMS(GSM_Nr, s);
  }
}

void setup() {
  pinMode(Relay1, OUTPUT); digitalWrite(Relay1, 1);
  pinMode(Relay2, OUTPUT); digitalWrite(Relay2, 1);
  pinMode(Relay3, OUTPUT); digitalWrite(Relay3, 1);
  pinMode(Relay4, OUTPUT); digitalWrite(Relay4, 1);

  Serial.begin(9600);
  GSM.begin(9600);

  Serial.println(F("Initializing GSM..."));
  initModule(F("AT"), "OK", 1000);
  initModule(F("AT+CPIN?"), "READY", 1000);       // SIM present
  initModule(F("AT+CMGF=1"), "OK", 1000);         // SMS text mode
  initModule(F("AT+CNMI=2,2,0,0,0"), "OK", 1000); // push incoming SMS to serial
  Serial.println(F("Initialized Successfully"));

  // Restore last states (default to OFF if invalid)
  load1 = EEPROM.read(1); if (load1 != 0 && load1 != 1) load1 = 1;
  load2 = EEPROM.read(2); if (load2 != 0 && load2 != 1) load2 = 1;
  load3 = EEPROM.read(3); if (load3 != 0 && load3 != 1) load3 = 1;
  load4 = EEPROM.read(4); if (load4 != 0 && load4 != 1) load4 = 1;

  relays();
  delay(100);
}

void loop() {
  // Read any GSM data
  RxString = "";
  Counter = 0;
  while (GSM.available()) {
    delay(1);
    RxChar = (char)GSM.read();
    if (Counter < 240) { // prevent runaway strings
      RxString.concat(RxChar);
      Counter++;
    }
  }

  // If a new SMS was pushed, parse it
  if (Received(F("CMT:"))) {
    GetSMS();

    // Check authorization
    if (GSM_Nr == PHONE1 || GSM_Nr == PHONE2) {
      applyCommand(GSM_Msg);
      eeprom_write();
      relays();
    } else if (GSM_Nr.length() > 0) {
      // Optional: notify unauthorized sender
      sendSMS(GSM_Nr, "Unauthorized number.");
    }

    GSM_Nr = "";
    GSM_Msg = "";
  }
}
