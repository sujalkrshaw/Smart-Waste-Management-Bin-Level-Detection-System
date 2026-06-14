# System Architecture

HC-SR04 Ultrasonic Sensor measures the distance between the sensor and waste surface.

ESP32 processes sensor readings and calculates fill percentage.

Data is published using MQTT protocol to the EMQX broker.

Node-RED subscribes to the MQTT topic and updates the dashboard in real time.

When fill percentage exceeds 80%, alerts are generated and collection is recommended.

Flow:

Sensor → ESP32 → MQTT Broker → Node-RED → Dashboard → Alert System
