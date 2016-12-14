/* Simple Plugin API
 * Copyright (C) 2016 Wim Taymans <wim.taymans@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __SPA_BUFFER_H__
#define __SPA_BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _SpaBuffer SpaBuffer;

#define SPA_BUFFER_URI             "http://spaplug.in/ns/buffer"
#define SPA_BUFFER_PREFIX          SPA_BUFFER_URI "#"

/**
 * SpaMetaType:
 * @SPA_META_TYPE_INVALID: invalid metadata, should be ignored
 * @SPA_META_TYPE_HEADER: header metadata
 * @SPA_META_TYPE_POINTER: a generic pointer
 * @SPA_META_TYPE_VIDEO_CROP: video cropping region
 * @SPA_META_TYPE_RINGBUFFER: a ringbuffer
 */
typedef enum {
  SPA_META_TYPE_INVALID               = 0,
  SPA_META_TYPE_HEADER,
  SPA_META_TYPE_POINTER,
  SPA_META_TYPE_VIDEO_CROP,
  SPA_META_TYPE_RINGBUFFER,
  SPA_META_TYPE_SHARED,
} SpaMetaType;

/**
 * SpaMemType:
 * @SPA_MEM_TYPE_INVALID: invalid ptr, should be ignored
 * @SPA_MEM_TYPE_MEMPTR: ptr points to CPU accessible memory
 * @SPA_MEM_TYPE_MEMFD: fd is memfd, ptr can be mmapped
 * @SPA_MEM_TYPE_DMABUF: fd is dmabuf, ptr can be mmapped
 * @SPA_MEM_TYPE_ID: ptr is an id use SPA_PTR_TO_INT32. The definition of
 *          the ID is conveyed in some other way
 */
typedef enum {
  SPA_MEM_TYPE_INVALID               = 0,
  SPA_MEM_TYPE_MEMPTR,
  SPA_MEM_TYPE_MEMFD,
  SPA_MEM_TYPE_DMABUF,
  SPA_MEM_TYPE_ID,
} SpaMemType;

/**
 * SpaMemFlag:
 * @SPA_MEM_FLAG_NONE: no flags
 * @SPA_MEM_FLAG_READ: mem is readable
 * @SPA_MEM_FLAG_WRITE: mem is writable
 */
typedef enum {
  SPA_MEM_FLAG_NONE             = 0,
  SPA_MEM_FLAG_READ             = (1 << 0),
  SPA_MEM_FLAG_WRITE            = (1 << 1),
} SpaMemFlag;

#include <spa/defs.h>
#include <spa/port.h>
#include <spa/ringbuffer.h>

/**
 * SpaBufferFlags:
 * @SPA_BUFFER_FLAG_NONE: no flag
 * @SPA_BUFFER_FLAG_DISCONT: the buffer marks a data discontinuity
 * @SPA_BUFFER_FLAG_CORRUPTED: the buffer data might be corrupted
 * @SPA_BUFFER_FLAG_MARKER: the buffer contains a media specific marker
 * @SPA_BUFFER_FLAG_HEADER: the buffer contains a header
 * @SPA_BUFFER_FLAG_GAP: the buffer has been constructed to fill a gap
 *                       and contains media neutral data
 * @SPA_BUFFER_FLAG_DELTA_UNIT: the media cannot be decoded independently
 */
typedef enum {
  SPA_BUFFER_FLAG_NONE               =  0,
  SPA_BUFFER_FLAG_DISCONT            = (1 << 0),
  SPA_BUFFER_FLAG_CORRUPTED          = (1 << 1),
  SPA_BUFFER_FLAG_MARKER             = (1 << 2),
  SPA_BUFFER_FLAG_HEADER             = (1 << 3),
  SPA_BUFFER_FLAG_GAP                = (1 << 4),
  SPA_BUFFER_FLAG_DELTA_UNIT         = (1 << 5),
} SpaBufferFlags;

typedef struct {
  SpaBufferFlags flags;
  uint32_t seq;
  int64_t pts;
  int64_t dts_offset;
} SpaMetaHeader;

typedef struct {
  const char *ptr_type;
  void       *ptr;
} SpaMetaPointer;

/**
 * SpaMetaVideoCrop:
 * @x:
 * @y:
 * @width:
 * @height
 */
typedef struct {
  int   x, y;
  int   width, height;
} SpaMetaVideoCrop;

/**
 * SpaMetaRingbuffer:
 * @ringbuffer:
 */
typedef struct {
  SpaRingbuffer ringbuffer;
} SpaMetaRingbuffer;

/**
 * SpaMeta:
 * @type: metadata type
 * @data: pointer to metadata
 * @size: size of metadata
 */
typedef struct {
  SpaMetaType  type;
  void        *data;
  size_t       size;
} SpaMeta;

/**
 * SpaMemArea:
 * @offset: offset of start of data
 * @size: size of the data
 * @stride: stride of data
 *
 * The valid data in a data block. Offset and size are clamped
 * to the maxsize of the data block.
 */
typedef struct {
  off_t          offset;
  size_t         size;
  ssize_t        stride;
} SpaMemChunk;

/**
 * SpaMem:
 * @type: memory type
 * @flags: memory flags
 * @fd: file descriptor, can be -1
 * @offset: offset in @fd
 * @size: maximum size of the memory
 * @ptr: pointer to memory, can be NULL
 * @chunk: valid memory chunk
 *
 * A memory block.
 */
typedef struct {
  SpaMemType    type;
  SpaMemFlag    flags;
  int           fd;
  off_t         offset;
  size_t        size;
  void         *ptr;
} SpaMem;

typedef struct {
  SpaMem        mem;
  SpaMemChunk  *chunk;
} SpaMemRef;

/**
 * SpaBuffer:
 * @id: buffer id
 * @n_metas: number of metadata
 * @metas: array of @n_metas metadata
 * @n_mems: number of memory blocks
 * @mems: array of @n_mems memory blocks
 */
struct _SpaBuffer {
  uint32_t       id;
  unsigned int   n_metas;
  SpaMeta       *metas;
  unsigned int   n_mems;
  SpaMemRef     *mems;
};

typedef struct {
  SpaMem   meta_mem;
  SpaMem   chunk_mem;
} SpaMetaShared;

#define SPA_BUFFER_MEM_TYPE(b,n)   ((b)->mems[n].mem.type)
#define SPA_BUFFER_MEM_FLAGS(b,n)  ((b)->mems[n].mem.flags)
#define SPA_BUFFER_MEM_FD(b,n)     ((b)->mems[n].mem.fd)
#define SPA_BUFFER_MEM_OFFSET(b,n) ((b)->mems[n].mem.offset)
#define SPA_BUFFER_MEM_SIZE(b,n)   ((b)->mems[n].mem.size)
#define SPA_BUFFER_MEM_PTR(b,n)    ((b)->mems[n].mem.ptr)

#define SPA_BUFFER_OFFSET(b,n)     ((b)->mems[n].chunk->offset)
#define SPA_BUFFER_SIZE(b,n)       ((b)->mems[n].chunk->size)
#define SPA_BUFFER_STRIDE(b,n)     ((b)->mems[n].chunk->stride)
#define SPA_BUFFER_DATA(b,n)       (SPA_BUFFER_MEM_PTR (b,n) + SPA_BUFFER_OFFSET (b,n))

static inline void *
spa_buffer_find_meta (SpaBuffer *b, SpaMetaType type)
{
  unsigned int i;

  for (i = 0; i < b->n_metas; i++)
    if (b->metas[i].type == type)
      return b->metas[i].data;
  return NULL;
}

static inline size_t
spa_meta_type_get_size (SpaMetaType  type)
{
  static const size_t header_sizes[] = {
    0,
    sizeof (SpaMetaHeader),
    sizeof (SpaMetaPointer),
    sizeof (SpaMetaVideoCrop),
    sizeof (SpaMetaRingbuffer),
    sizeof (SpaMetaShared),
  };
  if (type <= 0 || type >= SPA_N_ELEMENTS (header_sizes))
    return 0;
  return header_sizes[type];
}

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __SPA_BUFFER_H__ */
