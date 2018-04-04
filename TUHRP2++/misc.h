#ifndef __MISC_H__
#define __MISC_H__

#ifndef __QNX__
#include "math.h"
#endif

#ifdef _WIN32
#include <windows.h>
#define DLLSFX	".dll"
#define DllHandle	HINSTANCE
#define DllLoad(a, opt)	LoadLibrary(a)
#define DllSymbol	GetProcAddress
#define DllUnload	FreeLibrary
#ifndef DllExport
# define DllExport	__declspec(dllexport)
#endif
#ifndef DllImport
# define DllImport	__declspec(dllimport)
#endif
#define RTLD_LAZY	0
#define RTLD_GLOBAL	0
//
#define UNUSED_VARIABLE(x) x
//
#define sem_t		HANDLE
#define sem_init(x,y,z)	{*x = CreateSemaphore(NULL, z, 256, NULL);}
#define sem_wait(x)	WaitForSingleObject(*x, INFINITE)
#define sem_post(x)	ReleaseSemaphore(*x,1,NULL)
//
#define pthread_t	HANDLE
#define pthread_create(a,b,c,d) \
	{ *a = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)c,d,0,NULL);}
#define pthread_join(a,b) WaitForSingleObject(a, INFINITE)
#define pthread_self()	GetCurrentThread()
//
#define pthread_mutex_t	HANDLE
#define pthread_mutex_init(x,y)	{*x = CreateMutex(NULL,FALSE,NULL);}
#define pthread_mutex_lock(x)	WaitForSingleObject(*x, INFINITE)
#define pthread_mutex_unlock(x)	ReleaseMutex(*x)
//
#define usleep(u)	Sleep((u)/1000)
#define pid_t		int
#define R_OK		0

#else

#include <dlfcn.h>
#include <pthread.h>
#ifdef __APPLE__
#define DLLSFX  ".dylib"
#if 1
typedef struct{
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    int count;
} unnamed_sem_t;
#define sem_t unnamed_sem_t
#define sem_init(x,y,z) { pthread_cond_init(&((x)->cond), NULL); pthread_mutex_init(&((x)->mutex), NULL); (x)->count = z;}
#define sem_wait(x) { pthread_mutex_lock(&((x)->mutex)); if ((x)->count <= 0) pthread_cond_wait(&((x)->cond), &((x)->mutex)); (x)->count--; pthread_mutex_unlock(&((x)->mutex)); }
#define sem_post(x) { pthread_mutex_lock(&((x)->mutex)); (x)->count++; pthread_cond_signal(&((x)->cond)); pthread_mutex_unlock(&((x)->mutex)); }
#else
#include <semaphore.h>
#endif
#else
#define DLLSFX	".so"
#include <semaphore.h>
#endif
#define DllHandle	void *
#define DllLoad(a,opt)	dlopen(a,opt)
#define DllSymbol	dlsym
#define DllUnload	dlclose
#define	DllExport
#define DllImport
//

#define UNUSED_VARIABLE(x)	x __attribute__((unused))

#include <unistd.h> // usleep, sleep
//
#endif

#ifndef FALSE
#define FALSE		0
#endif

#ifndef TRUE
#define TRUE		1
#endif

#define FX		0
#define FY		1
#define FZ		2
#define MX		3
#define MY		4
#define MZ		5

#if 0 // conflict with hrpModel2.h
#define X		0
#define Y		1
#define Z		2
#endif

#define WX		0
#define WY		1
#define WZ		2

#ifndef deg2rad
#define deg2rad(x)	(M_PI/180*(x))
#endif
#ifndef rad2deg
#define rad2deg(x)	((x)*180/M_PI)
#endif

#ifdef _WIN32
#define MAXHOSTNAMELEN	(64)
typedef int socklen_t;
#endif

#ifndef tick_t
#ifdef _WIN32
typedef LONGLONG tick_t;
#else
typedef unsigned long long tick_t;
#endif
#endif

tick_t DllExport get_tick();
double DllExport get_cpu_frequency();
#define tick2usec(t)	((t)*1e6/get_cpu_frequency())
#define tick2msec(t)	((t)*1e3/get_cpu_frequency())
#define tick2sec(t)	((t)/get_cpu_frequency())

#ifdef _WIN32
#ifndef M_PI
#define M_PI 3.14159265358979
#endif
#include <float.h> // for _isnan
#ifdef __cplusplus
inline int isnan(double x) { return _isnan(x); }
#else
#define isnan(x) _isnan(x)
#endif
double DllExport rint(double x);
#endif

#ifndef sqr
#define sqr(x)	((x)*(x))
#endif

#define LIMIT(v,max)	if (v > max){v=max;}else if (v < -max){v=-max;}
#define LPF(dT, omega, x, y)	((y) = (((dT)*(omega)/(1+(dT)*(omega)))*(x)+1/(1+(dT)*(omega))*(y)))

enum {INSIDE, OUTSIDE};
enum {TOE, HEEL};
enum {FB, RL};

#if defined(_WIN32) || defined(__APPLE__)
#define u_short	unsigned short
#endif
#endif // __MISC_H__
