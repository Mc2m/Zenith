
#ifndef _included_particle_mutex_h
#define _included_particle_mutex_h

#define PARTICLE_MUTEX_TYPE_NORMAL     0
#define PARTICLE_MUTEX_TYPE_RECURSIVE  1
#define PARTICLE_MUTEX_TYPE_ERRORCHECK 2
#define PARTICLE_MUTEX_TYPE_READWRITE  3

typedef struct _ParticleMutex ParticleMutex;

ParticleMutex *particle_mutex_new(unsigned int type);
void particle_mutex_free(ParticleMutex *m);

void particle_mutex_lock(ParticleMutex *m);
void particle_mutex_unlock(ParticleMutex *m);

#endif
