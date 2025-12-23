#ifndef BACKLIGHT_H
#define BACKLIGHT_H

#define BRIGHTNESS_PATH             "/sys/class/backlight/backlight/brightness"

#define MAX_BRIGHTNESS_LOGIC        100
#define MIN_BRIGHTNESS_LOGIC        0
#define MAX_BRIGHTNESS_ACTUAL       255
#define MIN_BRIGHTNESS_ACTUAL       0

int backlight_cmd_register(void);

#endif // BACKLIGHT_H