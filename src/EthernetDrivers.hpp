#ifndef _ETHERNET_DRIVER_HPP_
#define _ETHERNET_DRIVER_HPP_

#include <iostream>
#include <string.h>
#include "libschremote.h"

namespace EthernetDrivers
{
class EthDriver
{
private:
	static const bool AnIn[12];
	static const bool I2C[12];
	static bool PinUse[12];
	static int connected_devices;

protected:
	static char ip_addr[16];
	bool pin_config(int pin, sr_pin_type type);
	bool release_pin(int pin, int release_value = 2);
	bool Connect();


public:
	static SR_HANDLE srh;
	const char* log_error();
	EthDriver(char *ip_addr); //ipv4 '\0' terminated device address
	~EthDriver();
	EthDriver(const EthDriver &old); //ipv4 '\0' terminated device address
	EthDriver(char *bcast_addr, unsigned char mac[]); //ipv4 '\0' terminated Broadcast address, mac[6] - Mac Address

};

class UART_Driver: private EthDriver
{
public:
	static int STD_BAUD;
	struct UART{
		int rx;
		int tx;
	};

private:
	bool activated;
	int uart_module;
	int mode;
	int baud;
	static UART_Driver* UART_module_use[2];
	static const bool UART_SPI_CNT[12];
	UART device;
	void uart_config(bool odd_parity, bool two_stop_bit, int local_baud, bool inverted_logic);

public:
	inline static bool module_usage(int module);
	inline static UART_Driver* device_on_module(int module);
	bool activate(int uart_mod);
	bool activate();
	bool deactivate();
	inline bool inuse();

	bool read(unsigned char* arr, unsigned short* cnt = NULL, bool wait_data = false);
	bool send(unsigned char* arr, unsigned short cnt=1);
	UART_Driver(const EthDriver& driver, UART subdevice, int uart_mod = 2, bool odd_parity=true, bool two_stop_bit=true, int local_baud = STD_BAUD, bool inverted_logic = false);
	UART_Driver(char *ip_addr, UART subdevice, int uart_mod = 2, bool odd_parity=true, bool two_stop_bit=true, int local_baud = STD_BAUD, bool inverted_logic = false); //ipv4 '\0' terminated device address
	UART_Driver(char *bcast_addr, unsigned char mac[], UART subdevice, int uart_mod = 2, bool odd_parity=true, bool two_stop_bit=true, int local_baud = STD_BAUD, bool inverted_logic = false); //ipv4 '\0' terminated Broadcast address, mac[6] - Mac Address
	~UART_Driver();


};


class GPIOin_Driver: private EthDriver
{
private:
	int pin;
	sr_pin_type pull; //1= input, 0=output
public:
	void read(bool *b);

	GPIOin_Driver(const EthDriver& driver, int t_pin, sr_pin_type type = sr_pt_din_pullup);
	GPIOin_Driver(char *ip_addr, int t_pin, sr_pin_type type = sr_pt_din_pullup); //ipv4 '\0' terminated device address
	GPIOin_Driver(char *bcast_addr, unsigned char mac[], int t_pin,sr_pin_type type = sr_pt_din_pullup); //ipv4 '\0' terminated Broadcast address, mac[6] - Mac Address
	~GPIOin_Driver();


};



} // end namespace EthernetDrivers

#endif // _ETHERNET_DRIVER_HPP_
