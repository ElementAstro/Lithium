#include "eventloop.h"

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>

// Generic functions for timing and high-resolution timing
static inline int64_t get_time_in_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

static inline double get_time_in_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

// Thread-safe atomic counters
static atomic_int callback_counter = 0;
static atomic_int timer_counter = 0;
static atomic_int workproc_counter = 0;

/* Structures to hold callback, timer, and work procedure information */
typedef struct Callback {
    atomic_bool in_use; /* flag to mark this record is active */
    int fd;             /* file descriptor to watch for read */
    void *ud;           /* user's data handle */
    CBF *fp;            /* callback function */
} Callback;

static Callback *callbacks = NULL;
static int ncback = 0;   /* number of entries in callbacks[] */
static int ncbinuse = 0; /* number of entries in callbacks[] marked in_use */

/* Timers and work procedures handled similarly */
typedef struct TimerFunction {
    double trigger_time; /* trigger time, ms from epoch */
    int interval;        /* repeat timer if interval > 0, ms */
    void *ud;            /* user's data handle */
    TCF *fp;             /* timer function */
    int tid;             /* unique id for this timer */
    struct TimerFunction *next;
} TimerFunction;

static TimerFunction *timers = NULL; /* linked list of timer functions */
static int tid_counter = 0;          /* unique timer ID source */

typedef struct WorkProcedure {
    atomic_bool in_use; /* flag to mark this record is active */
    void *ud;           /* user's data handle */
    WPF *fp;            /* work procedure function */
} WorkProcedure;

static WorkProcedure *work_procs = NULL;
static int nwproc = 0;
static int nwpinuse = 0;

/* Function prototypes */
static void run_work_procs(void);
static void call_callback(fd_set *rfdp);
static void check_timers(void);
static void event_loop_iteration(void);
static void run_immediates(void);
static void insert_timer(TimerFunction *node);
static TimerFunction *find_timer(int tid);

/* Infinite loop to dispatch callbacks, work procs, and timers as necessary. */
void eventLoop(void) {
    while (true) {
        event_loop_iteration();
    }
}

/* Timer callback used to implement deferLoop and deferLoop0.
 * The argument is a pointer to an int which we set to 1 when the timer fires.
 */
static void deferTO(void *p) { *(int *)p = 1; }

/* Allow other timers/callbacks/workprocs to run until time out in maxms
 * or *flagp becomes non-zero. Wait forever if maxms is 0.
 * Return 0 if flag flipped, else -1 if it timed out without flipping.
 */
int deferLoop(int maxms, int *flagp) {
    int timeout_flag = 0;
    int timer_id = maxms > 0 ? addTimer(maxms, deferTO, &timeout_flag) : 0;

    while (!*flagp) {
        event_loop_iteration();
        if (timeout_flag) {
            return -1;  // Timer expired
        }
    }

    if (timer_id) {
        rmTimer(
            timer_id);  // Cancel the timer if the flag was set before timeout
    }
    return 0;
}

/* Allow other timers/callbacks/workprocs to run until time out in maxms
 * or *flagp becomes zero. Wait forever if maxms is 0.
 * Return 0 if flag flipped, else -1 if it timed out without flipping.
 */
int deferLoop0(int maxms, int *flagp) {
    int timeout_flag = 0;
    int timer_id = maxms > 0 ? addTimer(maxms, deferTO, &timeout_flag) : 0;

    while (*flagp) {
        event_loop_iteration();
        if (timeout_flag) {
            return -1;  // Timer expired
        }
    }

    if (timer_id) {
        rmTimer(
            timer_id);  // Cancel the timer if the flag was set before timeout
    }
    return 0;
}

/* Add a callback */
int addCallback(int fd, CBF *fp, void *ud) {
    atomic_fetch_add_explicit(&callback_counter, 1, memory_order_relaxed);
    callbacks = realloc(callbacks, callback_counter * sizeof(Callback));
    if (!callbacks) {
        perror(realloc);
        exit(EXIT_FAILURE);
    }

    Callback *cb = &callbacks[callback_counter - 1];
    atomic_store(&cb->in_use, true);
    cb->fd = fd;
    cb->fp = fp;
    cb->ud = ud;

    atomic_fetch_add_explicit(&ncbinuse, 1, memory_order_relaxed);
    return callback_counter - 1;
}

/* Remove a callback */
void rmCallback(int cid) {
    if (cid < 0 || cid >= callback_counter) {
        return;
    }

    Callback *cb = &callbacks[cid];
    if (atomic_load(&cb->in_use)) {
        atomic_store(&cb->in_use, false);
        atomic_fetch_sub_explicit(&ncbinuse, 1, memory_order_relaxed);
    }
}

/* Insert timer in sorted order */
static void insert_timer(TimerFunction *node) {
    TimerFunction **it = &timers;
    while (*it && (*it)->trigger_time < node->trigger_time) {
        it = &(*it)->next;
    }
    node->next = *it;
    *it = node;
}

/* Add a new timer */
static int add_timer_impl(int delay, int interval, TCF *fp, void *ud) {
    TimerFunction *node = malloc(sizeof(TimerFunction));
    if (!node) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    node->trigger_time = get_time_in_ms() + delay;
    node->interval = interval;
    node->fp = fp;
    node->ud = ud;
    node->tid = ++tid_counter;

    insert_timer(node);

    return node->tid;
}

int addTimer(int ms, TCF *fp, void *ud) {
    return add_timer_impl(ms, 0, fp, ud);
}

int addPeriodicTimer(int ms, TCF *fp, void *ud) {
    return add_timer_impl(ms, ms, fp, ud);
}

/* Remove a timer */
void rmTimer(int tid) {
    TimerFunction **it = &timers;
    while (*it && (*it)->tid != tid) {
        it = &(*it)->next;
    }
    if (*it) {
        TimerFunction *node = *it;
        *it = node->next;
        free(node);
    }
}

/* Return remaining time in ms for a timer */
int remainingTimer(int tid) {
    TimerFunction *it = find_timer(tid);
    if (!it) {
        return -1;
    }
    return (int)(it->trigger_time - get_time_in_ms());
}

/* Return remaining time in ns for a timer */
int64_t nsecRemainingTimer(int tid) {
    TimerFunction *it = find_timer(tid);
    if (!it) {
        return -1;
    }
    return (int64_t)((it->trigger_time - get_time_in_ms()) * 1000000);
}

/* Add a new work procedure */
int addWorkProc(WPF *fp, void *ud) {
    atomic_fetch_add_explicit(&workproc_counter, 1, memory_order_relaxed);
    work_procs = realloc(work_procs, workproc_counter * sizeof(WorkProcedure));
    if (!work_procs) {
        perror("realloc");
        exit(EXIT_FAILURE);
    }

    WorkProcedure *wp = &work_procs[workproc_counter - 1];
    atomic_store(&wp->in_use, true);
    wp->fp = fp;
    wp->ud = ud;

    atomic_fetch_add_explicit(&nwpinuse, 1, memory_order_relaxed);
    return workproc_counter - 1;
}

/* Remove a work procedure */
void rmWorkProc(int wid) {
    if (wid < 0 || wid >= workproc_counter) {
        return;
    }

    WorkProcedure *wp = &work_procs[wid];
    if (atomic_load(&wp->in_use)) {
        atomic_store(&wp->in_use, false);
        atomic_fetch_sub_explicit(&nwpinuse, 1, memory_order_relaxed);
    }
}

/* Execute next work procedure */
static void run_work_procs(void) {
    for (int i = 0; i < workproc_counter; i++) {
        WorkProcedure *wp = &work_procs[i];
        if (atomic_load(&wp->in_use)) {
            wp->fp(wp->ud);
        }
    }
}

/* Call callback functions whose file descriptors are ready */
static void call_callback(fd_set *rfdp) {
    for (int i = 0; i < callback_counter; i++) {
        Callback *cb = &callbacks[i];
        if (atomic_load(&cb->in_use) && FD_ISSET(cb->fd, rfdp)) {
            cb->fp(cb->fd, cb->ud);
        }
    }
}

/* Check and trigger timers */
static void check_timers(void) {
    double now = get_time_in_ms();
    while (timers && timers->trigger_time <= now) {
        TimerFunction *node = timers;
        node->fp(node->ud);

        if (node->interval > 0) {
            node->trigger_time += node->interval;
            TimerFunction *next_node = node->next;
            insert_timer(node);
            timers = next_node;
        } else {
            timers = node->next;
            free(node);
        }
    }
}

/* Main event loop iteration */
static void event_loop_iteration(void) {
    struct timeval tv;
    fd_set rfd;
    int maxfd = -1;

    FD_ZERO(&rfd);
    for (int i = 0; i < callback_counter; i++) {
        Callback *cb = &callbacks[i];
        if (atomic_load(&cb->in_use)) {
            FD_SET(cb->fd, &rfd);
            if (cb->fd > maxfd) {
                maxfd = cb->fd;
            }
        }
    }

    if (timers) {
        double next_timer_ms = timers->trigger_time - get_time_in_ms();
        if (next_timer_ms < 0)
            next_timer_ms = 0;
        tv.tv_sec = (time_t)(next_timer_ms / 1000);
        tv.tv_usec = (long)((next_timer_ms - tv.tv_sec * 1000) * 1000);
    } else {
        tv.tv_sec = 1;
        tv.tv_usec = 0;
    }

    int ns = select(maxfd + 1, &rfd, NULL, NULL, timers ? &tv : NULL);
    if (ns < 0) {
        perror("select");
        return;
    }

    check_timers();
    if (ns > 0) {
        call_callback(&rfd);
    }

    run_work_procs();
    run_immediates();
}

/* Immediate work management */
typedef struct ImmediateWork {
    void *ud;
    TCF *fp;
    struct ImmediateWork *next;
} ImmediateWork;

static ImmediateWork *immediates = NULL;

void addImmediateWork(TCF *fp, void *ud) {
    ImmediateWork *work = malloc(sizeof(ImmediateWork));
    if (!work) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    work->fp = fp;
    work->ud = ud;
    work->next = immediates;
    immediates = work;
}

static void run_immediates(void) {
    while (immediates) {
        ImmediateWork *work = immediates;
        immediates = work->next;
        work->fp(work->ud);
        free(work);
    }
}

static TimerFunction *find_timer(int tid) {
    TimerFunction *node = timers;
    while (node && node->tid != tid) {
        node = node->next;
    }
    return node;
}

/* "INDI" wrappers for eventloop functions */
typedef void(IE_CBF)(int readfiledes, void *userpointer);
typedef void(IE_TCF)(void *userpointer);
typedef void(IE_WPF)(void *userpointer);

int IEAddCallback(int readfiledes, IE_CBF *fp, void *p) {
    return addCallback(readfiledes, (CBF *)fp, p);
}

void IERmCallback(int callbackid) { rmCallback(callbackid); }

int IEAddTimer(int millisecs, IE_TCF *fp, void *p) {
    return addTimer(millisecs, (TCF *)fp, p);
}

int IEAddPeriodicTimer(int millisecs, IE_TCF *fp, void *p) {
    return addPeriodicTimer(millisecs, (TCF *)fp, p);
}

int IERemainingTimer(int timerid) { return remainingTimer(timerid); }

int64_t IENSecsRemainingTimer(int timerid) {
    return nsecRemainingTimer(timerid);
}

void IERmTimer(int timerid) { rmTimer(timerid); }

int IEAddWorkProc(IE_WPF *fp, void *p) { return addWorkProc((WPF *)fp, p); }

void IERmWorkProc(int workprocid) { rmWorkProc(workprocid); }

int IEDeferLoop(int maxms, int *flagp) { return deferLoop(maxms, flagp); }

int IEDeferLoop0(int maxms, int *flagp) { return deferLoop0(maxms, flagp); }

#if ENABLE_DEBUG
#include <sys/time.h>
#include <unistd.h>

int mycid;
int mywid;
int mytid;

int user_a;
int user_b;
int counter;

void wp(void *ud) {
    struct timeval tv;

    gettimeofday(&tv, NULL);
    printf("workproc @ %ld.%03ld %d %d\n", (long)tv.tv_sec,
           (long)tv.tv_usec / 1000, counter, ++(*(int *)ud));
}

void to(void *ud) { printf("timeout %d\n", (int)ud); }

void stdinCB(int fd, void *ud) {
    char c;

    if (read(fd, &c, 1) != 1) {
        perror("read");
        return;
    }

    switch (c) {
        case '+':
            counter++;
            break;
        case '-':
            counter--;
            break;

        case 'W':
            mywid = addWorkProc(wp, &user_b);
            break;
        case 'w':
            rmWorkProc(mywid);
            break;

        case 'c':
            rmCallback(mycid);
            break;

        case 't':
            rmTimer(mytid);
            break;
        case '1':
            mytid = addTimer(1000, to, (void *)1);
            break;
        case '2':
            mytid = addTimer(2000, to, (void *)2);
            break;
        case '3':
            mytid = addTimer(3000, to, (void *)3);
            break;
        case '4':
            mytid = addTimer(4000, to, (void *)4);
            break;
        case '5':
            mytid = addTimer(5000, to, (void *)5);
            break;
        default:
            return; /* silently absorb other chars like \n */
    }

    printf("callback: %d\n", ++(*(int *)ud));
}

int main(int ac, char *av[]) {
    (void)addCallback(0, stdinCB, &user_a);
    eventLoop();
    exit(0);
}
#endif