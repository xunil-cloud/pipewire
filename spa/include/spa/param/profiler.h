/* Simple Plugin API
 *
 * Copyright © 2020 Wim Taymans
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef SPA_PARAM_PROFILER_H
#define SPA_PARAM_PROFILER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <spa/param/param.h>

/** properties for SPA_TYPE_OBJECT_Profiler */
enum spa_profiler {
	SPA_PROFILER_START,

	SPA_PROFILER_START_Driver	= 0x10000,	/**< driver related profiler properties */
	SPA_PROFILER_info,				/**< Generic info, counter and CPU load */
	SPA_PROFILER_clock,				/**< clock information */
	SPA_PROFILER_driverBlock,			/**< generic driver info block */

	SPA_PROFILER_START_Follower	= 0x20000,	/**< follower related profiler properties */
	SPA_PROFILER_followerBlock,			/**< generic follower info block */

	SPA_PROFILER_START_CUSTOM	= 0x1000000,
};

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* SPA_PARAM_PROFILER_H */
