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

/**
	\brief Parse a data notification that come in

	\description Parses a data notification from packet form to a struct where the data can be more easily
								accessed and manipulated. ALL data is copied in big endian format

	\params data - array of bytes recieved

	\retval bool - CMD_SUCCESS/CMD_FAIL
**/
bool Smartmesh_API::parseDataNotification(data_notif *notif, uint8_t *data){
	uint8_t *dataOriginal = data;
	data+=2;
	if(*data != NOTIF)
		return CMD_FAIL;
	
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

// Parses the get network info command response
bool Smartmesh_API::parseNetworkInfo(network_info *info, uint8_t *data){
	uint8_t *dataOriginal = data;
	if(*(data+2) != 0x40)// Not a data packet
		return CMD_FAIL;
	
	// Copy data over to data struct
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
	data += 48;
	uint16_t temp = verifyPacket(START_CHECKSUM, dataOriginal, 43+4);
	uint16_t fcs_temp = *(data - 1) | *(data - 2) << 8;
	if(temp != fcs_temp)
		return CMD_FAIL;
	
	return CMD_SUCCESS;
}

// Clear network statistics
bool Smartmesh_API::clearStatistics(){
	init_packet(0, CLEAR_STAT);
	checksumData(START_CHECKSUM, send_data, 4);
	sendUart->send_array(send_data, 8);
	
	return CMD_SUCCESS;
}

// Get Hardware Information
bool Smartmesh_API::getHardwareInfo(){
	init_packet(0, GET_SYS_INFO);
	checksumData(START_CHECKSUM, send_data, 4);
	sendUart->send_array(send_data, 8);
	
	return CMD_SUCCESS;
}

// Parse hardware information
bool Smartmesh_API::parseHardwareInfo(system_info *info, uint8_t *data){
	if(*(data+5) != RC_OK)
		return CMD_FAIL;
	
	memcpy((uint8_t*)&info->rc_code, data+=5, sizeof(info->rc_code));
	memcpy((uint8_t*)&info->mac_address, data+=sizeof(info->rc_code), sizeof(info->mac_address));
	memcpy((uint8_t*)&info->hw_model, data+=sizeof(info->mac_address), sizeof(info->hw_model));
	memcpy((uint8_t*)&info->hw_rev, data+=sizeof(info->hw_model), sizeof(info->hw_rev));
	memcpy((uint8_t*)&info->sw_major, data+=sizeof(info->hw_rev), sizeof(info->sw_major));
	memcpy((uint8_t*)&info->sw_minor, data+=sizeof(info->sw_major), sizeof(info->sw_minor));
	memcpy((uint8_t*)&info->sw_patch, data+=sizeof(info->sw_minor), sizeof(info->sw_patch));
	memcpy((uint8_t*)&info->sw_build, data+=sizeof(info->sw_patch), sizeof(info->sw_build));
	
	return CMD_SUCCESS;
}

// Setup network manager
bool Smartmesh_API::setNetworkConfig(uint8_t *network_id){
	init_packet(21, SET_NTWK_CONFIG);
	memcpy(send_data + 5, network_id, 2);
	send_data[7] = 8;// Tx power(8dbm)
	send_data[8] = 1;// Frame profile
	send_data[9] = 0;// Max motes
	send_data[10] = 10;
	send_data[11] = 23;// Base bandwidth(9s between each packet default)
	send_data[12] = 28;
	send_data[13] = 1;// Downstream frame multiplier
	send_data[14] = 3;// Number of parents for each mote
	send_data[15] = 0;// Both energy and carrier detect if network has congestion
	send_data[16] = 0x7F;// Use all channels
	send_data[17] = 0xFF;
	send_data[18] = 0x1;// Autostart network
	send_data[19] = 0;// Reserved
	send_data[20] = 0;// Backbone frame mode
	send_data[21] = 1;// Backbone size
	send_data[22] = 0;// Radiotest is off
	send_data[23] = 1;// Bandwidth over-provisioning multiplier
	send_data[24] = 0x2C;
	send_data[25] = 0xFF;// Use all channels
	checksumData(START_CHECKSUM, send_data, 25);
	sendUart->send_array(send_data, 29);// Send the packet to the network manager
	
	return CMD_SUCCESS;
}

// Gets the current network configuration
bool Smartmesh_API::getNetworkConfig(){
	init_packet(0, GET_NET_CONFIG);
	
	checksumData(START_CHECKSUM, send_data, 4);
	sendUart->send_array(send_data, 8);
	
	return CMD_SUCCESS;
}

// Set the join key of the network manager
bool Smartmesh_API::setJoinKey(uint8_t *jkey){
	init_packet(16, SET_COMMON_JKEY);
	memcpy(send_data+5, jkey, 16);
	checksumData(START_CHECKSUM, send_data, 20);
	sendUart->send_array(send_data, 24);

	return CMD_SUCCESS;
}

// Gets the mote configuration from its mote id
bool Smartmesh_API::getMoteConfigFromMoteId(uint16_t moteid){
	init_packet(2, GET_MOTE_CFG_BY_ID);
	send_data[5] = (moteid >> 8)&0xFF;
	send_data[6] = moteid&0xFF;
	
	checksumData(START_CHECKSUM, send_data, 6);
	sendUart->send_array(send_data, 10);
	
	return CMD_SUCCESS;
}

// Get mote information from its mac address
bool Smartmesh_API::getMoteInfo(uint8_t *mac_address){
	init_packet(8, GET_MOTE_INFO);
	memcpy(send_data+5, mac_address, 8);
	
	checksumData(START_CHECKSUM, send_data, 12);
	sendUart->send_array(send_data, 16);
	
	return CMD_SUCCESS;
}

// Parses the get mote info command
bool Smartmesh_API::parseGetMoteInfo(mote_info *info, uint8_t *data){
	if(*(data+5) != RC_OK)
		return CMD_FAIL;
	
	// Copy the data over to the struct
	memcpy((uint8_t*)&info->rc_code, data+=5, sizeof(info->rc_code));
	memcpy((uint8_t*)&info->mac_address, data+=sizeof(info->rc_code), sizeof(info->mac_address));
	memcpy((uint8_t*)&info->state, data+=sizeof(info->mac_address), sizeof(info->state));
	memcpy((uint8_t*)&info->numNbrs, data+=sizeof(info->state), sizeof(info->numNbrs));
	memcpy((uint8_t*)&info->numGoodNbrs, data+=sizeof(info->numNbrs), sizeof(info->numGoodNbrs));
	memcpy((uint8_t*)&info->requestedBw, data+=sizeof(info->numGoodNbrs), sizeof(info->requestedBw));
	memcpy((uint8_t*)&info->totalNeededBw, data+=sizeof(info->requestedBw), sizeof(info->totalNeededBw));
	memcpy((uint8_t*)&info->assignedBw, data+=sizeof(info->totalNeededBw), sizeof(info->assignedBw));
	memcpy((uint8_t*)&info->packetsReceived, data+=sizeof(info->assignedBw), sizeof(info->packetsReceived));
	memcpy((uint8_t*)&info->packetsLost, data+=sizeof(info->packetsReceived), sizeof(info->packetsLost));
	memcpy((uint8_t*)&info->avgLatency, data+=sizeof(info->packetsLost), sizeof(info->avgLatency));
	memcpy((uint8_t*)&info->stateTime, data+=sizeof(info->avgLatency), sizeof(info->stateTime));
	memcpy((uint8_t*)&info->numJoins, data+=sizeof(info->stateTime), sizeof(info->numJoins));
	memcpy((uint8_t*)&info->hopDepth, data+=sizeof(info->numJoins), sizeof(info->hopDepth));
	
	return CMD_SUCCESS;
}

bool Smartmesh_API::resetManager(){
	init_packet(9, RESET_CMD);
	memset(send_data + 5, 0, 9);
	
	checksumData(START_CHECKSUM, send_data, 13);
	sendUart->send_array(send_data, 17);
}

bool Smartmesh_API::parseNetworkConfig(network_config *config, uint8_t *data){
	if(data[5] != RC_OK)
		return CMD_FAIL;
	
	// Copy the data over to the struct
	memcpy((uint8_t*)&config->rc_code, data+=5, sizeof(config->rc_code));
	memcpy((uint8_t*)&config->networkId, data+=sizeof(config->rc_code), sizeof(config->networkId));
	memcpy((uint8_t*)&config->apTxPower, data+=sizeof(config->networkId), sizeof(config->apTxPower));
	memcpy((uint8_t*)&config->frameProfile, data+=sizeof(config->apTxPower), sizeof(config->frameProfile));
	memcpy((uint8_t*)&config->maxMotes, data+=sizeof(config->frameProfile), sizeof(config->maxMotes));
	memcpy((uint8_t*)&config->baseBandwidth, data+=sizeof(config->maxMotes), sizeof(config->baseBandwidth));
	memcpy((uint8_t*)&config->downFrameMultVal, data+=sizeof(config->baseBandwidth), sizeof(config->downFrameMultVal));
	memcpy((uint8_t*)&config->numParents, data+=sizeof(config->downFrameMultVal), sizeof(config->numParents));
	memcpy((uint8_t*)&config->ccaMode, data+=sizeof(config->numParents), sizeof(config->ccaMode));
	memcpy((uint8_t*)&config->channelList, data+=sizeof(config->ccaMode), sizeof(config->channelList));
	memcpy((uint8_t*)&config->autoStartNetwork, data+=sizeof(config->channelList), sizeof(config->autoStartNetwork));
	memcpy((uint8_t*)&config->locMode, data+=sizeof(config->autoStartNetwork), sizeof(config->locMode));
	memcpy((uint8_t*)&config->bbMode, data+=sizeof(config->locMode), sizeof(config->bbMode));
	memcpy((uint8_t*)&config->bbSize, data+=sizeof(config->bbMode), sizeof(config->bbSize));
	memcpy((uint8_t*)&config->isRadioTest, data+=sizeof(config->bbSize), sizeof(config->isRadioTest));
	memcpy((uint8_t*)&config->bwMult, data+=sizeof(config->isRadioTest), sizeof(config->bwMult));
	memcpy((uint8_t*)&config->oneChannel, data+=sizeof(config->bwMult), sizeof(config->oneChannel));
	
	return CMD_SUCCESS;
}

bool Smartmesh_API::getMoteConfigFromMac(uint8_t *mac_addr){
	init_packet(9, GET_MOTE_CONFIG);
	memcpy(send_data+5, mac_addr, 8);
	memset(send_data+13, 0, 1);
	
	checksumData(START_CHECKSUM, send_data, 13);
	sendUart->send_array(send_data, 17);
	
	return CMD_SUCCESS;
}