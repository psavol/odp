/* Copyright (c) 2018, Linaro Limited
 * All rights reserved.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 */

#ifndef ODP_RING_MPMC_INTERNAL_H_
#define ODP_RING_MPMC_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <odp/api/atomic.h>
#include <odp/api/cpu.h>
#include <odp/api/hints.h>
#include <odp_align_internal.h>
#include <odp/api/plat/atomic_inlines.h>
#include <odp/api/plat/cpu_inlines.h>

/* Ring of uint32_t data
 *
 * Ring stores head and tail counters. Ring indexes are formed from these
 * counters with a mask (mask = ring_size - 1), which requires that ring size
 * must be a power of two.
 *
 *    0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
 *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 *  | E | E |   |   |   |   |   |   |   |   |   | E | E | E | E | E |
 *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 *        ^       ^               ^           ^
 *        |       |               |           |
 *     r_tail  r_head          w_tail      w_head
 *
 **/
typedef struct {
	/* Writer head and tail */
	odp_atomic_u32_t w_head;
	odp_atomic_u32_t w_tail;

	/* Reader head and tail */
	odp_atomic_u32_t ODP_ALIGNED_CACHE r_head;
	odp_atomic_u32_t r_tail;
	uint32_t mask;
	uint32_t *data;
} ring_mpmc_t;

/* 32-bit CAS with memory order selection */
static inline int ring_mpmc_cas_mo_u32(odp_atomic_u32_t *atom,
				       uint32_t *old_val, uint32_t new_val,
				       int mo_success, int mo_failure)
{
	return __atomic_compare_exchange_n(&atom->v, old_val, new_val,
					   0 /* strong */,
					   mo_success,
					   mo_failure);
}

/* Initialize ring */
static inline void ring_mpmc_init(ring_mpmc_t *ring, uint32_t *data,
				  uint32_t size)
{
	odp_atomic_init_u32(&ring->w_head, 0);
	odp_atomic_init_u32(&ring->w_tail, 0);
	odp_atomic_init_u32(&ring->r_head, 0);
	odp_atomic_init_u32(&ring->r_tail, 0);
	ring->mask = size - 1;
	ring->data = data;
}

/* Dequeue data from the ring head. Num is smaller than ring size. */
static inline uint32_t ring_mpmc_deq_multi(ring_mpmc_t *ring, uint32_t data[],
					   uint32_t num)
{
	uint32_t old_head, new_head, w_tail, num_data, i;
	uint32_t mask = ring->mask;

	/* Load/CAS acquire of r_head ensures that w_tail load happens after
	 * r_head load, and thus r_head value is always behind or equal to
	 * w_tail value. This thread owns data between old and new r_head. */
	old_head = odp_atomic_load_acq_u32(&ring->r_head);

	do {
		w_tail   = odp_atomic_load_acq_u32(&ring->w_tail);
		num_data = w_tail - old_head;

		/* Ring is empty */
		if (num_data == 0)
			return 0;

		/* Try to take all available */
		if (num > num_data)
			num = num_data;

		new_head = old_head + num;

	} while (odp_unlikely(ring_mpmc_cas_mo_u32(&ring->r_head, &old_head,
						   new_head,
						   __ATOMIC_ACQ_REL,
						   __ATOMIC_ACQUIRE) == 0));

	/* Read data. CAS acquire-release ensures that data read
	 * does not move above from here. */
	for (i = 0; i < num; i++)
		data[i] = ring->data[(old_head + 1 + i) & mask];

	/* Wait until other reads have updated the tail */
	while (odp_unlikely(odp_atomic_load_acq_u32(&ring->r_tail) != old_head))
		odp_cpu_pause();

	/* Release the new reader tail, writers acquire it. */
	odp_atomic_store_rel_u32(&ring->r_tail, new_head);

	return num;
}

/* Enqueue multiple data into the ring tail. Num is smaller than ring size. */
static inline uint32_t ring_mpmc_enq_multi(ring_mpmc_t *ring,
					   const uint32_t data[],
					   uint32_t num)
{
	uint32_t old_head, new_head, r_tail, num_free, i;
	uint32_t mask = ring->mask;
	uint32_t size = mask + 1;

	/* Load/CAS acquire ensures that w_head load happens after r_tail load,
	 * and thus r_tail value is always behind or equal to w_head value.
	 * This thread owns data between old and new w_head. */
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

		new_head = old_head + num;

	} while (odp_unlikely(ring_mpmc_cas_mo_u32(&ring->w_head, &old_head,
						   new_head,
						   __ATOMIC_ACQ_REL,
						   __ATOMIC_ACQUIRE) == 0));

	/* Write data */
	for (i = 0; i < num; i++)
		ring->data[(old_head + 1 + i) & mask] = data[i];

	/* Wait until other writers have updated the tail */
	while (odp_unlikely(odp_atomic_load_acq_u32(&ring->w_tail) != old_head))
		odp_cpu_pause();

	/* Release the new writer tail, readers acquire it. */
	odp_atomic_store_rel_u32(&ring->w_tail, new_head);

	return num;
}

/* Check if ring is empty */
static inline int ring_mpmc_is_empty(ring_mpmc_t *ring)
{
	uint32_t head = odp_atomic_load_u32(&ring->r_head);
	uint32_t tail = odp_atomic_load_u32(&ring->w_tail);

	return head == tail;
}

#ifdef __cplusplus
}
#endif

#endif
