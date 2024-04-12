#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Firebase_ESP_Client.h>
#include <SoftwareSerial.h>
#include <Arduino.h>
#include <SPIFFS.h>
#include <FS.h>

const char *ssid = "Galaxy A04s 2566";
const char *password = "mobilehotspot2.0";

//web server part
AsyncWebServer server(80);

//firebase part
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

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

const char index_html[] PROGMEM =
  R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <div class="logo">
        <img src="/internet-of-things.png" width="120px" height="120px">
    </div>
    <title>Anaerobic Reactor</title>
    <style>
    *{
    font-family: monospace;
    }

    h1 {
      color: aqua;
      text-align: center;
      margin-top: 1cm;
    }

    .logo{
      text-align: center;
    }

    .sensor_container {
      text-align: center;
      margin-top: 30px;
    }

    .sensor {
      display: inline-block;
      margin-top: 10px; 
      margin-bottom: 10px;
      margin-left: 50px;
      margin-right: 50px;
      width: 180px;
      height: 180px;
      background-image: linear-gradient(90deg,rgb(40, 36, 36),rgb(74, 72, 72));
      border: 1px solid #4c3b3b;
      border-radius: 8px;
      margin: 20px;
      padding: 10px;
      display: inline-block;
      transition: transform 0.3s ease;
    }

    .sensor:hover{
      transform: scale(1.2);
    }

    .sensor img {
      display: block; 
      margin-bottom: 5px; 
      margin: 0 auto;
      width: 120px;
      height: 120px;
    }


    .sensor p{
      font-size: 20px;
    }

    .methane {
      color: rgb(207, 4, 96); 
    }

    .hydrogen {
      color: rgb(35, 141, 145); 
    }

    .temperature {
      color: red; 
    }

    .humidity {
      color: rgb(226, 85, 25); 
    }

    .soil_moisture {
      color: rgb(216, 94, 216); 
    }

    body{
      background-image: linear-gradient(90deg,rgb(40, 36, 36),rgb(74, 72, 72));
    }

    .sensor_name{
      text-align: center;
      font-size: 20px;
      margin-top: 10px;
    }

    .sensor_value {
      text-align: center;
      font-size: 24px;
      margin-top: 20px;
    }

    .gauge_container {
      text-align: center;
      margin-top: 20px;
    }

    .progress_container {
      display: inline-block;
      margin: 0 80px;
    }
    </style>
  </head>
  <body>
    <h1>Anaerobic Reactor</h1>
    <div class="sensor_container">
    <div class="sensor">
      <img src="/methane.png" alt="Methane" width="120px" height="120px">
      <p class="methane">Methane: <span class="sensor_value" id="methane_value">0</span></p>
    </div>
    <div class="sensor">
      <img src="/hydrogen.png" alt="Hydrogen" width="120px" height="120px">
      <p class="hydrogen">Hydrogen: <span class="sensor_value" id="hydrogen_value">0</span></p>
    </div>
    <div class="sensor">
       <img src="/temperature.png" alt="Temperature" width="120px" height="120px">
      <p class="temperature">Temperature: <span class="sensor_value" id="temperature_value">0</span></p>
    </div>
    <div class="sensor">
      <img src="/humidity.png" alt="Humidity" width="120px" height="120px">
      <p class="humidity">Humidity: <span class="sensor_value" id="humidity_value">0</span></p>
    </div>
    <div class="sensor">
       <img src="/soil_analysis.png" alt="Moisture" width="120px" height="120px">
      <p class="soil_moisture">Moisture: <span class="sensor_value" id="moisture_value">0</span></p>
    </div>
    </div>
    <div class="gauge_container">
    <!-- Circular progress bars will be dynamically created here -->
    </div>
    <script>
      function setSensorValuesFromESP32() {
      var xhttp = new XMLHttpRequest();
    
      xhttp.open("GET", "/sensor_values", true);
    
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            var sensorValues = JSON.parse(this.responseText);
            setSensorValues(sensorValues);
        }
      }; 
      xhttp.send();
      }
      function setSensorValues(sensorValues) {
        document.getElementById("methane_value").innerText = sensorValues.methane;
        document.getElementById("hydrogen_value").innerText = sensorValues.hydrogen;
        document.getElementById("temperature_value").innerText = sensorValues.temperature;
        document.getElementById("humidity_value").innerText = sensorValues.humidity;
        document.getElementById("moisture_value").innerText = sensorValues.moisture;
        createCircularProgress();
      }
      function createCircularProgress() {
        const colors = ['rgb(207, 4, 96)', 'rgb(35, 141, 145)', 'red', 'rgb(226, 85, 25)', 'rgb(216, 94, 216)'];
        const sensors = document.querySelectorAll('.sensor');

        sensors.forEach((sensor, index) => {
          const sensorValue = parseInt(sensor.querySelector('.sensor_value').innerText);

          const progressContainer = document.createElement('div');
          progressContainer.classList.add('progress_container');

          const svg = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
          svg.setAttribute('width', '100');
          svg.setAttribute('height', '100');

          const borderCircle = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
          borderCircle.setAttribute('cx', '50');
          borderCircle.setAttribute('cy', '50');
          borderCircle.setAttribute('r', '45');
          borderCircle.setAttribute('fill', 'none');
          borderCircle.setAttribute('stroke', colors[index]);
          borderCircle.setAttribute('stroke-width', '8');
        
          borderCircle.setAttribute('stroke-dasharray', `0 ${2 * Math.PI * 45}`);
        
          borderCircle.style.transition = 'stroke-dasharray 1s ease-in-out';
        
          borderCircle.setAttribute('transform', 'rotate(-90 50 50)');
        
          svg.appendChild(borderCircle);

          const text = document.createElementNS('http://www.w3.org/2000/svg', 'text');
          text.setAttribute('x', '50');
          text.setAttribute('y', '58'); 
          text.setAttribute('text-anchor', 'middle');
          text.setAttribute('font-size', '16');
          text.setAttribute('fill', colors[index]);
          text.textContent = sensorValue + '%';
          svg.appendChild(text);

          progressContainer.appendChild(svg);

          document.querySelector('.gauge_container').appendChild(progressContainer);
        
          setTimeout(() => {
            borderCircle.setAttribute('stroke-dasharray', `${(sensorValue / 100) * 2 * Math.PI * 45} ${2 * Math.PI * 45}`);
            }, 100);
          });
        } 


          setSensorValuesFromESP32();
      </script>
    </body>
  </html>
)rawliteral";


void setup() {
  Serial.begin(115200);

  EspSerial.begin(9600);

  if (!SPIFFS.begin(true)) {
    Serial.println("An error occurred while mounting SPIFFS...");
    return;
  }

  Serial.println("SPIFFS initialized successfully!");
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  server.begin();

}

void loop() {
  delay(10000);
  String data = EspSerial.readStringUntil('\r');

  // Find the indices of each sensor reading
  int space1Index = data.indexOf(' ');  // Index of first space
  int space2Index = data.indexOf(' ', space1Index + 1);
  int space3Index = data.indexOf(' ', space2Index + 1);
  int space4Index = data.indexOf(' ', space3Index + 1);

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

  if (Firebase.ready() && signupOK) {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send_P(200,"text/html", index_html);
    });

    server.on("/sensor_values", HTTP_GET, [methane, hydrogen, temperature, humidity, moisture](AsyncWebServerRequest *request) {
      String response = "{\"methane\": " + String(methane) + ", \"hydrogen\": " + String(hydrogen) + ", \"temperature\": " + String(temperature) + ", \"humidity\": " + String(humidity) + ", \"moisture\": " + String(moisture) + "}";
      request->send(200, "application/json", response);
    });

    server.on("/methane.png", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/methane.png", "image/png");
    });

    server.on("/hydrogen.png", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/hydrogen.png", "image/png");
    });

    server.on("/temperature.png", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/temperature.png", "image/png");
    });

    server.on("/humidity.png", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/humidity.png", "image/png");
    });

    server.on("/soil_analysis.png", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/soil_analysis.png", "image/png");
    });

    
    if (Firebase.RTDB.setFloat(&fbdo, "DHT/methane", methane)) {
      Serial.print("Methane: ");
      Serial.println(methane);
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "DHT/hydrogen", hydrogen)) {
      Serial.print("Hydrogen: ");
      Serial.println(hydrogen);
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "DHT/temperature", temperature)) {
      Serial.print("Temperature: ");
      Serial.println(temperature);
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "DHT/humidity", humidity)) {
      Serial.print("Humidity: ");
      Serial.println(humidity);
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "DHT/moisture", moisture)) {
      Serial.print("Moisture: ");
      Serial.println(moisture);
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
  Serial.println("______________________________");
}
