#ifndef PN532_H
#define PN532_H

#include <Arduino.h>
#include "Adafruit_PN532.h"
#include <Wire.h>
#include <SPI.h>
#include <ArduinoJson.h>


void PN532_Init();
void PN532_Process();

#endif