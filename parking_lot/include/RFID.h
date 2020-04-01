#pragma once
#include "MFRC522.h"
#include <string>

// RFID Functions
void getCardID(std::string &currentCardID);
void printCardID(std::string &cardID);
bool checkList(std::string &cardID);