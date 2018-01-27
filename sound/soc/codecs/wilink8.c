/*
 * Driver for Wilink8 Bluetooth SCO
 * Copyright 2018 Adam Serbinski <adam@serbinski.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#include <sound/soc.h>

#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h>
#include <linux/skbuff.h>

static const struct snd_soc_dapm_widget bt_sco_widgets[] = {
	SND_SOC_DAPM_INPUT("RX"),
	SND_SOC_DAPM_OUTPUT("TX"),
};

static const struct snd_soc_dapm_route bt_sco_routes[] = {
	{ "Capture", NULL, "RX" },
	{ "TX", NULL, "Playback" },
};

static int bt_sco_hw_params(struct snd_pcm_substream *substream,
			    struct snd_pcm_hw_params *params,
			    struct snd_soc_dai *cpu_dai)
{
	u8 rate8k = 1;
	struct hci_dev *hdev;

	switch (params_rate(params)) {
	case 8000:
		rate8k = 1;
		break;
	case 16000:
		rate8k = 0;
		break;
	default:
		dev_err(cpu_dai->dev, "Bad rate: %d\n", params_rate(params));
		return -EINVAL;
	}

	if ((hdev = hci_dev_get(0))){
		const u8 pcm8k[34] = {0x00, 0x02, 0x01, 0x40, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x01, 0x00, 0x01, 0x10, 0x00, 0x01, 0x00, 0x00, 0x00, 0x10, 0x00, 0x21, 0x00, 0x01, 0x10, 0x00, 0x21, 0x00, 0x00, 0x00};
		const u8 pcm16k[34] = {0x00, 0x04, 0x01, 0x80, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x01, 0x00, 0x01, 0x10, 0x00, 0x01, 0x00, 0x00, 0x00, 0x10, 0x00, 0x21, 0x00, 0x01, 0x10, 0x00, 0x21, 0x00, 0x00, 0x00};
		struct sk_buff *skb = __hci_cmd_sync(hdev, 0xfd06, 34, (rate8k ? pcm8k : pcm16k), HCI_INIT_TIMEOUT);
		if (!IS_ERR(skb)) {
			kfree_skb(skb);
		}
	}

	return 0;
}

static struct snd_soc_dai_ops bt_sco_dai_ops = {
	.hw_params	= bt_sco_hw_params,
};

static struct snd_soc_dai_driver bt_sco_dai[] = {
	{
		.name = "bt-sco-pcm",
		.playback = {
			.stream_name = "Playback",
			.channels_min = 2,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
		.capture = {
			 .stream_name = "Capture",
			.channels_min = 2,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
		.ops = &bt_sco_dai_ops,
	}
};

static struct snd_soc_codec_driver soc_codec_dev_bt_sco = {
	.component_driver = {
		.dapm_widgets		= bt_sco_widgets,
		.num_dapm_widgets	= ARRAY_SIZE(bt_sco_widgets),
		.dapm_routes		= bt_sco_routes,
		.num_dapm_routes	= ARRAY_SIZE(bt_sco_routes),
	},
};

static int bt_sco_probe(struct platform_device *pdev)
{
	return snd_soc_register_codec(&pdev->dev, &soc_codec_dev_bt_sco,
				      bt_sco_dai, ARRAY_SIZE(bt_sco_dai));
}

static int bt_sco_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);

	return 0;
}

static const struct platform_device_id bt_sco_driver_ids[] = {
	{
		.name		= "wilink8-sco",
	},
	{},
};
MODULE_DEVICE_TABLE(platform, bt_sco_driver_ids);

#if defined(CONFIG_OF)
static const struct of_device_id bt_sco_codec_of_match[] = {
	{ .compatible = "linux,wilink8-sco", },
	{},
};
MODULE_DEVICE_TABLE(of, bt_sco_codec_of_match);
#endif

static struct platform_driver bt_sco_driver = {
	.driver = {
		.name = "wilink8-sco",
		.of_match_table = of_match_ptr(bt_sco_codec_of_match),
	},
	.probe = bt_sco_probe,
	.remove = bt_sco_remove,
	.id_table = bt_sco_driver_ids,
};

module_platform_driver(bt_sco_driver);

MODULE_AUTHOR("Adam Serbinski <adam@serbinski.com>");
MODULE_DESCRIPTION("ASoC Wilink8 bluetooth sco link driver");
MODULE_LICENSE("GPL");
