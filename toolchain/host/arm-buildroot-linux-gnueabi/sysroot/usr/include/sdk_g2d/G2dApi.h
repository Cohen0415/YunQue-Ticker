#ifndef __G2DAPI_H__
#define __G2DAPI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/types.h>
#include <linux/g2d_driver_enh.h>

namespace g2dapi
{

// typedef struct RECT {
//     int left;
//     int top;
//     int width;            // preview width
//     int height;           // preview height
// } stRECT;

typedef enum {
    G2D_ROTATE90,
    G2D_ROTATE180,
    G2D_ROTATE270,
    G2D_FLIP_HORIZONTAL,
    G2D_FLIP_VERTICAL,
    G2D_MIRROR45,
    G2D_MIRROR135,
} g2dRotateAngle;

// typedef struct {
//     g2d_fmt_enh     format;
//     __u32           width;
//     __u32           height;
//     __u32           align[3];
//     stRECT          clip_rect;
//     __u8            alpha;
//     g2d_alpha_mode_enh   mode;
//     int             fd;
//     __u32           use_phy_addr;
// } g2d_image_user;


int g2dInit();
int g2dUnit(int g2dHandle);
int g2dClip(int g2dHandle, void *psrc, int src_w, int src_h, int src_x, int src_y, int width, int height, void *pdst, int dst_w, int dst_h, int dst_x, int dst_y);
int g2dScale(int g2dHandle, unsigned char *src, int src_width, int src_height, unsigned char *dst, int dst_width, int dst_height);
int g2dRotate(int g2dHandle, g2dRotateAngle angle, unsigned char *src, int src_width, int src_height, unsigned char *dst, int dst_width, int dst_height);

int g2dFormatConv(int g2dHandle, void *psrc, int src_format, int src_w, int src_h,
                  int src_crop_x, int src_crop_y, int src_crop_w, int src_crop_h,
                  void *pdst, int dst_format, int dst_w, int dst_h,
                  int dst_crop_x, int dst_crop_y, int dst_crop_w, int dst_crop_h);

int g2dClipByFd(int g2dHandle, int inFd, int flag, int infmt, int src_w, int src_h, int src_x, int src_y, int src_width, int src_height,
                int outFd, int outfmt, int dst_w, int dst_h, int dst_x, int dst_y, int dst_width, int dst_height);


int g2d_clip(int devfd, int src_fd, int width, int height, int src_x, int src_y, int src_w, int src_h, int dst_fd, int dst_x, int dst_y, int dst_w, int dst_h, g2d_fmt_enh pix_fmt = G2D_FORMAT_YUV420UVC_U1V1U0V0);
int g2d_xCompose(int devfd, int src_w, int src_h, int src_fd1, int src_fd2, int src_fd3, int src_fd4, int dst_fd, g2d_fmt_enh pix_fmt = G2D_FORMAT_YUV420UVC_U1V1U0V0);

}

#ifdef __cplusplus
}
#endif

#endif
