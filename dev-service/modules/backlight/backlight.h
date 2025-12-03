#ifndef BACKLIGHT_H
#define BACKLIGHT_H

#define BRIGHTNESS_PATH "/sys/class/backlight/backlight/brightness"

int backlight_cmd_register(void);

#endif // BACKLIGHT_H