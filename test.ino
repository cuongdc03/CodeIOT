#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <Wire.h>
#include "Adafruit_PWMServoDriver.h"

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
#define MIN_PULSE_WIDTH 600
#define MAX_PULSE_WIDTH 2600
#define FREQUENCY 50

const char* ssid = "ktx";
const char* password = "12345678";

const char* serverAddress = "13.229.105.152"; // IP của máy chủ WebSocket
const int serverPort = 2345; // Cổng của máy chủ WebSocket
const char* path = ""; // Đường dẫn của WebSocket nếu có

WebSocketsClient webSocket;

#define Trig_Sensor_2 14 //D5
#define Echo_Sensor_2 12 //D6

#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.3937011

unsigned long previousRequestTime = 0;
const unsigned long requestInterval = 1000; // Thời gian giữa các yêu cầu là 3 giây

int pulseWidth(int angle) {
  int pulse_wide = map(angle, 0, 180, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
  int analog_value = int(float(pulse_wide) / 1000000 * FREQUENCY * 4096);
  return analog_value;
}

float get_distance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  float duration = pulseIn(echo, HIGH);
  int distanceCm = duration * SOUND_VELOCITY / 2;

  return distanceCm;
}

void setup() {
  pinMode(Trig_Sensor_2, OUTPUT);
  pinMode(Echo_Sensor_2, INPUT);
  pwm.begin();
  pwm.setPWMFreq(FREQUENCY);
  pwm.setPWM(1, 0, pulseWidth(90));
  pwm.setPWM(2, 0, pulseWidth(90));
  pwm.setPWM(4, 0, pulseWidth(90));
  pwm.setPWM(8, 0, pulseWidth(90));
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  webSocket.begin(serverAddress, serverPort, path);
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  unsigned long currentMillis = millis();
  webSocket.loop();

  if (currentMillis - previousRequestTime >= requestInterval) {
    if (webSocket.isConnected()) {
      int distance = get_distance(Trig_Sensor_2, Echo_Sensor_2);
      Serial.println(distance);
      if (distance > 0 && distance < 50) {
        pwm.setPWM(1, 0, pulseWidth(0));
        delay(2000);
        webSocket.sendTXT("Activate Cam");
        pwm.setPWM(1, 0, pulseWidth(90));
      }
      previousRequestTime = currentMillis;
    } else {
      Serial.println("WebSocket not connected");
    }
  }
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("Disconnected");
      break;
    case WStype_CONNECTED:
      Serial.println("Connected to server");
      break;
    case WStype_TEXT:
      Serial.println("Received response: ");
      Serial.println((char *)payload);
      handleWebSocketMessage((char *)payload);
      break;
  }
}

void handleWebSocketMessage(char *message) {
  String trashType = String(message);

  if (trashType.equals("Paper")) {
    Serial.println("SERVO: Paper");
    pwm.setPWM(2, 0, pulseWidth(160)); // quay trái
    delay(1000);
    pwm.setPWM(2, 0, pulseWidth(90));
    delay(1000);
    pwm.setPWM(8, 0, pulseWidth(160)); // quay trái
    delay(1000);
    pwm.setPWM(8, 0, pulseWidth(90));
    delay(1000);
  } else if (trashType.equals("Metal")) {
    Serial.println("SERVO: Metal");
    pwm.setPWM(2, 0, pulseWidth(160)); // quay trái
    delay(1000);
    pwm.setPWM(2, 0, pulseWidth(90));
    delay(1000);
    pwm.setPWM(8, 0, pulseWidth(20)); // quay trái
    delay(1000);
    pwm.setPWM(8, 0, pulseWidth(90));
    delay(1000);


  } else if (trashType.equals("Plastic")) {
    Serial.println("SERVO: Plastic");
    pwm.setPWM(2, 0, pulseWidth(20)); // quay phải
    delay(1000);
    pwm.setPWM(2, 0, pulseWidth(90));
    delay(1000);
    pwm.setPWM(4, 0, pulseWidth(160)); // quay trái
    delay(1000);
    pwm.setPWM(4, 0, pulseWidth(90));
    delay(1000);

  } else if (trashType.equals("Cardboard")) {
    Serial.println("SERVO: Cardboard");
    pwm.setPWM(2, 0, pulseWidth(20)); // quay phải
    delay(1000);
    pwm.setPWM(2, 0, pulseWidth(90));
    delay(1000);
    pwm.setPWM(4, 0, pulseWidth(20)); // quay trái
    delay(1000);
    pwm.setPWM(4, 0, pulseWidth(90));
    delay(1000);

  } else {
    Serial.println("Unknown trash type");
  }
}
