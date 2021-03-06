/*
 * dm3730_evk.c  --  SoC audio for DM3730_EVK
 *
 * Author: Steve Sakoman <steve@sakoman.com>
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

#include <linux/clk.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <mach/gpio.h>
#include <plat/mcbsp.h>

#include "omap-mcbsp.h"
#include "omap-pcm.h"
#include "../codecs/twl4030.h"

static int dm3730_evk_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	unsigned int fmt;
	int ret;

	switch (params_channels(params)) {
	case 2: /* Stereo I2S mode */
		fmt =	SND_SOC_DAIFMT_I2S |
			SND_SOC_DAIFMT_NB_NF |
			SND_SOC_DAIFMT_CBM_CFM;
		break;
	case 4: /* Four channel TDM mode */
		fmt =	SND_SOC_DAIFMT_DSP_A |
			SND_SOC_DAIFMT_IB_NF |
			SND_SOC_DAIFMT_CBM_CFM;
		break;
	default:
		return -EINVAL;
	}

	/* Set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai, fmt);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec DAI configuration\n");
		return ret;
	}

	/* Set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai, fmt);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu DAI configuration\n");
		return ret;
	}

	/* Set the codec system clock for DAC and ADC */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, 26000000,
				     SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec system clock\n");
		return ret;
	}

	return 0;
}

static struct snd_soc_ops dm3730_evk_ops = {
	.hw_params = dm3730_evk_hw_params,
};

/* Digital audio interface glue - connects codec <--> CPU */
static struct snd_soc_dai_link dm3730_evk_dai = {
	.name = "TWL4030",
	.stream_name = "TWL4030",
	.cpu_dai = &omap_mcbsp_dai[0],
	.codec_dai = &twl4030_dai[TWL4030_DAI_HIFI],
	.ops = &dm3730_evk_ops,
};

/* Audio machine driver */
static struct snd_soc_card snd_soc_dm3730_evk = {
	.name = "dm3730_evk",
	.platform = &omap_soc_platform,
	.dai_link = &dm3730_evk_dai,
	.num_links = 1,
};

/* Audio subsystem */
static struct snd_soc_device dm3730_evk_snd_devdata = {
	.card = &snd_soc_dm3730_evk,
	.codec_dev = &soc_codec_dev_twl4030,
};

static struct platform_device *dm3730_evk_snd_device;

static int __init dm3730_evk_soc_init(void)
{
	int ret;

	if (!machine_is_dm3730_evk()) {
		pr_debug("Not DM3730_EVK!\n");
		return -ENODEV;
	}

	pr_info("DM3730_EVK SoC init\n");

	dm3730_evk_snd_device = platform_device_alloc("soc-audio", -1);
	if (!dm3730_evk_snd_device) {
		printk(KERN_ERR "Platform device allocation failed\n");
		return -ENOMEM;
	}

	platform_set_drvdata(dm3730_evk_snd_device, &dm3730_evk_snd_devdata);
	dm3730_evk_snd_devdata.dev = &dm3730_evk_snd_device->dev;
	*(unsigned int *)dm3730_evk_dai.cpu_dai->private_data = 1; /* McBSP2 */

	ret = platform_device_add(dm3730_evk_snd_device);
	if (ret)
		goto err1;

	return 0;

err1:
	printk(KERN_ERR "Unable to add platform device\n");
	platform_device_put(dm3730_evk_snd_device);

	return ret;
}

static void __exit dm3730_evk_soc_exit(void)
{
	platform_device_unregister(dm3730_evk_snd_device);
}

module_init(dm3730_evk_soc_init);
module_exit(dm3730_evk_soc_exit);

MODULE_AUTHOR("Steve Sakoman <steve@sakoman.com>");
MODULE_DESCRIPTION("ALSA SoC DM3730_EVK");
MODULE_LICENSE("GPL");
