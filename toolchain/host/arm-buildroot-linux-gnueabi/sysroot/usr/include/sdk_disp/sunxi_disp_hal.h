#ifndef _SUNXI_DISP_HAL_H
#define _SUNXI_DISP_HAL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <sdk_disp/sunxi_display2.h>

#define SCREEN_0 0
#define SCREEN_1 1
#define SCREEN_2 2

#define SCREEN_NUM SCREEN_2
#define LAYER_NUM 4
#define CHANNEL_NUM 4

//for video layer
#define FB_VIDEO_CHANNEL_ID 0
#define FB_VIDEO_LAYER_ID 0

//for ui layer
#define FB_UI_CHANNEL_ID 1
#define FB_UI_LAYER_ID 0

#define DISP_DEV "/dev/disp"

// layer info compose and decompose
#define COMP_LAYER_HDL(s, c, l) (((s & 0xf) << 16) | ((c & 0xf) << 8) | (l & 0xf))
#define HDL_2_SCREENID(hdl) ((hdl >> 16) & 0xf)
#define HDL_2_CHANNELID(hdl) ((hdl >> 8) & 0xf)
#define HDL_2_LAYERID(hdl) ((hdl) & 0xf)

struct sdk_disp_layer_config {
    disp_layer_config2 layer_cfg2;
    int layer_status;
};

struct fb_src_info {
    unsigned int width;             // fb size width
    unsigned int height;            // fb size height
    enum disp_pixel_format format;  // fb size format
};

class SunxiDisplay
{
public:
    const SunxiDisplay &operator=(const SunxiDisplay &);
    static SunxiDisplay *get_instance();

    // 申请layer函数，获取该layer_config2属性
    int request_layer(int screen_id, int channel, int layer_id);

    // 释放layer_config2函数，关闭layer图层并初始化该layer的全局变量属性
    int release_layer(int layer_hdl);

    // 使能layer图层，根据fd与phy_addr动态选择set_config还是set_config2
    int enable_layer(int layer_hdl);

    // 关闭layer图层，根据fd与phy_addr动态选择set_config还是set_config2
    int disable_layer(int layer_hdl);

    // 设置layer的zorder
    int set_layer_zorder(int layer_hdl, int zorder);

    // 设置layer的alpha_mode
    int set_layer_alpha(int layer_hdl, int alpha_mode, int alpha_value);

    // 设置layer的surface信息
    int set_layer_frame(int layer_hdl, struct disp_rect *frame);

    // 设置layer的fb addr信息
    int set_layer_fb_addr(int layer_hdl, struct fb_src_info *fb_src, int fb_fd);

    // 设置layer的fb crop信息
    int set_layer_fb_crop(int layer_hdl, struct disp_rect64 *fb_crop);

    // 通过hdl获取layer的disp_layer_info2信息
    int get_layer_info2(int layer_hdl, struct disp_layer_info2 *layer_info2);

    // 通过id获取layer的disp_layer_info2信息
    int get_layer_info2_by_id(int screen_id, int channel, int layer_id, struct disp_layer_info2 *layer_info2);

    // 设置layer的layer_config2信息
    int set_layer_info2(int layer_hdl, struct disp_layer_info2 *layer_info2);

    // 设置显示屏的vsync状态
    int enable_disp_vsync(int screen_id, int enable);

    // 获取显示屏的显示宽度
    int get_screen_width(int screen_id);

    // 获取显示屏的显示高度
    int get_screen_height(int screen_id);

    // 获取显示屏的输出类型
    int get_screen_output_type(int screen_id);

    // 切换显示屏的输出模式
    int switch_output_type(int screen_id, int output_type, int output_mode);

    // 设置lcd亮度，仅pwm模式有效
    int set_lcd_brightness(int bright_value);

    // 获取lcd亮度，仅pwm模式有效
    int get_lcd_brightness(void);

    // 打开/关闭lcd背光，仅pwm模式有效
    int set_lcd_backlight(bool enable);

    // 打印当前layer的属性
    void print_sdk_layer_cfg(int layer_hdl);

private:

    SunxiDisplay();
    ~SunxiDisplay();

    static SunxiDisplay *m_disp;
    static pthread_mutex_t p_mutex_t;

    int disp_fd;
    int fb0_width;
    int fb0_height;

    sdk_disp_layer_config sdk_layer_cfg[SCREEN_NUM][CHANNEL_NUM][LAYER_NUM];
};

#ifdef  __cplusplus
}
#endif

#endif // _SUNXI_DISP_HAL_H
