#include "GSM.h"


// default constructor 
ATCommands::ATCommands()
{
}


void ATCommands::begin(std::string serial, const at_command_t *commands, 
	uint32_t size, const uint16_t bufferSize, const char *terminator)
{
		terminator = "\r\n";
    this->serial = serial;
    this->term = terminator;
    this->bufferString.reserve(bufferSize);
    this->bufferSize = bufferSize;

    registerCommands(commands, size);
    clearBuffer();
}

/**
 * @brief parseCommand
 * Checks the incoming buffer to ensure it begins with AT and then seeks to
 * determine if the command is of type RUN, TEST, READ or WRITE.  For WRITE
 * commands the buffer is furthered parsed to extract parameters.  Once
 * all actions are complete and the command type is determined the command
 * is compared against the delcared array (atCommands) to find a matching
 * command name.  If a match is found the function is passed to the handler
 * for later execution.
 * @return true 
 * @return false 
 */
bool ATCommands::parseCommand()
{
    uint16_t pos = 2;
    uint8_t type;

    // validate input
    if (this->bufferPos == 0)
    {
        // fall through
        setDefaultHandler(NULL);
        return true;
    }

    if (this->bufferString.find("AT") != 0)
    {
        return false;
    }

    for (uint16_t i = 2; i < this->bufferSize; i++)
    {
        // if null terminator is reached assume a RUN command
        if (this->bufferString[i] == '\0')
        {
            type = AT_COMMAND_RUN;
            break;
        }

        // validate the command
        if (isValidCmdChar(this->bufferString[i]) == 0)
        {
            return false;
        }

        // determine command type
        if (this->bufferString[i] == '=')
        {
            // check if TEST or WRITE command
            if (this->bufferString[i + 1] == '?')
            {
                type = AT_COMMAND_TEST;
                break;
            }
            else
            {
                type = AT_COMMAND_WRITE;
                break;
            }
        }
        if (this->bufferString[i] == '?')
        {
            type = AT_COMMAND_READ;
            break;
        }

        pos++;
    }
    this->command = this->bufferString.substr(2, pos);
    this->AT_COMMAND_TYPE = type;
    int8_t cmdNumber = -1;

    // search for matching command in array
    for (uint8_t i = 0; i < this->numberOfCommands; i++)
    {
        if (command == (atCommands[i].at_cmdName))
        {
            cmdNumber = i;
            break;
        }
    }

    // if no match is found
    if (cmdNumber == -1)
    {
        clearBuffer();
        return false;
    }

    // handle the different commands
    switch (type)
    {
    case AT_COMMAND_RUN:
		{
        setDefaultHandler(this->atCommands[cmdNumber].at_runCmd);
        return true;
		}
    case AT_COMMAND_READ:
		{
        setDefaultHandler(this->atCommands[cmdNumber].at_readCmd);
        return true;
		}
    case AT_COMMAND_TEST:
		{
        setDefaultHandler(this->atCommands[cmdNumber].at_testCmd);
        return true;
		}
    case AT_COMMAND_WRITE:
		{
			if (parseParameters(pos))
			{
					setDefaultHandler(this->atCommands[cmdNumber].at_writeCmd);
					return true;
			}
			else return false;
		}
    default: return false;

    }
}

/**
 * @brief parseParameters
 * Called mainly by parseCommand as an extention to tokenize parameters
 * usually supplied in WRITE (eg AT+COMMAND=param1,param2) commands.  
 * @param pos 
 * @return true 
 * @return false 
 */
bool ATCommands::parseParameters(uint16_t pos)
{

    this->bufferString = this->bufferString.substr(pos + 1, this->bufferSize - pos + 1);

    return true;
}

// checks if more tokens in buffer
bool ATCommands::hasNext()
{
    if (tokenPos < bufferSize)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief next
 * Iterates through the tokenized parameters and 
 * returns NULL when there is nothing more
 * @return char* 
 */
std::string ATCommands::next()
{
    // if boundaries are reached return null
		if (tokenPos >= this->bufferSize)
    {
        tokenPos = bufferSize;
        return "";
    }

    std::string result = "";
    int delimiterIndex = this->bufferString.find(",", tokenPos);

    if (delimiterIndex == -1)
    {
        result = this->bufferString.substr(tokenPos);
        tokenPos = this->bufferSize;
        return result;
    }
    else
    {
        result = this->bufferString.substr(tokenPos, delimiterIndex);
        tokenPos = delimiterIndex + 1;
        return result;
    }
}

/**
 * @brief setDefaultHandler
 * Sets the function handler (callback) on the user's side.
 * @param function 
 */
void ATCommands::setDefaultHandler(bool (*function)(ATCommands *))
{
    this->defaultHandler = function;
}

/**
 * @brief processCommand
 * Invokes the defined handler to process callback
 */
void ATCommands::processCommand()
{
    if (defaultHandler != NULL)
		{
        if ((*defaultHandler)(this))
        {
            this->ok();
        }
        else
        {
            this->error();
        }
		}
}

/**
 * @brief registerCommands
 * Registers the command array for use later in parseCommand
 * @param commands 
 * @param size 
 * @return true 
 * @return false 
 */
void ATCommands::registerCommands(const at_command_t *commands, uint32_t size)
{
    atCommands = commands;
    numberOfCommands = (uint16_t)(size / sizeof(at_command_t));
}

/**
 * @brief clearBuffer
 * resets the buffer and other buffer-related variables
 */
void ATCommands::clearBuffer()
{
    //for (uint16_t i = 0; i < this->buffer->size; i++)
    this->bufferString = "";
    termPos = 0;
    bufferPos = 0;
    tokenPos = 0;
}

/**
 * @brief ok
 * prints OK to terminal
 * 
 */
void ATCommands::ok()
{
    bluetooth._printf("OK");
}

/**
 * @brief error
 * prints ERROR to terminal
 * 
 */
void ATCommands::error()
{
    bluetooth._printf("ERROR");
}

/**
 * @brief isValidCmdChar
 * Validating input commands
 * @param ch 
 * @return int 
 */
int ATCommands::isValidCmdChar(const char ch)
{
    return (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || (ch == '+') || (ch == '#') 
			|| (ch == '$') || (ch == '@') || (ch == '_') || (ch == '=') || (ch == '?');
		
}

/**
 * @brief update
 * Main function called by the loop.  Reads in available charactrers and writes
 * to the buffer.  When the line terminator is found continues to parse and eventually
 * process the command.
 * @return AT_COMMANDS_ERRORS 
 */
AT_COMMANDS_ERRORS ATCommands::update()
{

    for(int i = 0; i < serial.length(); i++)
    {
        int ch = serial[i];

//#ifdef AT_COMMANDS_DEBUG
        bluetooth._printf("Read: bufferSize=");

				bluetooth._printf("%d", this->bufferSize);
        bluetooth._printf(" bufferPos=");
        bluetooth._printf("%d",bufferPos);
        bluetooth._printf(" termPos=");
        bluetooth._printf("%d",termPos);
        if (ch < 32)
        {
            bluetooth._printf(" ch=#");
            bluetooth._printf("%c",ch);
        }
        else
        {
            bluetooth._printf(" ch=[");
            bluetooth._printf("%c",ch);
            bluetooth._printf("]");
        }
//#endif
        if (ch <= 0)
        {
            continue;
        }

        if (bufferPos < this->bufferSize)
        {
            writeToBuffer(ch);
        }

        if (term[termPos] != ch)
        {
            termPos = 0;
            continue;
        }

        if (term[++termPos] == 0)
        {

//#ifdef AT_COMMANDS_DEBUG
            bluetooth._printf("Received: [");
            for (uint32_t n = 0; n < this->bufferSize; n++)
            {
                bluetooth._printf("%c",this->bufferString[n]);
            }
            bluetooth._printf("]");
//#endif

            if (!parseCommand())
            {
                this->error();
                clearBuffer();
                //return;
            }

            processCommand();

            clearBuffer();
        }
    }
		return AT_COMMANDS_SUCCESS;
}

/**
 * @brief writeToBuffer
 * Adds the input to the buffer
 * @param data 
 */
void ATCommands::writeToBuffer(int data)
{
    // EOL not added to the buffer
    if ((char)data != 13 && (char)data != 10)
    {
        this->bufferString += (char)data;
        bufferPos++;
    }
}

/**
 * @brief bluetooth._printf
 * writes data including line terminators
 */
void ATCommands::serial_write(std::string s)
{
	//TODO: write to serial
}


