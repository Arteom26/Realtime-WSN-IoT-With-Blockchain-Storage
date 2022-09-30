#include "API.h"

using namespace std;

/**
	\brief  Constructor for SMartmesh IP API wrapper class
	
	\description TODO
	
	\param  TODO
	
	\retval None
**/
Smartmesh_API::Smartmesh_API(UART *uart){
	sendUart = uart;
	cliSeqNum = 0;
}

/**
	\brief Runs a RFC 1662 CRC on an array of bytes and stores the checksum according to Smartmesh IP docs

	\description Runs the CRC on the array starting off with the fcs checksum. Once the checksum calculation
								is complete, auto stores the checksum in the two bytes before last flag bit. 

	\param fcs - Current checksum if none use 0xFFFF
				 data - pointer to array of bytes
				 len - length of the array not counting flag bytes

	\retval None
**/
void Smartmesh_API::checksumData(uint16_t fcs, uint8_t *data, uint16_t len){// Pass in 0xFFFF for fcs if not checksum yet
	data++;// Bypass the flag bit
	while(len--)
		fcs = (fcs >> 8) ^ fcstab[(fcs ^ *data++) & 0xFF];// Update the checksum for the current data 
	
	fcs ^= 0xFFFF;// Must XOR the checksum calculation result
	*data = fcs & 0xFF;// MSB
	data++;
	*data = (fcs >> 8) & 0xFF;// LSB
}

uint16_t Smartmesh_API::verifyPacket(uint16_t fcs, uint8_t *data, uint16_t len){
	data++;
	while(len--)
		fcs = (fcs >> 8) ^ fcstab[(fcs ^ *data++) & 0xFF];
	
	fcs ^= 0xFFFF;
	return fcs;
}

/**
	\brief Initializes API packet flags and header

	\decription Sets basic packet information according to the Smartmesh API protocol. Currently,
							 all packets sent will require the network manager to send an 'ack' packet back

	\params length - length of data portion of packet
					command - Command to place within packet header refer to defines/docs

	\retval None
**/
void Smartmesh_API::init_packet(uint8_t length, uint8_t command){
	send_data[0] = 0x7E;
	send_data[1] = 0x02;
	send_data[2] = command;
	send_data[3] = cliSeqNum;
	send_data[4] = length;
	send_data[7+length] = 0x7E;// End of packet transmittion
	cliSeqNum++;// Increment user sending number
}

/**
	\brief Initialize a connection with a Smartmesh IP network manager

	\description Sets up initial communication with the network manager(hello packet). Must be run
								before any other command can be executed. Default API version if 4.0

	\params None

	\retval	CMD_SUCCESS - command excecuted successfully
					CMD_FAIL - command failed => TODO
**/
bool Smartmesh_API::mgr_init(){
	init_packet(3, USR_HELLO);
	send_data[1] = 0x0;
	send_data[5] = API_APP_VER;
	send_data[6] = 0x00;//cliSeqNum - 1;// Must be same as one that was sent first
	send_data[7] = 0;
	checksumData(START_CHECKSUM, send_data, 7);
	sendUart->send_array(send_data, 11);
	return CMD_SUCCESS;
}

/**
	\brief Subscribe to certain notifications for the network manager

	\description This command will subscribe to notifications specified within the sub parameter. The
								unpack filter if set to 1 will not require the application to send an 'ack' packet 
								back to the network manager.

	\params sub - the subscribe filter
					unpack - the unpack filter

	\retval	CMD_SUCCESS - command excecuted successfully
					CMD_FAIL - command failed => TODO: add sub/unpack verification
**/
bool Smartmesh_API::subscribe(uint32_t sub, uint32_t unpack){
	init_packet(8, SUBSCRIBE);
	
	memcpy(send_data + 5, &sub, 4);// Copy bytes over to array
	memcpy(send_data + 9, &unpack, 4);
	checksumData(START_CHECKSUM, send_data, 12);
	sendUart->send_array(send_data, 16);// Send whole payload including flags/checksum
	
	return CMD_SUCCESS;
}

/**
	\brief Send a packet to a mote by mac address(0xFFFFFFFF for all motes)

	\description This function sends a packet to the specified mac address. Destination UDP port must be 
								correct for packet to be recieved

	\params macAddr - mac address of mote 
					priority - either PACKET_PRIORITY_LOW, PACKET_PRIORITY_MED, PACKET_PRIORITY_HIGH
					src - source UDP port 
					dst - destination UDP port
					data - array of data to send to the mote
					length - length of the array that will be sent

	\retval CMD_SUCCESS - command excecuted successfully
					CMD_FAIL - command failed => TODO
**/
bool Smartmesh_API::sendData(uint64_t macAddr, uint8_t priority, uint16_t src, uint16_t dst, uint8_t *data, uint8_t length){
	init_packet(length + 14, SEND_DATA);
	memcpy(send_data + 5, &macAddr, 8);
	send_data[13] = priority;
	memcpy(send_data + 14, &src, 2);
	memcpy(send_data + 16, &dst, 2);
	send_data[18] = 0;
	memcpy(send_data + 19, data, length);
	checksumData(START_CHECKSUM, send_data, length + 18);
	
	return CMD_SUCCESS;
}

/**
	\brief Get type of packet recieved

	\description Returns the packet type from the packet header(3rd byte after flag)

	\params data - array of bytes recieved

	\retval Packet type
**/
uint8_t Smartmesh_API::packetType(uint8_t *data){
	data+=2;
	return *data;
}

bool Smartmesh_API::parseDataNotification(data_notif *notif, uint8_t *data){
	uint8_t *dataOriginal = data;
	data+=2;
	if(*data != NOTIF)
		return CMD_FAIL;
	//memcpy(notif, data+=4, sizeof(*notif)-90);
	
	memcpy(&notif->seconds, data+=4, sizeof(notif->seconds));
	memcpy(&notif->us, data+=sizeof(notif->seconds), sizeof(notif->us));
	memcpy(&notif->macAddr, data+=sizeof(notif->us), sizeof(notif->macAddr));
	memcpy(&notif->src, data+=sizeof(notif->macAddr), sizeof(notif->src));
	memcpy(&notif->dest, data+=sizeof(notif->src), sizeof(notif->dest));
	data+=sizeof(notif->dest);
	int i = 0;
	while(1){
		if(*data == 0x7E){// Verify checksum
			uint16_t temp = verifyPacket(START_CHECKSUM, dataOriginal, i-2);
			uint16_t fcs_temp = *(data - 1) | *(data - 2) << 8;
			notif->length = i-2;
			break;
		}
		if(i > 90)
			return CMD_FAIL;
		
		memset(&notif->data[i], *data, 1);// Copy data byte over
		data++;
		i++;
	}
	
	return CMD_SUCCESS;
}

// Sends the get network info command
bool Smartmesh_API::getNetworkInfo(){
	init_packet(0, GET_NETWORK_INFO);
	checksumData(START_CHECKSUM, send_data, 4);
	sendUart->send_array(send_data, 8);
	
	return CMD_SUCCESS;
}

// Parses the get netowkr info command response
bool Smartmesh_API::parseNetworkInfo(network_info *info, uint8_t *data){
	uint8_t *dataOriginal = data;
	//data+=2;
	if(*(data+2) != 0x40)
		return CMD_FAIL;
	// Copy data over to data struct
	//memcpy(info, data+=5, sizeof(*info));
	//data += 43;
	
	memcpy((uint8_t*)&info->rc_code, data+=5, sizeof(info->rc_code));
	memcpy((uint8_t*)&info->num_motes, data+=sizeof(info->rc_code), sizeof(info->num_motes));
	memcpy((uint8_t*)&info->asnSize, data+=sizeof(info->num_motes), sizeof(info->asnSize));
	memcpy((uint8_t*)&info->adv_state, data+=sizeof(info->asnSize), sizeof(info->adv_state));
	memcpy((uint8_t*)&info->downStreamFrame, data+=sizeof(info->adv_state), sizeof(info->downStreamFrame));
	memcpy((uint8_t*)&info->network_reliability, data+=sizeof(info->downStreamFrame), sizeof(info->network_reliability));
	memcpy((uint8_t*)&info->netPathStability, data+=sizeof(info->network_reliability), sizeof(info->netPathStability));
	memcpy((uint8_t*)&info->latency, data+=sizeof(info->netPathStability), sizeof(info->latency));
	memcpy((uint8_t*)&info->netState, data+=sizeof(info->latency), sizeof(info->netState));
	memcpy((void*)&info->ipv6AddrHigh, data+=sizeof(info->netState), sizeof(info->ipv6AddrHigh));
	memcpy((void*)&info->ipv6AddrLow, data+=sizeof(info->ipv6AddrHigh), sizeof(info->ipv6AddrLow));
	memcpy((uint8_t*)&info->numLostPackets, data+=sizeof(info->ipv6AddrLow), sizeof(info->numLostPackets));
	memcpy((uint8_t*)&info->numArrivedPackets, data+=sizeof(info->numLostPackets), sizeof(info->numArrivedPackets));
	memcpy((uint8_t*)&info->maxHops, data+=sizeof(info->numArrivedPackets), sizeof(info->maxHops));	
	data+=sizeof(info->maxHops);
	// Verify the CRC
	uint16_t temp = verifyPacket(START_CHECKSUM, dataOriginal, 43+4);
	uint16_t fcs_temp = *(data - 1) | *(data - 2) << 8;
	if(temp != fcs_temp)
		return CMD_FAIL;
	
	return CMD_SUCCESS;
}

// Clear network statistics
bool Smartmesh_API::clearStatistics(){
	init_packet(0, CLEAR_STAT);
	
	return CMD_SUCCESS;
}

// Get Hardware Information
bool Smartmesh_API::getHardwareInfo(){
	init_packet(0, GET_SYS_INFO);
	
	return CMD_SUCCESS;
}

// Parse hardware information
bool Smartmesh_API::parseHardwareInfo(system_info *info, uint8_t *data){
	data += 5;
	memcpy(info, data, sizeof(*info));// Copy the data over
	//toBigEndian((uint8_t*)info->mac_address, 8);// Swap bytes around
	//toBigEndian((uint8_t*)info->sw_build, 2);
	
	return CMD_SUCCESS;
}
