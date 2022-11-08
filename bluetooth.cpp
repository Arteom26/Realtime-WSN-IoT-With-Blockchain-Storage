#include "bluetooth.h"
#include <string>
#include <algorithm>

template <typename T>
T littleToBigEndian(T value){
	union{
		T val;
		uint8_t bytes[sizeof(T)];
	}src, dst;
	
	src.val = value;
	std::reverse_copy(src.bytes, src.bytes+sizeof(T), dst.bytes);
	
	return dst.val;
}

// This task will parse any data recived thorugh bluetooth
void bluetoothParse(void* unused){
	uint8_t recieved_data;
	
	while(1){
		xQueueReceive(bluetoothData, &recieved_data, portMAX_DELAY);// Wait for data to come in
		uint16_t motes;
		switch(recieved_data){// Main state machine for bluetooth data
			case 'A':// Network ID recieved
				uint8_t netid[2];
				xQueueReceive(bluetoothData, &netid[0], portMAX_DELAY);
				xQueueReceive(bluetoothData, &netid[1], portMAX_DELAY);
				xSemaphoreTake(apiInUse, portMAX_DELAY);
				api.setNetworkConfig(netid);// Setup the network config
				xSemaphoreGive(apiInUse);
				bluetooth._printf("A");
				break;
			
			case 'B':// Setup common join key
				uint8_t jkey[16];
				for(int i = 0;i < 16;i++)
					xQueueReceive(bluetoothData, &jkey[i], portMAX_DELAY);// Get the joinkey
				xSemaphoreTake(apiInUse, portMAX_DELAY);
				api.setJoinKey(jkey);
				xSemaphoreGive(apiInUse);
				bluetooth._printf("B");
				vTaskDelay(3500);// Give time for the network manager to process data
				break;
			
			case 'C':// Get the mote list
				xSemaphoreTake(apiInUse, portMAX_DELAY);
				api.getNetworkInfo();
				xSemaphoreGive(apiInUse);
				xSemaphoreTake(getNetworkInfo, portMAX_DELAY);// Wait for data to be ready
				motes = (smartmeshData[6] << 8)|smartmeshData[7];
				if(motes == 0)// No motes are currently on the network
					break;
				xSemaphoreTake(bluetoothInUse, portMAX_DELAY);
				bluetooth._printf("C");
				for(int i = 1;i <= motes;i++){// Step 2 => get all the mote mac addresses
					xSemaphoreTake(apiInUse, portMAX_DELAY);
					api.getMoteConfigFromMoteId(i+1);
					xSemaphoreGive(apiInUse);
					xSemaphoreTake(moteConfigWasGotFromID, portMAX_DELAY);
					bluetooth._printf("D");
					bluetooth.send_array(smartmeshData+6, 8);
				}
				bluetooth._printf("E");
				xSemaphoreGive(bluetoothInUse);
				break;
				
			case 'X':// Reset Statistics
				xSemaphoreTake(apiInUse, portMAX_DELAY);
				api.clearStatistics();
				xSemaphoreGive(apiInUse);
				//bluetooth._printf("Statistics Cleared!\n");
				break;
			
			case 'G':// Get the current network information
				xSemaphoreTake(apiInUse, portMAX_DELAY);
				api.getNetworkConfig();
				xSemaphoreTake(getNetworkConfig, portMAX_DELAY);
				network_config config;
				api.parseNetworkConfig(&config, smartmeshData);
				xSemaphoreGive(apiInUse);
			
				xSemaphoreTake(bluetoothInUse, portMAX_DELAY);
				bluetooth._printf("J");// Send network/network manager configuration
				bluetooth.send_array((uint8_t*)&config.networkId, 2);// 1. Send the network ID(2 bytes)
				bluetooth.send_array((uint8_t*)&config.apTxPower, 1);// 2. Send the TX power of the manager(1 byte)
			
				xSemaphoreTake(apiInUse, portMAX_DELAY);
				api.getNetworkInfo();
				xSemaphoreTake(getNetworkInfo, portMAX_DELAY);// Wait for data to be ready
				network_info info;
				api.parseNetworkInfo(&info, smartmeshData);
				xSemaphoreGive(apiInUse);	
			
				bluetooth.send_array((uint8_t*)&info.num_motes, 2);// 3. Number of motes currently connected(2 bytes)
				bluetooth.send_array((uint8_t*)&info.ipv6AddrHigh, 16);// 4. Send IPV6 address(16 bytes)
				xSemaphoreGive(bluetoothInUse);
				break;

			case 'I':// Get mote information
				uint8_t mac_addr1[8];
				for(int i = 0;i < 8;i++)
					xQueueReceive(bluetoothData, &mac_addr1[i], portMAX_DELAY);// Get the mac address
				api.getMoteInfo(mac_addr1);
				xSemaphoreTake(getMoteInfo, portMAX_DELAY);
				xSemaphoreTake(bluetoothInUse, portMAX_DELAY);
				bluetooth._printf("F");// Mote information command
				bluetooth.send_array(smartmeshData + 6, 8);// Send mac address
				bluetooth.send_array(smartmeshData + 29, 12);
				xSemaphoreGive(bluetoothInUse);
				break;
			
			case 'E':// Has a connection been established with the network manager
				xSemaphoreTake(bluetoothInUse, portMAX_DELAY);
				if(connectedToManager)
					bluetooth._printf("K1");
				else
					bluetooth._printf("K0");
				xSemaphoreGive(bluetoothInUse);
				break;
			
			case 'H':// Clear statistics command
				xSemaphoreTake(apiInUse, portMAX_DELAY);
				api.clearStatistics();
				xSemaphoreGive(apiInUse);	
				xSemaphoreTake(bluetoothInUse, portMAX_DELAY);
				bluetooth._printf("M");
				xSemaphoreGive(bluetoothInUse);
				break;
			
			default:// TODO: add improved default handling
				bluetooth._printf("X");
				break;
		}
	}
}
