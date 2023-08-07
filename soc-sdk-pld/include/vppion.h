/*
 * drivers/staging/android/uapi/ion.h
 *
 * Copyright (C) 2011 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _LINUX_ION_H
#define _LINUX_ION_H

#include <linux/ioctl.h>
#include <linux/types.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * enum ion_heap_types - list of all possible types of heaps
 * @ION_HEAP_TYPE_SYSTEM_N:	 memory allocated via vmalloc
 * @ION_HEAP_TYPE_SYSTEM_CONTIG_N: memory allocated via kmalloc
 * @ION_HEAP_TYPE_CARVEOUT_N:	 memory allocated from a prereserved
 *				 carveout heap, allocations are physically
 *				 contiguous
 * @ION_HEAP_TYPE_DMA_N:		 memory allocated via DMA API
 * @ION_NUM_HEAPS_N:		 helper for iterating over heaps, a bit mask
 *				 is used to identify the heaps, so only 32
 *				 total heap types are supported
 */
enum ion_heap_type_n {
	ION_HEAP_TYPE_SYSTEM_N,
	ION_HEAP_TYPE_SYSTEM_CONTIG_N,
	ION_HEAP_TYPE_CARVEOUT_N,
	ION_HEAP_TYPE_CHUNK_N,
	ION_HEAP_TYPE_DMA_N,
	ION_HEAP_TYPE_CUSTOM_N, /*
			       * must be last so device specific heaps always
			       * are at the end of this enum
			       */
};

#define ION_NUM_HEAP_IDS		(sizeof(unsigned int) * 8)

/**
 * allocation flags - the lower 16 bits are used by core ion, the upper 16
 * bits are reserved for use by the heaps themselves.
 */

/*
 * mappings of this buffer should be cached, ion will do cache maintenance
 * when the buffer is mapped for dma
 */
#define ION_FLAG_CACHED 1

/**
 * DOC: Ion Userspace API
 *
 * create a client by opening /dev/ion
 * most operations handled via following ioctls
 *
 */

/**
 * struct ion_allocation_data-n - metadata passed from userspace for allocations
 * @len:		size of the allocation
 * @heap_id_mask:	mask of heap ids to allocate from
 * @flags:		flags passed to heap
 * @handle:		pointer that will be populated with a cookie to use to
 *			refer to this allocation
 *
 * Provided by userspace as an argument to the ioctl
 */
struct ion_allocation_data_n {
	__u64 len;
	__u32 heap_id_mask;
	__u32 flags;
	__u32 fd;
	__u32 unused;
	__u64 paddr;
};

#define MAX_HEAP_NAME			32

/**
 * struct ion_heap_data_n - data about a heap
 * @name - first 32 characters of the heap name
 * @type - heap type
 * @heap_id - heap id for the heap
 */
struct ion_heap_data_n {
	char name[MAX_HEAP_NAME];
	__u32 type;
	__u32 heap_id;
	__u32 reserved0;
	__u32 reserved1;
	__u32 reserved2;
};

/**
 * struct ion_heap_query_n - collection of data about all heaps
 * @cnt - total number of heaps to be copied
 * @heaps - buffer to copy heap data
 */
struct ion_heap_query_n {
	__u32 cnt; /* Total number of heaps to be copied */
	__u32 reserved0; /* align to 64bits */
	__u64 heaps; /* buffer to be populated */
	__u32 reserved1;
	__u32 reserved2;
};

/**
 * struct ion_custom_data - metadata passed to/from userspace for a custom ioctl
 * @cmd:	the custom ioctl function to call
 * @arg:	additional data to pass to the custom ioctl, typically a user
 *		pointer to a predefined structure
 *
 * This works just like the regular cmd and arg fields of an ioctl.
 */
struct ion_custom_data {
	unsigned int cmd;
	unsigned long arg;
};

typedef struct _ion_dev_fd_
{
    int dev_fd;/*ion dev fd*/
    char name[4];/*if u fill in the value of ion_dev_fd,u must fill in ion_dev_fd name as ion*/
}ion_dev_fd_s;

typedef struct ion_para_s {
	unsigned long length;
	unsigned long pa;
	int memFd;
	ion_dev_fd_s ionDevFd;
	void* va;
}ion_para;

struct bitmain_cache_range {
	void *start;
	size_t size;
};

struct bitmain_heap_info {
	unsigned int id;
	unsigned long total_size;
	unsigned long avail_size;
};

#define ION_IOC_BITMAIN_FLUSH_RANGE             1
#define ION_IOC_BITMAIN_GET_HEAP_INFO           2
#define ION_IOC_BITMAIN_INVALIDATE_RANGE             3

#define ION_IOC_MAGIC		'I'

/**
 * DOC: ION_IOC_ALLOC - allocate memory
 *
 * Takes an ion_allocation_data_n struct and returns it with the handle field
 * populated with the opaque handle for the allocation.
 */
#define ION_IOC_ALLOC_N		_IOWR(ION_IOC_MAGIC, 0, \
				      struct ion_allocation_data_n)

/**
 * DOC: ION_IOC_CUSTOM - call architecture specific ion ioctl
 *
 * Takes the argument of the architecture specific ioctl to call and
 * passes appropriate userdata for that ioctl
 */
#define ION_IOC_CUSTOM		_IOWR(ION_IOC_MAGIC, 6, struct ion_custom_data)

/**
 * DOC: ION_IOC_HEAP_QUERY - information about available heaps
 *
 * Takes an ion_heap_query_n structure and populates information about
 * available Ion heaps.
 */
#define ION_IOC_HEAP_QUERY_N     _IOWR(ION_IOC_MAGIC, 8, \
					struct ion_heap_query_n)

#define VppIonErr(Str) fprintf( stderr, "%s\n", Str ), exit( 1 )
#define VppIonAssert(cond) do { if (!(cond)) { printf("%s = %d, abort\n", __FUNCTION__, __LINE__); abort(); } } while (0)

void* ionMalloc(int devFd, ion_para *para);
void ionFree(ion_para *para);
void ion_flush(const ion_dev_fd_s *ion_dev_fd, void * va, int va_len);
void ion_invalidate(const ion_dev_fd_s *ion_dev_fd, void * va, int va_len);

#if defined(__cplusplus)
}
#endif

#endif /* _LINUX_ION_H */
