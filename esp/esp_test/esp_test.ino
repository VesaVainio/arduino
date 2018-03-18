#include <ESP8266WiFi.h>        // Include the Wi-Fi library

const char* ssid     = "VerkonSSID";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "VerkonSalasana";     // The password of the Wi-Fi network
const char* host     = "vesa-iot-test.azurewebsites.net";
const char* query    = "api/SaveMessage?code=rmIvDDo7P9A4wfjZDsZI9PnBf3kH8vMCm4CY9xTXAGOrg3r9gOlcjw==&temperature=13";

void setup() {
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');
  
  WiFi.begin(ssid, password);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid); 
  Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer

  WiFiClient client;

  if (client.connect(host, 80))
  {
    Serial.println("Connected to host");
    client.print(String("GET /") + query + " HTTP/1.1\r\n" +
             "Host: " + host + "\r\n" +
             "Connection: close\r\n" +
             "\r\n"
            );

    Serial.println("[Response:]");
    while (client.connected())
    {
      if (client.available())
      {
        String line = client.readStringUntil('\n');
        Serial.println(line);
      }
    }
    client.stop();
    Serial.println("\n[Disconnected]");
  }
  else
  {
    Serial.println("Connection failure");
  }
}

void loop() { }
