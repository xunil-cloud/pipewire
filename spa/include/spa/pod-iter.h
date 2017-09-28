/* Simple Plugin API
 * Copyright (C) 2017 Wim Taymans <wim.taymans@gmail.com>
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

#ifndef __SPA_POD_ITER_H__
#define __SPA_POD_ITER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

#include <spa/defs.h>
#include <spa/pod-utils.h>

struct spa_pod_iter {
	const void *data;
	uint32_t size;
	uint32_t offset;
};

static inline void spa_pod_iter_init(struct spa_pod_iter *iter, const void *data, uint32_t size, uint32_t offset)
{
	iter->data = data;
	iter->size = size;
	iter->offset = offset;
}

static inline struct spa_pod *spa_pod_iter_current(struct spa_pod_iter *iter)
{
	if (iter->offset + 8 <= iter->size) {
		struct spa_pod *pod = SPA_MEMBER(iter->data, iter->offset, struct spa_pod);
		if (SPA_POD_SIZE(pod) <= iter->size)
			return pod;
	}
	return NULL;
}

static inline void spa_pod_iter_advance(struct spa_pod_iter *iter, struct spa_pod *current)
{
	if (current)
		iter->offset += SPA_ROUND_UP_N(SPA_POD_SIZE(current), 8);
}

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __SPA_POD_H__ */
