/*
 * omap34xxcam.h
 *
 * Copyright (C) 2006--2009 Nokia Corporation
 * Copyright (C) 2007--2009 Texas Instruments
 *
 * Contact: Sakari Ailus <sakari.ailus@nokia.com>
 *          Tuukka Toivonen <tuukka.o.toivonen@nokia.com>
 *
 * Originally based on the OMAP 2 camera driver.
 *
 * Written by Sakari Ailus <sakari.ailus@nokia.com>
 *            Tuukka Toivonen <tuukka.o.toivonen@nokia.com>
 *            Sergio Aguirre <saaguirre@ti.com>
 *            Mohit Jalori
 *            Sameer Venkatraman
 *            Leonides Martinez
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifndef OMAP34XXCAM_H
#define OMAP34XXCAM_H

#include <media/v4l2-int-device.h>
#include "isp/isp.h"

#define CAM_NAME			"omap34xxcam"
#define CAM_SHORT_NAME			"omap3"

#define OMAP34XXCAM_XCLK_NONE	-1
#define OMAP34XXCAM_XCLK_A	0
#define OMAP34XXCAM_XCLK_B	1

#define OMAP34XXCAM_SLAVE_SENSOR	0   // 摄像头 从机  传感器
#define OMAP34XXCAM_SLAVE_LENS		1
#define OMAP34XXCAM_SLAVE_FLASH		2 /* This is the last slave! */

/* mask for omap34xxcam_slave_power_set */
#define OMAP34XXCAM_SLAVE_POWER_SENSOR	(1 << OMAP34XXCAM_SLAVE_SENSOR)
#define OMAP34XXCAM_SLAVE_POWER_LENS	(1 << OMAP34XXCAM_SLAVE_LENS)
#define OMAP34XXCAM_SLAVE_POWER_SENSOR_LENS \
	(OMAP34XXCAM_SLAVE_POWER_SENSOR | OMAP34XXCAM_SLAVE_POWER_LENS)
#define OMAP34XXCAM_SLAVE_POWER_FLASH	(1 << OMAP34XXCAM_SLAVE_FLASH)
#define OMAP34XXCAM_SLAVE_POWER_ALL	-1

#define OMAP34XXCAM_VIDEODEVS		4

/* #define OMAP34XXCAM_POWEROFF_DELAY (2 * HZ) */

struct omap34xxcam_device;
struct omap34xxcam_videodev;

/**
 * struct omap34xxcam_sensor_config - struct for vidioc_int_g_priv ioctl
 * @sensor_isp: Is sensor smart/SOC or raw
 * @capture_mem: Size limit to mmap buffers.
 * @ival_default: Default frame interval for sensor.
 */
 
struct omap34xxcam_sensor_config {
	int sensor_isp;  // 传感器是 smart soc 还是 raw
	u32 capture_mem;  // 映射缓冲区的大小限制
	struct v4l2_fract ival_default;  // 传感器默认帧间隔 
};

struct omap34xxcam_lens_config {
};

struct omap34xxcam_flash_config {
};

// omap34xx cam硬件配置
struct omap34xxcam_hw_config {
	int dev_index; /* Index in omap34xxcam_sensors */  // omap34摄像头传感器 序号
	int dev_minor; /* Video device minor number */  // 摄像头设备 小号
	int dev_type; /* OMAP34XXCAM_SLAVE_* */   
	union {  // 集合
		struct omap34xxcam_sensor_config sensor;  // 传感器配置
		struct omap34xxcam_lens_config lens;
		struct omap34xxcam_flash_config flash;
	} u;
	int cur_input;
};

/**
 * struct omap34xxcam_videodev - per /dev/video* structure
 * @mutex: serialises access to this structure
 * @cam: pointer to cam hw structure
 * @master: we are v4l2_int_device master
 * @sensor: sensor device
 * @lens: lens device
 * @flash: flash device
 * @slaves: how many slaves we have at the moment
 * @vfd: our video device
 * @index: index of this structure in cam->vdevs
 * @users: how many users we have
 * @power_state: Current power state
 * @power_state_wish: New power state when poweroff_timer expires
 * @power_state_mask: Bitmask of devices to set the new power state
 * @poweroff_timer: Timer for dispatching poweroff_work
 * @poweroff_work: Work for slave power state change
 * @sensor_config: ISP-speicific sensor configuration
 * @lens_config: ISP-speicific lens configuration
 * @flash_config: ISP-speicific flash configuration
 * @streaming: streaming file handle, if streaming is enabled
 * @want_timeperframe: Desired timeperframe
 * @want_pix: Desired pix
 * @pix: Current pix
 */
struct omap34xxcam_videodev {
	struct mutex mutex; /* serialises access to this structure */

	struct omap34xxcam_device *cam;
	struct v4l2_int_device master;

#define vdev_sensor slave[OMAP34XXCAM_SLAVE_SENSOR]
#define vdev_lens slave[OMAP34XXCAM_SLAVE_LENS]
#define vdev_flash slave[OMAP34XXCAM_SLAVE_FLASH]
	struct v4l2_int_device *slave[OMAP34XXCAM_SLAVE_FLASH + 1];

	/* number of slaves attached */
	int slaves;

	/*** video device parameters ***/
	struct video_device *vfd;

	/*** general driver state information ***/
	int index;
	atomic_t users;
	enum v4l2_power power_state[OMAP34XXCAM_SLAVE_FLASH + 1];
#ifdef OMAP34XXCAM_POWEROFF_DELAY
	enum v4l2_power power_state_wish;
	int power_state_mask;
	struct timer_list poweroff_timer;
	struct work_struct poweroff_work;
#endif /* OMAP34XXCAM_POWEROFF_DELAY */

#define vdev_sensor_config slave_config[OMAP34XXCAM_SLAVE_SENSOR].u.sensor
#define vdev_lens_config slave_config[OMAP34XXCAM_SLAVE_LENS].u.lens
#define vdev_flash_config slave_config[OMAP34XXCAM_SLAVE_FLASH].u.flash
	struct omap34xxcam_hw_config slave_config[OMAP34XXCAM_SLAVE_FLASH + 1];

#define vdev_sensor_mode slave_mode[OMAP34XXCAM_SLAVE_SENSOR]
#define vdev_lens_mode slave_mode[OMAP34XXCAM_SLAVE_LENS]
#define vdev_flash_mode slave_mode[OMAP34XXCAM_SLAVE_FLASH]
	int slave_mode[OMAP34XXCAM_SLAVE_FLASH + 1];

	/*** capture data ***/
	struct file *streaming;
	struct v4l2_fract want_timeperframe;
	struct v4l2_pix_format want_pix;
	struct v4l2_pix_format pix;
};

/**
 * struct omap34xxcam_device - per-device data structure
 * @vdevs: /dev/video specific structures
 */
struct omap34xxcam_device {
	struct omap34xxcam_videodev vdevs[OMAP34XXCAM_VIDEODEVS];
	struct device *isp;
};

/**
 * struct omap34xxcam_fh - per-filehandle data structure
 * @vbq_lock: spinlock for the videobuf queue
 * @vbq: V4L2 video buffer queue structure
 * @field_count: field counter for videobuf_buffer
 * @vdev: our /dev/video specific structure
 */
struct omap34xxcam_fh {
	spinlock_t vbq_lock; /* spinlock for the videobuf queue */
	struct videobuf_queue vbq;
	atomic_t field_count;
	struct omap34xxcam_videodev *vdev;
};

#endif /* ifndef OMAP34XXCAM_H */
