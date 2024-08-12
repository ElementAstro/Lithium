#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/types.h>
#include <threads.h>
#include <unistd.h>

// Platform-specific includes for Windows
#ifdef _WIN32
#include <windows.h>
#endif

/** \typedef CBF
    \brief Signature of a callback function.
*/
typedef void(CBF)(int fd, void *);

/** \typedef WPF
    \brief Signature of a work procedure function.
*/
typedef void(WPF)(void *);

/** \typedef TCF
    \brief Signature of a timer function.
*/
typedef void(TCF)(void *);

#ifdef __cplusplus
extern "C" {
#endif

/** \fn void eventLoop()
    \brief Main calls this when ready to hand over control.
*/
extern void eventLoop();

/** Register a new callback, \e fp, to be called with \e ud as argument when \e
 * fd is ready.
 *
 * \param fd file descriptor.
 * \param fp a pointer to the callback function.
 * \param ud a pointer to be passed to the callback function when called.
 * \return a unique callback id for use with rmCallback().
 */
extern int addCallback(int fd, CBF *fp, void *ud);

/** Remove a callback function.
 *
 * \param cid the callback ID returned from addCallback().
 */
extern void rmCallback(int cid);

/** Add a new work procedure, fp, to be called with ud when nothing else to do.
 *
 * \param fp a pointer to the work procedure callback function.
 * \param ud a pointer to be passed to the callback function when called.
 * \return a unique id for use with rmWorkProc().
 */
extern int addWorkProc(WPF *fp, void *ud);

/** Remove the work procedure with the given \e id, as returned from
 * addWorkProc().
 *
 * \param wid the work procedure callback ID returned from addWorkProc().
 */
extern void rmWorkProc(int wid);

/** Register a new single-shot timer function, \e fp, to be called with \e ud as
 * argument after \e ms.
 *
 * \param ms timer period in milliseconds.
 * \param fp a pointer to the callback function.
 * \param ud a pointer to be passed to the callback function when called.
 * \return a unique id for use with rmTimer().
 */
extern int addTimer(int ms, TCF *fp, void *ud);

/** Register a new periodic timer function, \e fp, to be called with \e ud as
 * argument after \e ms.
 *
 * \param ms timer period in milliseconds.
 * \param fp a pointer to the callback function.
 * \param ud a pointer to be passed to the callback function when called.
 * \return a unique id for use with rmTimer().
 */
extern int addPeriodicTimer(int ms, TCF *fp, void *ud);

/** Returns the timer's remaining value in milliseconds left until the timeout.
 *
 * \param tid the timer callback ID returned from addTimer() or
 * addPeriodicTimer() \return  If the timer not exists, the returned value will
 * be -1.
 */
extern int remainingTimer(int tid);

/** Returns the timer's remaining value in nanoseconds left until the timeout.
 *
 * \param tid the timer callback ID returned from addTimer() or
 * addPeriodicTimer() \return  If the timer not exists, the returned value will
 * be -1.
 */
extern int64_t nsecRemainingTimer(int tid);

/** Remove the timer with the given \e id, as returned from addTimer() or
 * addPeriodicTimer().
 *
 * \param tid the timer callback ID returned from addTimer() or
 * addPeriodicTimer。
 */
extern void rmTimer(int tid);

/** Register a given function to be called once after the current loop
 * \param fp a pointer to the callback function.
 * \param ud a pointer to be passed to the callback function when called.
 */
extern void addImmediateWork(TCF *fp, void *ud);

/* utility functions */
extern int deferLoop(int maxms, int *flagp);
extern int deferLoop0(int maxms, int *flagp);

#ifdef __cplusplus
}
#endif
