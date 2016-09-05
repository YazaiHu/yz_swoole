/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.in by autoheader.  */

/* Whether to build swoole as dynamic module */
#define COMPILE_DL_SWOOLE 1

/* have accept4 */
/* #undef HAVE_ACCEPT4 */

/* have clock_gettime */
/* #undef HAVE_CLOCK_GETTIME */

/* cpu affinity? */
/* #undef HAVE_CPU_AFFINITY */

/* have daemon */
#define HAVE_DAEMON 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* have epoll */
/* #undef HAVE_EPOLL */

/* have eventfd */
/* #undef HAVE_EVENTFD */

/* have execinfo */
#define HAVE_EXECINFO 1

/* have gcc aio */
#define HAVE_GCC_AIO 1

/* have hiredis */
#define HAVE_HIREDIS 1

/* have inotify */
/* #undef HAVE_INOTIFY */

/* have inotify_init1 */
/* #undef HAVE_INOTIFY_INIT1 */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* have kqueue */
#define HAVE_KQUEUE 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* have mkostemp */
/* #undef HAVE_MKOSTEMP */

/* have pthread_mutex_timedlock */
/* #undef HAVE_MUTEX_TIMEDLOCK */

/* have mysqli extension */
/* #undef HAVE_MYSQLI */

/* have mysqlnd extension */
/* #undef HAVE_MYSQLND */

/* have nghttp2 */
/* #undef HAVE_NGHTTP2 */

/* have openssl */
#define HAVE_OPENSSL 1

/* have pcre */
#define HAVE_PCRE 1

/* have pthread_barrier_init */
/* #undef HAVE_PTHREAD_BARRIER */

/* have SO_REUSEPORT? */
#define HAVE_REUSEPORT 1

/* have pthread_rwlock_init */
#define HAVE_RWLOCK 1

/* have sendfile */
#define HAVE_SENDFILE 1

/* have signalfd */
/* #undef HAVE_SIGNALFD */

/* have pthread_spin_lock */
/* #undef HAVE_SPINLOCK */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* have timerfd */
/* #undef HAVE_TIMERFD */

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
/* #undef NO_MINUS_C_MINUS_O */

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* enable async_httpclient support */
#define SW_ASYNC_HTTPCLIENT 1

/* enable async_mysql support */
#define SW_ASYNC_MYSQL 1

/* do we enable swoole debug */
/* #undef SW_DEBUG */

/* have mysqli */
#define SW_HAVE_MYSQLI 1

/* have mysqlnd */
#define SW_HAVE_MYSQLND 1

/* have zlib */
#define SW_HAVE_ZLIB 1

/* enable sockets support */
#define SW_SOCKETS 1

/* enable aerospike support */
#define SW_USE_AEROSPIKE 1

/* enable http2.0 support */
/* #undef SW_USE_HTTP2 */

/* enable openssl support */
/* #undef SW_USE_OPENSSL */

/* enable async-redis support */
#define SW_USE_REDIS 1

/* enable ringbuffer support */
/* #undef SW_USE_RINGBUFFER */

/* do we enable youzan debug */
/* #undef YZ_DEBUG */
