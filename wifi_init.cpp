#include <SPI.h>
#include <WiFi101.h>

extern void printWifiStatus();

void wifi_init(const char* wlan_ssid, const char* wlan_pass) 
{
  /* Initialise the module */
  Serial.println(F("\nInitializing WiFi Sheild 101 ..."));
  if (WiFi.status() == WL_NO_SHIELD) 
  {
      Serial.println(F("WiFi shield not present"));
      // don't continue:
      while (true);
  }

  // attempt to connect to Wifi network:
  int status;
  do {
      Serial.print(F("Attempting to connect to SSID: "));
      Serial.println(wlan_ssid);
      // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
      status = WiFi.begin(wlan_ssid, wlan_pass);   
      // delay(1); // wait few seconds for connection  
  } while (status != WL_CONNECTED);

  // you're connected now, so print out the status:
  printWifiStatus();
}


void printWifiStatus() 
{
  // print the SSID of the network you're attached to:
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print(F("IP Address: "));
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print(F("signal strength (RSSI):"));
  Serial.print(rssi);
  Serial.println(F(" dBm"));
}



