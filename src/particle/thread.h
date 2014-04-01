
#ifndef _included_particle_thread_h
#define _included_particle_thread_h

typedef struct _ParticleThread ParticleThread;

ParticleThread *particle_thread_create(size_t (*fn)(void *), void *data);
size_t particle_thread_join(ParticleThread *pt);


#endif
