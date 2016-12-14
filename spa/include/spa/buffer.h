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
} SpaMetaType;

/**
 * SpaMemType:
 * @SPA_MEM_TYPE_INVALID: invalid data, should be ignored
 * @SPA_MEM_TYPE_MEMPTR: data points to CPU accessible memory
 * @SPA_MEM_TYPE_MEMFD: fd is memfd, data can be mmapped
 * @SPA_MEM_TYPE_DMABUF: fd is dmabuf, data can be mmapped
 * @SPA_MEM_TYPE_ID: data is an id use SPA_PTR_TO_INT32. The definition of
 *          the ID is conveyed in some other way
 */
typedef enum {
  SPA_MEM_TYPE_INVALID               = 0,
  SPA_MEM_TYPE_MEMPTR,
  SPA_MEM_TYPE_MEMFD,
  SPA_MEM_TYPE_DMABUF,
  SPA_MEM_TYPE_ID,
} SpaMemType;

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
 * SpaMem:
 * @type: memory type
 * @flags: memory flags
 * @fd: file descriptor
 * @offset: offset in @fd
 * @size: size of the memory
 * @ptr: pointer to memory
 */
typedef struct {
  SpaMemType     type;
  int            flags;
  int            fd;
  off_t          offset;
  size_t         size;
  void          *ptr;
} SpaMem;

/**
 * SpaMemRef:
 * @mem_id: the SpaMem id
 * @offset: offset in @mem_id
 * @size: size in @mem_id
 * @ptr: pointer to mem
 *
 * A reference to a block of memory
 */
typedef struct {
  SpaMem      *mem;
  off_t        offset;
  size_t       size;
  void        *ptr;
} SpaMemRef;

/**
 * SpaMeta:
 * @type: metadata type
 * @mem: reference to the memory holding the metadata
 */
typedef struct {
  SpaMetaType  type;
  SpaMemRef    mem;
} SpaMeta;

#define SPA_META_MEM_TYPE(m)      ((m)->mem.mem->type)
#define SPA_META_MEM_FLAGS(m)     ((m)->mem.mem->flags)
#define SPA_META_MEM_FD(m)        ((m)->mem.mem->fd)
#define SPA_META_MEM_OFFSET(m)    ((m)->mem.mem->offset)
#define SPA_META_MEM_SIZE(m)      ((m)->mem.mem->size)
#define SPA_META_MEM_PTR(m)       ((m)->mem.mem->ptr)

#define SPA_META_MEMREF_MEM(m)    ((m)->mem.mem)
#define SPA_META_MEMREF_OFFSET(m) ((m)->mem.offset)
#define SPA_META_MEMREF_SIZE(m)   ((m)->mem.size)
#define SPA_META_MEMREF_PTR(m)    ((m)->mem.ptr)

#define SPA_META_TYPE(m)          ((m)->type)
#define SPA_META_PTR(m)           SPA_META_MEMREF_PTR(m)
#define SPA_META_SIZE(m)          SPA_META_MEMREF_SIZE(m)

typedef struct {
  off_t    offset;
  size_t   size;
  ssize_t  stride;
} SpaChunk;

typedef struct {
  SpaMem        *mem;
  off_t          offset;
  size_t         size;
  SpaChunk      *chunk;
} SpaChunkRef;

/**
 * SpaData:
 * @mem: reference to memory holding the data
 * @chunk: reference to valid chunk of memory
 */
typedef struct {
  SpaMemRef      mem;
  SpaChunkRef    chunk;
} SpaData;

#define SPA_DATA_MEM_TYPE(d)      ((d)->mem.mem->type)
#define SPA_DATA_MEM_FLAGS(d)     ((d)->mem.mem->flags)
#define SPA_DATA_MEM_FD(d)        ((d)->mem.mem->fd)
#define SPA_DATA_MEM_OFFSET(d)    ((d)->mem.mem->offset)
#define SPA_DATA_MEM_SIZE(d)      ((d)->mem.mem->size)
#define SPA_DATA_MEM_PTR(d)       ((d)->mem.mem->ptr)

#define SPA_DATA_MEMREF_MEM(d)    ((d)->mem.mem)
#define SPA_DATA_MEMREF_OFFSET(d) ((d)->mem.offset)
#define SPA_DATA_MEMREF_SIZE(d)   ((d)->mem.size)
#define SPA_DATA_MEMREF_PTR(d)    ((d)->mem.ptr)

#define SPA_DATA_CHUNK_OFFSET(d)  ((d)->chunk.chunk->offset)
#define SPA_DATA_CHUNK_SIZE(d)    ((d)->chunk.chunk->size)
#define SPA_DATA_CHUNK_STRIDE(d)  ((d)->chunk.chunk->stride)
#define SPA_DATA_CHUNK_PTR(d)     (SPA_DATA_MEMREF_PTR(d) + SPA_DATA_CHUNK_OFFSET(d))

/**
 * SpaBuffer:
 * @id: buffer id
 * @n_mems: number of mem
 * @mems: array of @n_mems memory blocks
 * @n_metas: number of metadata
 * @metas: array of @n_metas metadata
 * @n_datas: number of data pointers
 * @datas: array of @n_datas data pointers
 */
struct _SpaBuffer {
  uint32_t       id;
  unsigned int   n_metas;
  SpaMeta       *metas;
  unsigned int   n_datas;
  SpaData       *datas;
};

static inline void *
spa_buffer_find_meta (SpaBuffer *b, SpaMetaType type)
{
  unsigned int i;

  for (i = 0; i < b->n_metas; i++)
    if (b->metas[i].type == type)
      return b->metas[i].mem.ptr;
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
  };
  if (type <= 0 || type >= SPA_N_ELEMENTS (header_sizes))
    return 0;
  return header_sizes[type];
}

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __SPA_BUFFER_H__ */
