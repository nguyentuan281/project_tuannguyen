// Code khối cơ cấu chấp hành phân loại

#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

#define MQTT_CLIENT_ID "mqttx"
#define MQTT_BROKER "broker.emqx.io"
#define MQTT_USER "phi_den"
#define MQTT_PASSWORD "123Phi"
#define MQTT_PORT 1883

#define SERVO_PIN 26
#define HN1_PIN 33
#define HN2_PIN 25
#define MOTOR_PIN 32
#define LIGHT_PIN 2

WiFiClient espClient;
PubSubClient client(espClient);
Servo servo; 
String label = "";
int count = 0;
int prevHN1State = HIGH; 
volatile unsigned long last_micros;
long debouncing_time = 200;

const char* wifi_ssid = "Hi";
const char* wifi_password = "khang221020201";

// MQTT Topics
const char* TOPIC_LABEL = "Label";
const char* TOPIC_Servo = "Servo";
const char* TOPIC_Motor = "Motor";
const char* TOPIC_Count = "Count";
const char* TOPIC_State = "State";

// Function to connect to WiFi
void connectWiFi() {
  WiFi.begin(wifi_ssid, wifi_password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi!");
}

// Function to connect to MQTT broker
void connectMQTT() {
  client.setServer(MQTT_BROKER, MQTT_PORT);
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected!");
      client.subscribe(TOPIC_LABEL);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// Callback function for incoming MQTT messages
void messageCallback(char* topic, byte* message, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)message[i];
  }
  
  if (String(topic) == TOPIC_LABEL) {
    label = msg;
    Serial.println("Received Label: " + label);
    if (label == "Yes") {
        controlMotor(true);
        controlServo(true);
        while(digitalRead(HN2_PIN) != 0 ){
          Serial.println("Wait");
        }
        count++;
        client.publish(TOPIC_Count, String(count).c_str());
        controlServo(false);
        //controlMotor(true);
        
      } else {
        controlMotor(true);
  }
}

// Function to control motor
void controlMotor(bool state) {
  digitalWrite(MOTOR_PIN, state ? HIGH : LOW);
  client.publish(TOPIC_Motor, state ? "On" : "Off");
  Serial.println(state ? "Motor On" : "Motor Off");
}

// Function to control servo
void controlServo(bool state) {
  int duty = state ? 15 : 90;  
  servo.write(duty);
  client.publish(TOPIC_Servo, state ? "Active" : "Inactive");
  Serial.println(state ? "Servo Active" : "Servo Inactive");
}

void setup() {
  Serial.begin(115200);
  servo.attach(SERVO_PIN); 
  pinMode(HN1_PIN, INPUT);
  pinMode(HN2_PIN, INPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  
  connectWiFi();
  client.setCallback(messageCallback);
  connectMQTT();
  controlMotor(true);
  controlServo(false);
  client.publish(TOPIC_Count, String(count).c_str());
}

void loop() {
  client.loop();

  int currentHN1State = digitalRead(HN1_PIN); // write current state of HN1
 
  if (currentHN1State != prevHN1State) {
    prevHN1State = currentHN1State;  //update state
    if (currentHN1State == LOW) {
      if((long)(micros() - last_micros) >= debouncing_time * 1000){
      last_micros = micros();
      Serial.println("Detection active " + label);
      controlMotor(false);
      client.publish(TOPIC_State, "On");
       }
    }
  }


}

