#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Cấu hình thông tin WiFi
const char* ssid = "ktx";
const char* password = "12345678";

// Khởi tạo server trên port 80
ESP8266WebServer server(80);

// Khai báo chân GPIO cho cảm biến siêu âm
const int trigPin1 = D1;
const int echoPin1 = D2;
const int trigPin2 = D3;
const int echoPin2 = D4;
const int trigPin3 = D5;
const int echoPin3 = D6;
const int trigPin4 = D7;
const int echoPin4 = D8;

// Biến toàn cục lưu trữ khoảng cách cảm biến
long distances[4] = {0, 0, 0, 0};
long previousDistances[4] = {0, 0, 0, 0};

// Biến thời gian để kiểm soát tần suất cập nhật cảm biến
unsigned long lastSensorUpdate = 0;
const unsigned long sensorUpdateInterval = 1000; // Cập nhật mỗi giây

// Hàm đọc khoảng cách từ cảm biến siêu âm
long readUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // Giới hạn thời gian chờ là 30ms
  if (duration == 0) {
    return -1; // Trả về -1 nếu không đọc được giá trị
  }
  return duration * 0.034 / 2;
}

// Hàm cập nhật giá trị từ các cảm biến
void updateSensorReadings() {
  for (int i = 0; i < 4; i++) {
    previousDistances[i] = distances[i];
  }

  distances[0] = readUltrasonic(trigPin1, echoPin1);
  distances[1] = readUltrasonic(trigPin2, echoPin2);
  distances[2] = readUltrasonic(trigPin3, echoPin3);
  distances[3] = readUltrasonic(trigPin4, echoPin4);

  for (int i = 0; i < 4; i++) {
    if (distances[i] == -1) {
      distances[i] = previousDistances[i];
    }
  }
}

// Xử lý yêu cầu tại endpoint "/sensors"
void handleSensorReadings() {
  String jsonResponse = "{";
  jsonResponse += "\"paper\": " + String(distances[3]) + ",";
  jsonResponse += "\"metal\": " + String(distances[2]) + ",";
  jsonResponse += "\"plastic\": " + String(distances[1]) + ",";
  jsonResponse += "\"cardboard\": " + String(distances[0]);
  jsonResponse += "}";

  server.send(200, "application/json", jsonResponse);
}

void setup() {
  Serial.begin(115200);

  // Cấu hình chân GPIO
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(trigPin3, OUTPUT);
  pinMode(echoPin3, INPUT);
  pinMode(trigPin4, OUTPUT);
  pinMode(echoPin4, INPUT);

  // Kết nối WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // In ra địa chỉ IP của ESP8266
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Cấu hình endpoint và server
  server.on("/sensors", handleSensorReadings);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Cập nhật giá trị cảm biến nếu đã đến thời gian quy định
  unsigned long currentMillis = millis();
  if (currentMillis - lastSensorUpdate >= sensorUpdateInterval) {
    lastSensorUpdate = currentMillis;
    updateSensorReadings();
  }

  server.handleClient();
}
