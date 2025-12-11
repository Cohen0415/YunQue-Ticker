#ifndef __DMA_UAPI__H___
#define __DMA_UAPI__H___

#include <linux/ioctl.h>
#include <linux/types.h>
#ifdef __cplusplus
extern "C" {
#endif
/**
 * DOC: DMABUF Heaps Userspace API
 */

/* Valid FD_FLAGS are O_CLOEXEC, O_RDONLY, O_WRONLY, O_RDWR */
#define DMA_HEAP_VALID_FD_FLAGS (O_CLOEXEC | O_ACCMODE)

/* Currently no heap flags */
#define DMA_HEAP_VALID_HEAP_FLAGS (0)

/**
 * struct dma_heap_allocation_data - metadata passed from userspace for
 *                                      allocations
 * @len:		size of the allocation
 * @fd:			will be populated with a fd which provides the
 *			handle to the allocated dma-buf
 * @fd_flags:		file descriptor flags used when allocating
 * @heap_flags:		flags passed to heap
 *
 * Provided by userspace as an argument to the ioctl
 */
struct dma_heap_allocation_data {
    __u64 len;
    __u32 fd;
    __u32 fd_flags;
    __u64 heap_flags;
};

typedef struct sunxi_drm_phys_data {
    int handle;
    unsigned int tee_addr;
    unsigned int phys_addr;
    unsigned int size;
} sunxi_drm_phys_data;


struct _dma_buf_sync {
    __u64 flags;
};

#define DMA_HEAP_IOC_MAGIC 'H'
/**
 * DOC: DMA_HEAP_IOC_ALLOC - allocate memory from pool
 *
 * Takes a dma_heap_allocation_data struct and returns it with the fd field
 * populated with the dmabuf handle of the allocation.
 */
#define DMA_HEAP_IOC_ALLOC _IOWR(DMA_HEAP_IOC_MAGIC, 0x0, struct dma_heap_allocation_data)
#define DMA_HEAP_GET_ADDR _IOWR(DMA_HEAP_IOC_MAGIC, 0x1, sunxi_drm_phys_data)

#define DMA_BUF_BASE 'b'
#define DMA_BUF_IOCTL_SYNC _IOW(DMA_BUF_BASE, 0, struct _dma_buf_sync)

#define DMA_BUF_SYNC_READ (1 << 0)
#define DMA_BUF_SYNC_WRITE (2 << 0)
#define DMA_BUF_SYNC_RW (DMA_BUF_SYNC_READ | DMA_BUF_SYNC_WRITE)
#define DMA_BUF_SYNC_START (0 << 2)
#define DMA_BUF_SYNC_END (1 << 2)
#define DMA_BUF_SYNC_VALID_FLAGS_MASK (DMA_BUF_SYNC_RW | DMA_BUF_SYNC_END)


int dma_open_dev();
void dma_close_dev();
int dma_alloc(int size);
// unsigned int getVirByfd(int dmafd);
// unsigned int getPhyByfd(int dmafd);
void dma_free(int fd);
int dma_sync(int dmafd);


#ifdef __cplusplus
}
#endif

#endif