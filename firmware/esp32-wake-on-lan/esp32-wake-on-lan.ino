/*
 * esp32-wake-on-lan
 *
 * Turns on a PC from anywhere using an ESP32 on the same local network.
 *
 *   iPhone Shortcut --HTTP--> Adafruit IO <--MQTT/TLS-- ESP32 --magic packet--> PC
 *                   <--HTTP--  status feed <--publish--  (pings the PC)
 *
 * - The ESP32 only makes outbound connections, so it works behind CGNAT and
 *   needs no open ports on the router.
 * - Pressing the board's BOOT button also sends the magic packet (handy for
 *   testing the local network part on its own).
 *
 * All personal settings live in secrets.h (copy secrets.example.h).
 */

#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <ESP32Ping.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include "secrets.h"
#include "adafruit_io_ca.h"

// ---------- Hardware ----------
const int BUTTON_PIN = 0;  // on-board BOOT button
const int LED_PIN    = 2;  // on-board blue LED

// ---------- Network targets ----------
const uint8_t   PC_MAC[6] = PC_MAC_ADDRESS;
const IPAddress PC_IP(PC_IP_ADDRESS);
const IPAddress BROADCAST_IP(BROADCAST_ADDRESS);
const uint16_t  WOL_PORT = 9;

// ---------- Timing ----------
const unsigned long PING_INTERVAL_MS      = 15000;   // check the PC every 15 s
const unsigned long HEARTBEAT_INTERVAL_MS = 300000;  // re-publish status every 5 min
const unsigned long KEEPALIVE_INTERVAL_MS = 60000;   // ping the broker every minute
const int           MQTT_MAX_RETRIES      = 5;

// ---------- MQTT ----------
WiFiUDP udp;
WiFiClientSecure tls;
Adafruit_MQTT_Client mqtt(&tls, "io.adafruit.com", 8883, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Subscribe commandFeed(&mqtt, AIO_USERNAME "/feeds/" AIO_FEED_COMMAND);
Adafruit_MQTT_Publish   statusFeed (&mqtt, AIO_USERNAME "/feeds/" AIO_FEED_STATUS);

// ---------- State ----------
bool pcOnline = false;
bool statusPublished = false;
unsigned long lastPingAt = 0;
unsigned long lastHeartbeatAt = 0;
unsigned long lastKeepaliveAt = 0;

// ---------- Helpers ----------
void blinkLed(int times, int intervalMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, LOW);  delay(intervalMs);
    digitalWrite(LED_PIN, HIGH); delay(intervalMs);
  }
}

// ---------- Wake-on-LAN ----------
// A magic packet is 6 bytes of 0xFF followed by the target MAC repeated 16 times.
void sendMagicPacket() {
  uint8_t packet[102];
  memset(packet, 0xFF, 6);
  for (int i = 1; i <= 16; i++) {
    memcpy(packet + i * 6, PC_MAC, 6);
  }

  udp.beginPacket(BROADCAST_IP, WOL_PORT);
  udp.write(packet, sizeof(packet));
  udp.endPacket();

  Serial.println("Magic packet sent");
  blinkLed(3, 100);
}

// ---------- Connections ----------
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));  // blink while connecting
    delay(250);
    Serial.print(".");
  }

  digitalWrite(LED_PIN, HIGH);  // solid = connected
  Serial.printf("\nWi-Fi connected: %s\n", WiFi.localIP().toString().c_str());
}

void connectMQTT() {
  if (mqtt.connected()) return;

  Serial.print("MQTT... ");
  int retriesLeft = MQTT_MAX_RETRIES;
  int8_t result;
  while ((result = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(result));
    mqtt.disconnect();
    delay(5000);
    if (--retriesLeft == 0) ESP.restart();  // still failing: start over
  }

  Serial.println("connected");
  statusPublished = false;  // publish the current status again after reconnecting
}

// ---------- PC status ----------
void checkPcStatus() {
  if (millis() - lastPingAt < PING_INTERVAL_MS) return;
  lastPingAt = millis();

  bool online = Ping.ping(PC_IP, 2);
  bool changed = (online != pcOnline);
  bool heartbeatDue = (millis() - lastHeartbeatAt > HEARTBEAT_INTERVAL_MS);

  if (changed || heartbeatDue || !statusPublished) {
    pcOnline = online;
    statusFeed.publish(online ? "online" : "offline");
    statusPublished = true;
    lastHeartbeatAt = millis();
    Serial.printf("PC is %s\n", online ? "online" : "offline");
  }
}

// ---------- Commands ----------
void handleCommands() {
  Adafruit_MQTT_Subscribe* subscription;
  while ((subscription = mqtt.readSubscription(100))) {
    if (subscription != &commandFeed) continue;

    String command = (char*)commandFeed.lastread;
    command.trim();
    command.toLowerCase();
    Serial.printf("Command received: %s\n", command.c_str());

    if (command == WAKE_COMMAND) sendMagicPacket();
  }
}

void handleButton() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    sendMagicPacket();
    delay(1000);  // simple debounce
  }
}

void keepBrokerAlive() {
  if (millis() - lastKeepaliveAt < KEEPALIVE_INTERVAL_MS) return;
  lastKeepaliveAt = millis();
  if (!mqtt.ping()) mqtt.disconnect();
}

// ---------- Main ----------
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  connectWiFi();
  tls.setCACert(ADAFRUIT_IO_ROOT_CA);  // verify io.adafruit.com's certificate
  mqtt.subscribe(&commandFeed);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  connectMQTT();

  handleCommands();
  handleButton();
  checkPcStatus();
  keepBrokerAlive();
}
