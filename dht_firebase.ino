#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <SoftwareSerial.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Galaxy A04s 2566"
#define WIFI_PASSWORD "mobilehotspot2.0"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBIH9nEBH5iWU02W2d2Qq4ofH7nlS1dvu0"

// Insert RTDB URL
#define DATABASE_URL "https://dht11-fe021-default-rtdb.firebaseio.com/"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

// Define SoftwareSerial object
SoftwareSerial EspSerial(16, 17);

// Flag for successful signup
bool signupOK = false;

void setup(){
  Serial.begin(115200);
  EspSerial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop(){
  delay(1000);
  String data = EspSerial.readStringUntil('\r');

  // Find the indices of each sensor reading
  int space1Index = data.indexOf(' '); // Index of first space
  int space2Index = data.indexOf(' ', space1Index + 1); // Index of second space
  int space3Index = data.indexOf(' ', space2Index + 1); // Index of third space
  int space4Index = data.indexOf(' ', space3Index + 1); // Index of fourth space

  // Extract each sensor reading as a substring
  String methaneStr = data.substring(0, space1Index);
  String hydrogenStr = data.substring(space1Index + 1, space2Index);
  String temperatureStr = data.substring(space2Index + 1, space3Index);
  String humidityStr = data.substring(space3Index + 1, space4Index);
  String moistureStr = data.substring(space4Index + 1);

  // Convert strings to integers
  int methane = methaneStr.toInt();
  int hydrogen = hydrogenStr.toInt();
  int temperature = temperatureStr.toInt();
  int humidity = humidityStr.toInt();
  int moisture = moistureStr.toInt();
  
  if (Firebase.ready() && signupOK ) {
    if (Firebase.RTDB.setFloat(&fbdo, "DHT/methane", methane)){
       Serial.print("Methane: ");
       Serial.println(methane);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "DHT/hydrogen", hydrogen)){
       Serial.print("Hydrogen: ");
       Serial.println(hydrogen);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "DHT/temperature", temperature)){
       Serial.print("Temperature: ");
       Serial.println(temperature);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    
    if (Firebase.RTDB.setFloat(&fbdo, "DHT/humidity", humidity)){
       Serial.print("Humidity: ");
       Serial.println(humidity);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "DHT/moisture", moisture)){
       Serial.print("Moisture: ");
       Serial.println(moisture);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
  Serial.println("______________________________");
}
