/**

hdc1008.c - (c) 2015 Thomas Hangstörfer

Communicate with a TI HDC1008 sensor via i2c

This code was tested with a Adafruit HDC1008 breakout board.
https://learn.adafruit.com/adafruit-hdc1008-temperature-and-humidity-sensor-breakout

HDC1008 Datasheet: http://www.ti.com/lit/ds/symlink/hdc1008.pdf

Usage:

hdc1008 - communicate with HDC1008-sensor via i2c - (c)2015, Thomas Hangstörfer
Usage: hdc1008 [param]
    -a <address>: address of the device on the i2c-bus (default: 0x40)
    -i <bus-num>: number of the i2c-bus (default: 1 -> /dev/i2c-1)
    -m <mode>   : mode = 0: Temperature or Humidity is acquired. (default)
                : mode = 1: Temperature and Humidity are acquired in sequence, Temperature first.
    -e <heat>   : heat = 0: Heater disabled. (default)
                : heat = 1: Heater enabled.
    -t : read and print temperature
    -h : read and print humidity
    -c : read configuration from device. Write config to device otherwise.
    -v : be verbose

**/


#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define REG_TEMP 0x00
#define REG_HUM  0x01
#define REG_CFG  0x02

#define MSK_CFG_RST  (0x1<<15) // Software reset bit
#define MSK_CFG_HEAT (0x1<<13) // Heater
#define MSK_CFG_MODE (0x1<<12) // Mode of acquisition
#define MSK_CFG_BTST (0x1<<11) // Battery Status
#define MSK_CFG_TRES (0x1<<10) // Temperature measurement resolution
#define MSK_CFG_HRES (0x3<< 8) // Humidity measurement resolution

typedef struct {
	unsigned int  i2c_bus;
	unsigned int  i2c_address;
	unsigned char heater;
	unsigned char mode;
	unsigned char battery;
	unsigned char temp_res;
	unsigned char hum_res;
} hdc1008_config;


int file = 0;
int v = 0; // verbose
hdc1008_config config = { 1, 0x40, 0, 0, 0, 1, 2 };

void getConfig();
void writeConfig();
void printConfig();

void getConfig()
{
	char buf[2] = { REG_CFG };

	if (write(file, buf, 1) != 1) {
		perror("Failed to write to i2c bus");
		return;
	}

	if (read(file, buf, 2) != 2) {
		perror("Failed to read from i2c bus");
		return;
	} else {
		long cfg = (buf[0] << 8) + buf[1];
		config.heater   = (cfg & MSK_CFG_HEAT)!=0;
		config.mode     = (cfg & MSK_CFG_MODE)!=0;
		config.battery  = (cfg & MSK_CFG_BTST)!=0;
		config.temp_res = (cfg & MSK_CFG_TRES)!=0;
		config.hum_res  = (unsigned char)((cfg & MSK_CFG_HRES)>>8);
	}
}

void writeConfig()
{
	long cfg = ((config.heater << 13) & MSK_CFG_HEAT) + ((config.mode << 12) & MSK_CFG_MODE) + ((config.temp_res << 10) & MSK_CFG_TRES) + ((config.hum_res << 8 ) &  MSK_CFG_HRES);
	char hbyte = ((cfg >> 8) & 0xFF);
	char lbyte = 0; // reserved, must be 0
	char buf[3] = { REG_CFG, hbyte, lbyte };

	if (write(file, buf, 3) != 3) {
		perror("  Failed to set pointer register to config");
		return;
	} else {
		if (v) printf("config written: 0x%02x%02x\n", hbyte, lbyte);
	}
}

void printConfig()
{
	if (v) printf("heater                 = %s\n", config.heater==0?"disabled":"enabled");
	if (v) printf("mode                   = %s\n", config.mode==0?"temperature or humidity is acquired":"temperature and humidity are acquired in sequence. temp first.");
	if (v) printf("battery                = %s\n", config.battery==0?">2.8V":"<2.8V");
	if (v) printf("temperature resolution = %i bit\n", config.temp_res==0?14:11);
	if (v) printf("humidity resolution    = %i bit\n", config.hum_res==0?14:config.hum_res==1?11:8);
}

float getTemp()
{
	char buf[2] = { REG_TEMP };

	if (write(file, buf, 1) != 1) {
		perror("Failed to write to i2c bus");
		return 0.0;
	}
	usleep(7 * 1000); // wait 7ms, acc. to datasheet

	if (read(file, buf, 2) != 2) {
		perror("Failed to read from i2c bus");
		return 0.0;
	} else {
		int raw_temp = (buf[0] << 8) + buf[1];
		return ((raw_temp / 65536.0) * 165.0) - 40.0;
	}
}

float getHum()
{
	char buf[2] = { REG_HUM };

	if (write(file, buf, 1) != 1) {
		perror("Failed to write to i2c bus");
		return 0.0;
	}
	usleep(7 * 1000); // wait 7ms, acc. to datasheet

	if (read(file, buf, 2) != 2) {
		perror("Failed to read from i2c bus");
		return 0.0;
	} else {
		int raw_hum = (buf[0] << 8) + buf[1];
		return (raw_hum / 65536.0) * 100.0;
	}
}

void printUsage()
{
	printf("hdc1008 - communicate with HDC1008-sensor via i2c - (c)2015, Thomas Hangstörfer\n");
	printf("Usage: hdc1008 [param]\n");
	printf("\t-a <address>: address of the device on the i2c-bus (default: 0x40)\n");
	printf("\t-i <bus-num>: number of the i2c-bus (default: 1 -> /dev/i2c-1)\n");
	printf("\t-m <mode>   : mode = 0: Temperature or Humidity is acquired. (default)\n");
	printf("\t            : mode = 1: Temperature and Humidity are acquired in sequence, Temperature first.\n");
	printf("\t-e <heat>   : heat = 0: Heater disabled. (default)\n");
	printf("\t            : heat = 1: Heater enabled.\n");
	printf("\t-t : read and print temperature\n");
	printf("\t-h : read and print humidity\n");
	printf("\t-c : read configuration from device. Write config to device otherwise.\n");
	printf("\t-v : be verbose\n");
	printf("\n");
}

int main(int argc, char *argv[])
{
	int readTemp = 0;
	int readHum = 0;
	int readConfig = 0;
	int mode = -1;
	int opt_heater = -1;
	int c = 0;

	while ((c = getopt (argc, argv, "vthc?i:a:m:e:")) != -1)
	{
	switch (c)
	{
	case 'v':
		v = 1;
		break;
	case 't':
		readTemp = 1;
		break;
	case 'h':
		readHum = 1;
		break;
	case 'c':
		readConfig = 1;
		break;
	case 'i':
		config.i2c_bus = atoi(optarg);
		//if (v) printf("i2c_bus = %i\n", config.i2c_bus);
		break;
	case 'm':
		mode = atoi(optarg);
		if ( mode > 1 || mode < 0 )
		{
			printf("Error: invalid param for mode: %i\n", mode);
			exit(1);
		}
	case 'e':
		opt_heater = atoi(optarg);
		if ( opt_heater > 1 || opt_heater < 0 )
		{
			printf("Error: invalid param for heater: %i\n", opt_heater);
			exit(1);
		}
		break;
		break;
	case 'a':
		config.i2c_address = strtol(optarg, NULL, 16);
		//if (v) printf("i2c_address = 0x%02x\n", config.i2c_address);
		break;
	case '?':
		printUsage();
		return 1;
	default:
		break;
	}
	}
	
	char filename[1024];
	snprintf(filename, 1023, "/dev/i2c-%i", config.i2c_bus);
	if (v) printf("open %s\n", filename);

	if((file = open(filename, O_RDWR)) < 0)
	{
		perror("Failed to open the i2c bus");
		exit(1);
	}
	
	if(ioctl(file, I2C_SLAVE, config.i2c_address) < 0)
	{
		perror("Failed to acquire bus access and/or talk to slave.\n");
		exit(1);
	}
    	
	if ( readConfig != 0 )
	{
		getConfig();
		printConfig();
	}
	else
	{
		if ( mode != -1 )
			config.mode = mode;
		if ( opt_heater != -1 )
			config.heater = opt_heater;
		config.temp_res = 1;
		config.hum_res = 2;
		if (v) printConfig();
		writeConfig();
	}

	if ( readHum != 0 )
	{
		if (v)
			printf("Humidity: %f%%\n", getHum());
		else
			printf("%f ", getHum());
	}
	if ( readTemp != 0 )
	{
		if (v)
			printf("Temperature: %f°C\n", getTemp());
		else
			printf("%f ", getTemp());
	}
    
	close(file);
	printf("\n");
	return 0;
}
