#include "crc16.h"
#include <String.h>
#include <SoftwareSerial.h>
#include <WiFi.h>
uint8_t outdata[] = {0x02, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x38 } ;
uint8_t incomingdata[9] ;
String WindWay[] = {"北風","東北風","東風","東南風","南風","西南風","西風","西北風" } ;

// --------WIFI USE
uint8_t MacData[6];
char ssid[] = "IOT";      // your network SSID (name)
char pass[] = "0123456789";     // your network password
int keyIndex = 0;                 // your network key Index number (needed only for WEP)
IPAddress  Meip ,Megateway ,Mesubnet ;
String MacAddress ;
int status = WL_IDLE_STATUS;

WiFiServer server(80);

#define RXPin  0
#define TXPin  1

 SoftwareSerial mySerial(RXPin, TXPin); // RX, TX
 //modbusDevice dev = new modbusDevice() ;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600)  ;
 Serial.println("Program Start Here");
 mySerial.begin(9600) ;
   if (WiFi.status() == WL_NO_SHIELD) 
       {
            Serial.println("WiFi shield not present");
            // don't continue:
            while (true);
      }
  String fv = WiFi.firmwareVersion();
  if (fv != "1.1.0") 
      {
           Serial.println("Please upgrade the firmware");
      }
    MacAddress = GetWifiMac() ; // get MacAddress
    ShowMac() ;       //Show Mac Address
    
  // attempt to connect to Wifi network:
    initializeWiFi();
    server.begin();
  // you're connected now, so print out the status:
    ShowInternetStatus();
}

void loop() {
    

                  

   // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          if (GetWindCheck()) 
          {
                 // output the value of each analog input pin
                 client.print("Wind Direction :(");
                 client.print(WindWay[CalcWind(incomingdata[3],incomingdata[4])]);
                 client.print(" and  ");
                 client.print(CalcWind(incomingdata[5],incomingdata[6]));
                 client.print(" 度)");
          }

          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}
int CalcWind(uint8_t Hi, uint8_t Lo)
{
    return (  Hi *  256+ Lo  ) ;
}
boolean GetWindCheck()
{
      sendQuery() ;
    if (receiveQuery())
        {
        //    Serial.println(incomingdata[3],HEX) ;
        //    Serial.println(incomingdata[4],HEX) ;
         //   Serial.println(( ( (double)incomingdata[3]*256+(double)incomingdata[4] )/10) ) ;
            
            if ( CompareCRC16(ModbusCRC16(incomingdata,7),incomingdata[8],incomingdata[7]) )
                {
                  return (true)  ;
                }
                else
                {
                  return (false)  ;
                }
                
        }
        else
        {
            return (false) ; 
          
        }
        
}

void sendQuery()
{
    mySerial.write(outdata,8) ;
      
}

boolean receiveQuery()
{
    boolean ret = false ;
    unsigned strtime = millis() ;
      while(true)
        {
            if ( (millis() - strtime) > 2000)
            {
                 ret = false ;
                  return  ret ;
            }
              
            if (mySerial.available() >= 9)
                {
                    mySerial.readBytes(incomingdata, 9) ;
                      ret = true ;
                      return  ret ;
                }
        }
}


void ShowMac()
{
  
     Serial.print("MAC:");
     Serial.print(MacAddress);
     Serial.print("\n");

}




String GetWifiMac()
{
   String tt ;
    String t1,t2,t3,t4,t5,t6 ;
    WiFi.status();    //this method must be used for get MAC
  WiFi.macAddress(MacData);
  
  Serial.print("Mac:");
   Serial.print(MacData[0],HEX) ;
   Serial.print("/");
   Serial.print(MacData[1],HEX) ;
   Serial.print("/");
   Serial.print(MacData[2],HEX) ;
   Serial.print("/");
   Serial.print(MacData[3],HEX) ;
   Serial.print("/");
   Serial.print(MacData[4],HEX) ;
   Serial.print("/");
   Serial.print(MacData[5],HEX) ;
   Serial.print("~");
   
   t1 = print2HEX((int)MacData[0]);
   t2 = print2HEX((int)MacData[1]);
   t3 = print2HEX((int)MacData[2]);
   t4 = print2HEX((int)MacData[3]);
   t5 = print2HEX((int)MacData[4]);
   t6 = print2HEX((int)MacData[5]);
 tt = (t1+t2+t3+t4+t5+t6) ;
Serial.print(tt);
Serial.print("\n");
  
  return tt ;
}
String  print2HEX(int number) {
  String ttt ;
  if (number >= 0 && number < 16)
  {
    ttt = String("0") + String(number,HEX);
  }
  else
  {
      ttt = String(number,HEX);
  }
  return ttt ;
}





void printWifiData() 
{
  // print your WiFi shield's IP address:
  Meip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(Meip);
  Serial.print("\n");

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);

  // print your subnet mask:
  Mesubnet = WiFi.subnetMask();
  Serial.print("NetMask: ");
  Serial.println(Mesubnet);

  // print your gateway address:
  Megateway = WiFi.gatewayIP();
  Serial.print("Gateway: ");
  Serial.println(Megateway);
}

void ShowInternetStatus()
{
    
        if (WiFi.status())
          {
               Meip = WiFi.localIP();
               Serial.print("Get IP is:");
               Serial.print(Meip);
               Serial.print("\n");
              
          }
          else
          {
                       Serial.print("DisConnected:");
                       Serial.print("\n");
          }

}

void initializeWiFi() {
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
  //   status = WiFi.begin(ssid);

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.print("\n Success to connect AP:") ;
  Serial.print(ssid) ;
  Serial.print("\n") ;
  
}
