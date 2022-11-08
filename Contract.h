#ifndef __CONTRACT_H
#define __CONTRACT_H

/*
    Contract.h - An implementation for the storage contract developed for SmartmeshIP
    Can be broadened to include general contracts
*/

#include <stdint.h>

// Contract address to send data to
//const uint8_t contract_addr[20] = {};

// Send data to the blockchain
void sendData(uint8_t *data);

// Call the store data contract method
void storeData(uint8_t *macAddr, uint8_t *data);

// Get the latest data for a mac address
uint8_t *getLatestData(uint8_t *macAddr);

// Get certain data by timestamp
uint8_t *getDataByTimestamp(uint8_t *timestamp, uint8_t *macAddr);

// Get certain data by the point number it was added
uint8_t *getDataByPoint(uint8_t *point, uint8_t *macAddr);

#endif