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
			case 0xB:// Network ID recieved
				uint8_t netid[2];
				xQueueReceive(bluetoothData, &netid[0], portMAX_DELAY);
				xQueueReceive(bluetoothData, &netid[1], portMAX_DELAY);
				api.setNetworkConfig(netid);// Setup the network config
				bluetooth._printf("Network ID Set!\n");
				break;
			
			case 0xC:// Setup common join key
				uint8_t jkey[16];
				for(int i = 0;i < 16;i++)
					xQueueReceive(bluetoothData, &jkey[i], portMAX_DELAY);// Get the joinkey
				api.setJoinKey(jkey);
				bluetooth._printf("Join Key Set!\n");
				break;
			
			case 'C':// Get the mote list
				api.getNetworkInfo();
				xSemaphoreTake(getNetworkInfo, portMAX_DELAY);// Wait for data to be ready
				motes = (smartmeshData[6] << 8)|smartmeshData[7];
				bluetooth._printf("Number of Motes: %d", motes);// Print the number of motes to the GUI
				for(int i = 1;i <= motes;i++){// Step 2 => get all the mote mac addresses
					api.getMoteConfigFromMoteId(i);
					xSemaphoreTake(moteConfigWasGotFromID, portMAX_DELAY);
					uint64_t mac_address = ((uint64_t)smartmeshData[6] << 56)|((uint64_t)smartmeshData[7] << 48)|
					((uint64_t)smartmeshData[8] << 40)|((uint64_t)smartmeshData[9] << 32)|(smartmeshData[10] << 24)|
					(smartmeshData[11] << 16)|(smartmeshData[12] << 8)|smartmeshData[13];
					bluetooth._printf("Mac Address: %08X\n", mac_address);
				}
				break;
				
			case 'D':// Reset Statistics
				api.clearStatistics();
				bluetooth._printf("Statistics Cleared!\n");
				break;
			
			case 0xF:// Get the current network information
				api.getNetworkInfo();
				xSemaphoreTake(getNetworkInfo, portMAX_DELAY);// Wait for data to be ready
				network_info info;
				api.parseNetworkInfo(&info, smartmeshData);
				xSemaphoreTake(bluetoothInUse, portMAX_DELAY);
				// Print but REVERSE bytes first
				bluetooth._printf("Motes Connected: %d\n", littleToBigEndian<uint16_t>(info.num_motes));
				bluetooth._printf("Latency: %d\n", littleToBigEndian<uint32_t>(info.latency));
				bluetooth._printf("IPV6: %04X%04X", littleToBigEndian<uint32_t>(info.ipv6AddrHigh&0xFFFFFFFF), littleToBigEndian<uint32_t>((info.ipv6AddrHigh >> 32)&0xFFFFFFFF));
				bluetooth._printf("%04X%04X\n", littleToBigEndian<uint32_t>(info.ipv6AddrLow&0xFFFFFFFF), littleToBigEndian<uint32_t>((info.ipv6AddrLow >> 32)&0xFFFFFFFF));
				xSemaphoreGive(bluetoothInUse);
				break;
			
			case 0x1B:// Get the current network configuration
				api.getNetworkConfig();
				xSemaphoreTake(getNetworkConfig, portMAX_DELAY);
				network_config config;
				xSemaphoreTake(bluetoothInUse, portMAX_DELAY);
				bluetooth._printf("Network ID: %d\n", littleToBigEndian<uint16_t>(info.num_motes));
				xSemaphoreGive(bluetoothInUse);
				break;
			
			case 'G':// Get mote information
				break;
			
			case 'H':// Get mote configuration(from mac address rather than mote id)
				break;
			
			default:// TODO: add improved default handling
				bluetooth._printf("Error! Invalid Command\n");
				break;
		}
	}
}