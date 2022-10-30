#include <string>
#include <stdint.h>
#include <sstream>
#include "gsm_usart.h"
#include "bluetooth.h"

#ifndef __GSM_H
#define __GSM_H



//#ifdef AT_COMMANDS_DEBUG



// converts any type to string using a 
// template function to determine types

//#endif


#define AT_HEAD "AT"
#define AT_SUCCESS "OK"
#define AT_ERROR "ERROR"
#define AT_CME_ERROR "+CME ERROR:"


typedef class ATCommands ATCommands;


typedef enum ternary
{
    AT_COMMANDS_SUCCESS = 0,
    AT_COMMANDS_ERROR_BUFFER_FULL,
    AT_COMMANDS_ERROR_SYNTAX
} AT_COMMANDS_ERRORS;

/**
 * @brief Command type enum
 * 
 */

typedef enum
{
    AT_COMMAND_READ,
    AT_COMMAND_WRITE,
    AT_COMMAND_TEST,
    AT_COMMAND_RUN,
		AT_COMMAND_UNKNOWN
} AT_COMMAND_TYPE;

/**
 * @brief at_command_t struct
 *  borrowed from esp-at used to define an AT command 
*/
typedef struct
{
    char *at_cmdName;                  // command name
    bool (*at_runCmd)(ATCommands *);   // RUN command function pointer
    bool (*at_testCmd)(ATCommands *);  // TEST command function pointer
    bool (*at_readCmd)(ATCommands *);  // READ command function pointer
    bool (*at_writeCmd)(ATCommands *); // WRITE command function pointer

} at_command_t;


/* 
 * Look up table for START MARK string
 * 2D char array including NULL char
 */
#define SM_LUT_SIZE 7
static char SM_LUT[7][3] = 
{
    "",             // Default - skipped by search
    "+",            // AT+...
    "#",            // AT#...
    "$",            // AT$...
    "%",            // AT%...
    "\\",           // AT\...
    "&"             // AT&...
};

/* 
 * Look up table for END MARK string
 * ENDMARK represents the command type 
 */
#define EM_LUT_SIZE 6
static char EM_LUT[6][3] =
{
    "",             // Default - skipped by search
    "=?",           // Test
    "?",            // Get
    "=",            // Set
    ":",            // URC - unsolicited result code
    "\r"            // Exec
};




class ATCommands
{
	private: 
		// placeholder for number of commands in the array
		std::uint16_t numberOfCommands;
    const at_command_t *atCommands;

    // input buffers
		std::string bufferString;
		uint16_t bufferSize;
    uint16_t bufferPos; // keeps track of the buffer
    const char *term; // used to check end of string
		
		void writeToBuffer(int data);

    // input validation
    static int isValidCmdChar(const char c);
    static uint8_t hexToChar(const char ch);

    // registers command array
    void registerCommands(const at_command_t *commands, uint32_t size);

    // command parsing
    uint8_t AT_COMMAND_TYPE; // the type of command (see enum declaration)
    //char *params;            // command parameters in the case of a WRITE
    uint16_t tokenPos; // position of token used when splitting parameters
    uint16_t termPos;
    bool parseCommand();                // determines the command and command type
    bool parseParameters(uint16_t pos); // parses parameters in the case of a WRITE command
    void processCommand();              // invokes handler to process command

    // callbacks
    bool (*defaultHandler)(ATCommands *);
    void setDefaultHandler(bool (*function)(ATCommands *));
		
		
		
	public: 
		
    ATCommands();
		std::string serial; // stores incoming serial data
		void serial_write(std::string data);


    // register serial port, commands and buffers
		void begin(std::string serial, const at_command_t *commands, 
			uint32_t size, const uint16_t bufferSize, const char *terminator);

    // command parsing
		std::string command; // the command (eg: +TEST in AT+TEST)

    /**
	 * @brief Checks the Serial port, reads the input buffer and calls a matching command handler.
	 * @return SERIAL_COMMANDS_SUCCESS when successful or SERIAL_COMMANDS_ERROR_XXXX on error.
	 */
    AT_COMMANDS_ERRORS update();

    /**
	 * @brief Clears the buffer, and resets the indexes.
	 */
    void clearBuffer();

    /**
     * @brief retrieves next token
     * 
     * @return String 
     */
		 std::string next();

    /**
     * @brief indicates if there are more tokens in the buffer
     */
    bool hasNext();

    void ok();
		
    void error();
		
};



#endif

