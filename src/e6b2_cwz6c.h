#ifndef E6B2_CWZ6C_H
#define E6B2_CWZ6C_H

#include <Arduino.h>
#include <Encoder.h>
extern long oldPosition;
extern long encoder_delta;

void e6b2_cwz6c_init();
void e6b2_cwz6c_process();
void display_e6b2();
#endif