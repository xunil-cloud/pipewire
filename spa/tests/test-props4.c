/* Spa
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <spa/log.h>
#include <lib/debug.h>

#if 0
/*
  ( "Format",
    ( "video", "raw" ),
    {
      "format":    ( "seu", "I420", ( "I420", "YUY2" ) ),
      "size":      ( "Rru", R(320, 242), ( R(1,1), R(MAX, MAX)) ),
      "framerate": ( "Fru", F(25, 1), ( F(0,1), F(MAX, 1)) )
    }
  )

  ( struct
  { object
  [ array

   1: s = string    :  "value"
      i = int       :  <number>
      l = long      :  <number>
      f = float     :  <float>
      d = double    :  <float>
      b = bool      :  true | false
      R = rectangle : [ <width>, <height> ]
      F = fraction  : [ <num>, <denom> ]

   2: - = default (only default value present)
      e = enum	        : [ <value>, ... ]
      f = flags	        : [ <number> ]
      m = min/max	: [ <min>, <max> ]
      s = min/max/step  : [ <min>, <max>, <step> ]

   3: u = unset		: value is unset, choose from options or default
      o = optional	: value does not need to be set
      r = readonly      : value is read only
      d = deprecated    : value is deprecated
*/
#endif

struct spa_pod_maker {
	struct spa_pod_builder b;
	struct spa_pod_frame frame[20];
	int depth;

};

static inline void spa_pod_maker_init(struct spa_pod_maker *maker,
				      char *data, int size)
{
	spa_pod_builder_init(&maker->b, data, size);
	maker->depth = 0;
}

static inline bool spa_is_white_space(char val)
{
	switch (val) {
	case ' ': case '\t': case '\n': case '\r':
		return true;
	}
	return false;
}
static inline bool spa_is_alpha_num(char val)
{
	switch (val) {
	case 'a' ... 'z': case 'A' ... 'Z': case '0' ... '9': case '_':
		return true;
	}
	return false;
}
static inline int64_t spa_parse_int(const char *str, char **endptr)
{
	int64_t res = 0;
	int i;

	for (; spa_is_white_space(*str); str++);
	if (*str == '#') {
		struct { char *pat; int len; int64_t val; } vals[] = {
			{ "#INT32_MAX#", strlen("#INT32_MAX#"), INT32_MAX },
			{ "#INT32_MIN#", strlen("#INT32_MIN#"), INT32_MIN },
			{ "#INT64_MAX#", strlen("#INT64_MAX#"), INT64_MAX },
			{ "#INT64_MIN#", strlen("#INT64_MIN#"), INT64_MIN }};

		for (i = 0; i < SPA_N_ELEMENTS(vals); i++) {
			if (strncmp(str, vals[i].pat, vals[i].len) == 0) {
				res = vals[i].val;
				*endptr = (char *) (str + vals[i].len);
				break;
			}
		}
		return res;
	}
	else
		return strtoll(str, endptr, 10);
}

static inline const char *spa_parse_string(const char *str, char **endptr)
{
	for (*endptr = (char *)str+1; **endptr != '\"' && **endptr != '\0'; (*endptr)++);
	return str + 1;
}

static inline void *
spa_pod_maker_build(struct spa_pod_maker *maker,
		    const char *format, ...)
{
	va_list args;
	const char *start, *strval;
	int64_t intval;
	double doubleval;
	char last;
	struct spa_rectangle *rectval;
	struct spa_fraction *fracval;
	int len;

	va_start(args, format);
	while (*format != '\0') {
		switch (*format) {
		case '[':
			spa_pod_builder_push_struct(&maker->b, &maker->frame[maker->depth++]);
			break;
		case '(':
			spa_pod_builder_push_array(&maker->b, &maker->frame[maker->depth++]);
			break;
		case '{':
			spa_pod_builder_push_map(&maker->b, &maker->frame[maker->depth++]);
			break;
		case ']': case '}': case ')':
			spa_pod_builder_pop(&maker->b, &maker->frame[--maker->depth]);
			break;
		case '\"':
			start = spa_parse_string(format, (char **) &format);
			len = format - start;
			for (format++;spa_is_white_space(*format); format++);
			if (*format == ':')
				spa_pod_builder_key_len(&maker->b, start, len);
			else
				spa_pod_builder_string_len(&maker->b, start, len);
			continue;
		case '@':
		case '%':
			last = *format;
			format++;
			switch (*format) {
			case 's':
				strval = va_arg(args, char *);
				spa_pod_builder_string_len(&maker->b, strval, strlen(strval));
				break;
			case 'i':
				spa_pod_builder_int(&maker->b, va_arg(args, int));
				break;
			case 'I':
				spa_pod_builder_id(&maker->b, va_arg(args, int));
				break;
			case 'l':
				spa_pod_builder_long(&maker->b, va_arg(args, int64_t));
				break;
			case 'f':
				spa_pod_builder_float(&maker->b, va_arg(args, double));
				break;
			case 'd':
				spa_pod_builder_double(&maker->b, va_arg(args, double));
				break;
			case 'b':
				spa_pod_builder_bool(&maker->b, va_arg(args, int));
				break;
			case 'z':
			{
				void *ptr  = va_arg(args, void *);
				int len = va_arg(args, int);
				spa_pod_builder_bytes(&maker->b, ptr, len);
				break;
			}
			case 'p':
				spa_pod_builder_pointer(&maker->b, 0, va_arg(args, void *));
				break;
			case 'h':
				spa_pod_builder_fd(&maker->b, va_arg(args, int));
				break;
			case 'a':
			{
				int child_size = va_arg(args, int);
				int child_type = va_arg(args, int);
				int n_elems = va_arg(args, int);
				void *elems = va_arg(args, void *);
				spa_pod_builder_array(&maker->b, child_size, child_type, n_elems, elems);
				break;
			}
			case 'P':
				spa_pod_builder_primitive(&maker->b, va_arg(args, struct spa_pod *));
				break;
			case 'R':
				rectval = va_arg(args, struct spa_rectangle *);
				spa_pod_builder_rectangle(&maker->b, rectval->width, rectval->height);
				break;
			case 'F':
				fracval = va_arg(args, struct spa_fraction *);
				spa_pod_builder_fraction(&maker->b, fracval->num, fracval->denom);
				break;
			}
			if (last == '@') {
				format = va_arg(args, const char *);
				continue;
			}
			break;
		case '0' ... '9': case '-': case '+': case '#':
			start = format;
			intval = spa_parse_int(start, (char **) &format);
			if (*format == '.') {
				doubleval = strtod(start, (char **) &format);
				if (*format == 'f')
					spa_pod_builder_float(&maker->b, doubleval);
				else
					spa_pod_builder_double(&maker->b, doubleval);
				continue;
			}
			for (;spa_is_white_space(*format); format++);
			switch (*format) {
			case 'x':
				spa_pod_builder_rectangle(&maker->b, intval,
						spa_parse_int(format+1, (char **) &format));
				break;
			case '/':
				spa_pod_builder_fraction(&maker->b, intval,
						spa_parse_int(format+1, (char **) &format));
				break;
			case 'l':
				spa_pod_builder_long(&maker->b, intval);
				format++;
				break;
			default:
				spa_pod_builder_int(&maker->b, intval);
				break;
			}
			continue;
		}
		format++;
	}
	va_end(args);

	return SPA_POD_BUILDER_DEREF(&maker->b, maker->frame[maker->depth].ref, void);
}

int main(int argc, char *argv[])
{
	struct spa_pod_maker m = { 0, };
	char buffer[4096];
	struct spa_pod *fmt;

	spa_pod_maker_init(&m, buffer, sizeof(buffer));
	fmt = spa_pod_maker_build(&m,
				"[ \"Format\", "
				" [\"video\", \"raw\" ], "
				" { "
				"   \"format\":    [ \"eu\", \"I420\", [ \"I420\",\"YUY2\" ] ], "
				"   \"size\":      [ \"ru\", 320x242, [ 1x1, #INT32_MAX#x#INT32_MAX# ] ], "
				"   \"framerate\": [ \"ru\", 25/1, [ 0/1, #INT32_MAX#/1 ] ] "
				" } "
				"] ");
	spa_debug_pod(fmt);

	spa_pod_maker_init(&m, buffer, sizeof(buffer));
	fmt = spa_pod_maker_build(&m,
				"[ \"Format\", "
				" [\"video\", %s ], "
				" { "
				"   \"format\":    [ \"eu\", \"I420\", [ %s, \"YUY2\" ] ], "
				"   \"size\":      [ \"ru\", 320x242, [ %R, #INT32_MAX#x#INT32_MAX# ] ], "
				"   \"framerate\": [ \"ru\", %F, [ 0/1, #INT32_MAX#/1 ] ] "
				" } "
				"] ",
					"raw",
					"I420",
					&(struct spa_rectangle){ 1, 1 },
					&(struct spa_fraction){ 25, 1 }
				);
	spa_debug_pod(fmt);

	{
		const char *format = "S16";
		int rate = 44100, channels = 2;

		fmt = spa_pod_maker_build(&m,
                                "[ \"Format\", "
                                " [\"audio\", \"raw\" ], "
                                " { "
                                "   \"format\":   [@s", format, "] "
                                "   \"rate\":     [@i", rate, "] "
                                "   \"channels\": [@i", channels, "] "
                                " } "
                                "] ");
		spa_debug_pod(fmt);
	}

	{
		const char *format = "S16";
		int rate = 44100, channels = 2;
		struct spa_rectangle rects[3] = { { 1, 1 }, { 2, 2},  {3, 3}};
		struct spa_pod_int pod = SPA_POD_INT_INIT(12);
		uint8_t bytes[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };

		spa_pod_maker_build(&m,
                                "[ \"Format\", "
                                " [\"audio\", \"raw\" ], ");
		fmt = spa_pod_maker_build(&m,
                                " { "
                                "   \"format\":   [ %s ] "
                                "   \"rate\":     [ %i, ( 44100, 48000, 96000 ) ]"
                                "   \"foo\":      %i, ( 1.1, 2.2, 3.2  )"
                                "   \"baz\":      ( 1.1f, 2.2f, 3.2f )"
                                "   \"bar\":      ( 1x1, 2x2, 3x2 )"
                                "   \"faz\":      ( 1/1, 2/2, 3/2 )"
                                "   \"wha\":      %a, "
                                "   \"fuz\":      %P, "
//                                "   \"fur\":      ( (1, 2), (7, 8), (7, 5) ) "
//                                "   \"fur\":      ( [1, 2], [7, 8], [7, 5] ) "
                                "   \"buz\":      %z, "
                                "   \"boo\":      %p, "
                                "   \"foz\":      %h, "
                                " } "
                                "] ", format, rate, channels,
				sizeof(struct spa_rectangle), SPA_POD_TYPE_RECTANGLE, 3, rects,
				&pod,
				bytes, sizeof(bytes),
				fmt,
				STDOUT_FILENO);
		spa_debug_pod(fmt);
	}

	return 0;
}
