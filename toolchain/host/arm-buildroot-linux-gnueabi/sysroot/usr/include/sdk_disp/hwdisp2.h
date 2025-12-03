#ifndef _HWDISPLAY2_H
#define _HWDISPLAY2_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <semaphore.h>
#include <pthread.h>
#include <sdk_disp/sunxi_display2.h>

#define SCREEN_0 0
#define SCREEN_1 1
#define SCREEN_2 2
#define SCREEN_NUM SCREEN_2
#define LAYER_NUM 4
#define CHANNEL_NUM 4

//for video layer
#define VIDEO_LAYER_ID 0
#define VIDEO_CHANNEL_ID 0

//for screen 0
#define FB_CHANNEL_ID 1
#define FB_LAYER_ID 0

//for screen 1
#define FB1_CHANNEL_ID 1
#define FB1_LAYER_ID 1

#define DISP_DEV "/dev/disp"
#define RET_OK 0
#define RET_FAIL -1

// layer info compose and decompose
#define COMP_LAYER_HDL(s, c, l) (((s & 0xf) << 16) | ((c & 0xf) << 8) | (l & 0xf))
#define HDL_2_SCREENID(hdl) ((hdl >> 16) & 0xf)
#define HDL_2_CHANNELID(hdl) ((hdl >> 8) & 0xf)
#define HDL_2_LAYERID(hdl) ((hdl) & 0xf)

struct view_info {
    unsigned int x;
    unsigned int y;
    unsigned int w;
    unsigned int h;
    unsigned int screen_id;
    unsigned int channel;
    unsigned int layer;
};

struct src_info {
    unsigned int w;
    unsigned int h;
    unsigned int format;
};

namespace android
{

class HwDisplay
{
public:
    const HwDisplay &operator=(const HwDisplay &);
    static HwDisplay *getInstance();

    int hwd_other_screen(int screen_id, int type, int mode);
    int hwd_screen1_mode(int mode);

    int aut_hwd_layer_request(struct view_info *surface, unsigned int screen_id, unsigned int channel, unsigned int layer_id);
    int aut_hwd_layer_release(unsigned int layer_hdl);
    int aut_hwd_layer_set_src(unsigned int layer_hdl, struct src_info *src, unsigned long fb_addr, int fb_share_fd = -1);
    int aut_hwd_layer_render(unsigned int layer_hdl, void *fb_addr0, void *fb_addr1, int fb_share_fd = -1);
    int aut_hwd_layer_set_rect(unsigned int layer_hdl, struct view_info *src_rect);
    int aut_hwd_layer_set_zorder(unsigned int layer_hdl, int zorder);
    int aut_hwd_layer_open(unsigned int layer_hdl);
    int aut_hwd_layer_close(unsigned int layer_hdl);

    int aut_hwd_vsync_enable(int screen, int enable);

    int aut_hwd_layer_sufaceview(unsigned int layer_hdl, struct view_info *surface);
    int aut_hwd_layer_set_alpha(unsigned int layer_hdl, int alpha_mode, int alpha_value);
    int aut_hwd_set_layer_info(unsigned int layer_hdl, disp_layer_info2 *info);
    disp_layer_config2 *aut_hwd_get_layer_config_by_id(int screen_id, int channel, int layer_id);
    disp_layer_config2 *aut_hwd_get_layer_config(unsigned int layer_hdl);

    int getScreenWidth(int screen_id);
    int getScreenHeight(int screen_id);
    int getScreenOutputType(int screen_id);
    int getLayerFrameId(int screen_id, int channel, int layer_id);
    int getDispFd(void);

    bool CameraDispEnableFlag[LAYER_NUM];
    struct view_info Surface[LAYER_NUM];

    unsigned int layerObj[LAYER_NUM];
    int ScreenId[LAYER_NUM];
    int ChanelId[LAYER_NUM];
    int LayerId[LAYER_NUM];
    int Zorder[LAYER_NUM];

    int lcdxres;
    int lcdyres;

private:

    HwDisplay();
    HwDisplay(const HwDisplay &);
    ~HwDisplay();

    int hwd_init(void);
    int hwd_exit(void);

    int hwd_screen1_disable_output();
    int hwd_screen1_enable_output();

    static HwDisplay *sHwDisplay;
    int mDisp_fd;

    static bool mInitialized;
    static pthread_mutex_t sLock;

    disp_layer_config2 layer_cfg[SCREEN_NUM][CHANNEL_NUM][LAYER_NUM];
    int layer_stat[SCREEN_NUM][CHANNEL_NUM][LAYER_NUM];

    int cur_screen1_mode;
};
}

#ifdef  __cplusplus
}
#endif

#endif


