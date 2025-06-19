#include <WiFi.h>
#include <WebServer.h>

// Define pins
#define IN1 25
#define IN2 33
#define Motor1 18
#define Motor2 23
#define Motor3 32
#define Motor4 22
#define Motor5 19
#define Motor6 21

const char* ssid = "BrailleBot";
const char* password = "braille123";

WebServer server(80);

// Track direction state
bool directionForward = true;

const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Braille Bot</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      background: #f4f4f4;
      padding-top: 50px;
    }
    h1 {
      color: #333;
    }
    .button-container {
      margin-top: 20px;
    }
    button {
      padding: 12px 24px;
      margin: 10px;
      font-size: 16px;
      border: none;
      border-radius: 6px;
      background-color: #007BFF;
      color: white;
      cursor: pointer;
      transition: background-color 0.3s ease;
    }
    button:hover {
      background-color: #0056b3;
    }
  </style>
</head>
<body>
  <h1>Braille Bot Motor Control</h1>
  <div class="button-container">
    <button onclick="fetch('/motor?m=1')">Motor 1</button>
    <button onclick="fetch('/motor?m=2')">Motor 2</button>
    <button onclick="fetch('/motor?m=3')">Motor 3</button>
    <button onclick="fetch('/motor?m=4')">Motor 4</button>
    <button onclick="fetch('/motor?m=5')">Motor 5</button>
    <button onclick="fetch('/motor?m=6')">Motor 6</button>
  </div>
  <div class="button-container">
    <button onclick="fetch('/dir?d=1')">FWD</button>
    <button onclick="fetch('/dir?d=2')">RESET</button>
  </div>
  <div class="button-container">
    <button onclick="fetch('/pattern?a=1')">Pattern A</button>
    <button onclick="fetch('/pattern?b=1')">Pattern B</button>
    <button onclick="fetch('/pattern?c=1')">Pattern C</button>
  </div>
  <div class="button-container">
    <button onclick="fetch('/animate')">Animate</button>
  </div>
</body>
</html>
)rawliteral";

void setMotorState(int motorPin, bool state) {
  digitalWrite(motorPin, state ? HIGH : LOW);
}

void allMotorsOff() {
  digitalWrite(Motor1, LOW);
  digitalWrite(Motor2, LOW);
  digitalWrite(Motor3, LOW);
  digitalWrite(Motor4, LOW);
  digitalWrite(Motor5, LOW);
  digitalWrite(Motor6, LOW);
}

void welcomeAnim() {
  setMotorState(Motor1, true);
  setMotorState(Motor2, true);
  setMotorState(Motor3, true);
  setMotorState(Motor4, true);
  setMotorState(Motor5, true);
  setMotorState(Motor6, true);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  directionForward = true;

  delay(550);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  directionForward = false;
  delay(550);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  directionForward = true;
  delay(550);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  directionForward = false;
  delay(550);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  directionForward = true;

  allMotorsOff();
}

void setup() {
  Serial.begin(9600);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(Motor1, OUTPUT);
  pinMode(Motor2, OUTPUT);
  pinMode(Motor3, OUTPUT);
  pinMode(Motor4, OUTPUT);
  pinMode(Motor5, OUTPUT);
  pinMode(Motor6, OUTPUT);

  welcomeAnim();
  allMotorsOff();

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  directionForward = true;
  WiFi.disconnect(true, true);  // FULL reset of WiFi including saved credentials
  delay(100);

  WiFi.softAPdisconnect(true);  // Disconnect any active AP
  delay(100);

  WiFi.softAP(ssid, password);
  Serial.println("AP Started. IP:");
  Serial.println(WiFi.softAPIP());

  server.on("/", []() {
    server.send(200, "text/html", htmlPage);
  });

  server.on("/motor", []() {
    int m = server.arg("m").toInt();
    int pin = 0;

    if (m == 1) pin = Motor1;
    else if (m == 2) pin = Motor2;
    else if (m == 3) pin = Motor3;
    else if (m == 4) pin = Motor4;
    else if (m == 5) pin = Motor5;
    else if (m == 6) pin = Motor6;

    if (pin != 0) {
      bool current = digitalRead(pin);
      setMotorState(pin, !current);
    }

    server.send(200, "text/plain", "OK");
  });

  server.on("/dir", []() {
    int d = server.arg("d").toInt();
    if (d == 1) {
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      directionForward = true;
    } else if (d == 2) {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      directionForward = false;
      delay(250);
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      directionForward = true;
      allMotorsOff();
    }

    server.send(200, "text/plain", "Direction Set");
  });

  server.on("/pattern", []() {
    if (!directionForward) {
      server.send(200, "text/plain", "Motors disabled in reverse direction");
      return;
    }

    if (server.hasArg("a")) {
      allMotorsOff();
      digitalWrite(Motor1, HIGH);
      server.send(200, "text/plain", "Pattern A set (Motor 1 on)");
    } else if (server.hasArg("b")) {
      allMotorsOff();
      digitalWrite(Motor1, HIGH);
      digitalWrite(Motor3, HIGH);
      server.send(200, "text/plain", "Pattern B set (Motors 1+3 on)");
    } else if (server.hasArg("c")) {
      allMotorsOff();
      digitalWrite(Motor1, HIGH);
      digitalWrite(Motor2, HIGH);
      server.send(200, "text/plain", "Pattern C set (Motors 1+2 on)");
    } else {
      server.send(400, "text/plain", "Invalid pattern");
    }
  });

  server.on("/animate", []() {
    welcomeAnim();
    server.send(200, "text/plain", "Animation Done");
  });

  server.begin();
}

void loop() {
  server.handleClient();
}
