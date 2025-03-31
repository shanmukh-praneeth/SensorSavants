#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"
#include <MQUnifiedsensor.h>
#include <ThingSpeak.h>

// Replace with your network credentials
const char* ssid = "ToughThroat";      // Your Wi-Fi SSID
const char* password = "Shanmukh";  // Your Wi-Fi Password

// ThingSpeak channel details
unsigned long channelID = 2708635; // Replace with your channel ID
const char* apiKey = "I91D6D4PGC36D8FK";       // Replace with your ThingSpeak API key
WiFiClient client;


#define DHTPIN 25
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define SOIL_MOISTURE_PIN 32
#define MQ9_PIN 34
#define MQ135_PIN 35
#define LDR_PIN 13

#define RAIN_SENSOR1_PIN 4
#define RAIN_SENSOR2_PIN 14
#define RAIN_SENSOR3_PIN 27
#define RAIN_SENSOR4_PIN 26

void setup() {
  Serial.begin(115200);

  connectwifi();

  ThingSpeak.begin(client);
}

void connectwifi(){
  delay(1000);

  // Connect to Wi-Fi
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid,password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }

  Serial.println("Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  //Temperature And Humidity
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if(isnan(temperature)||isnan(humidity)){
    Serial.println("Failed to read from DHT11");
  }
  else{
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println("°C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
  }

  // Soil Moisture Logic
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);

  // Map sensor reading to percentage (adjust values for your sensor calibration)
  int soil_moisture_value = map(soilMoisture, 2300, 300, 0, 100);

  Serial.print("Soil Moisture: ");
  Serial.print(soil_moisture_value);
  Serial.println("%");

  //MQ9 reading
  int mq9_value = analogRead(MQ9_PIN);

  //MQ135 reading
  int mq135_value = analogRead(MQ135_PIN);

  //Light Intensity
  int ldrValue = (digitalRead(LDR_PIN)==LOW)?1:0;

  Serial.print("LDR Value: ");
  Serial.println(ldrValue);
  

  //Rain Sensor
  int rain1 = (digitalRead(RAIN_SENSOR1_PIN)==LOW)?1:0;
  int rain2 = (digitalRead(RAIN_SENSOR2_PIN)==LOW)?1:0;
  int rain3 = (digitalRead(RAIN_SENSOR3_PIN)==LOW)?1:0;
  int rain4 = (digitalRead(RAIN_SENSOR4_PIN)==LOW)?1:0;

  int isRaining = (rain1 & rain2) | (rain1 & rain3) | (rain1 & rain4) | (rain2 & rain3) | (rain2 & rain4) | (rain3 & rain4);

  if(isRaining == LOW){
    Serial.println("Its Raining");
  }
  else{
    Serial.println("Not Raining");
  }
  // Serial.println(rain1);
  // if(rain1 == LOW){
  //   Serial.println("Its Raining");
  // }
  // else{
  //   Serial.println("Not Raining");
  // }
  
  // Check thresholds
  String warningMessage = "";
  if (temperature < 18 || temperature > 26) {
    warningMessage += "Temperature out of range! ";
    Serial.println();
  }
  if (humidity < 50 || humidity > 70) {
    warningMessage += "Humidity out of range! ";
    Serial.println();
  }
  if (soil_moisture_value < 60) {
  warningMessage += "Soil too dry! ";
  Serial.println();
  }
  if (soil_moisture_value > 80) {
    warningMessage += "Soil too wet! ";
    Serial.println();
  }
  if(mq9_value<1500){
    warningMessage += "Safe";
    Serial.println();
  }
  if(mq9_value>1500 && mq9_value<2500){
    warningMessage += "Approaching dangerous condition";
    Serial.println();
  }
  if(mq9_value>2500){
    warningMessage += "Dangerous levels";
    Serial.println();
  }
    if(mq135_value<2000){
    warningMessage += "Safe";
    Serial.println();
  }
  if(mq135_value>2000 && mq135_value<3000){
    warningMessage += "Approaching dangerous condition";
    Serial.println();
  }
  if(mq135_value>3000){
    warningMessage += "Dangerous levels";
    Serial.println();
  }
  if (ldrValue == 0) {
    warningMessage += "Insufficient light! ";
    Serial.println();
  }
  if (isRaining==LOW) {
    warningMessage += "Rain detected! ";
    Serial.println();
  }

  // Send Data to ThingSpeak
  ThingSpeak.setField(1, temperature);       // Field 1: Temperature
  ThingSpeak.setField(2, humidity);          // Field 2: Humidity
  ThingSpeak.setField(3, soil_moisture_value);      // Field 3: Soil Moisture
  ThingSpeak.setField(4, ldrValue);    // Field 4: Light Intensity
  ThingSpeak.setField(5, isRaining);         // Field 5: Rain Status (1: Yes, 0: No)
  ThingSpeak.setField(6,mq9_value);
  ThingSpeak.setField(7,mq135_value);
  ThingSpeak.setStatus(warningMessage); // Send warning message to ThingSpeak status


  int status = ThingSpeak.writeFields(channelID, apiKey);
  if (status == 200) {
    Serial.println("Data sent to ThingSpeak successfully!");
  } else {
    Serial.print("Failed to send data to ThingSpeak. Error: ");
    Serial.println(status);
  }

  // Delay between updates
  delay(900000); // Update ThingSpeak every 15 minutes
}
