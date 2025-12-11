#ifndef _WATER_MARK_INTERFACE_H
#define _WATER_MARK_INTERFACE_H

#define MAX_WATERMARK_NUM 5
#define MAX_WATERMARK_LENGTH 20

typedef struct {
    int mPositionX;
    int mPositionY;
    char content[MAX_WATERMARK_LENGTH];
} SingleWaterMark;

typedef struct {
    int mWaterMarkNum;
    SingleWaterMark mSingleWaterMark[MAX_WATERMARK_NUM];
} WaterMarkMultiple;

typedef struct {
    unsigned char *y;
    unsigned char *c;
    int posx;
    int posy;
    int width;
    int height;
    float resolution_rate;
    char *display;
} WaterMarkInData;

int doWaterMark(WaterMarkInData *indata, void *ctrl);
int doWaterMarkMultiple(WaterMarkInData *indata, void *ctrl, void *multi, char *content, char *time_watermark);
void *initialWaterMark(int wm_height);
void *initWaterMarkMultiple();
int releaseWaterMark(void *ctrl);
int releaseWaterMarkMultiple(void *multi);

#endif /* _WATER_MARK_INTERFACE_H */
