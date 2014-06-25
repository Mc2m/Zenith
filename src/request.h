
#ifndef _included_zenith_request_h
#define _included_zenith_request_h

typedef struct _ZRequest {
	void *data;
	unsigned char waiting : 1;

	void (*resume)(lua_State *L,struct _ZRequest *r);
	void (*clear)(struct _ZRequest *r);
} ZRequest;

typedef void (*ZRInitFunc) (ZRequest *);

size_t ZAddRequestTemplate(ZRInitFunc init);

// ============================================================================

typedef struct ZRManager ZRManager;

void ZRequestManagerInit(void);

void ZRequestManagerDestroy(void);

ZRManager *ZRequestManagerNew(lua_State *L);

void ZRequestManagerFree(ZRManager *m);

void ZRequestManagerRun(ZRManager *m);

#endif
