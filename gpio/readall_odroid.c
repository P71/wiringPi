/*
 * readall.c:
 *	The readall functions - getting a bit big, so split them out.
 *	Copyright (c) 2012-2017 Gordon Henderson
 ***********************************************************************
 * This file is part of wiringPi:
 *	https://projects.drogon.net/raspberry-pi/wiringpi/
 *
 *    wiringPi is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    wiringPi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with wiringPi.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <wiringPi.h>
/*----------------------------------------------------------------------------*/
#include <wiringOdroid.h>

extern int wpMode ;

/*----------------------------------------------------------------------------*/
#ifndef TRUE
#  define 	TRUE	(1==1)
#  define	FALSE	(1==2)
#endif

/*----------------------------------------------------------------------------*/
/*
 * doReadallExternal:
 *	A relatively crude way to read the pins on an external device.
 *	We don't know the input/output mode of pins, but we can tell
 *	if it's an analog pin or a digital one...
 */
/*----------------------------------------------------------------------------*/
static void doReadallExternal (void)
{
	int pin ;

	printf ("+------+---------+--------+\n") ;
	printf ("|  Pin | Digital | Analog |\n") ;
	printf ("+------+---------+--------+\n") ;

	for (pin = wiringPiNodes->pinBase ; pin <= wiringPiNodes->pinMax ; ++pin)
		printf ("| %4d |  %4d   |  %4d  |\n", pin, digitalRead (pin), analogRead (pin)) ;

	printf ("+------+---------+--------+\n") ;
}

/*----------------------------------------------------------------------------*/
static const char *alts [] =
{
	"IN", "OUT", "ALT"
} ;

/*----------------------------------------------------------------------------*/
static const int physToWpi [64] = 
{
	-1,	// 0
	-1, -1,	// 1, 2
	 8, -1,
	 9, -1,
	 7, 15,
	-1, 16,
	 0,  1,
	 2, -1,
	 3,  4,
	-1,  5,
	12, -1,
	13,  6,
	14, 10,
	-1, 11,	// 25, 26
	30, 31,	// Actually I2C, but not used
	21, -1,
	22, 26,
	23, -1,
	24, 27,
	25, 28,
	-1, 29,
	-1, -1,
	-1, -1,
	-1, -1,
	-1, -1,
	-1, -1,
	17, 18,
	19, 20,
	-1, -1, -1, -1, -1, -1, -1, -1, -1
} ;

/*----------------------------------------------------------------------------*/
static const char *physNamesOdroidC1 [64] =
{
	NULL,

	"    3.3V", "5V      ",
	"   SDA.1", "5V      ",
	"   SCL.1", "GND(0V) ",
	"GPIO. 83", "TxD1    ",
	" GND(0V)", "RxD1    ",
	"GPIO. 88", "GPIO. 87",
	"GPIO.116", "GND(0V) ",
	"GPIO.115", "GPIO.104",
	"    3.3V", "GPIO.102",
	"    MOSI", "GND(0V) ",
	"    MISO", "GPIO.103",
	"    SCLK", "CE0     ",
	" GND(0V)", "GPIO.118",
	"   SDA.2", "SCL.2   ",
	"GPIO.101", "GND(0V) ",
	"GPIO.100", "GPIO. 99",
	"GPIO.108", "GND(0V) ",
	"GPIO.97 ", "GPIO. 98",
	"   AIN.1", "1V8     ",
	" GND(0V)", "AIN.0   ",

	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,
} ;

/*----------------------------------------------------------------------------*/
static const char *physNamesOdroidC2_Rev2 [64] =
{
	NULL,

	"    3.3V", "5V      ",
	"   SDA.1", "5V      ",
	"   SCL.1", "GND(0V) ",
	"GPIO.249", "TxD1    ",
	" GND(0V)", "RxD1    ",
	"GPIO.247", "GPIO.238",
	"GPIO.239", "GND(0V) ",
	"GPIO.237", "GPIO.236",
	"    3.3V", "GPIO.233",
	"GPIO.235", "GND(0V) ",
	"GPIO.232", "GPIO.231",
	"GPIO.230", "GPIO.229",
	" GND(0V)", "GPIO.225",
	"   SDA.2", "SCL.2   ",
	"GPIO.228", "GND(0V) ",
	"GPIO.219", "GPIO.224",
	"GPIO.234", "GND(0V) ",
	"GPIO.214", "GPIO.218",
	"   AIN.1", "1V8     ",
	" GND(0V)", "AIN.0   ",

	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,
} ;

/*----------------------------------------------------------------------------*/
static const char *physNamesOdroidC2_Rev1 [64] =
{
	NULL,

	"    3.3V", "5V      ",
	"   SDA.1", "5V      ",
	"   SCL.1", "GND(0V) ",
	"GPIO.214", "--------",
	" GND(0V)", "--------",
	"GPIO.219", "GPIO.218",
	"GPIO.247", "GND(0V) ",
	"--------", "GPIO.235",
	"    3.3V", "GPIO.233",
	"GPIO.238", "GND(0V) ",
	"GPIO.237", "GPIO.234",
	"GPIO.236", "GPIO.248",
	" GND(0V)", "GPIO.249",
	"   SDA.2", "SCL.2   ",
	"GPIO.232", "GND(0V) ",
	"GPIO.231", "GPIO.230",
	"GPIO.239", "GND(0V) ",
	"GPIO.228", "GPIO.229",
	"   AIN.1", "1V8     ",
	" GND(0V)", "AIN.0   ",

	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,
} ;

/*----------------------------------------------------------------------------*/
static const char *physNamesOdroidXU3 [64] =
{
	NULL,

	"    3.3V", "5V      ",
	"I2C1.SDA", "5V      ",
	"I2C1.SCL", "GND(0V) ",
	"GPIO. 18", "UART0.TX",
	" GND(0V)", "UART0.RX",
	"GPIO.174", "GPIO.173",
	"GPIO. 21", "GND(0V) ",
	"GPIO. 22", "GPIO. 19",
	"    3.3V", "GPIO. 23",
	"    MOSI", "GND(0V) ",
	"    MISO", "GPIO. 24",
	"    SCLK", "CE0     ",
	" GND(0V)", "GPIO. 25",
	"I2C5.SDA", "I2C5.SCL",
	"GPIO. 28", "GND(0V) ",
	"GPIO. 30", "GPIO. 29",
	"GPIO. 31", "GND(0V) ",
	"POWER ON", "GPIO. 33",
	"   AIN.0", "1V8     ",
	" GND(0V)", "AIN.3   ",

	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,
} ;

/*----------------------------------------------------------------------------*/
static const char *physNamesOdroidN1 [64] =
{
	NULL,

	"    3.3V", "5V      ",
	"I2C1.SDA", "5V      ",
	"I2C1.SCL", "GND(0V) ",
	"GPIO. 18", "UART0.TX",
	" GND(0V)", "UART0.RX",
	"GPIO.174", "GPIO.173",
	"GPIO. 21", "GND(0V) ",
	"GPIO. 22", "GPIO. 19",
	"    3.3V", "GPIO. 23",
	"    MOSI", "GND(0V) ",
	"    MISO", "GPIO. 24",
	"    SCLK", "CE0     ",
	" GND(0V)", "GPIO. 25",
	"I2C5.SDA", "I2C5.SCL",
	"GPIO. 28", "GND(0V) ",
	"GPIO. 30", "GPIO. 29",
	"GPIO. 31", "GND(0V) ",
	"POWER ON", "GPIO. 33",
	"   AIN.0", "1V8     ",
	" GND(0V)", "AIN.3   ",

	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,
} ;

/*----------------------------------------------------------------------------*/
static void readallPhysOdroid (int model, int rev, int physPin, const char *physNames[])
{
	int pin ;

	if ((physPinToGpio (physPin) == -1) && (physToWpi [physPin] == -1))
		printf (" |      |    ") ;
	else if (physPinToGpio (physPin) != -1)
		printf (" |  %3d | %3d", physPinToGpio (physPin), physToWpi [physPin]);
	else
		printf (" |      | %3d", physToWpi [physPin]);

	printf (" | %s", physNames [physPin]) ;

	if ((physToWpi [physPin] == -1) || (physPinToGpio (physPin) == -1))
		printf (" |      |  ") ;
	else {
		if (wpMode == WPI_MODE_GPIO)
			pin = physPinToGpio (physPin);
		else if (wpMode == WPI_MODE_PHYS)
			pin = physPin ;
		else
			pin = physToWpi [physPin];

		printf (" | %4s", alts [getAlt (pin)]) ;
		printf (" | %d", digitalRead (pin)) ;
	}

	// Pin numbers:
	printf (" | %2d", physPin) ;
	++physPin ;
	printf (" || %-2d", physPin) ;

	// Same, reversed
	if ((physToWpi [physPin] == -1) || (physPinToGpio (physPin) == -1))
		printf (" |   |     ") ;
	else {
		if (wpMode == WPI_MODE_GPIO)
			pin = physPinToGpio (physPin);
		else if (wpMode == WPI_MODE_PHYS)
			pin = physPin ;
		else
			pin = physToWpi [physPin];

		printf (" | %d", digitalRead (pin));
		printf (" | %-4s", alts [getAlt (pin)]);
	}

	printf (" | %-6s", physNames [physPin]);

	if ((physPinToGpio (physPin) == -1) && (physToWpi [physPin] == -1))
		printf (" |     |     ") ;
	else if (physPinToGpio (physPin) != -1)
		printf (" | %-3d |  %-3d", physToWpi [physPin], physPinToGpio (physPin));
	else
		printf (" | %-3d |     ", physToWpi [physPin]);

	printf (" |\n") ;
}

/*----------------------------------------------------------------------------*/
void ReadallOdroid (int model, int rev, const char *physNames[])
{
	int pin ;

	printf (" | GPIO | wPi |   Name   | Mode | V | Physical | V | Mode |   Name   | wPi | GPIO |\n") ;
	printf (" +------+-----+----------+------+---+----++----+---+------+----------+-----+------+\n") ;
	for (pin = 1 ; pin <= 40 ; pin += 2)
		readallPhysOdroid (model, rev, pin, physNames) ;
	printf (" +------+-----+----------+------+---+----++----+---+------+----------+-----+------+\n") ;
}

/*----------------------------------------------------------------------------*/
/*
 * doReadall:
 *	Read all the GPIO pins
 *	We also want to use this to read the state of pins on an externally
 *	connected device, so we need to do some fiddling with the internal
 *	wiringPi node structures - since the gpio command can only use
 *	one external device at a time, we'll use that to our advantage...
 */
/*----------------------------------------------------------------------------*/
void doReadall (void)
{
	int model, rev, mem, maker, overVolted;
	const char (*physNames)[];

	// External readall
	if (wiringPiNodes != NULL) {
		doReadallExternal ();
		return ;
	}

	piBoardId (&model, &rev, &mem, &maker, &overVolted) ;

	switch (model) {
	case MODEL_ODROID_C1:
		printf (" +------+-----+----------+------+- Model ODROID-C -+------+----------+-----+------+\n") ;
		physNames = physNamesOdroidC1;
	break;
	case MODEL_ODROID_C2:
		printf (" +------+-----+----------+------+ Model  ODROID-C2 +------+----------+-----+------+\n") ;
		if (rev == 1)
			physNames = physNamesOdroidC2_Rev1;
		else
			physNames = physNamesOdroidC2_Rev2;
	break;
	case MODEL_ODROID_XU3:
		printf (" +------+-----+----------+------ Model ODROID-XU3/4 ------+----------+-----+------+\n") ;
		physNames = physNamesOdroidXU3;
	break;
	case MODEL_ODROID_N1:
		physNames = physNamesOdroidN1;
	break;
	default:
		printf ("Oops - unable to determine board type... model: %d\n", model) ;
	return;
	}
	ReadallOdroid(model, rev, physNames);
}

/*----------------------------------------------------------------------------*/
/*
 * doAllReadall:
 *	Force reading of all pins regardless of Pi model
 */
/*----------------------------------------------------------------------------*/
void doAllReadall (void)
{
	doReadall();
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
