#include "EthernetDrivers.hpp"
#include <stdio.h>
#include <stdexcept>

namespace EthernetDrivers
{
//ROSA project constants definitions
const int INDUC_L=11;
const int INDUC_R=3;
const UART_Driver::UART SONAR={5,6};
const UART_Driver::UART uC={2,4};
const UART_Driver::UART PANTILT={7,9};

//EthDriver initializations
SR_HANDLE EthDriver::srh=NULL;
int EthDriver::connected_devices=0;
char EthDriver::ip_addr[16];
const bool EthDriver::AnIn[12]={0,0,0,0,1,1,1,1,1,1,0,0};
const bool EthDriver::I2C[12]={1,2,0,0,0,0,0,0,1,2,0,0};
bool EthDriver::PinUse[12]={0,0,0,0,0,0,0,0,0,0,0,0};

bool EthDriver::release_pin(int pin, int release_value){
	if(release_value != 2)
		return !(PinUse[pin]=sr_pin_set(srh, pin, (bool) release_value));
	PinUse[pin]=false;
	return true;
}

bool EthDriver::pin_config(int pin, sr_pin_type type){
	if(PinUse[pin])
		return false;
	if(type == sr_pt_analog_in && AnIn[pin])
		throw std::domain_error("Pin not available for analog input.");
	PinUse[pin]=true;
	return sr_pin_setup(srh,pin,type);
}

bool EthDriver::Connect(){
	srh = sr_open_eth(ip_addr);
	/*		if (!srh)
		printf("Failed to allocate a handle\n");*/
	return (srh!=NULL);

}

const char* EthDriver::log_error(){
	return sr_error_info(srh);
}
EthDriver::EthDriver(const EthDriver &old){
	connected_devices++;}

EthDriver::~EthDriver(){
	//	printf("Disconnecting Driver %i..\n",(int)connected_devices);
	if(--connected_devices) return;
	sr_close(srh);
	srh=NULL;
}

EthDriver::EthDriver(char *ip_addr){
	connected_devices++;
	if(srh)
		return;

	memcpy(EthDriver::ip_addr, ip_addr,strlen(ip_addr)+1);
	Connect();
}

EthDriver::EthDriver(char *bcast_addr, unsigned char mac[]){
	connected_devices++;
	if(srh)
		return;

	SR_IPLIST *list = sr_discover(bcast_addr);
	if (list == NULL)
	{
		/*			printf("Discover: FAIL\n");*/
	}
	else
	{
		SR_IPLIST *l = list;
		while (l != NULL)
		{
			if(memcmp ( mac, l->mac, 6 ))
				l = l->next;
			else
			{
								sprintf(ip_addr,"%u.%u.%u.%u", (unsigned int) l->ip & 0xff, (unsigned int) (l->ip>>8) & 0xff, (unsigned int) (l->ip>>16) & 0xff, (unsigned int) (l->ip>>24) & 0xff);
				Connect();
				break;
			}

		}
		sr_discover_free(list);
	}
}







//UART_Driver initializations
int UART_Driver::STD_BAUD=9600;
const bool UART_Driver::UART_SPI_CNT[12]={1,1,1,0,1,1,1,1,1,1,1,0};
UART_Driver* UART_Driver::UART_module_use[2]={NULL,NULL};


bool UART_Driver::module_usage(int module){
	return UART_module_use[module]!=NULL;
}

UART_Driver* UART_Driver::device_on_module(int module){
	return UART_module_use[module];
}

bool UART_Driver::inuse(){
	return (this==UART_module_use[uart_module]);
}

bool UART_Driver::activate(){
	if(UART_module_use[uart_module]!=NULL)
		UART_module_use[uart_module]->deactivate();
	if(sr_uart_enable(srh, uart_module, mode, baud, device.rx, device.tx))
		UART_module_use[uart_module]=this;
	else
		return false;

	activated=true;
	return true;
}

bool UART_Driver::activate(int uart_mod){
	//UART enabling
	if(uart_mod>1 || uart_mod<0)
		return false;

	uart_module = uart_mod;

	return activate();
}

bool UART_Driver::deactivate(){
	return sr_uart_disable(srh, uart_module)
			&&	release_pin(device.rx)
			&&  release_pin(device.tx)
			&&  !(activated=false)
			&&  !(UART_module_use[uart_module]=NULL);
}

void UART_Driver::uart_config(bool odd_parity, bool two_stop_bit, int local_baud, bool inverted_logic){
	//Pin configuration with error checking
	bool done =	pin_config(device.rx, sr_pt_din_pullup);
	if(inverted_logic)
		done = done && pin_config(device.tx, sr_pt_dout_opendrain_short);
	else
		done = done && pin_config(device.tx, sr_pt_dout_opendrain_open);
	if(!done)
		throw std::invalid_argument("Bad Pin");

	//UART mode check
	mode=0;
	if(odd_parity)
		mode = mode |  UART_MODE_PARITY_ODD;
	else
		mode = mode | UART_MODE_PARITY_EVEN;
	if(two_stop_bit)
		mode = mode | UART_MODE_STOP_TWO;
}

UART_Driver::UART_Driver(const EthDriver& driver, UART subdevice, int uart_mod, bool odd_parity, bool two_stop_bit, int local_baud, bool inverted_logic): EthDriver(driver), device(subdevice), uart_module(uart_mod), baud(local_baud){
	//Check if it's a UART enable pin
	if(!(UART_SPI_CNT[subdevice.rx]&&UART_SPI_CNT[subdevice.tx]))
		throw std::domain_error("UART pin expected");

	//Configure UART
	uart_config(odd_parity, two_stop_bit, local_baud, inverted_logic);
}

UART_Driver::UART_Driver(char *ip_addr, UART subdevice, int uart_mod, bool odd_parity, bool two_stop_bit, int local_baud, bool inverted_logic): EthDriver(ip_addr), device(subdevice), uart_module(uart_mod), baud(local_baud){
	//Check if it's a UART enable pin
	if(!(UART_SPI_CNT[subdevice.rx]&&UART_SPI_CNT[subdevice.tx]))
		throw std::domain_error("UART pin expected");

	//Configure UART
	uart_config(odd_parity, two_stop_bit, local_baud, inverted_logic);
}

UART_Driver::UART_Driver(char *bcast_addr, unsigned char mac[], UART subdevice, int uart_mod, bool odd_parity, bool two_stop_bit, int local_baud, bool inverted_logic): EthDriver(bcast_addr, mac), device(subdevice), uart_module(uart_mod), baud(local_baud){
	//Check if it's a UART enable pin
	if(!(UART_SPI_CNT[subdevice.rx]&&UART_SPI_CNT[subdevice.tx]))
		throw std::domain_error("UART pin expected");

	//Configure UART
	uart_config(odd_parity, two_stop_bit, local_baud, inverted_logic);
}

bool UART_Driver::send(unsigned char *arr, unsigned short cnt){
	if(!activated)
		return false;
	if(cnt==1)
		return sr_uart_write(srh, uart_module, *arr);

	return sr_uart_write_arr(srh, uart_module, arr, cnt);

}

bool UART_Driver::read(unsigned char* arr, unsigned short* cnt, bool wait_data){
	if(!activated)
		return false;

	unsigned short readed;

	if(cnt==NULL)
		if(wait_data)
			return sr_uart_read(srh, uart_module, arr);
		else
			return (sr_uart_read(srh, uart_module, arr, &readed) && readed == 1);

	if(wait_data)
		return sr_uart_read_arr(srh, uart_module, arr, *cnt);

	return (sr_uart_read_arr(srh, uart_module, arr, *cnt, &readed) && (*cnt=readed)!=0 );



}

UART_Driver::~UART_Driver(){
deactivate();

}


//GPIOin Definitions

GPIOin_Driver::GPIOin_Driver(const EthDriver& driver, int t_pin,sr_pin_type type): EthDriver(driver), pin(t_pin){
	if(type != sr_pt_din && type != sr_pt_din_pullup && type != sr_pt_analog_in)
		throw std::domain_error("Input type expected");
	if(!pin_config(pin, type))
		throw std::invalid_argument("Bad Pin");
}

GPIOin_Driver::GPIOin_Driver(char *ip_addr, int t_pin,sr_pin_type type): EthDriver(ip_addr),pin(t_pin){
	if(type != sr_pt_din && type != sr_pt_din_pullup && type != sr_pt_analog_in)
		throw std::domain_error("Input type expected");
	if(!pin_config(pin, type))
		throw std::invalid_argument("Bad Pin");
}
GPIOin_Driver::GPIOin_Driver(char *bcast_addr, unsigned char mac[], int t_pin,sr_pin_type type): EthDriver(bcast_addr, mac),pin(t_pin){
	if(type != sr_pt_din && type != sr_pt_din_pullup && type != sr_pt_analog_in)
		throw std::domain_error("Input type expected");
	if(!pin_config(pin, type))
		throw std::invalid_argument("Bad Pin");
}
void GPIOin_Driver::read(bool *b){
	sr_pin_get(srh,pin,b);
}

GPIOin_Driver::~GPIOin_Driver(){
	release_pin(pin);
}

}
