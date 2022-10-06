#include "bluetooth.h"

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
				api.setNetworkConfig((uint16_t)(netid[1]|netid[0]<<8));// Setup the network config
				bluetooth.send_array((uint8_t*)"Connected!\n",10);
				//bluetooth._printf("Connected!\n");
				break;
			
			case 0xC:// Setup common join key
				uint8_t jkey[16];
				for(int i = 0;i < 16;i++)
					xQueueReceive(bluetoothData, &jkey[i], portMAX_DELAY);// Get the joinkey
				api.setJoinKey(jkey);
				bluetooth._printf("Join Key Set!");
				break;
			
			case 'C':// Get the mote list
				api.getNetworkInfo();
				xSemaphoreTake(getNetworkInfo, portMAX_DELAY);// Wait for data to be ready
				motes = (smartmeshData[6] << 8)|smartmeshData[7];
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
			
			case 'E':// Get the current network information
				api.getNetworkInfo();
				xSemaphoreTake(getNetworkInfo, portMAX_DELAY);// Wait for data to be ready
				network_info info;
				api.parseNetworkInfo(&info, smartmeshData);
				xSemaphoreTake(bluetoothInUse, portMAX_DELAY);
				bluetooth._printf("Motes Connected: %d\n", info.num_motes);
				bluetooth._printf("Latency: %d\n", info.latency);
				bluetooth._printf("IPV6: %08X",info.ipv6AddrHigh);
				bluetooth._printf("%08X\n",info.ipv6AddrLow);
				xSemaphoreGive(bluetoothInUse);
				break;
			
			case 'F':// Get the current network configuration
				api.getNetworkConfig();
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
