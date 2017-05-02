#include <Time.h>
#include <TimeLib.h>


//Ethernet Library tutorial credit to
//http://bildr.org/2011/06/arduino-ethernet-pin-control/
#include <Ethernet.h>
#include <SPI.h>
#include <EthernetUdp.h>
#include <EEPROM.h>

boolean reading = false;
byte ip[] = {10, 3, 108, 250};
byte gateway[] = {10, 3, 108, 1};
byte subnet[] = {255, 255, 252, 0};

byte timeServer[] = {129,6,15,30}; // time.nist.gov NTP server

boolean currentState = false;

//Placeholder MAC, can be replaced with value printed on ethernet shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetServer server = EthernetServer(80); //80 = http communication

//void checkForClient();
int getNtpTime();
void sendNTPpacket(char* address);

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

int getNtpTime() {
  Serial.println("CALLED NTP METHOD");
  Udp.begin(8888);
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(2000);
  if (Udp.parsePacket()) {
    Serial.println("Parsing UDP");
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    // the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, extract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    const unsigned long seventyYears = 2208988800UL;
    unsigned long epoch = secsSince1900 - seventyYears;
    Serial.println(epoch);
    return epoch;
  }
  Serial.println("Done with NTP");
  Udp.stop();
}

// send an NTP request to the time server at the given address
void sendNTPpacket(byte* address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // send a packet requesting a timestamp:
  Serial.println("Sending UDP packet");
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
  Serial.println("Sent ntp packet");
}

void checkForClient() {
  EthernetClient client = server.available();

  if (client) {

    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    boolean sentHeader = false;
    String lastCommand = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        if (reading && c == ' ') {
          reading = false;
          executeCommand(lastCommand);
        }
        if (c == '?') reading = true; //found the ?, begin reading the info

        if (reading) {
          lastCommand += c;

        }

        if (c == '\n' && currentLineIsBlank) {
          client.println(F("HTTP/1.1 200 OK"));
          client.println(F("Content-Type: text/html"));
          client.println(F("Connection: close"));
          client.println();
          // send web page
          client.println(F("<!DOCTYPE html>"));
          client.println(F("<html>"));
          client.println(F("<head>"));
          client.println(F("<title>Arduino Web Page</title>"));
          client.println(F("</head>"));
          client.println(F("<body>"));
          if (currentState) {
            client.println(F("<p>"));
            client.println(F("<h1>The Heat Rises...</h1>"));
            client.println(F("<img src=\"https://images-na.ssl-images-amazon.com/images/G/01/aplusautomation/vendorimages/4e14cd59-bc1c-4f0b-8ad4-26cf216c2d5d.jpg._CB290777952_.jpg\" alt = \"image not found\" style=\"width:450px;height:450px;\"><br />"));
            client.println(F("<input type=\"button\" style = \"postion: absolute; top: 450; left: (450-75)/2; height: 20px; width: 50px; background-color:red; cursor:pointer\" value=\"On\" onclick=\"window.location.href='?o'\"/>"));
            client.println(F("<input type=\"button\" style = \"postion: absolute; top: 450; left: (450-75)/2; height: 20px; width: 50px; background-color:white; cursor:pointer\" value=\"Off\" onclick=\"window.location.href='?f'\"/>"));
          }
          else {
            client.println(F("</p>"));
            client.println(F("<h1>It's Tea Time</h1>"));
            client.println(F("<img src=\"https://images-na.ssl-images-amazon.com/images/G/01/aplusautomation/vendorimages/4e14cd59-bc1c-4f0b-8ad4-26cf216c2d5d.jpg._CB290777952_.jpg\" alt = \"image not found\" style=\"width:450px;height:450px;\"><br />"));
            client.println(F("<input type=\"button\" style = \"postion: absolute; top: 450; left: (450-75)/2; height: 20px; width: 50px; background-color:white; cursor:pointer\" value=\"On\" onclick=\"window.location.href='?o'\"/>"));
            client.println(F("<input type=\"button\" style = \"postion: absolute; top: 450; left: (450-75)/2; height: 20px; width: 50px; background-color:red; cursor:pointer\" value=\"Off\" onclick=\"window.location.href='?f'\"/>"));
          }
          client.println(F("<script language=\"JavaScript\">function showInput() {return document.getElementById(\"user_input\").value;}</script>"));
          client.println(F("<input type=\"button\" style = \"height: 20px; width: 50px; background-color:white; cursor:pointer\" value=\"Toggle\" onclick=\"window.location.href='http://192.168.0.2/?t'\"/>"));
          client.println(F("<form>Select a time:<input type=\"time\" name=\"CLOCK=\" id = \"user_input\"><input type=\"submit\" value = \"Submit\" onclick =\"window.location.href='http://192.168.0.2/?CLOCK'+showInput();\"><br/></form>"));
          //"<form>Select a time:<input type="time" name="usr_time"></form>"
          client.println(F("</body>"));
          client.println(F("</html>"));
          break;  //If 2 /ns are recieved, end the reading phase
        }

        if (c == '\n') {
          currentLineIsBlank = true;
        }

      }
    }
    delay(1); // give the web browser time to receive the data
    client.stop(); // close the connection:

  }

}

void makeOn() {
  if (!currentState) toggleRelay();
}
void makeOff() {
  if (currentState) toggleRelay();
}

void toggleRelay() {   //Toggle relay on/off
  digitalWrite(7, currentState ? LOW : HIGH);
  currentState = !currentState;
}

void setTimer(String timer) { //Timer stores value as ten min increment 0 = 12:00AM 143 = 11:50PM
  if (timer.length() == 5) {
    int hrs = timer.substring(0, 2).toInt();
    int mins = timer.substring(3, 5).toInt();
    EEPROM.update(0, hrs);
    EEPROM.update(1, mins);
  }
}
long getTime() { // returns time as millis elapsed since midnight
  long val = EEPROM.read(0) * 10 + EEPROM.read(1);
  return val != ((long)255) * 255 ? val : -1;
}

void executeCommand(String command) {
  if (command.length() >= 7 && command.substring(1, 6) == "CLOCK") {
    String timerValue = command.substring(6, command.length());
    while (timerValue.indexOf('%') >= 0) {
      timerValue = timerValue.substring(0, timerValue.indexOf('%')) + timerValue.substring(timerValue.indexOf('%') + 3);
    }
    timerValue = timerValue.substring(1);
    timerValue = timerValue.substring(0, timerValue.length() - 2) + ":" + timerValue.substring(timerValue.length() - 2);
    if (timerValue.length() == 4)
      timerValue = "0" + timerValue;
    setTimer(timerValue);
  }
  else {
    for (int i = 0; i < command.length(); i++) {
      char c = command.charAt(i);
      switch (c) {
        case 'o':
          makeOn();
          break;
        case 'f':
          makeOff();
          break;
        case 't':
          toggleRelay();
          break;
      }
    }
  }
}

void setup() {

  pinMode(7, OUTPUT);  //Set the relay control pin to output mode
  Serial.begin(9600);
  Ethernet.begin(mac);
  Serial.println(Ethernet.localIP());
  delay(2000);
  setTime(getNtpTime());
  delay(2000);

  pinMode(LED_BUILTIN, OUTPUT);
  server.begin();
}

void loop() {

  // listen for incoming clients, and process request.
  checkForClient();

  if (hour() * 60 + minute() == getTime()) { //If the time is equal
    makeOn();
    delay(1000 * 60);
  }
}

