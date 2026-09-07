#ifndef CODEXION_H
# define CODEXION_H

#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h> 

typedef enum e_state
{
	COMPILING,
	DEBUGGING,
	REFACTORING
}	t_state;

typedef enum e_scheduler
{
	FIFO,
	EDF
}	t_scheduler;

typedef struct s_dongle
{
	int				id;				// dongle number
	int				in_use;			// 1 if currently held by a coder, 0 if free
	long			released_at;	// timestamp of last release - used for cooldown check
	pthread_mutex_t	lock;			// protects in_use and released_at
}	t_dongle;

typedef struct s_request
{
	struct s_coder	*coder;			// who is waiting
	long			arrival_time;	// timestamp when the request was made - used for FIFO
	long			deadline;		// last_compile_start + time_to_burnout, fixed at request time - used for EDF
}	t_request;

typedef struct s_queue
{
	t_request		*requests;	// array-based heap of pending requests
	int				size;		// how many requests are currently waiting
	int				capacity;	// allocated size of the array
	pthread_mutex_t	lock;		// protects the queue while coders insert/extract
	pthread_cond_t	cond;		// used to wake a waiting coder once granted access
}	t_queue;

typedef struct s_config
{
	int			number_of_coders;
	long		time_to_burnout;
	long		time_to_compile;
	long		time_to_debug;
	long		time_to_refactor;
	int			number_of_compiles_required;
	long		dongle_cooldown;
	t_scheduler	scheduler;
}	t_config;

typedef struct s_shared
{
	t_queue			queue;		// shared queue of pending dongle requests (fifo/edf)
	pthread_mutex_t	log_lock;	// serializes all printf/fprintf calls, so lines never mix
	int				stop;		// becomes 1 once the simulation must stop
	pthread_mutex_t	stop_lock;	// protects the stop flag from concurrent read/write
	long			start_time;	// absolute timestamp when the simulation began
	pthread_t		monitor;
}	t_shared;

typedef struct s_coder
{
	int				id;					// the id of the coder
	t_state			status;				// the current phase (the enum above)
	long			last_compile_start;	// timestamp - the beginning of the last compilation
	int				compiles_done;		// how many compilations it has
	t_dongle		*left;				// pointer to the left dongle
	t_dongle		*right;				// pointer to the right dongle
	pthread_t		thread;				// thread handle, used later for pthread_join
	pthread_mutex_t	lock;				// protects last_compile_start and compiles_done
	t_config		*config;			// pointer to shares, read_only parameters
	t_shared		*shared;			// pointer to shared mutable resources: queue, log_lock, stop flag
}	t_coder;

typedef struct	s_monitor_args
{
	t_config	*config;
	t_shared	*shared;
	t_coder		*coders;
}	t_monitor_args;

// cleanup.c

void		cleanup(t_config *config, t_shared *shared, t_dongle *dongles, t_coder *coders);

// coder.c
void		*coder_routine(void *arg);

// dongle.c

void		lock_dongles(t_coder *coder);
void		unlock_dongles(t_coder *coder);
int			acquire_dongles(t_coder *coder);

// heap.c

void		sift_up(t_queue *queue, int i, t_scheduler scheduler);
void		sift_down(t_queue *queue, int i, t_scheduler scheduler);
void		queue_push(t_queue *queue, t_request request, t_scheduler scheduler);
t_request	queue_pop(t_queue *queue, t_scheduler scheduler);

// init.c

long		get_time_ms(void);
int 		setup(t_config *config, t_shared *shared, t_dongle **dongles, t_coder **coders);

// logging.c

void		log_state(t_coder *coder, char *message);

// monitor.c

void		*monitor_routine(void *arg);

// parsing.c

int 		arg_checker(int ac, char **av);
void    	fill_config(char **av, t_config *config);

// release.c

void		release_dongles(t_coder *coder);

// scheduler.c

int			has_priority(t_request *a, t_request *b, t_scheduler scheduler);
int			is_front(t_queue *queue, t_coder *coder);
void		queue_remove(t_queue *queue, t_coder *coder, t_scheduler scheduler);

// simulation.c

int			run_simulation(t_config *config, t_shared *shared, t_coder *coders);

// sync.c

int 		is_stopped(t_shared *shared);
void		request_stop(t_shared *shared);

#endif