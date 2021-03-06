/****************************************************************************************************************************
 * ESP8266_AT_Client.cpp - Dead simple web-server.
 * For ESP8266 AT-command running shields
 *
 * ESP8266_AT_WebServer is a library for the ESP8266 AT-command shields to run WebServer
 *
 * Forked and modified from Arduino ESP8266_AT library
 * Built by Khoi Hoang https://github.com/khoih-prog/ESP8266_AT_WebServer
 * Licensed under MIT license
 * Version: 1.0.0
 *
 *
 * Version Modified By   Date      Comments
 * ------- -----------  ---------- -----------
 *  1.0.0   K Hoang      12/02/2020 Initial coding for Arduino Mega, Teensy, etc
 *****************************************************************************************************************************/

#include <inttypes.h>

#include "ESP8266_AT.h"
#include "ESP8266_AT_Client.h"
#include "ESP8266_AT_Server.h"

#include "utility/ESP8266_AT_Drv.h"
#include "utility/ESP8266_AT_Debug.h"


ESP8266_AT_Client::ESP8266_AT_Client() : _sock(255)
{
}

ESP8266_AT_Client::ESP8266_AT_Client(uint8_t sock) : _sock(sock)
{
}


////////////////////////////////////////////////////////////////////////////////
// Overrided Print methods
////////////////////////////////////////////////////////////////////////////////

// the standard print method will call write for each character in the buffer
// this is very slow on ESP
size_t ESP8266_AT_Client::print(const __FlashStringHelper *ifsh)
{
	return printFSH(ifsh, false);
}

// if we do override this, the standard println will call the print
// method twice
size_t ESP8266_AT_Client::println(const __FlashStringHelper *ifsh)
{
	return printFSH(ifsh, true);
}


////////////////////////////////////////////////////////////////////////////////
// Implementation of Client virtual methods
////////////////////////////////////////////////////////////////////////////////

int ESP8266_AT_Client::connectSSL(const char* host, uint16_t port)
{
	return connect(host, port, SSL_MODE);
}

int ESP8266_AT_Client::connectSSL(IPAddress ip, uint16_t port)
{
	char s[16];
	sprintf_P(s, PSTR("%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
	return connect(s, port, SSL_MODE);
}

int ESP8266_AT_Client::connect(const char* host, uint16_t port)
{
    return connect(host, port, TCP_MODE);
}

int ESP8266_AT_Client::connect(IPAddress ip, uint16_t port)
{
	char s[16];
	sprintf_P(s, PSTR("%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);

	return connect(s, port, TCP_MODE);
}

/* Private method */
int ESP8266_AT_Client::connect(const char* host, uint16_t port, uint8_t protMode)
{
	LOGINFO1(F("Connecting to"), host);

	_sock = ESP8266_AT_Class::getFreeSocket();

    if (_sock != NO_SOCKET_AVAIL)
    {
    	if (!ESP8266_AT_Drv::startClient(host, port, _sock, protMode))
			return 0;

    	ESP8266_AT_Class::allocateSocket(_sock);
    }
	else
	{
    	LOGERROR(F("No socket available"));
    	return 0;
    }
    return 1;
}

size_t ESP8266_AT_Client::write(uint8_t b)
{
	  return write(&b, 1);
}

size_t ESP8266_AT_Client::write(const uint8_t *buf, size_t size)
{
	if ( (_sock >= MAX_SOCK_NUM) || (size==0) )
	{
		setWriteError();
		return 0;
	}

	bool r = ESP8266_AT_Drv::sendData(_sock, buf, size);
	if (!r)
	{
		setWriteError();
		LOGERROR1(F("Failed to write to socket"), _sock);
		delay(4000);
		stop();
		return 0;
	}

	return size;
}

int ESP8266_AT_Client::available()
{
	if (_sock != 255)
	{
		int bytes = ESP8266_AT_Drv::availData(_sock);
		if (bytes>0)
		{
			return bytes;
		}
	}

	return 0;
}

int ESP8266_AT_Client::read()
{
	uint8_t b;
	if (!available())
		return -1;

	bool connClose = false;
	ESP8266_AT_Drv::getData(_sock, &b, false, &connClose);

	if (connClose)
	{
		ESP8266_AT_Class::releaseSocket(_sock);
		_sock = 255;
	}

	return b;
}

int ESP8266_AT_Client::read(uint8_t* buf, size_t size)
{
	if (!available())
		return -1;
	return ESP8266_AT_Drv::getDataBuf(_sock, buf, size);
}

int ESP8266_AT_Client::peek()
{
	uint8_t b;
	if (!available())
		return -1;

	bool connClose = false;
	ESP8266_AT_Drv::getData(_sock, &b, true, &connClose);

	if (connClose)
	{
		ESP8266_AT_Class::releaseSocket(_sock);
		_sock = 255;
	}

	return b;
}

void ESP8266_AT_Client::flush()
{
	while (available())
		read();
}

void ESP8266_AT_Client::stop()
{
	if (_sock == 255)
		return;

	LOGINFO1(F("Disconnecting "), _sock);

	ESP8266_AT_Drv::stopClient(_sock);

	ESP8266_AT_Class::releaseSocket(_sock);
	_sock = 255;
}


uint8_t ESP8266_AT_Client::connected()
{
	return (status() == ESTABLISHED);
}


ESP8266_AT_Client::operator bool()
{
  return _sock != 255;
}

////////////////////////////////////////////////////////////////////////////////
// Additional WiFi standard methods
////////////////////////////////////////////////////////////////////////////////


uint8_t ESP8266_AT_Client::status()
{
	if (_sock == 255)
	{
		return CLOSED;
	}

	if (ESP8266_AT_Drv::availData(_sock))
	{
		return ESTABLISHED;
	}

	if (ESP8266_AT_Drv::getClientState(_sock))
	{
		return ESTABLISHED;
	}

	ESP8266_AT_Class::releaseSocket(_sock);
	_sock = 255;

	return CLOSED;
}

IPAddress ESP8266_AT_Client::remoteIP()
{
	IPAddress ret;
	ESP8266_AT_Drv::getRemoteIpAddress(ret);
	return ret;
}

////////////////////////////////////////////////////////////////////////////////
// Private Methods
////////////////////////////////////////////////////////////////////////////////

size_t ESP8266_AT_Client::printFSH(const __FlashStringHelper *ifsh, bool appendCrLf)
{
	size_t size = strlen_P((char*)ifsh);
	
	if ( (_sock >= MAX_SOCK_NUM) || (size==0) )
	{
		setWriteError();
		return 0;
	}

	bool r = ESP8266_AT_Drv::sendData(_sock, ifsh, size, appendCrLf);
	if (!r)
	{
		setWriteError();
		LOGERROR1(F("Failed to write to socket"), _sock);
		delay(4000);
		stop();
		return 0;
	}

	return size;
}
