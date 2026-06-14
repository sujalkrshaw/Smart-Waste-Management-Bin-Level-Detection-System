#include <WiFi.h>
#include <PubSubClient.h>

// ===============================
// HC-SR04 Pins
// ===============================
#define TRIG_PIN 5
#define ECHO_PIN 18

// ===============================
// LED Pins
// ===============================
#define GREEN_LED 26
#define YELLOW_LED 27
#define RED_LED 14

// ===============================
// Buzzer Pin
// ===============================
#define BUZZER 25

// ===============================
// Bin Configuration
// ===============================
const float BIN_HEIGHT = 30.0;

// ===============================
// WiFi Credentials
// ===============================
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ===============================
// MQTT Configuration
// ===============================
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;

const char* mqtt_topic =
"smartwaste/bin1/data";

// ===============================
// MQTT Objects
// ===============================
WiFiClient espClient;
PubSubClient client(espClient);

// ===============================
// Ultrasonic Function
// ===============================
float measureDistance() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration =
      pulseIn(ECHO_PIN, HIGH);

  float distance =
      duration * 0.034 / 2;

  return distance;
}

// ===============================
// WiFi Connection
// ===============================
void connectWiFi() {

  Serial.println();
  Serial.println("Connecting WiFi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// ===============================
// MQTT Reconnect
// ===============================
void reconnectMQTT() {

  while (!client.connected()) {

    Serial.print("Connecting MQTT...");

    String clientId =
        "SmartBinESP32-";

    clientId += String(random(1000));

    if (client.connect(clientId.c_str())) {

      Serial.println(" Connected");

    } else {

      Serial.print(" Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retry in 5 sec");

      delay(5000);
    }
  }
}

// ===============================
// Setup
// ===============================
void setup() {

  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  pinMode(BUZZER, OUTPUT);

  Serial.println();
  Serial.println("================================");
  Serial.println("Smart Waste Management System");
  Serial.println("================================");

  connectWiFi();

  client.setServer(
      mqtt_server,
      mqtt_port
  );
}

// ===============================
// Main Loop
// ===============================
void loop() {

  if (!client.connected()) {

    reconnectMQTT();
  }

  client.loop();

  float distance =
      measureDistance();

  // Sensor Validation
  if (distance <= 0 ||
      distance > BIN_HEIGHT) {

    distance = BIN_HEIGHT;
  }

  float fillPercentage =
      ((BIN_HEIGHT - distance)
      / BIN_HEIGHT) * 100.0;

  fillPercentage =
      constrain(
          fillPercentage,
          0,
          100
      );

  String status;
  String alert;

  // ==========================
  // Empty
  // ==========================
  if (fillPercentage < 40) {

    status = "EMPTY";
    alert = "NO";

    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);

    noTone(BUZZER);
  }

  // ==========================
  // Half Full
  // ==========================
  else if (fillPercentage < 80) {

    status = "HALF_FULL";
    alert = "NO";

    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(RED_LED, LOW);

    noTone(BUZZER);
  }

  // ==========================
  // Full
  // ==========================
  else {

    status = "FULL";
    alert = "YES";

    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, HIGH);

    tone(BUZZER, 1000);
  }

  // ==========================
  // JSON Payload
  // ==========================
  String payload = "{";
  payload += "\"distance\":";
  payload += String(distance, 2);
  payload += ",";

  payload += "\"fill\":";
  payload += String(fillPercentage, 2);
  payload += ",";

  payload += "\"status\":\"";
  payload += status;
  payload += "\",";

  payload += "\"alert\":\"";
  payload += alert;
  payload += "\"}";

  // Publish MQTT
  client.publish(
      mqtt_topic,
      payload.c_str()
  );

  // ==========================
  // Serial Output
  // ==========================
  Serial.println();
  Serial.println("----------------------------");

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  Serial.print("Fill Level: ");
  Serial.print(fillPercentage);
  Serial.println(" %");

  Serial.print("Status: ");
  Serial.println(status);

  Serial.print("Alert: ");
  Serial.println(alert);

  Serial.print("MQTT Topic: ");
  Serial.println(mqtt_topic);

  Serial.print("Payload: ");
  Serial.println(payload);

  Serial.println("----------------------------");

  delay(3000);
}