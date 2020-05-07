/* PipeWire
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

#ifndef PIPEWIRE_OT_H
#define PIPEWIRE_OT_H

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif
#include <inttypes.h>

enum ot_type {
	OT_NULL,
	OT_BOOL,
	OT_NUMBER,
	OT_STRING,
	OT_ARRAY,
	OT_OBJECT,
};

struct ot_string {
	int32_t len;
	const char *val;
};

union ot_val {
	bool b;
	double d;
	struct ot_string s;

	int32_t i;
	int64_t l;
	const void *cp;
	void *p;
};

struct ot_key {
	int32_t index;
	struct ot_string k;
};

#define OT_INIT_KEYN(_i,_k,_kl)	(struct ot_key) { .index = _i, .k.val = _k, .k.len = _kl }
#define OT_INIT_KEY(_i,_k)	OT_INIT_KEYN(_i,_k,-1)

struct ot_node {
	enum ot_type type;

#define NODE_FLAG_FLAT		(1<<0)	/**< suggest flat display */
#define NODE_FLAG_EXPENSIVE	(1<<1)	/**< expensive container to enter */
#define NODE_FLAG_INT		(1<<2)	/**< number is integer */
#define NODE_FLAG_NO_KEY	(1<<3)	/**< don't print key */
#define NODE_FLAG_MULTI		(1<<4)	/**< node contains multiple values */
	uint32_t flags;

	struct ot_key key;		/**< key of this node */
	union ot_val v;			/**< value of node */

	struct ot_node *parent;

	int (*iterate) (struct ot_node *node, const struct ot_key *key, struct ot_node *sub);

	/* private state */
	union ot_val extra[8];
	char buffer[64];
};

#define OT_INIT_NULLN(_x,_k,_kl)		(struct ot_node){ .type = OT_NULL, .key = { _x, { _kl, _k } } }
#define OT_INIT_BOOLN(_x,_k,_kl,_v)		(struct ot_node){ .type = OT_BOOL, .key = { _x, { _kl, _k } }, .v.b = _v }
#define OT_INIT_NUMBERNI(_x,_k,_kl,_v)		(struct ot_node){ .type = OT_NUMBER, .flags = NODE_FLAG_INT, .key = { _x, { _kl, _k } }, .v.d = _v }
#define OT_INIT_NUMBERN(_x,_k,_kl,_v)		(struct ot_node){ .type = OT_NUMBER, .key = { _x, { _kl, _k } }, .v.d = _v }
#define OT_INIT_STRINGN(_x,_k,_kl,_v,_vl)	(struct ot_node){ .type = _v ? OT_STRING : OT_NULL, .key = { _x, { _kl, _k } }, .v.s = { _vl, _v } }
#define OT_INIT_ARRAYN(_x,_k,_kl,_i)		(struct ot_node){ .type = OT_ARRAY, .key = { _x, { _kl, _k} }, .iterate = _i }
#define OT_INIT_OBJECTN(_x,_k,_kl,_i)		(struct ot_node){ .type = OT_OBJECT, .key = { _x, { _kl, _k } }, .iterate = _i }

#define OT_INIT_NULL(_x,_k)		OT_INIT_NULLN(_x,_k,-1)
#define OT_INIT_BOOL(_x,_k,_v)		OT_INIT_BOOLN(_x,_k,-1,_v)
#define OT_INIT_NUMBERI(_x,_k,_v)	OT_INIT_NUMBERNI(_x,_k,-1,_v)
#define OT_INIT_NUMBER(_x,_k,_v)	OT_INIT_NUMBERN(_x,_k,-1,_v)
#define OT_INIT_STRING(_x,_k,_v)	OT_INIT_STRINGN(_x,_k,-1,_v,-1)
#define OT_INIT_ARRAY(_x,_k,_i)		OT_INIT_ARRAYN(_x,_k,-1,_i)
#define OT_INIT_OBJECT(_x,_k,_i)	OT_INIT_OBJECTN(_x,_k,-1,_i)

/** iterate over node
 *  \returns 1 when a new item is returned in sub, 0 when finished.
 */
#define ot_node_iterate(node,key,sub)					\
({									\
	int res = 0;							\
	if ((node)->iterate)						\
		res = (node)->iterate(node, key, sub);			\
	res;								\
})

#ifdef __cplusplus
}
#endif

#endif /* PIPEWIRE_OT_H */
