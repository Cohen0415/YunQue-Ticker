#ifndef SYSINFO_H
#define SYSINFO_H

#define CPU_TEMP_PATH   "/sys/class/thermal/thermal_zone0/temp"
#define VER_PATH        "/proc/cpuinfo"

int sysinfo_cmd_register(void);

#endif // SYSINFO_H
