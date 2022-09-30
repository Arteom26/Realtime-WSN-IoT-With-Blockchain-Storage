#ifndef __API_H
#define __API_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "SERCOM.h"

#define API_APP_VER 4
#define SW_VER 143

// Command Defines
#define USR_HELLO 0x01
#define MGR_HELLO_RESPONSE 0x02
#define MGR_HELLO 0x03
#define NOTIF 0x14
#define RESET_CMD 0x15
#define SUBSCRIBE 0x16
#define GET_TIME 0x17
#define SET_NTWK_CONFIG 0x1A
#define CLEAR_STAT 0x1F
#define EXCH_MOTE_JKEY 0x21
#define EXCH_NETID 0x22
#define RADIO_TEST_TX 0x23
#define RADIO_TEST_RX 0x25
#define GET_RADIO_TEST_STAT 0x26
#define SET_ACL_ENTRY 0x27
#define GET_NEXT_ACL_ENTRY 0x28
#define DELETE_ACL_ENTRY 0x29
#define PING_MOTE 0x2A
#define GET_LOG 0x2B
#define SEND_DATA 0x2C
#define START_NETWORK 0x2D
#define GET_SYS_INFO 0x2E
#define GET_MOTE_CONFIG 0x2F
#define GET_PATH_INFO 0x30
#define GET_NEXT_PATH_INFO 0x31
#define SET_ADVERTISING 0x32
#define SETz_DOWNSTREAM_FRAME 0x33
#define GET_MANAGER_STAT 0x35
#define SET_TIME 0x36
#define GET_LICENSE 0x37
#define SET_LICENSE 0x38
#define SET_CLI_USER 0x3A
#define SEND_IP 0x3B
#define RESTORE_FACTORY 0x3D
#define GET_MOTE_INFO 0x3E
#define GET_NET_CONFIG 0x3F
#define GET_NETWORK_INFO 0x40
#define GET_MOTE_CFG_BY_ID 0x41
#define SET_COMMON_JKEY 0x42
#define GET_IP_CONFIG 0x43
#define SET_IP_CONFIG 0x44
#define DELETE_MOTE 0x45
#define GET_MOTE_LINKS 0x46

// Notification Types
#define NOTIF_EVENT 0x1
#define NOTIF_LOG 0x2
#define NOTIF_DATA 0x4
#define NOTIF_IPDATA 0x5
#define NOTIF_HEALTH 0x6

// Subscription filters
#define SUB_EVENT 0x02
#define SUB_LOG 0x04
#define SUB_DATA 0x10
#define SUB_IPDATA 0x20
#define SUB_HEALTH 0x40

// Event Types
#define MOTE_RESET 0x00
#define NETWQRK_RESET 0x01
#define COMMAND_FINISHED 0x02
#define MOTE_JOIN 0x03
#define MOTE_OPERATIONAL 0x04
#define MOTE_LOST 0x5
#define NETWORK_TIME 0x06
#define PING_RESPONSE 0x07
#define PATH_CREATE 0x0A
#define PATH_DELETE 0x0B
#define PACKET_SENT 0x0C
#define MOTE_CREATE 0x0D
#define MOTE_DELETE 0x0E
#define JOIN_FAILED 0x0F
#define INVALID_MIC 0x20

// Response Codes
#define RC_OK 0x00
#define RC_INVALID_COMMAND 0x01
#define RC_INVALID_ARGUMENT 0x02
#define RC_END_OF_LIST 0x0B
#define RC_NO_RESOURCES 0x0C
#define RC_IN_PROGRESS 0x0D
#define RC_NACK 0x0E
#define RC_WRITE_FAIL 0x0F
#define RC_VALIDATION_ERROR 0x10
#define RC_INV_STATE 0x11
#define RC_NOT_FOUND 0x12
#define RC_UNSUPPORTED 0x13

// Packet priority
#define PACKET_PRIORITY_LOW 0x0
#define PACKET_PRIORITY_MED 0x1
#define PACKET_PRIORITY_HIGH 0x2

#define CMD_FAIL 0x0
#define CMD_SUCCESS 0x1

struct command_params{
  int commandType;
  uint8_t error_code;
  uint64_t time;// Since 1970
  uint8_t ipv6_addr[16];// IPV6 address of mote
  uint8_t mac_addr[8];// Mote MAC address
  uint8_t payload[128];// Payload recieved => maximum size is 128 bytes
};

struct send_params{
  uint8_t commandType;
  uint8_t macAddr;
  uint8_t *data;// Optional data to transmit => if sending packet
  uint8_t *payload;// Payload for manager packet
  uint32_t sub_filter;
  uint32_t unpack_filter;
};

// Data notification strcuture
typedef struct{
	uint64_t seconds;
	uint32_t us;
	uint64_t macAddr;
	uint16_t src;
	uint16_t dest;
	uint8_t data[90];
	uint8_t length;
}data_notif;

// Network Information Structure
typedef struct{
	uint8_t rc_code;// 0 = good
	uint16_t num_motes;// Total motes in system
	uint16_t asnSize;
	uint8_t adv_state;
	uint8_t downStreamFrame;
	uint8_t network_reliability;// Percentage
	uint8_t netPathStability;// Percentage
	uint32_t latency;// In ms
	uint8_t netState;
	uint64_t ipv6AddrHigh;
	uint64_t ipv6AddrLow;
	#if SW_VER >= 130// In version 1.3(130) and later
	uint32_t numLostPackets;
	uint64_t numArrivedPackets;
	uint8_t maxHops;
	#endif
}network_info;

// Hardware Information Structure
typedef struct{
	uint8_t rc_code;
	uint64_t mac_address;
	uint8_t hw_model;
	uint8_t hw_rev;
	uint8_t sw_major;
	uint8_t sw_minor;
	uint8_t sw_patch;
	uint16_t sw_build;
}system_info;

#define START_CHECKSUM 0xFFFF

static uint16_t fcstab[256] = {
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
  0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
  0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
  0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
	0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
	0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
	0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
	0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
	0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
	0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
	0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
	0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
	0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
	0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
	0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
	0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
	0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
	0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
	0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
	0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
	0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
	0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
	0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
	0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
	0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
	0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
	0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
	0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
	0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
	0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
	0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
	0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

// Class declaration
class Smartmesh_API{
  public:
    uint8_t send_data[128];
    bool commandRecieved;// Is set whenever data was recieved from manager
  
    Smartmesh_API(UART *uart);
    bool sendCommand(send_params params);
    bool mgr_init();
    bool subscribe(uint32_t sub, uint32_t unpack);
    bool sendData(uint64_t macAddr, uint8_t priority, uint16_t src, uint16_t dst, uint8_t *data, uint8_t length);
    uint8_t packetType(uint8_t *data);
		bool getNetworkInfo(void);
		bool parseDataNotification(data_notif *notif, uint8_t *data);
		bool parseNetworkInfo(network_info *info, uint8_t *data);
		bool clearStatistics(void);
		bool getHardwareInfo(void);
		bool parseHardwareInfo(system_info *info, uint8_t *data);
		bool setNetworkConfig();
    
  private:
		UART *sendUart;// Uart port for where to send data to(a seperate class)
    uint8_t mgrSeqNum;
    uint8_t cliSeqNum;

    void init_packet(uint8_t length, uint8_t command);
    void checksumData(uint16_t fcs, uint8_t *data, uint16_t len);
    uint16_t verifyPacket(uint16_t fcs, uint8_t *data, uint16_t len);
		template <typename T>
		void swapEndianess(T &val);
};

#endif
