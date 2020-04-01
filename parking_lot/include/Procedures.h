#pragma once

// LED Functions
void setLED(bool red, bool green, bool blue);

// Procedures
void openProcedure(int &gate_distance_cm);
void intruderProcedure();
void closeProcedure();
void fish_ISR();
void register_ISR();
