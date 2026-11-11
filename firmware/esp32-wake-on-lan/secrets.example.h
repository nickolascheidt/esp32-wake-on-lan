#pragma once

// Copy this file to secrets.h and fill in your own values.
// secrets.h is listed in .gitignore and must never be committed.

// ---------- Wi-Fi (2.4 GHz only) ----------
#define WIFI_SSID "your-network"
#define WIFI_PASS "your-password"

// ---------- Adafruit IO ----------
// Username and Active Key: io.adafruit.com -> key icon ("My Key")
#define AIO_USERNAME "your-adafruit-io-username"
#define AIO_KEY      "your-adafruit-io-key"

// Feed keys (create both feeds in Adafruit IO first)
#define AIO_FEED_COMMAND "pc-command"
#define AIO_FEED_STATUS  "pc-status"

// Value sent to the command feed that wakes the PC (compared in lowercase)
#define WAKE_COMMAND "wake"

// ---------- Target PC ----------
// MAC address of the PC's wired network card (ipconfig /all -> "Physical Address")
#define PC_MAC_ADDRESS {0x00, 0x11, 0x22, 0x33, 0x44, 0x55}

// PC's local IP, used to check whether it is online (reserve it on your router)
#define PC_IP_ADDRESS 192, 168, 1, 100

// Broadcast address of your local network (usually x.x.x.255)
#define BROADCAST_ADDRESS 192, 168, 1, 255
