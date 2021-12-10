/** \file beep-driver-sysfs.c
 * \brief implement the beep evdev driver
 * \author Copyright (C) 2000-2010 Johnathan Nightingale
 * \author Copyright (C) 2010-2013 Gerfried Fuchs
 * \author Copyright (C) 2019 Hans Ulrich Niedermann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *
 * \defgroup beep_driver_evdev Linux evdev API driver
 * \ingroup beep_driver
 *
 * @{
 *
 */


#include <stddef.h>
#include <errno.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <linux/input.h>

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "beep-compiler.h"
#include "beep-drivers.h"

#include "beep-library.h"
#include "beep-log.h"




#define LOG_MODULE "sysfs"


static
int open_checked_device(const char *const device_name)
{
	LOG_VERBOSE("checking device %s",device_name);
    const int fd = open(device_name,O_WRONLY);
    if (fd == -1) {
        return -1;
    }
    return fd;
}


static
bool driver_detect(beep_driver *driver, const char *console_device)
{
    if (console_device) {
        LOG_VERBOSE("driver_detect %p %s",
                    (void *)driver, console_device);
    } else {
        LOG_VERBOSE("driver_detect %p %p",
                    (void *)driver, (const void *)console_device);
    }
    if (console_device) {
        const int fd = open_checked_device(console_device);
        if (fd >= 0) {
            driver->device_fd = fd;
            driver->device_name = console_device;
            return true;
        }
    } else {
        static const char *const default_name ="/sys/class/pwm/pwmchip0/pwm0/duty_cycle";
        static const char *const default_name2 ="/sys/class/pwm/pwmchip0/pwm0/period";
        const int fd = open_checked_device(default_name);
        const int fd2 = open_checked_device(default_name2);
        if (fd >= 0 && fd2>=0 ) {
            driver->device_fd = fd;
            driver->device_fd2 = fd2;
            driver->device_name = default_name;
            return true;
        }
    }
    return false;
}


static
void driver_init(beep_driver *driver)
{
    LOG_VERBOSE("driver_init %p", (void *)driver);
}


static
void driver_fini(beep_driver *driver)
{
    LOG_VERBOSE("driver_fini %p", (void *)driver);
    close(driver->device_fd);
    close(driver->device_fd2);
    driver->device_fd = -1;
    driver->device_fd2 = -1;
}


static
void driver_begin_tone(beep_driver *driver, const uint16_t freq)
{
	char buf[64];
	char buf2[64];
	uint32_t period=1000000000/freq;
    sprintf(buf,"%d\n", period/2);
    sprintf(buf2,"%d\n", period);
    LOG_VERBOSE("driver_begin_tone %p %u -> %s,%s", (void *)driver, freq,buf2,buf);
    

	if(write(driver->device_fd,buf,strlen(buf))<0){
		safe_error_exit("write to pwm duty");
	}
	
	if(write(driver->device_fd2,buf2,strlen(buf))<0){
		safe_error_exit("write to pwm period ");
	}
}


static
void driver_end_tone(beep_driver *driver)
{
	char buf[64];
    sprintf(buf,"%ld\n", 0L);
    LOG_VERBOSE("driver_end_tone %p -> %s", (void *)driver, buf);
	if(write(driver->device_fd,buf,strlen(buf))<0){
		safe_error_exit("write to pwm");
	}
}


static
beep_driver driver_data =
    {
     "sysfs",
     NULL,
     driver_detect,
     driver_init,
     driver_fini,
     driver_begin_tone,
     driver_end_tone,
     0,
     NULL,
     0
    };


static
void beep_driver_evdev_constructor(void)
    CONSTRUCTOR_FUNCTION;

static
void beep_driver_evdev_constructor(void)
{
    LOG_VERBOSE("beep_driver_sysfs_constructor");
    beep_drivers_register(&driver_data);
}


/** @} */


/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
