#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#include <DHT.h>

#define WIFI_SSID "Cha"
#define WIFI_PASSWORD "12345678"
#define THINGSPEAK_CHANNEL_ID 2059878 // Replace with your channel ID
#define THINGSPEAK_API_KEY "SKVGCMIHA5TVNVD8" // Replace with your API key
#define DHTPIN 4
#define DHTTYPE DHT22
#define TRIG_PIN 5
#define ECHO_PIN 18
#define LED_PIN 14
#define MPU6050_ADDR 0x68 // MPU6050 I2C address

WiFiClient client;
DHT dht(DHTPIN, DHTTYPE);
Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(LED_PIN, OUTPUT);
  if (!mpu.begin(MPU6050_ADDR)) {
    Serial.println("Failed to find MPU6050 chip");
    while (1);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  ThingSpeak.begin(client);
  dht.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  int readData = dht.read(DHTPIN); 
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  float f = dht.readTemperature(true);
  
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float acc_x = a.acceleration.x;
  float acc_y = a.acceleration.y;
  float acc_z = a.acceleration.z;

  float gyro_x = g.gyro.x;
  float gyro_y = g.gyro.y;
  float gyro_z = g.gyro.z;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  float distance = pulseIn(ECHO_PIN, HIGH) / 58.0;

  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, distance);
  ThingSpeak.setField(4, acc_x);
  ThingSpeak.setField(5, acc_y);
  ThingSpeak.setField(6, acc_z);
  
    if (temperature > 30) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
  


  int response = ThingSpeak.writeFields(THINGSPEAK_CHANNEL_ID, THINGSPEAK_API_KEY);
  if (response == 200) {
    Serial.println("Values sent to ThingSpeak successfully!");
  } else {
    Serial.println("Failed to send values to ThingSpeak.");
  }
  Serial.println("------");
  Serial.println((String)"x:"+acc_x+" y:"+acc_y+" z:"+acc_z);

  Serial.print("huminity :"); 
 Serial.print(humidity);
  Serial.println(" %"); 
 Serial.print("distance :");
 Serial.println(distance);
  Serial.print("temperature :");
 Serial.print(temperature);
 Serial.println(" celcius degree "); 
  delay(250);
}
