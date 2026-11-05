
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

const int LED_PIN = 5;
const int BUTTON_PIN = 0;

const char* WIFI_NAME = "KeepTheLight-ESP32";
const char* WIFI_PASSWORD = "survive123";

WebServer server(80);

enum GameState {
  READY,
  PLAYING,
  GAME_OVER
};

GameState gameState = READY;

int energy = 60;
int buttonPresses = 0;

unsigned long startTime = 0;
unsigned long lastDrain = 0;
unsigned long finalTime = 0;

bool lastReading = HIGH;
bool stableButton = HIGH;
unsigned long debounceTime = 0;

const unsigned long DEBOUNCE_MS = 50;

const char webpage[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Keep the Light Alive</title>

<style>
* {
  box-sizing: border-box;
}

body {
  margin: 0;
  min-height: 100vh;
  background: radial-gradient(
    ellipse at center,
    #1b2040 0%,
    #090b16 65%,
    #05060c 100%
  );
  color: #f4f2ff;
  font-family: Arial, sans-serif;
  display: flex;
  justify-content: center;
  align-items: center;
  text-align: center;
  padding: 24px;
}

.container {
  width: 100%;
  max-width: 500px;
}

.subtitle {
  font-size: 11px;
  letter-spacing: 4px;
  color: #a6a8c4;
}

h1 {
  font-size: clamp(28px, 6vw, 44px);
  letter-spacing: -1px;
  margin: 22px 0 12px;
}

.description {
  font-size: 14px;
  color: #a1a3b9;
  line-height: 1.7;
}

.orb-area {
  height: 270px;
  display: flex;
  align-items: center;
  justify-content: center;
}

.orb {
  width: 150px;
  height: 150px;
  border-radius: 50%;
  background: radial-gradient(
    circle at 35% 30%,
    #ffffff,
    #c3e8ff 18%,
    #879cf6 48%,
    #5546a6 80%
  );
  box-shadow:
    0 0 40px #879cff88,
    0 0 100px #6677ff66,
    0 0 160px #5555ff33;
  animation: breathe 3s ease-in-out infinite;
  transition: opacity 0.4s;
}

@keyframes breathe {
  0%, 100% {
    transform: scale(0.92);
  }
  50% {
    transform: scale(1.08);
  }
}

.orb.dead {
  animation: none;
  filter: grayscale(1);
  opacity: 0.12 !important;
}

.status {
  font-size: 12px;
  letter-spacing: 3px;
  color: #bdc7ff;
  margin-bottom: 28px;
}

.energy-header {
  display: flex;
  justify-content: space-between;
  color: #bfc2d6;
  font-size: 12px;
  margin-bottom: 12px;
}

.energy-bar {
  width: 100%;
  height: 9px;
  background: #292c43;
  border-radius: 20px;
  overflow: hidden;
}

.energy-fill {
  width: 60%;
  height: 100%;
  background: linear-gradient(
    90deg,
    #7287ff,
    #c2aaff
  );
  border-radius: 20px;
  transition: width 0.3s;
}

.stats {
  display: flex;
  gap: 12px;
  margin-top: 24px;
}

.stat {
  flex: 1;
  background: #ffffff08;
  border: 1px solid #ffffff15;
  border-radius: 15px;
  padding: 20px 10px;
}

.number {
  font-size: 28px;
  font-weight: bold;
  margin-bottom: 8px;
}

.label {
  font-size: 10px;
  letter-spacing: 1px;
  color: #9296af;
}

button {
  margin-top: 30px;
  padding: 16px 40px;
  background: #c5caff;
  color: #141629;
  border: none;
  border-radius: 30px;
  font-weight: bold;
  cursor: pointer;
}

button:hover {
  background: #e0e3ff;
}

.instructions {
  margin-top: 25px;
  font-size: 12px;
  line-height: 1.8;
  color: #8b90aa;
}

.footer {
  margin-top: 40px;
  font-size: 10px;
  letter-spacing: 2px;
  color: #595e7d;
}
</style>
</head>

<body>

<div class="container">

  <div class="subtitle">
    A PHYSICAL-DIGITAL EXPERIENCE
  </div>

  <h1>KEEP THE LIGHT ALIVE</h1>

  <p class="description">
    Every light needs someone to protect it.<br>
    How long can you keep yours alive?
  </p>

  <div class="orb-area">
    <div class="orb" id="orb"></div>
  </div>

  <div class="status" id="status">
    READY TO BEGIN
  </div>

  <div class="energy-header">
    <span>LIGHT ENERGY</span>
    <span id="energyText">60%</span>
  </div>

  <div class="energy-bar">
    <div class="energy-fill" id="energyFill"></div>
  </div>

  <div class="stats">

    <div class="stat">
      <div class="number" id="time">0</div>
      <div class="label">SECONDS ALIVE</div>
    </div>

    <div class="stat">
      <div class="number" id="presses">0</div>
      <div class="label">BUTTON PRESSES</div>
    </div>

  </div>

  <button id="startButton" onclick="startGame()">
    START GAME
  </button>

  <p class="instructions">
    Press the physical BOOT button on your ESP32<br>
    to restore the light's energy.<br>
    Each press restores 12 energy points.
  </p>

  <div class="footer">
    ESP32 INTERACTIVE SURVIVAL GAME
  </div>

</div>

<script>

async function startGame() {
  try {
    const response = await fetch("/start");

    if (!response.ok) {
      throw new Error("Failed to start game");
    }

    updateGame();
  } catch (error) {
    document.getElementById("status").textContent =
      "CONNECTION ERROR";
  }
}

async function updateGame() {
  try {
    const response = await fetch("/state", {
      cache: "no-store"
    });

    if (!response.ok) {
      throw new Error("Connection failed");
    }

    const data = await response.json();

    document.getElementById("energyText").textContent =
      data.energy + "%";

    document.getElementById("energyFill").style.width =
      data.energy + "%";

    document.getElementById("time").textContent =
      data.time;

    document.getElementById("presses").textContent =
      data.presses;

    const orb = document.getElementById("orb");
    const status = document.getElementById("status");
    const button = document.getElementById("startButton");

    if (data.state === "READY") {
      status.textContent = "READY TO BEGIN";
      button.textContent = "START GAME";
      button.style.display = "inline-block";
      orb.classList.remove("dead");
      orb.style.opacity = "1";
    }

    if (data.state === "PLAYING") {
      status.textContent = "THE LIGHT IS ALIVE";
      button.style.display = "none";
      orb.classList.remove("dead");

      orb.style.opacity =
        String(0.25 + data.energy / 135);
    }

    if (data.state === "GAME_OVER") {
      status.textContent = "THE LIGHT HAS FADED";
      button.textContent = "TRY AGAIN";
      button.style.display = "inline-block";
      orb.classList.add("dead");
    }

  } catch (error) {
    document.getElementById("status").textContent =
      "CONNECTION LOST";
  }
}

updateGame();
setInterval(updateGame, 300);

</script>
</body>
</html>
)HTML";

String stateName() {
  if (gameState == PLAYING) return "PLAYING";
  if (gameState == GAME_OVER) return "GAME_OVER";
  return "READY";
}

void startGameLogic() {
  gameState = PLAYING;
  energy = 60;
  buttonPresses = 0;

  startTime = millis();
  lastDrain = millis();
  finalTime = 0;

  digitalWrite(LED_PIN, HIGH);
}

void endGame() {
  gameState = GAME_OVER;
  energy = 0;
  finalTime = (millis() - startTime) / 1000;

  digitalWrite(LED_PIN, LOW);
}

void pressButton() {
  if (gameState != PLAYING) return;

  energy += 12;

  if (energy > 100) {
    energy = 100;
  }

  buttonPresses++;
}

void sendState() {
  unsigned long seconds = 0;

  if (gameState == PLAYING) {
    seconds = (millis() - startTime) / 1000;
  }

  if (gameState == GAME_OVER) {
    seconds = finalTime;
  }

  String json = "{";
  json += "\"state\":\"" + stateName() + "\",";
  json += "\"energy\":" + String(energy) + ",";
  json += "\"time\":" + String(seconds) + ",";
  json += "\"presses\":" + String(buttonPresses);
  json += "}";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(LED_PIN, LOW);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_NAME, WIFI_PASSWORD);

  Serial.println("Wi-Fi started");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", webpage);
  });

  server.on("/state", HTTP_GET, sendState);

  server.on("/start", HTTP_GET, []() {
    startGameLogic();
    server.send(200, "text/plain", "Started");
  });

  server.begin();
}

void loop() {
  server.handleClient();

  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastReading) {
    debounceTime = millis();
  }

  if (millis() - debounceTime > DEBOUNCE_MS) {
    if (reading != stableButton) {
      stableButton = reading;

      if (stableButton == LOW) {
        pressButton();
      }
    }
  }

  lastReading = reading;

  if (gameState == PLAYING) {
    unsigned long now = millis();

    while (now - lastDrain >= 1000 &&
           gameState == PLAYING) {
      lastDrain += 1000;
      energy -= 3;

      if (energy <= 0) {
        endGame();
      }
    }
  }
}
