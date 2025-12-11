//AWMemory.h
#ifndef _AW_MEMORY_H_
#define _AW_MEMORY_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
extern int open_cedar_dev();
extern int close_cedar_dev();
extern unsigned long ion_get_viraddr_from_fd(int fd, int size);
extern unsigned long ion_get_phyaddr_from_fd(int fd);
extern int ion_return_phyaddr(int fd, unsigned long phy_addr);
extern int ion_return_viraddr(unsigned long viraddr, int size);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif //_AW_MEMORY_H_
