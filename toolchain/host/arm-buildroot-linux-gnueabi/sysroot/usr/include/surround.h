#ifndef _SURROUND_H_
#define _SURROUND_H_
void* process_init(void *handle, int samp_rate, int chn, int num_frame);
void surround_pro_in_out(void *handle, short* in, short* out, int datanum);
void process_exit(void *handle);
void set_space(void *handle, double space_gain);
void set_bass(void *handle, double sub_gain);
void set_defintion(void *handle, double defintion_gain);
#endif
