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

#include <errno.h>

#include "midifile.h"

#define DEFAULT_TEMPO	500	/* 120 BPM is the default */

static inline uint16_t parse_be16(const uint8_t *in)
{
	return (in[0] << 8) | in[1];
}

static inline uint32_t parse_be32(const uint8_t *in)
{
	return (in[0] << 24) | (in[1] << 16) | (in[2] << 8) | in[3];
}

static int read_mthd(struct midi_file *mf)
{
	uint8_t buffer[14];
	int res;

	if ((res = mf->events->read(mf->data, mf->offset, buffer, 14)) != 14)
		return res < 0 ? res : -EIO;

	if (memcmp(buffer, "MThd", 4))
		return -EIO;

	mf->size = parse_be32(buffer + 4);
	mf->format = parse_be16(buffer + 8);
	mf->ntracks = parse_be16(buffer + 10);
	mf->division = parse_be16(buffer + 12);
	mf->offset += 14;
	return 0;
}

int midi_file_open(struct midi_file *mf, int mode,
		const struct midi_events *events, void *data)
{
	int res;

	spa_zero(*mf);

	mf->events = events;
	mf->data = data;

	if ((res = read_mthd(mf)) < 0)
		return res;

	spa_list_init(&mf->tracks);
	mf->tick = 0;
	mf->tempo = DEFAULT_TEMPO;

	return 0;
}

int midi_file_close(struct midi_file *mf)
{
	return 0;
}

static int read_mtrk(struct midi_file *mf, struct midi_track *track)
{
	uint8_t buffer[8];
	int res;

	if ((res = mf->events->read(mf->data, mf->offset, buffer, 8)) != 8)
		return res < 0 ? res : -EIO;

	if (memcmp(buffer, "MTrk", 4))
		return -EIO;

	track->start = mf->offset+8;
	track->offset = 0;
	track->size = parse_be32(buffer + 4);
	mf->offset += track->size + 8;
	return 0;
}

static int parse_varlen(struct midi_file *mf, struct midi_track *tr, uint32_t *result)
{
	uint32_t value;
	uint8_t buffer[1];
	int i, res;

	value = 0;
	for (i = 0; i < 4; i++) {
		if (tr->offset >= tr->size) {
			tr->eof = true;
			return  0;
		}

		if ((res = mf->events->read(mf->data, tr->start + tr->offset, buffer, 1)) != 1)
			return res < 0 ? res : -EIO;

		tr->offset++;

		value = (value << 7) | ((buffer[0]) & 0x7f);
		if ((buffer[0] & 0x80) == 0)
			break;
	}
	*result = value;
	return 0;
}


static int peek_event(struct midi_file *mf, struct midi_track *tr, struct midi_event *event)
{
	uint8_t buffer[4], status;
	uint32_t size = 0;
	int res;

	if (tr->eof || tr->offset > tr->size)
		return -EIO;

	if ((res = mf->events->read(mf->data, tr->start + tr->offset, buffer, 1)) != 1)
		return res < 0 ? res : -EIO;

	status = buffer[0];
	if ((status & 0x80) == 0) {
		if ((tr->running_status & 0x80) == 0)
			status = 0;
		else
			status = tr->running_status;
	}

	event->event = buffer[0];
	event->status = status;
	event->track = tr;
	event->tick = tr->tick;
	event->offset = tr->offset;

	//fprintf(stderr, "%p: %04x %02x %02x\n", tr, tr->start + tr->offset, buffer[0], status);

	tr->offset++;

	if (status < 0xf0) {
		size++;
		if (status < 0xc0 || status >= 0xe0)
			size++;
	} else {
		if (status == 0xff) {
			if ((res = mf->events->read(mf->data, tr->start + tr->offset, buffer, 1)) != 1)
				return res < 0 ? res : -EIO;

			tr->offset++;

			if ((res = parse_varlen(mf, tr, &size)) < 0)
				return res;
			fprintf(stderr, "offs:%d meta %d size %d\n", event->offset, buffer[0], size);
			if (buffer[0] == 0x2f)
				tr->eof = true;
			size+=2;
			event->offset++;
		} else if (status == 0xf0 || status == 0xf7) {
			if ((res = parse_varlen(mf, tr, &size)) < 0)
				return res;
			fprintf(stderr, "offs:%d meta %d size %d\n", event->offset, buffer[0], size);
			size+=1;
		} else {
			return -EIO;
		}
	}
	if (buffer[0] & 0x80 && status < 0xf0)
		event->offset++;

	tr->offset = event->offset;

	event->offset += tr->start;
	event->size = size;

	if (status < 0xf8)
	      tr->running_status = status;

	return 0;
}

int midi_file_add_track(struct midi_file *mf, struct midi_track *track)
{
	int res;
	uint32_t delta_time;

	if ((res = read_mtrk(mf, track)) < 0)
		return res;

	if ((res = parse_varlen(mf, track, &delta_time)) < 0)
		return res;

	track->tick = delta_time;
	spa_list_init(&track->events);
	spa_list_append(&mf->tracks, &track->link);

#if 0
	fprintf(stderr, "track %p %04x %04x\n", track, track->start, track->size);

	while (1) {
		struct midi_event ev;

		if (peek_event(mf, track, &ev) < 0)
			break;

		fprintf(stderr, "%p %04x %04x %02x %02x %zd\n", track, (int)ev.tick, ev.offset, ev.event, ev.status, ev.size);

		midi_file_consume_event(mf, &ev);
	}
#endif
	track->tick = delta_time;
	track->offset = 0;
	return 0;

}
int midi_file_peek_event(struct midi_file *mf, struct midi_event *event)
{
	struct midi_track *tr, *found = NULL;

	spa_list_for_each(tr, &mf->tracks, link) {
		if (tr->eof)
			continue;
		if (found == NULL || tr->tick < found->tick)
			found = tr;
	}
	if (found == NULL)
		return -EIO;

	return peek_event(mf, found, event);
}

int midi_file_consume_event(struct midi_file *mf, struct midi_event *event)
{
	struct midi_track *tr = event->track;
	uint32_t delta_time;
	int res;

	tr->offset = event->offset - tr->start + event->size;
	if ((res = parse_varlen(mf, tr, &delta_time)) < 0)
		return res;
	tr->tick += delta_time;
	return 0;
}

int midi_file_add_event(struct midi_file *mf, struct midi_track *track, struct midi_event *event)
{
	spa_list_append(&track->events, &event->link);
	return 0;
}

