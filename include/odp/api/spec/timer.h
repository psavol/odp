/* Copyright (c) 2013-2018, Linaro Limited
 * Copyright (c) 2019-2020, Nokia
 *
 * All rights reserved.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 */

/**
 * @file
 *
 * ODP timer service
 */

#ifndef ODP_API_SPEC_TIMER_H_
#define ODP_API_SPEC_TIMER_H_
#include <odp/visibility_begin.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup odp_timer ODP TIMER
 *  Timer generating timeout events.
 *  @{
 */

/**
 * @typedef odp_timer_pool_t
 * ODP timer pool handle
 */

/**
 * @def ODP_TIMER_POOL_INVALID
 * Invalid timer pool handle
 */

/**
 * Timer type
 */
typedef enum {
	/** Single shot timer with absolute expiration time */
	ODP_TIMER_ABS = 0x01,

	/** Single shot timer with relative expiration time */
	ODP_TIMER_REL = 0x02,

	/** Single shot timer (absolute or relative) */
	ODP_TIMER_SINGLE = 0x03,

	/** Periodic timer */
	ODP_TIMER_PERIODIC = 0x10,

	/** Single shot and periodic timers in the same timer pool */
	ODP_TIMER_SINGLE_AND_PERIODIC = 0x20

} odp_timer_type_t;

/**
 * Clock sources for timers in timer pool.
 */
typedef enum {
	/** Use CPU clock as clock source for timers */
	ODP_CLOCK_CPU,
	/** Use external clock as clock source for timers */
	ODP_CLOCK_EXT
	/* Platform dependent which other clock sources exist */
} odp_timer_clk_src_t;

/**
 * @typedef odp_timer_t
 * ODP timer handle
 */

/**
 * @def ODP_TIMER_INVALID
 * Invalid timer handle
 */

/**
 * @typedef odp_timeout_t
 * ODP timeout handle
 */

/**
 * @def ODP_TIMEOUT_INVALID
 * Invalid timeout handle
 */

/**
 * Return values of timer set calls.
 */
typedef enum {
	/** Timer set operation succeeded */
	ODP_TIMER_SUCCESS = 0,

	/** Timer set operation failed because expiration time is too near to
	 *  the current time. */
	ODP_TIMER_TOO_NEAR = -1,

	/** Timer set operation failed because expiration time is too far from
	 *  the current time. */
	ODP_TIMER_TOO_FAR = -2,

	/** Timer set operation failed */
	ODP_TIMER_FAIL = -3

} odp_timer_set_t;

/** Timer expiration time is too near to the current time. */
#define ODP_TIMER_TOOEARLY ODP_TIMER_TOO_NEAR

/** Timer expiration time is too far from the current time. */
#define ODP_TIMER_TOOLATE  ODP_TIMER_TOO_FAR

/** Timer set operation failed because no event specified and no event
 *  present in the timer (timer inactive/expired). */
#define ODP_TIMER_NOEVENT ODP_TIMER_FAIL

/**
 * @def ODP_TIMER_POOL_NAME_LEN
 * Maximum timer pool name length in chars including null char
 */

/**
 * Timer period
 */
typedef struct {
	/** Period in nanoseconds. Set to zero if hertz is used. */
	uint64_t period_ns;

	/** Period in hertz. Set to zero if nanoseconds are used. */
	uint64_t period_hz;

} odp_timer_period_t;

/**
 * Timer pool parameters
 */
typedef struct {
	/** Timeout resolution in nanoseconds. Timer pool must serve timeouts
	 *  with this or higher resolution. The minimum valid value (highest
	 *  resolution) is defined by timer resolution capability. When this
	 *  parameter is used, set 'res_hz' to zero. */
	uint64_t res_ns;

	/** Timeout resolution in hertz. This may be used to specify the highest
	 *  required resolution in hertz instead of nanoseconds. When this
	 *  parameter is used, set 'res_ns' to zero. */
	uint64_t res_hz;

	/** Minimum relative timeout in nanoseconds. All requested timeouts
	 *  will be at least this many nanoseconds after the current
	 *  time of the timer pool. Timer set functions return an error, if too
	 *  short timeout was requested. The value may be also smaller than
	 *  the requested resolution. */
	uint64_t min_tmo;

	/** Maximum relative timeout in nanoseconds. All requested timeouts
	 *  will be at most this many nanoseconds after the current
	 *  time of the timer pool. Timer set functions return an error, if too
	 *  long timeout was requested. */
	uint64_t max_tmo;

	/** Maximum timeout period
	 *
	 *  This parameter is considered only when periodic timers are used.
	 *  It specifies the longest period to be requested for any periodic
	 *  timer of the pool. Depending on implementation, the best performance
	 *  may be reached when (in hertz) the requested resolution is
	 *  a multiple of each requested period, and each requested period is
	 *  a multiple of the maximum period.
	 *
	 *  For example, res_hz = 1 333 000 Hz, max_period_hz = 1 333 Hz and
	 *  requested periods are 1 333 Hz, 2 666 Hz, 33 325 Hz, ...
	 */
	odp_timer_period_t max_period;

	/** Number of timers needed. Application will create in maximum this
	 *  many concurrent timers from the timer pool. */
	uint32_t num_timers;

	/** Thread private timer pool. When zero, multiple thread may use the
	 *  timer pool concurrently. When non-zero, only single thread uses the
	 *  timer pool (concurrently). */
	int priv;

	/** Timer types
	 *
	 *  Specify if single shot, periodic or both timer types will be created
	 *  from the timer pool:
	 *
	 *  ODP_TIMER_SINGLE:     Only single shot timers (default)
	 *  ODP_TIMER_PERIODIC:   Only periodic timers
	 *  ODP_TIMER_SINGLE_AND_PERIODIC: Both single shot and periodic timers
	 **/
	odp_timer_type_t type;

	/** Clock source for timers */
	odp_timer_clk_src_t clk_src;

} odp_timer_pool_param_t;

/**
 * Timer resolution capability
 */
typedef struct {
	/** Timeout resolution in nanoseconds */
	uint64_t res_ns;

	/** Timeout resolution in hertz */
	uint64_t res_hz;

	/** Minimum relative timeout in nanoseconds */
	uint64_t min_tmo;

	/** Maximum relative timeout in nanoseconds */
	uint64_t max_tmo;

} odp_timer_res_capability_t;

/**
 * Timer capability
 */
typedef struct {
	/** Maximum number of timer pools over all clock sources
	 *
	 * The total number of timer pools that can be created combining
	 * different clock sources.
	 */
	uint32_t max_pools_combined;

	/** Maximum number of timer pools for the requested clock source */
	uint32_t max_pools;

	/** Supported timer types
	 *
	 *  ODP_TIMER_SINGLE:                      Only single shot timers
	 *  ODP_TIMER_PERIODIC:                    Only periodic timers
	 *  ODP_TIMER_SINGLE | ODP_TIMER_PERIODIC: Single shot and periodic
	 *                                         timers supported, but not in
	 *                                         the same timer pool
	 *  ODP_TIMER_SINGLE_AND_PERIODIC:         Single shot and periodic
	 *                                         timers supported in the same
	 *                                         timer pool
	 */
	odp_timer_type_t type;

	/** Maximum number of single shot timers in a pool
	 *
	 * The value of zero means that limited only by the available
	 * memory size for the pool. */
	uint32_t max_timers;

	/** Maximum number of periodic timers in a pool
	 *
	 * The value of zero means that limited only by the available
	 * memory size for the pool. */
	uint32_t max_timers_periodic;

	/** Highest timer resolution in nanoseconds.
	 *
	 *  This defines the highest resolution supported by a timer.
	 *  It's the minimum valid value for 'res_ns' timer pool
	 *  parameter.
	 *
	 *  This value is equal to 'max_res.res_ns' capability.
	 */
	uint64_t highest_res_ns;

	/**
	 * Maximum resolution
	 *
	 * This defines the highest resolution supported by a timer, with
	 * limits to min/max timeout values. The highest resolution for a timer
	 * pool is defined by 'max_res.res_ns' in nanoseconds and
	 * 'max_res.res_hz' in hertz.
	 * When this resolution is used:
	 * - 'min_tmo' parameter value must be in minimum 'max_res.min_tmo'
	 * - 'max_tmo' parameter value must be in maximum 'max_res.max_tmo'
	 */
	odp_timer_res_capability_t max_res;

	/**
	 * Maximum timeout length
	 *
	 * This defines the maximum relative timeout value supported by a timer,
	 * with limits to min timeout and max resolution values. The maximum
	 * value for 'max_tmo' timer pool parameter is defined by
	 * 'max_tmo.max_tmo'. When this max timeout value is used:
	 * - 'min_tmo' parameter value must be in minimum 'max_tmo.min_tmo'
	 * - 'res_ns'  parameter value must be in minimum 'max_tmo.res_ns' or
	 * - 'res_hz'  parameter value must be in maximum 'max_tmo.res_hz'
	 */
	odp_timer_res_capability_t max_tmo;

	/**
	 * Scheduled queue destination support
	 *
	 * This defines whether schedule queues are supported as timeout
	 * destination queues.
	 * 0: Scheduled queues are not supported as timeout destination queues
	 * 1: Scheduled queues are supported as timeout destination queues
	 * @see odp_timer_alloc()
	 */
	odp_bool_t queue_type_sched;

	/**
	 * Plain queue destination support
	 *
	 * This defines whether plain queues are supported as timeout
	 * destination queues.
	 * 0: Plain queues are not supported as timeout destination queues
	 * 1: Plain queues are supported as timeout destination queues
	 * @see odp_timer_alloc()
	 */
	odp_bool_t queue_type_plain;
} odp_timer_capability_t;

/**
 * Timer create parameters
 */
typedef struct {
	/** User defined pointer value to be copied to timeouts */
	void *user_ptr;

	/** Select how periodic timer overruns are handled
	 *
	 * When application has not acknowledged (@see odp_timer_ack()) the
	 * previous timeout event
	 * of a periodic timer:
	 * 0: Timeout event delivery continues normally.
	 * 1: No more timeout events are delivered until application
	 *    acknowledges. Possible timer expiration overruns are catch up
	 *    by sending missing timeout events to the queue.
	 * 2: No more timeout events are delivered until application
	 *    acknowledges. Possible timer expiration overruns are skipped by
	 *    not sending missing timeout events to the queue.
	 */
	int overrun_mode;

} odp_timer_param_t;

/**
 * Query timer capabilities
 *
 * Outputs timer capabilities on success.
 *
 * @param      clk_src  Clock source for timers
 * @param[out] capa     Pointer to capability structure for output
 *
 * @retval 0 on success
 * @retval <0 on failure
 */
int odp_timer_capability(odp_timer_clk_src_t clk_src,
			 odp_timer_capability_t *capa);

/**
 * Timer resolution capability
 *
 * This function fills in capability limits for timer pool resolution and
 * min/max timeout values, based on either resolution or maximum timeout.
 * Set the required value to a resolution field (res_ns or res_hz) or to the
 * maximum timeout field (max_tmo), and set other fields to zero. A successful
 * call fills in the other fields. The call returns a failure, if the user
 * defined value exceeds capability limits. Outputted values are minimums for
 * 'res_ns' and 'min_tmo', and maximums for 'res_hz' and 'max_tmo'.
 *
 * @param         clk_src  Clock source for timers
 * @param[in,out] res_capa Resolution capability pointer for input/output.
 *                         Set either a resolution or max timeout field,
 *                         a successful call fills in other fields.
 *
 * @retval 0 on success
 * @retval <0 on failure
 */
int odp_timer_res_capability(odp_timer_clk_src_t clk_src,
			     odp_timer_res_capability_t *res_capa);

/**
 * Create a timer pool
 *
 * The use of pool name is optional. Unique names are not required.
 *
 * @param name       Name of the timer pool or NULL. Maximum string length is
 *                   ODP_TIMER_POOL_NAME_LEN.
 * @param params     Timer pool parameters. The content will be copied.
 *
 * @return Timer pool handle on success
 * @retval ODP_TIMER_POOL_INVALID on failure and errno set
 */
odp_timer_pool_t odp_timer_pool_create(const char *name,
				       const odp_timer_pool_param_t *params);

/**
 * Start a timer pool
 *
 * Start all created timer pools, enabling the allocation of timers.
 * The purpose of this call is to coordinate the creation of multiple timer
 * pools that may use the same underlying HW resources.
 * This function may be called multiple times.
 */
void odp_timer_pool_start(void);

/**
 * Destroy a timer pool
 *
 * Destroy a timer pool, freeing all resources.
 * All timers must have been freed.
 *
 * @param timer_pool  Timer pool
 */
void odp_timer_pool_destroy(odp_timer_pool_t timer_pool);

/**
 * Convert timer ticks to nanoseconds
 *
 * @param timer_pool  Timer pool
 * @param ticks       Timer ticks
 *
 * @return Nanoseconds
 */
uint64_t odp_timer_tick_to_ns(odp_timer_pool_t timer_pool, uint64_t ticks);

/**
 * Convert nanoseconds to timer ticks
 *
 * @param timer_pool  Timer pool
 * @param ns          Nanoseconds
 *
 * @return Timer ticks
 */
uint64_t odp_timer_ns_to_tick(odp_timer_pool_t timer_pool, uint64_t ns);

/**
 * Current tick value
 *
 * @param timer_pool  Timer pool
 *
 * @return Current time in timer ticks
 */
uint64_t odp_timer_current_tick(odp_timer_pool_t timer_pool);

/**
 * ODP timer pool information and configuration
 */
typedef struct {
	/** Parameters specified at creation */
	odp_timer_pool_param_t param;

	/** Number of currently allocated timers */
	uint32_t cur_timers;

	/** High watermark of allocated timers */
	uint32_t hwm_timers;

	/** Name of timer pool */
	const char *name;

} odp_timer_pool_info_t;

/**
 * Query timer pool configuration and current state
 *
 * @param      timer_pool  Timer pool
 * @param[out] info        Pointer to information buffer
 *
 * @retval 0 on success
 * @retval <0 on failure. Info could not be retrieved.
 */
int odp_timer_pool_info(odp_timer_pool_t timer_pool,
			odp_timer_pool_info_t *info);

/**
 * Allocate a timer
 *
 * Create a timer (allocating all necessary resources e.g. timeout event) from
 * the timer pool. The user_ptr is copied to timeouts and can be retrieved
 * using the odp_timeout_user_ptr() call.
 *
 * @param timer_pool  Timer pool
 * @param queue       Destination queue for timeout notifications
 * @param user_ptr    User defined pointer or NULL to be copied to timeouts
 *
 * @return Timer handle on success
 * @retval ODP_TIMER_INVALID on failure and errno set.
 */
odp_timer_t odp_timer_alloc(odp_timer_pool_t timer_pool, odp_queue_t queue, const void *user_ptr);

/**
 * Create timer
 *
 * Create a timer from the timer pool. The user_ptr parameter is copied to
 * timeout events and can be retrieved using the odp_timeout_user_ptr() call.
 *
 * @param timer_pool   Timer pool
 * @param timer_type   ODP_TIMER_SINGLE or ODP_TIMER_PERIODIC
 * @param queue        Destination queue for timeout events
 * @param param        Additional timer parameters. Use NULL if there are no
 *                     parameters.
 *
 * @return Timer handle on success
 * @retval ODP_TIMER_INVALID on failure
 */
odp_timer_t odp_timer_create(odp_timer_pool_t timer_pool,
			     odp_timer_type_t timer_type,
			     odp_queue_t queue,
			     odp_timer_param_t *param);

/**
 * Set a single shot timer
 *
 * This call is valid only for single shot timers. It starts a timer with
 * an expiration time and timeout event. Use 'type' parameter to select if
 * expiration time is passed as absolute timer ticks (ODP_TIMER_ABS) or
 * ticks relative to the current tick (ODP_TIMER_REL). Current timer tick
 * can be requested with odp_timer_current_tick().
 *
 * The timer must not be active already when calling this function. Use
 * odp_timer_reset() to reset an active timer.
 *
 * The user provided event can be of any event type, but only ODP_EVENT_TIMEOUT
 * type events (odp_timeout_t) carry timeout specific metadata. Furthermore,
 * timer performance may have been optimized for that event type. When the timer
 * expires, the event is enqueued to the destination queue of the timer.
 *
 * @param timer    Single shot timer
 * @param type     ODP_TIMER_ABS or ODP_TIMER_REL
 * @param tick     Expiration time in timer ticks. Absolute (ODP_TIMER_ABS) or
 *                 relative (ODP_TIMER_REL) expiration time.
 * @param tmo_ev   Timeout event. The event is enqueued when the timer expires.
 *
 * @return 0 on success, <0 on failure
 * @retval ODP_TIMER_SUCCESS  Success
 * @retval ODP_TIMER_TOO_NEAR Failure. Expiration time is too near to
 *                            the current time.
 * @retval ODP_TIMER_TOO_FAR  Failure. Expiration time is too far from
 *                            the current time.
 */
int odp_timer_set(odp_timer_t timer, odp_timer_type_t type, uint64_t tick,
		  odp_event_t tmo_ev);

/**
 * Set a periodic timer
 *
 * This call is valid only for periodic timers. It sets the timer to deliver
 * the timeout event periodically starting from the first expiration time
 * provided. The first expiration time must be within max_period (timer pool
 * parameter) from the current time. The requested period must not exceed
 * max_period.
 *
 * The timer must not be active when calling this function. Periodic timers
 * cannot be reset. If the period needs to be changed, the timer is first
 * cancelled and then started again with new parameters.
 *
 * The user provided event can be of any event type, but only ODP_EVENT_TIMEOUT
 * type events (odp_timeout_t) carry timeout specific metadata. Furthermore,
 * timer performance may have been optimized for that event type. The event is
 * enqueued to the destination queue of the timer on each period. Application
 * must acknowledge the event with odp_timer_ack() soon after receiving it.
 *
 * @param timer       Periodic timer
 * @param first_tick  The first expiration time in absolute timer ticks, or
 *                    zero if periodical expiration starts from the current
 *                    time. Must be within 'max_period' from the current time.
 * @param period      Timer expiration period. The period is relative to
 *                    'first_tick' and must not exceed max_period.
 * @param tmo_ev      Timeout event. The event is enqueued when the timer
 *                    expires (on each period).
 *
 * @return 0 on success, <0 on failure
 * @retval ODP_TIMER_SUCCESS  Success
 * @retval ODP_TIMER_TOO_NEAR Failure. The first expiration time is too near to
 *                            the current time, or the period is too short.
 * @retval ODP_TIMER_TOO_FAR  Failure. The first expiration time is too far from
 *                            the current time, or the period is too long.
 */
int odp_timer_set_period(odp_timer_t timer, uint64_t first_tick,
			 odp_timer_period_t period, odp_event_t tmo_ev);

/**
 * Reset a single shot timer
 *
 * This call is valid only for single shot timers (ODP_TIMER_ABS and
 * ODP_TIMER_REL). Resets an active timer with a new expiration time. Use 'type'
 * parameter to select if expiration time is passed as absolute timer ticks
 * (ODP_TIMER_ABS) or ticks relative to the current tick (ODP_TIMER_REL).
 *
 * @param timer    Single shot timer
 * @param type     ODP_TIMER_ABS or ODP_TIMER_REL
 * @param tick     New expiration time
 *
 * @return 0 on success, <0 on failure
 * @retval ODP_TIMER_SUCCESS  Success
 * @retval ODP_TIMER_TOO_NEAR Failure. Expiration time is too near to
 *                            the current time.
 * @retval ODP_TIMER_TOO_FAR  Failure. Expiration time is too far from
 *                            the current time.
 */
int odp_timer_reset(odp_timer_t timer, odp_timer_type_t type, uint64_t tick);

/**
 * Acknowledge a periodic timeout
 *
 * This call is valid only for periodic timers. Each timeout event from
 * a periodic timer must be acknowledged with this call. Acknowledgment should
 * be done soon after receiving the event to maintain period accuracy.
 * The timeout event must not be used or freed after a successful operation.
 * When the call returns failure, the event is not consumed and application
 * may use or free it normally. This happens e.g. when the timer was cancelled
 * close to the last period expiration and the timeout event is delivered
 * through the queue.
 *
 * @param timer    Periodic timer
 * @param tmo_ev   Timeout event that was received from the periodic timer
 *
 * @retval 0  Success. Timer consumed the timeout event.
 * @retval <0 Failure. Timer cancelled or inactive. The timeout event was
 *                     not consumed (application owns it).
 */
int odp_timer_ack(odp_timer_t timer, odp_event_t tmo_ev);

/**
 * Free a timer
 *
 * Free (destroy) a timer, reclaiming associated resources.
 * The timeout event for an active timer will be returned.
 * The timeout event for an expired timer will not be returned. It is the
 * responsibility of the application to handle this timeout when it is received.
 *
 * @param timer      Timer
 *
 * @return Event handle of timeout event
 * @retval ODP_EVENT_INVALID on failure
 */
odp_event_t odp_timer_free(odp_timer_t timer);

/**
 * Set (or reset) a timer with absolute expiration time
 *
 * This function sets a timer to expire at a specific time. If the timer is
 * already running (set and not yet expired), the function updates (resets) it
 * with a new expiration time and optionally with a new event. A successful
 * reset operation with a new event outputs the old event. A failed reset
 * operation does not modify the timer.
 *
 * The user provided event can be of any event type, but only ODP_EVENT_TIMEOUT
 * type events (odp_timeout_t) carry timeout specific metadata. Furthermore,
 * timer performance may have been optimized for that event type. When the timer
 * expires, the event is enqueued to the destination queue of the timer.
 *
 * @param         timer    Timer
 * @param         abs_tick Absolute expiration time in timer ticks
 * @param[in,out] tmo_ev   Pointer to an event handle. The event is enqueued
 *                         when the timer expires. Use NULL when resetting the
 *                         timer without changing the event. When resetting the
 *                         timer with a new event, a successful operation
 *                         outputs the old event here.
 *
 * @retval ODP_TIMER_SUCCESS  Success
 * @retval ODP_TIMER_TOO_NEAR Failure. Expiration time is too near to
 *                            the current time.
 * @retval ODP_TIMER_TOO_FAR  Failure. Expiration time is too far from
 *                            the current time.
 * @retval ODP_TIMER_FAIL     Failure. Set operation: No event provided.
 *                            Reset operation: Too late to reset the timer.
 *
 * @see odp_timer_set_rel(), odp_timer_alloc(), odp_timer_cancel()
 */
int odp_timer_set_abs(odp_timer_t timer, uint64_t abs_tick,
		      odp_event_t *tmo_ev);

/**
 * Set (or reset) a timer with relative expiration time
 *
 * Like odp_timer_set_abs(), but the expiration time is relative to the current
 * time: expiration tick = odp_timer_current_tick() + 'rel_tick'.
 *
 * @param         timer    Timer
 * @param         rel_tick Expiration time relative to current time of
 *                         the timer pool in timer ticks
 * @param[in,out] tmo_ev   Pointer to an event handle. The event is enqueued
 *                         when the timer expires. Use NULL when resetting the
 *                         timer without changing the event. When resetting the
 *                         timer with a new event, a successful operation
 *                         outputs the old event here.
 *
 * @retval ODP_TIMER_SUCCESS  Success
 * @retval ODP_TIMER_TOO_NEAR Failure. Expiration time is too near to
 *                            the current time.
 * @retval ODP_TIMER_TOO_FAR  Failure. Expiration time is too far from
 *                            the current time.
 * @retval ODP_TIMER_FAIL     Failure. Set operation: No event provided.
 *                            Reset operation: Too late to reset the timer.
 *
 * @see odp_timer_set_abs(), odp_timer_alloc(), odp_timer_cancel()
 */
int odp_timer_set_rel(odp_timer_t timer, uint64_t rel_tick,
		      odp_event_t *tmo_ev);

/**
 * Cancel a timer
 *
 * When cancelling a single shot timer, a successful operation prevents timer
 * expiration and returns the timeout event through 'tmo_ev' parameter. If the
 * timer has already expired, failure is returned and the timeout event will be
 * delivered to the destination queue.
 *
 * When cancelling a periodic timer, a successful operation prevents the timer
 * to expire on future periods. If the next period has expired already, the
 * timeout event will be delivered to the destination queue, otherwise it is
 * returned through 'tmo_ev' parameter. Cancel of a periodic timer returns
 * failure only in abnormal situations. Application does not acknowledge the
 * event outputted through 'tmo_ev', but may use or free it normally.
 *
 * @param      timer  Timer
 * @param[out] tmo_ev Pointer to event handle. A successful operation outputs
 *                    the timeout event or ODP_EVENT_INVALID (when the next
 *                    period expired already).
 *
 * @retval 0  Success. Active timer cancelled, timeout returned in 'tmo_ev'
 * @retval <0 Failure. Timer inactive or already expired.
 */
int odp_timer_cancel(odp_timer_t timer, odp_event_t *tmo_ev);

/**
 * Get timeout handle from a ODP_EVENT_TIMEOUT type event
 *
 * @param ev An event of type ODP_EVENT_TIMEOUT
 *
 * @return timeout handle
 */
odp_timeout_t odp_timeout_from_event(odp_event_t ev);

/**
 * Convert timeout handle to event handle
 *
 * @param tmo Timeout handle
 *
 * @return Event handle
 */
odp_event_t odp_timeout_to_event(odp_timeout_t tmo);

/**
 * Check for fresh timeout
 *
 * If the corresponding timer has been reset or cancelled since this timeout
 * was enqueued, the timeout is stale (not fresh).
 *
 * @param tmo Timeout handle
 * @retval 1 Timeout is fresh
 * @retval 0 Timeout is stale
 */
int odp_timeout_fresh(odp_timeout_t tmo);

/**
 * Return timer handle for the timeout
 *
 * @param tmo Timeout handle
 *
 * @return Timer handle
 */
odp_timer_t odp_timeout_timer(odp_timeout_t tmo);

/**
 * Timeout expiration tick
 *
 * Returns the absolute expiration time (in timer ticks) that was used to set
 * (or reset) the timer. For timers set with absolute expiration time this
 * equals the provided tick value.
 *
 * @param tmo Timeout handle
 *
 * @return Expiration tick
 */
uint64_t odp_timeout_tick(odp_timeout_t tmo);

/**
 * Return user pointer for the timeout
 *
 * The user pointer was specified when the timer was allocated.
 *
 * @param tmo Timeout handle
 *
 * @return User pointer
 */
void *odp_timeout_user_ptr(odp_timeout_t tmo);

/**
 * Timeout alloc
 *
 * Allocates timeout from pool. Pool must be created with ODP_POOL_TIMEOUT type.
 *
 * @param pool Pool handle
 *
 * @return Timeout handle
 * @retval ODP_TIMEOUT_INVALID  Timeout could not be allocated
 */
odp_timeout_t odp_timeout_alloc(odp_pool_t pool);

/**
 * Timeout free
 *
 * Frees the timeout back to the pool it was allocated from.
 *
 * @param tmo Timeout handle
 */
void odp_timeout_free(odp_timeout_t tmo);

/**
 * Print timer pool debug information
 *
 * Prints implementation specific debug information about
 * the timer pool to the ODP log.
 *
 * @param timer_pool  Timer pool handle
 */
void odp_timer_pool_print(odp_timer_pool_t timer_pool);

/**
 * Print timer debug information
 *
 * Prints implementation specific debug information about
 * the timer to the ODP log.
 *
 * @param timer       Timer handle
 */
void odp_timer_print(odp_timer_t timer);

/**
 * Print timeout debug information
 *
 * Prints implementation specific debug information about
 * the timeout to the ODP log.
 *
 * @param tmo         Timeout handle
 */
void odp_timeout_print(odp_timeout_t tmo);

/**
 * Get printable value for an odp_timer_pool_t
 *
 * @param timer_pool  odp_timer_pool_t handle to be printed
 *
 * @return uint64_t value that can be used to print/display this handle
 *
 * @note This routine is intended to be used for diagnostic purposes
 * to enable applications to generate a printable value that represents
 * an odp_timer_pool_t handle.
 */
uint64_t odp_timer_pool_to_u64(odp_timer_pool_t timer_pool);

/**
 * Get printable value for an odp_timer_t
 *
 * @param timer  odp_timer_t handle to be printed
 *
 * @return uint64_t value that can be used to print/display this handle
 *
 * @note This routine is intended to be used for diagnostic purposes
 * to enable applications to generate a printable value that represents
 * an odp_timer_t handle.
 */
uint64_t odp_timer_to_u64(odp_timer_t timer);

/**
 * Get printable value for an odp_timeout_t
 *
 * @param tmo  odp_timeout_t handle to be printed
 *
 * @return uint64_t value that can be used to print/display this handle
 *
 * @note This routine is intended to be used for diagnostic purposes
 * to enable applications to generate a printable value that represents
 * an odp_timeout_t handle.
 */
uint64_t odp_timeout_to_u64(odp_timeout_t tmo);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#include <odp/visibility_end.h>
#endif
