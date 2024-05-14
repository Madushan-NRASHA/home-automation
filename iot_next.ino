#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ESP32Servo.h>
#define WATER_SENSOR_PIN 34
#define GAS_SENSOR_PIN 39
#define SERVO_PIN 4
#define TRIG_PIN 12
#define ECHO_PIN 14
#define LED_PIN 2
#define LED_PIN_2 21
#define WIFI_SSID "OLAX_4G_E61B"
#define WIFI_PASSWORD "34787551"
#define API_KEY "AIzaSyB61RgvjKh09xXGxtUG3da2SxBxwsMLQz0"
#define DATABASE_URL "https://test-cart-72ea7-default-rtdb.firebaseio.com/"

FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

const int ledPin = LED_PIN;
const int ledpin2 = LED_PIN_2;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
String door;
String led;
String lightSensor;

// Function declarations
void connectToWiFi();
void initializeFirebase();
void tokenStatusCallback(token_info_t tokenInfo);
void readFirebaseData();
void sendDataToFirebase(int waterSensorValue, int gasSensorValue);
void alwaysOn();
void closeDoor();

Servo myservo;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(ledpin2, OUTPUT);
  Serial.begin(115200);

  connectToWiFi();
  initializeFirebase();

  myservo.attach(SERVO_PIN);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  readFirebaseData();
  delay(1000);
  int waterSensorValue = analogRead(WATER_SENSOR_PIN);
  if(waterSensorValue<=500){
    Serial.print("Water Sensor Value: ");
    Serial.println(waterSensorValue);
    waterSensodata(waterSensorValue);
  }
  int gasSensorValue = analogRead(GAS_SENSOR_PIN);
  if(gasSensorValue>=900){
    Serial.print("Gas Sensor Value: ");
    Serial.println(gasSensorValue);
    gasSensordata(gasSensorValue);
  }

  // if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
  //   sendDataPrevMillis = millis();
  //   // sendDataToFirebase(waterSensorValue, gasSensorValue);
  // }

  if (door == "on") {
    alwaysOn();
  } else if (door == "off") {
    closeDoor();
  }

  if (lightSensor == "on") {
       digitalWrite(ledPin, HIGH);

  } else if (lightSensor == "off") {
    digitalWrite(ledPin, LOW);
}
}
void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void initializeFirebase() {
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase initialization successful");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void tokenStatusCallback(token_info_t tokenInfo) {
  // Implementation of the tokenStatusCallback function
  // You can add the necessary code here
}

void readFirebaseData() {
  if (Firebase.RTDB.getString(&firebaseData, "/data")) {
    door = firebaseData.stringData();
    Serial.print("Read_data (door): ");
    Serial.println(door);
  } else {
    Serial.println(firebaseData.errorReason());
  }
  delay(1000);
  if (Firebase.RTDB.getString(&firebaseData, "/data2")) {
    led = firebaseData.stringData();
    Serial.print("Read_data (led): ");
    Serial.println(led);
  } else {
    Serial.println(firebaseData.errorReason());
  }
  delay(1000);
  if (Firebase.RTDB.getString(&firebaseData, "/data3")) {
    lightSensor = firebaseData.stringData();
    Serial.print("Read_data (lightSensor): ");
    Serial.println(lightSensor);
  } else {
    Serial.println(firebaseData.errorReason());
  }
  delay(1000);

}

void waterSensodata(int waterSensorValue) {
  if (Firebase.RTDB.setInt(&firebaseData, "get_Data/waterSensorValue", waterSensorValue)) {
    Serial.print("Water Sensor Value sent to Firebase: ");
    Serial.println(waterSensorValue);
  } else {
    Serial.println("Failed to send Water Sensor Value to Firebase");
    Serial.println("REASON: " + firebaseData.errorReason());
  }
}
void gasSensordata(int gasSensorValue){
  if (Firebase.RTDB.setFloat(&firebaseData, "get_Data/gasSensorValue", gasSensorValue)) {
    Serial.print("Gas Sensor Value sent to Firebase: ");
    Serial.println(gasSensorValue);
  } else {
    Serial.println("Failed to send Gas Sensor Value to Firebase");
    Serial.println("REASON: " + firebaseData.errorReason());
  }

}
void alwaysOn() {
  // Ultrasonic sensor measurements
  long duration, distance;

  // Trigger the ultrasonic sensor
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure the echo duration
  duration = pulseIn(ECHO_PIN, HIGH);

  // Convert the duration to distance (in centimeters)
  distance = (duration * 0.0343) / 2;

  // Print the distance to the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Change servo position based on the distance
  if (distance <= 10) {
    myservo.write(0);    // Set servo position to 0 degrees
  } else if(distance >= 11){
    myservo.write(100);  // Set servo position to 100 degrees
  }
}
void closeDoor() {
  long duration, distance;

  // Trigger the ultrasonic sensor
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure the echo duration
  duration = pulseIn(ECHO_PIN, HIGH);

  // Convert the duration to distance (in centimeters)
  distance = (duration * 0.0343) / 2;

  // Print the distance to the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance <= 10) {
    String stringValue = "on";

    // Set the string value in Firebase Realtime Database
    if (Firebase.RTDB.setInt(&firebaseData, "get_Data/DoorSensorValue", distance)) {
      Serial.print("String value set successfully: ");
      Serial.println(stringValue);
    } else {
      Serial.println("Error setting string value");
    }

    if (led == "on") {
      myservo.write(0);
    } else if (led == "off") {
      myservo.write(100);
    }
  } else if (distance >= 10) {
    String stringValue = "off";
    myservo.write(100);
    // Set the string value in Firebase Realtime Database
    if (Firebase.RTDB.setInt(&firebaseData, "get_Data/DoorSensorValue", distance)) {
      Serial.print("String value set successfully: ");
      Serial.println(stringValue);
    } else {
      Serial.println("Error setting string value");
    }
  }
}

