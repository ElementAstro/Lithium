#pragma once

#ifndef MAIN_FUNC
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif
#endif

#include "fifo_server.hpp"
#include "driver_info.hpp"
#include "client_info.hpp"
#include "concurrent.hpp"

#define HYDROGENPORT 7624                      /* default TCP/IP port to listen */
#define HYDROGENUNIXSOCK "/tmp/hydrogenserver" /* default unix socket path (local connections) */
#define MAXSBUF 512
#define MAXRBUF 49152        /* max read buffering here */
#define MAXWSIZ 49152        /* max bytes/write */
#define SHORTMSGSIZ 2048     /* buf size for most messages */
#define DEFMAXQSIZ 128       /* default max q behind, MB */
#define DEFMAXSSIZ 5         /* default max stream behind, MB */
#define DEFMAXRESTART 10     /* default max restarts */
#define MAXFD_PER_MESSAGE 16 /* No more than 16 buffer attached to a message */
#ifdef OSX_EMBEDED_MODE
#define LOGNAME "/Users/%s/Library/Logs/hydrogenserver.log"
#define FIFONAME "/tmp/hydrogenserverFIFO"
#endif

#define STRINGIFY_TOK(x) #x
#define TO_STRING(x) STRINGIFY_TOK(x)

#ifdef USE_LIBUV
static uv_loop_t* loop = uv_default_loop();
#else
static ev::default_loop loop;
#endif
static Fifo *fifo = nullptr;
static const char *me;                                         /* our name */
static int port = HYDROGENPORT;                                    /* public HYDROGEN port */
static int verbose;                                            /* chattiness */
static char *ldir;                                             /* where to log driver messages */
static unsigned int maxqsiz = (DEFMAXQSIZ * 1024 * 1024);      /* kill if these bytes behind */
static unsigned int maxstreamsiz = (DEFMAXSSIZ * 1024 * 1024); /* drop blobs if these bytes behind while streaming*/
static int maxrestarts = DEFMAXRESTART;

#ifndef MAIN_FUNC
void run_hydrogen_server(std::unordered_map<std::string,std::string> m_params);
void start_hydrogen_driver(const std::string &driver_binary,const std::string &driver_skeleton);
void stop_hydrogen_driver(const std::string &driver_binary, const std::string &driver_lable);
#endif