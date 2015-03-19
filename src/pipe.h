
#ifndef _included_zenith_pipe_h
#define _included_zenith_pipe_h

void ZPipeInitialize(void);
void ZPipeDestroy(void);

/**
 * Send data to another state through a pipe
 *
 * @param L                   The lua state.
 *                            This state must contain the pipe
 *                            then a delay value (0 if not waiting for an
 *                            answer) and at least one data to be sent.
 * @param idx                 The index of the pipe in the state.
 *
 * @return                    The number of parameters returned.
 *                            Returns 1 and a string on the stack if
 *                            an error occured.
 */
int ZPipeSend(lua_State *L, int idx);

/**
 * Retrieve data from another state through a pipe
 *
 * @param L                   The lua state.
 *                            This state must contain the pipe
 *                            then a listen duration (0 if not listening)
 *                            and optionally a boolean if all the data
 *                            must be recovered (false means retrieve
 *                            the first group of data send).
 * @param idx                 The index of the pipe in the state.
 *
 * @return                    The amount of data recovered.
 */
int ZPipeReceive(lua_State *L, int idx);

void ZPipeCreate(const char *name, lua_State *L1, lua_State *L2);

void ZPipeRegister(lua_State *L);

#endif
