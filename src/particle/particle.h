
#ifndef _included_particle_h
#define _included_particle_h

//------------- Thread --------------//

typedef struct _ParticleThread ParticleThread;

ParticleThread *particle_thread_create(size_t (*fn)(void *), void *data);
size_t particle_thread_join(ParticleThread *pt);

//------------- Mutex ---------------//

#define PARTICLE_MUTEX_TYPE_NORMAL     0
#define PARTICLE_MUTEX_TYPE_RECURSIVE  1
#define PARTICLE_MUTEX_TYPE_ERRORCHECK 2
#define PARTICLE_MUTEX_TYPE_READWRITE  3

typedef struct _ParticleMutex ParticleMutex;

ParticleMutex *particle_mutex_new(unsigned int type);
void particle_mutex_free(ParticleMutex *m);

void particle_mutex_lock(ParticleMutex *m);
void particle_mutex_unlock(ParticleMutex *m);

//------- Condition Variable --------//

typedef struct _PCV PCV;

PCV *particle_condition_var_new(ParticleMutex *m);
void particle_condition_var_free(PCV *v);

void particle_condition_var_sleep(PCV *v);
void particle_condition_var_wake(PCV *v);
void particle_condition_var_wake_single(PCV *v);

#endif
