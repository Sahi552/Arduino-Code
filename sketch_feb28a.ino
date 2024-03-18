#include <DHT.h>

#define DHTPIN 2            // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11       // DHT 11

DHT dht(DHTPIN, DHTTYPE);

const int MQ_PIN = A0;      // Analog pin connected to MQ2 sensor
const int SOIL_PIN = A1;    // Analog pin connected to soil moisture sensor

const float METHANE_RL = 10.0;    // MQ2 sensor resistance at 10000ppm of methane
const float METHANE_RO = 9.8;     // MQ2 sensor resistance in clean air
const float METHANE_SLOPE = 1.8;  // Slope of the methane curve
const float METHANE_INTERCEPT = -0.2; // Intercept of the methane curve

const float HYDROGEN_RL = 9.9;    // MQ2 sensor resistance at 10000ppm of hydrogen
const float HYDROGEN_RO = 10.2;   // MQ2 sensor resistance in clean air
const float HYDROGEN_SLOPE = 1.2; // Slope of the hydrogen curve
const float HYDROGEN_INTERCEPT = -0.5; // Intercept of the hydrogen curve

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  dht.begin();
  delay(10000);
}

void loop() {
  // Read sensor values
  int sensorValue = analogRead(MQ_PIN);
  int soilMoistureValue = analogRead(SOIL_PIN);
  float moisture;
  moisture = (100 - ((soilMoistureValue / 1023.00) * 100));

  // Convert sensor value to voltage
  float voltage = sensorValue * (5.0 / 1023.0);

  float methane_ppm = calculateGasConcentration(sensorValue, METHANE_RL, METHANE_RO, METHANE_SLOPE, METHANE_INTERCEPT);
  float hydrogen_ppm = calculateGasConcentration(sensorValue, HYDROGEN_RL, HYDROGEN_RO, HYDROGEN_SLOPE, HYDROGEN_INTERCEPT);

  // Read temperature and humidity from DHT11 sensor
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  Serial.print(methane_ppm);
  Serial.print(" ");
  Serial.print(hydrogen_ppm);
  Serial.print(" ");
  // Serial.print(" Temp: ");
  Serial.print(temperature);
  Serial.print(" ");
  Serial.print(humidity);
  Serial.print(" ");
  Serial.println(moisture);

  // if (soilMoistureValue >= 0 && soilMoistureValue <= 1023) {
  //   Serial.print(" Moist: ");
    
  //   Serial.println("%");
  // }
  // else {
  //   Serial.println("Failed to read data from soil moisture sensor");
  // }

  // Wait for some time before taking the next reading
  delay(5000);
}


float calculateGasConcentration(int adc_value, float RL, float RO, float slope, float intercept) {
  float Rs = (1023.0 / adc_value) - 1.0;
  float ratio = Rs / RO;
  float gas_ppm = (ratio - intercept) / slope;
  return gas_ppm;
}