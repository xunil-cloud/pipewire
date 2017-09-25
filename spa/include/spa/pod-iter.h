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

static inline bool spa_pod_iter_struct(struct spa_pod_iter *iter, const void *data, uint32_t size)
{
	if (data == NULL || size < 8 || SPA_POD_SIZE(data) > size
	    || SPA_POD_TYPE(data) != SPA_POD_TYPE_STRUCT)
		return false;

	spa_pod_iter_init(iter, data, size, sizeof(struct spa_pod_struct));
	return true;
}

static inline bool spa_pod_iter_map(struct spa_pod_iter *iter, const void *data, uint32_t size)
{
	if (data == NULL || size < 8 || SPA_POD_SIZE(data) > size
	    || SPA_POD_TYPE(data) != SPA_POD_TYPE_MAP)
		return false;

	spa_pod_iter_init(iter, data, size, sizeof(struct spa_pod_map));
	return true;
}

static inline bool spa_pod_iter_object(struct spa_pod_iter *iter, const void *data, uint32_t size)
{
	if (data == NULL || SPA_POD_SIZE(data) > size || SPA_POD_TYPE(data) != SPA_POD_TYPE_OBJECT)
		return false;

	iter->data = data;
	iter->size = size;
	iter->offset = sizeof(struct spa_pod_object);
	return true;
}

static inline bool spa_pod_iter_pod(struct spa_pod_iter *iter, struct spa_pod *pod)
{
	uint32_t offset;

	switch (SPA_POD_TYPE(pod)) {
	case SPA_POD_TYPE_STRUCT:
		offset = sizeof(struct spa_pod_struct);
		break;
	case SPA_POD_TYPE_MAP:
		offset = sizeof(struct spa_pod_map);
		break;
	case SPA_POD_TYPE_OBJECT:
		offset = sizeof(struct spa_pod_object);
		break;
	default:
		spa_pod_iter_init(iter, NULL, 0, 0);
		return false;
	}
	spa_pod_iter_init(iter, pod, SPA_POD_SIZE(pod), offset);
	return true;
}

static inline bool spa_pod_iter_has_next(struct spa_pod_iter *iter)
{
	return (iter->offset + 8 <= iter->size &&
		SPA_POD_SIZE(SPA_MEMBER(iter->data, iter->offset, struct spa_pod)) <= iter->size);
}

static inline struct spa_pod *spa_pod_iter_next(struct spa_pod_iter *iter)
{
	struct spa_pod *res = SPA_MEMBER(iter->data, iter->offset, struct spa_pod);
	iter->offset += SPA_ROUND_UP_N(SPA_POD_SIZE(res), 8);
	return res;
}

static inline struct spa_pod *spa_pod_iter_first(struct spa_pod_iter *iter, struct spa_pod *pod)
{
	if (!spa_pod_iter_pod(iter, pod) || !spa_pod_iter_has_next(iter))
		return NULL;
	return spa_pod_iter_next(iter);
}

static inline bool spa_pod_iter_getv(struct spa_pod_iter *iter, uint32_t type, va_list args)
{
	bool res = true;

	while (type && (res = spa_pod_iter_has_next(iter))) {
		struct spa_pod *pod = spa_pod_iter_next(iter);

		SPA_POD_COLLECT(pod, type, args, error);

		type = va_arg(args, uint32_t);
	}
	return res;
      error:
	return false;
}

static inline bool spa_pod_iter_get(struct spa_pod_iter *iter, uint32_t type, ...)
{
	va_list args;
	bool res;

	va_start(args, type);
	res = spa_pod_iter_getv(iter, type, args);
	va_end(args);

	return res;
}

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __SPA_POD_H__ */
