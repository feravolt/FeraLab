#ifndef __GSL_RINGBUFFER_H
#define __GSL_RINGBUFFER_H
#include <linux/msm_kgsl.h>
#include <linux/mutex.h>
#include "kgsl_log.h"
#include "kgsl_sharedmem.h"
#include "yamato_reg.h"
#define GSL_STATS_RINGBUFFER
#define GSL_RB_USE_MEM_RPTR
#define GSL_RB_USE_MEM_TIMESTAMP
#define GSL_DEVICE_SHADOW_MEMSTORE_TO_USER

#define GSL_RB_SIZE_8	 	0
#define GSL_RB_SIZE_16		1
#define GSL_RB_SIZE_32		2
#define GSL_RB_SIZE_64		3
#define GSL_RB_SIZE_128		4
#define GSL_RB_SIZE_256		5
#define GSL_RB_SIZE_512		6
#define GSL_RB_SIZE_1K  	7
#define GSL_RB_SIZE_2K  	8
#define GSL_RB_SIZE_4K  	9
#define GSL_RB_SIZE_8K  	10
#define GSL_RB_SIZE_16K 	11
#define GSL_RB_SIZE_32K 	12
#define GSL_RB_SIZE_64K 	13
#define GSL_RB_SIZE_128K	14
#define GSL_RB_SIZE_256K	15
#define GSL_RB_SIZE_512K	16
#define GSL_RB_SIZE_1M		17
#define GSL_RB_SIZE_2M		18
#define GSL_RB_SIZE_4M		19
#define	REG_CP_TIMESTAMP	REG_SCRATCH_REG0

static const unsigned int kgsl_cfg_rb_sizelog2quadwords = GSL_RB_SIZE_32K;
static const unsigned int kgsl_cfg_rb_blksizequadwords  = GSL_RB_SIZE_16;

struct kgsl_device;
struct kgsl_device_private;
struct kgsl_drawctxt;
struct kgsl_ringbuffer;

#define GSL_RB_MEMPTRS_SCRATCH_COUNT	 8
struct kgsl_rbmemptrs {
	volatile int  rptr;
	volatile int  wptr_poll;
};

#define GSL_RB_MEMPTRS_RPTR_OFFSET \
	(offsetof(struct kgsl_rbmemptrs, rptr))

#define GSL_RB_MEMPTRS_WPTRPOLL_OFFSET \
	(offsetof(struct kgsl_rbmemptrs, wptr_poll))

struct kgsl_rbstats {
	int64_t issues;
	int64_t words_total;
};


struct kgsl_ringbuffer {
	struct kgsl_device *device;
	uint32_t flags;

	struct kgsl_memdesc buffer_desc;

	struct kgsl_memdesc memptrs_desc;
	struct kgsl_rbmemptrs *memptrs;

	/*ringbuffer size */
	unsigned int sizedwords;
	unsigned int blksizequadwords;

	unsigned int wptr; /* write pointer offset in dwords from baseaddr */
	unsigned int rptr; /* read pointer offset in dwords from baseaddr */
	uint32_t timestamp;
	struct list_head memqueue;

#ifdef GSL_STATS_RINGBUFFER
	struct kgsl_rbstats stats;
#endif

};

#define GSL_HAL_SUBBLOCK_OFFSET(reg) ((unsigned int)((reg) - (0x2000)))
#define GSL_RB_WRITE(ring, data) \
	do { \
		mb(); \
		writel(data, ring); \
		ring++; \
	} while (0)

#ifdef GSL_DEVICE_SHADOW_MEMSTORE_TO_USER
#define GSL_RB_USE_MEM_TIMESTAMP
#endif

#ifdef GSL_RB_USE_MEM_TIMESTAMP
#define GSL_RB_MEMPTRS_SCRATCH_MASK 0x1
#define GSL_RB_INIT_TIMESTAMP(rb)

#else
#define GSL_RB_MEMPTRS_SCRATCH_MASK 0x0
#define GSL_RB_INIT_TIMESTAMP(rb) \
		kgsl_yamato_regwrite((rb)->device->id, REG_CP_TIMESTAMP, 0)
#endif

#ifdef GSL_RB_USE_MEM_RPTR
#define GSL_RB_CNTL_NO_UPDATE 0x0
#define GSL_RB_GET_READPTR(rb, data) \
	do { \
		*(data) = (rb)->memptrs->rptr; \
	} while (0)
#else
#define GSL_RB_CNTL_NO_UPDATE 0x1
#define GSL_RB_GET_READPTR(rb, data) \
	do { \
		kgsl_yamato_regread((rb)->device->id, REG_CP_RB_RPTR, (data)); \
	} while (0)
#endif

#ifdef GSL_RB_USE_WPTR_POLLING
#define GSL_RB_CNTL_POLL_EN 0x1
#define GSL_RB_UPDATE_WPTR_POLLING(rb) \
	do { (rb)->memptrs->wptr_poll = (rb)->wptr; } while (0)
#else
#define GSL_RB_CNTL_POLL_EN 0x0
#define GSL_RB_UPDATE_WPTR_POLLING(rb)
#endif

#ifdef GSL_STATS_RINGBUFFER
#define GSL_RB_STATS(x) x
#else
#define GSL_RB_STATS(x)
#endif

struct kgsl_mem_entry;

int kgsl_ringbuffer_issueibcmds(struct kgsl_device_private *dev_priv,
				int drawctxt_index,
				struct kgsl_ibdesc *ibdesc, unsigned int numibs,
				uint32_t *timestamp,
				unsigned int flags);

int kgsl_ringbuffer_init(struct kgsl_device *device);
int kgsl_ringbuffer_start(struct kgsl_ringbuffer *rb);
int kgsl_ringbuffer_stop(struct kgsl_ringbuffer *rb);
int kgsl_ringbuffer_close(struct kgsl_ringbuffer *rb);

void kgsl_ringbuffer_issuecmds(struct kgsl_device *device,
					unsigned int flags,
					unsigned int *cmdaddr,
					int sizedwords);

int kgsl_ringbuffer_gettimestampshadow(struct kgsl_device *device,
					unsigned int *sopaddr,
					unsigned int *eopaddr);

void kgsl_cp_intrcallback(struct kgsl_device *device);
#endif
