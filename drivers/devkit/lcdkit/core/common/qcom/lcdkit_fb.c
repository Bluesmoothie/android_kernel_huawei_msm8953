#include "mdss_mdp.h"
#include "lcdkit_panel.h"
#include "mdss_dsi.h"
#include "lcdkit_dbg.h"
#include "lcdkit_fb_util.h"
#include "lcdkit_dsi_status.h"

int  is_device_reboot = 0;
struct platform_device *lcdkit_fb_pdev;

void lcdkit_set_fb_pdev(struct platform_device *pdev)
{
	static char already_set = 0;

	/* judge if already set or not*/
	if (already_set)
	{
		LCDKIT_ERR("fb pdev already set\n");
	}
	else
	{
		lcdkit_fb_pdev = pdev;
		already_set = 1;

        LCDKIT_DEBUG("set fb pdev: %p.\n", pdev);
	}

	return;
}

/* get global ctrl_pdata pointer */
struct platform_device *lcdkit_get_fb_pdev(void)
{
    LCDKIT_DEBUG("get fb pdev: %p.\n", lcdkit_fb_pdev);

	return lcdkit_fb_pdev;
}

void *lcdkit_get_mfd_pdata(void)
{
    return platform_get_drvdata(lcdkit_get_fb_pdev());
}

static void mdss_fb_shutdown(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd = platform_get_drvdata(pdev);
	mfd->shutdown_pending = true;

	/* wake up threads waiting on idle or kickoff queues */
	wake_up_all(&mfd->idle_wait_q);
	wake_up_all(&mfd->kickoff_wait_q);

	lock_fb_info(mfd->fbi);
	mdss_fb_release_all(mfd->fbi, true);
	sysfs_notify(&mfd->fbi->dev->kobj, NULL, "show_blank_event");

	if (lcdkit_info.panel_infos.shutdown_sleep_support)
		is_device_reboot = 1;

	unlock_fb_info(mfd->fbi);
}

void mdss_fb_update_backlight_wq_handler(struct work_struct *work)
{
    (void)work;
	mdss_fb_update_backlight(lcdkit_get_mfd_pdata());
}

