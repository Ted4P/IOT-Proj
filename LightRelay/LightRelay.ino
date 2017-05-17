#include <Time.h>
#include <TimeLib.h>
#include <Ethernet.h>
#include <SPI.h>
#include <EthernetUdp.h>
#include <EEPROM.h>

boolean reading = false;

byte timeServer[] = {129, 6, 15, 30}; // time.nist.gov NTP server

boolean currentState;

//MAC hardware address for Ethernet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetServer server = NULL;

void sendNTPpacket(char* address);  //C++ Method headers

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets for UDP communication

// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

time_t getNtpTime() { //Return the NTP time
  Serial.println("CALLED NTP METHOD");
  Udp.begin(8888);
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(2000);
  if (Udp.parsePacket()) {
    Serial.println("Parsing UDP");
    //Received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read packet into the buffer
    // timestamp starts at byte 40 of the received packet and is four bytes (or two words) long

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    time_t secsSince1900 = highWord << 16 | lowWord;  //bitshift by 16 then OR the low words, combining into 32 bit datatype
    time_t seventyYears = 2208988800UL;
    time_t epoch = secsSince1900 - seventyYears;
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
  Udp.write(packetBuffer, NTP_PACKET_SIZE); //Write out data to buffer, will be analyzed in parent method
  Udp.endPacket();
  Serial.println("Sent ntp packet");
}

void checkForClient() {
  EthernetClient client = server.available();

  if (client) { //If a user has connected to the server

    //http request ends with a blank line
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
        //Sends client the webpage HTML5 code to display web app
        if (c == '\n' && currentLineIsBlank) {
          client.println(F("HTTP/1.1 200 OK")); //Declares content type and other setup materials
          client.println(F("Content-Type: text/html"));
          client.println(F("Connection: close"));
          client.println();
          client.println(F("<!DOCTYPE html>")); //Declares document type to be HTML5
          client.println(F("<html>"));
          client.println(F("<head>"));
          client.println(F("<title>Arduino Web Page</title>"));//Website title
          client.println(F("</head>"));
          client.println(F("<body>"));
          if (currentState) {//If the kettle is currently set to on...
            client.println(F("<p>")); //Add a header, images, and some buttons with off set to red to show selection
            client.println(F("<h1>The Heat Rises...</h1>"));
            client.println(F("<img src=\"https://images-na.ssl-images-amazon.com/images/G/01/aplusautomation/vendorimages/4e14cd59-bc1c-4f0b-8ad4-26cf216c2d5d.jpg._CB290777952_.jpg\" alt = \"image not found\" style=\"width:450px;height:450px;\"><br />"));
            client.println(F("<input type=\"button\" style = \"postion: absolute; top: 450; left: (450-75)/2; height: 20px; width: 50px; background-color:red; cursor:pointer\" value=\"On\" onclick=\"window.location.href='?o'\"/>"));//Send a ping to IP with command 'o' attached at the end
            client.println(F("<input type=\"button\" style = \"postion: absolute; top: 450; left: (450-75)/2; height: 20px; width: 50px; background-color:white; cursor:pointer\" value=\"Off\" onclick=\"window.location.href='?f'\"/>"));//Send a ping to IP with command 'f' attached at the end
          }
          else {//If the kettle is currently set to off...
            client.println(F("</p>")); //Add a different header, then images and buttons with off set to red to show selection
            client.println(F("<h1>It's Tea Time</h1>"));
            client.println(F("<img src=\"https://images-na.ssl-images-amazon.com/images/G/01/aplusautomation/vendorimages/4e14cd59-bc1c-4f0b-8ad4-26cf216c2d5d.jpg._CB290777952_.jpg\" alt = \"image not found\" style=\"width:450px;height:450px;\"><br />"));
            client.println(F("<input type=\"button\" style = \"postion: absolute; top: 450; left: (450-75)/2; height: 20px; width: 50px; background-color:white; cursor:pointer\" value=\"On\" onclick=\"window.location.href='?o'\"/>"));
            client.println(F("<input type=\"button\" style = \"postion: absolute; top: 450; left: (450-75)/2; height: 20px; width: 50px; background-color:red; cursor:pointer\" value=\"Off\" onclick=\"window.location.href='?f'\"/>"));
          }
          //Build current time, current scheduled time, and time input box and write to webpage
          client.println(F("<script language=\"JavaScript\">function showInput() {return document.getElementById(\"user_input\").value;}</script>"));
          client.println(F("<input type=\"button\" style = \"height: 20px; width: 50px; background-color:white; cursor:pointer\" value=\"Toggle\" onclick=\"window.location.href='/?t'\"/>"));
          client.println(F("<form>Enter a time (military format):<input type=\"time\" name=\"CLOCK=\" id = \"user_input\"><input type=\"submit\" value = \"Submit\" onclick =\"window.location.href='/?CLOCK'+showInput();\"><br/></form>"));
          char getTimeParagraph[50];
          char hourString[10];
          sprintf(hourString,"%d",((hour()-5)%12)+1);
          char minString[10];
          sprintf(minString,"%d",minute());
          boolean timeIsAM = (hour()-4)<12;
          char setHourString[10];
          sprintf(setHourString,"%d",((EEPROM.read(0)-5)%12)+1);    //Adjust for EST vs GMT
          char setMinString[10];
          sprintf(setMinString,"%d",EEPROM.read(1));
          boolean setTimeIsAM = (EEPROM.read(0)-4<12);    //Find if time is AM or PM
          strcpy(getTimeParagraph,"<p>The current time is ");
          strcat(getTimeParagraph,hourString);
          strcat(getTimeParagraph,":");
          strcat(getTimeParagraph,minString);
          strcat(getTimeParagraph, " ");
          strcat(getTimeParagraph, timeIsAM ? "AM":"PM"); 
          strcat(getTimeParagraph,"</p>");
          client.println(getTimeParagraph);
          char setTimeParagraph[50];    //Create buffer for text
          strcpy(setTimeParagraph,"<p>The kettle is set to turn on at ");
          strcat(setTimeParagraph,setHourString);
          strcat(setTimeParagraph,":");
          strcat(setTimeParagraph,setMinString);
          strcat(setTimeParagraph, " ");
          strcat(setTimeParagraph, setTimeIsAM ? "AM":"PM"); 
          strcat(setTimeParagraph,"</p>");
          client.println(setTimeParagraph);
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
  digitalWrite(7, currentState ? HIGH : LOW);
  currentState = !currentState;
}

void setTimer(String timer) { //Timer stores value as hour and minute, with military time
  if (timer.length() == 5) {
    int hrs = timer.substring(0, 2).toInt();
    int mins = timer.substring(3, 5).toInt();
    Serial.println(timer);
    EEPROM.update(0, hrs+4);  //Add 4 to the hour word to account for time zones 
    EEPROM.update(1, mins);
  }
}
long getTime() { // returns time as minutes elapsed since midnight
  long val = EEPROM.read(0) * 60 + EEPROM.read(1);
  return val != ((long)255) * 255 ? val : -1; //If time is unset (EEPROM defaults to 255) return -1, else return time
}

void executeCommand(String command) {   //Once a button ahs been clicked, interpret
  if (command.length() >= 7 && command.substring(1, 6) == "CLOCK") {  //Check if the command is to set the clock, and if so, parse the timestamp
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
      switch (c) {  //Change the status based on command issued
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
  currentState = false;
  pinMode(7, OUTPUT);  //Set the relay control pin to output mode
  digitalWrite(7, HIGH);    //Turn of relay
  Serial.begin(9600);
  Ethernet.begin(mac);    //Create server
  Serial.println(Ethernet.localIP());
  delay(2000);
  setTime(getNtpTime());    //Set the time from the NTP server
  delay(2000);
  server = EthernetServer(80); //80 = http communication
  pinMode(LED_BUILTIN, OUTPUT);
  server.begin();   //Start server
}

void loop() {
  // listen for incoming clients, and process request.
  checkForClient();

  if (hour() * 60 + minute() == getTime()) { //If the current time is equal to the scheduled time, turn kettle on
    makeOn();
    delay(100 * 60);
  }
  delay(100);
}

