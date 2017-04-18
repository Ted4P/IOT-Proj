
//Ethernet Library tutorial credit to 
//http://bildr.org/2011/06/arduino-ethernet-pin-control/
#include <Ethernet.h>
#include <SPI.h>
boolean reading = false;
byte ip[] = {192, 168, 0, 2};  
byte gateway[] = {192, 168, 0, 1}; 
byte subnet[] = {255, 255, 0, 0}; 


boolean currentState=false;

//Placeholder MAC, can be replaced with value printed on ethernet shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetServer server = EthernetServer(80); //80 = http communication

void setup(){
  pinMode(7, OUTPUT);  //Set the relay control pin to output mode
  Ethernet.begin(mac, ip, gateway, subnet);
  pinMode(LED_BUILTIN, OUTPUT);
  server.begin();
}

void loop(){

  // listen for incoming clients, and process request.
  checkForClient();
}

void checkForClient(){
  EthernetClient client = server.available();

  if (client) {

    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    boolean sentHeader = false;
    String lastCommand="";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        if(reading && c == ' ') reading = false;
        if(c == '?') reading = true; //found the ?, begin reading the info
        
        if(reading){
          Serial.print(c);

          lastCommand += c;
          executeCommand(lastCommand);
        }

        if (c == '\n' && currentLineIsBlank){
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close");
                    client.println();
                    // send web page
                    client.println("<!DOCTYPE html>");
                    client.println("<html>");
                    client.println("<head>");
                    client.println("<title>Arduino Web Page</title>");
                    client.println("</head>");
                    client.println("<body>");
                    //if(currentState){
                        //client.println("<p>");
                        //client.println("<h1>The Heat Rises...</h1>");
                        //client.println("<iframe src=\"//giphy.com/embed/NrqUE766fNTXO\" width=\"480\" height=\"360\" frameBorder=\"0\" class=\"giphy-embed\" allowFullScreen></iframe><p><a href=\"https://giphy.com/gifs/NrqUE766fNTXO\">via GIPHY</a></p>");
                        //client.println("<input type=\"button\" style = \"postion: absolute; top: 450; left: (450-75)/2; height: 20px; width: 50px; background-color:red; cursor:pointer\" value=\"On\" onclick=\"window.location.href='?o'\"/>");
                        //client.println("<input type=\"button\" style = \"postion: absolute; top: 450; left: (450-75)/2; height: 20px; width: 50px; background-color:white; cursor:pointer\" value=\"Off\" onclick=\"window.location.href='?f'\"/>");
                   // }
                   //else{
                        client.println("</p>");
                        client.println("<h1>It's Tea Time</h1>");
                        client.println("<img src=\"https://images-na.ssl-images-amazon.com/images/G/01/aplusautomation/vendorimages/4e14cd59-bc1c-4f0b-8ad4-26cf216c2d5d.jpg._CB290777952_.jpg\" alt = \"image not found\" style=\"width:450px;height:450px;\"><br />");
                        client.println("<input type=\"button\" style = \"postion: absolute; top: 450; left: (450-75)/2; height: 20px; width: 50px; background-color:white; cursor:pointer\" value=\"On\" onclick=\"window.location.href='?o'\"/>");
                        client.println("<input type=\"button\" style = \"postion: absolute; top: 450; left: (450-75)/2; height: 20px; width: 50px; background-color:red; cursor:pointer\" value=\"Off\" onclick=\"window.location.href='?f'\"/>");
                    //}
                    client.println("<input type=\"button\" style = \"height: 20px; width: 50px; background-color:white; cursor:pointer\" value=\"Toggle\" onclick=\"window.location.href='http://192.168.0.2/?t'\"/>");
                    client.println("</body>");
                    client.println("</html>");
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

void makeOn(){
  if(!currentState) toggleRelay();
}
void makeOff(){
  if(currentState) toggleRelay();
}

void toggleRelay(){    //Toggle relay on/off
  digitalWrite(7,currentState?LOW:HIGH);
  currentState=!currentState;
}

void executeCommand(String command){
    for(int i=0;i<command.length();i++){
        char c = command.charAt(i);
        switch(c){
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
