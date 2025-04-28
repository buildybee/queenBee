#include <Arduino.h>
#include "EspNowRcLink/Transmitter.h"
#include "ppm.h"
#include <ArduinoJson.h>  // Include the ArduinoJson library for parsing JSON

StaticJsonDocument<200> doc;  // Allocate the JSON document for parsing

// uncomment to activate simultor on channel 3
//#define SIM_TX_INTERVAL_MS (20 * 1000) // 20ms => 50Hz

// uncomment to use ppm on pin 13
// #define PPM_PIN 13

// uncomment to print some details to console
//#define PRINT_INFO

#define SERIAL_MODE


EspNowRcLink::Transmitter tx;
#ifdef PPM_PIN
PPM ppm;
#endif

int16_t channelData[8] = {122, 122, 0, 122, 0, 0, 0, 0}; 
static uint32_t lastSent = 0;



int mapJoystickValue(int value) {
  return map(value, 0, 255, 1000, 2000);
}

void setup()
{
  Serial.begin(115200);

#ifdef PPM_PIN
  ppm.begin(PPM_PIN, FALLING);
#endif
  tx.begin(true);
}

void loop()
{
  uint32_t now = micros();
  static int v = 0;
  static uint32_t delta = 0;

#ifdef PPM_PIN
  static uint32_t lastSent = 0;
  uint32_t sentPeriod = (now - lastSent);
  if(ppm.available() || sentPeriod >= 50000ul)
  {
    lastSent = now;
    delta = sentPeriod;
    for(size_t c = 0; c < 8; c++)
    {
      const int16_t val = ppm.get(c);
      tx.setChannel(c, val);
      if(c == 2) v = val;
    }
    tx.commit();
    //Serial.println(v);
  }
#endif

#ifdef SIM_TX_INTERVAL_MS
  static int sim_val = 0;
  static bool sim_dir = true;
  const int sim_rate = 4;

  static uint32_t sendNext = now + SIM_TX_INTERVAL_MS;

  // send rc channels
  if(now >= sendNext)
  {
    v = 1000 + sim_val;
    tx.setChannel(2, v);
    tx.commit();
    sendNext = now + TX_INTERVAL_MS;

    if(sim_dir)
    {
      sim_val += sim_rate;
      if(sim_val >= 1000) sim_dir = false;
    }
    else
    {
      sim_val -= sim_rate;
      if(sim_val <= 0) sim_dir = true;
    }
  }
#endif

#ifdef SERIAL_MODE
  static uint32_t lastSent = 0;
  uint32_t sentPeriod = (now - lastSent);
   if (Serial.available() || sentPeriod >= 50000ul) {
    lastSent = now;
    delta = sentPeriod;
    String jsonString = Serial.readStringUntil('}');  // Read until '}' to capture the JSON string
    jsonString += '}';  // Add the closing brace to complete the JSON format

    DeserializationError error = deserializeJson(doc, jsonString);  // Parse the incoming JSON string
    if (error) {
      Serial.println("Failed to parse JSON");
      return;
    }

    // Extract data for each channel using their keys
    channelData[0] = doc["P"];  // Pitch
    channelData[1] = doc["R"];  // Roll
    channelData[2] = doc["Y"];  // Yaw
    channelData[3] = doc["T"];  // Throttle
    channelData[4] = doc["A1"]; // Auxiliary 1
    channelData[5] = doc["A2"]; // Auxiliary 2
    channelData[6] = doc["A3"]; // Auxiliary 3
    channelData[7] = doc["A4"]; // Auxiliary 4

    // Map and send the values to the transmitter
    for (int i = 0; i < 8; i++) {
      tx.setChannel(i, mapJoystickValue(channelData[i]));
    }
    tx.commit();
  }
 

#endif
tx.update();

#ifdef PRINT_INFO
  static uint32_t printNext = now + 500000;
  if(now >= printNext)
  {
    Serial.printf("V: %d, P: %d, D: %d, C: %d\n", v, ppm.get(2), delta / 100, WiFi.channel());
    printNext = now + 500000;
  }
#else
  (void)v;
  (void)delta;
#endif
}