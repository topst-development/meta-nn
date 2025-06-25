// SPDX-License-Identifier: (GPL-2.0-or-later OR MIT)
/*
 * Copyright (C) Telechips Inc.
 */

#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

int get_target(int fd, uint8_t address, uint8_t Reg)
{
	int iRet = 0;
	uint8_t Val = 0;

	struct i2c_msg messages[] = 
	{
		{ address, 0, 1, &Reg },
		{ address, I2C_M_RD, 1, &Val },
	};

	struct i2c_rdwr_ioctl_data ioctl_data = { messages, 2 };
	int result = ioctl(fd, I2C_RDWR, &ioctl_data);

	iRet = Val;
	return iRet;
}

int main()
{
	const char * device = "/dev/i2c-1";
	const uint8_t address = 0x48;

	int fd = open(device, O_RDWR);
	if (fd < 0) 
	{ 
		return 1; 
	}
	
	int target = get_target(fd, address, 0x40);
	if (target < 0) 
	{
		return 1;
	}
      	
	printf("Target is %d.\n", target);

	close(fd);
	return 0;
}
