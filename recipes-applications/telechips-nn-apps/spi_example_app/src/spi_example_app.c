// SPDX-License-Identifier: (GPL-2.0-or-later OR MIT)
/*
 * Copyright (C) Telechips Inc.
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static const char *device = "/dev/spidev3.0";
static uint8_t mode = 0;
static uint8_t bits = 8;
static uint32_t speed = 500000;
static uint16_t delay = 0;

static void transfer(int fd, uint8_t uAddr, uint8_t uVal)
{
	int ret;
	uint8_t tx[2]= {0,0};
	uint8_t rx[ARRAY_SIZE(tx)] = {0, };
	struct spi_ioc_transfer tr;

	tx[0] = uAddr;
	tx[1] = uVal;
	
	memset(&tr, 0, sizeof(tr));
	tr.tx_buf = (unsigned long)tx;
	tr.rx_buf = (unsigned long)rx;
	tr.len = ARRAY_SIZE(tx);
	tr.delay_usecs = delay;
	tr.speed_hz = speed;
	tr.bits_per_word = bits;
	tr.cs_change = 0;

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		printf("can't send spi message");
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;
 
	fd = open(device, O_RDWR);
	if (fd < 0)
	{
		return 0;
	}
	
	//Max7219 Init
	transfer(fd, 0x09, 0x00);
	usleep(1000 * 10);
	transfer(fd, 0x0a, 0x03);
	usleep(1000 * 10);
	transfer(fd, 0x0b, 0x07);
	usleep(1000 * 10);
	transfer(fd, 0x0c, 0x01);
	usleep(1000 * 10);
	transfer(fd, 0x0f, 0x00);
	
	usleep(1000 * 100);

	//Display '1'
	transfer(fd, 0x01, 0x08);
	transfer(fd, 0x02, 0x18);
	transfer(fd, 0x03, 0x28);
	transfer(fd, 0x04, 0x08);
	transfer(fd, 0x05, 0x08);
	transfer(fd, 0x06, 0x08);
	transfer(fd, 0x07, 0x08);
	transfer(fd, 0x08, 0x7E);

	close(fd);
 
	return ret;
}
