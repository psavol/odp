/* Copyright (c) 2018, Linaro Limited
 * All rights reserved.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 */

#ifndef ODP_RING_SCHED_INTERNAL_H_
#define ODP_RING_SCHED_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <odp/api/atomic.h>
#include <odp/api/cpu.h>
#include <odp/api/hints.h>
#include <odp_align_internal.h>
#include <odp/api/plat/atomic_inlines.h>
#include <odp/api/plat/cpu_inlines.h>

/* Ring for scheduled queues
 *
 * Ring stores 32 bit data, and maintains head and tail counters. Ring indexes
 * are formed from these counters with a mask (mask = ring_size - 1), which
 * requires that ring size must be a power of two.
 *
 * Ring supports multiple producers, but only single consumer (the scheduler).
 * All counter values are 31 bits (wrap at 0x7fffffff). The writer tail counter
 * mains a flag that the reader (scheduler) sets when it has determined that
 * the ring is empty. The next writer clears the flag. It is used to decide
 * when a queue needs to be added into scheduling.
 *
 *    0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
 *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 *  | E | E |   |   |   |   |   |   |   |   |   | E | E | E | E | E |
 *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 *        ^                       ^           ^
 *        |                       |           |
 *     r_tail                  w_tail      w_head
 *
 */
typedef struct {
	odp_atomic_u32_t ODP_ALIGNED_CACHE r_tail;
	odp_atomic_u32_t w_head;
	odp_atomic_u32_t w_tail;

} ring_sched_t;

#define RING_SCHED_GET_FLAG(x)   ((x) & 0x80000000)
#define RING_SCHED_ADD_FLAG(x)   ((x) | 0x80000000)
#define RING_SCHED_GET_CNT(x)    ((x) & 0x7fffffff)
#define RING_SCHED_NEW_CNT(x, n) (((x) + (n)) & 0x7fffffff)

/* Initialize ring */
static inline void ring_sched_init(ring_sched_t *ring)
{
	odp_atomic_init_u32(&ring->r_tail, 0);
	odp_atomic_init_u32(&ring->w_head, 0);
	/* Queue is not scheduled */
	odp_atomic_init_u32(&ring->w_tail, RING_SCHED_ADD_FLAG(0));
}

/* Dequeue data from the ring head. Num is smaller than ring size. */
static inline uint32_t ring_sched_deq_multi(ring_sched_t *ring,
					    uint32_t *ring_data,
					    uint32_t ring_mask,
					    uint32_t data[],
					    uint32_t num, int update_status)
{
	uint32_t head, new_head, w_tail, new_tail, num_data, i;

	/* Load acquire ensures that r_tail is read before w_tail */
	head = odp_atomic_load_acq_u32(&ring->r_tail);
	odp_prefetch(&ring_data[(head + 1) & ring_mask]);

	do {
		w_tail   = odp_atomic_load_acq_u32(&ring->w_tail);
		num_data = w_tail - head;

		if (num_data)
			break;

		/* Ring is empty. If requested, try to set the flag. */
		if (update_status) {
			new_tail = RING_SCHED_ADD_FLAG(w_tail);
			if (odp_likely(odp_atomic_cas_u32(&ring->w_tail,
							  &w_tail, new_tail)))
				return 0;
		} else {
			return 0;
		}

	} while (1);

	/* Take all available */
	if (num > num_data)
		num = num_data;

	new_head = RING_SCHED_NEW_CNT(head, num);

	/* Read data. */
	for (i = 0; i < num; i++)
		data[i] = ring_data[(head + 1 + i) & ring_mask];

	/* Release the new reader tail, writers acquire it. */
	odp_atomic_store_rel_u32(&ring->r_tail, new_head);

	return num;
}

/* Enqueue multiple data into the ring tail. Num is smaller than ring size. */
static inline uint32_t ring_sched_enq_multi(ring_sched_t *ring,
					    uint32_t *ring_data,
					    uint32_t ring_mask,
					    const uint32_t data[],
					    uint32_t num, int *sched_status)
{
	uint32_t old_head, new_head, w_tail, r_tail, num_free, i;
	int sched;
	uint32_t size = ring_mask + 1;

	do {
		r_tail   = odp_atomic_load_acq_u32(&ring->r_tail);
		old_head = odp_atomic_load_acq_u32(&ring->w_head);

		num_free = size - (old_head - r_tail);

		/* Ring is full */
		if (num_free == 0)
			return 0;

		/* Try to use all available */
		if (num > num_free)
			num = num_free;

		new_head = RING_SCHED_NEW_CNT(old_head, num);

	} while (odp_unlikely(odp_atomic_cas_u32(&ring->w_head, &old_head,
						new_head) == 0));

	/* Write data. This will not move above load acquire of w_head. */
	for (i = 0; i < num; i++)
		ring_data[(old_head + 1 + i) & ring_mask] = data[i];

	/* Wait until other writers have updated the tail */
	w_tail = odp_atomic_load_u32(&ring->w_tail);

	while (odp_unlikely(RING_SCHED_GET_CNT(w_tail) != old_head)) {
		odp_cpu_pause();
		w_tail = odp_atomic_load_u32(&ring->w_tail);
	}

	/* Release the new writer tail, always clear the flag. Reader may have
	 * set the flag. */
	do {
		if (RING_SCHED_GET_FLAG(w_tail))
			sched = 1;
		else
			sched = 0;

	} while (odp_unlikely(odp_atomic_cas_rel_u32(&ring->w_tail, &w_tail,
						     new_head) == 0));

	*sched_status = sched;

	return num;
}

/* Check if ring is empty */
static inline int ring_sched_is_empty(ring_sched_t *ring)
{
	uint32_t head = odp_atomic_load_u32(&ring->r_tail);
	uint32_t tail = odp_atomic_load_u32(&ring->w_tail);

	return head == RING_SCHED_GET_CNT(tail);
}

#ifdef __cplusplus
}
#endif

#endif
