#ifndef _ERROR_H_
#define _ERROR_H_

typedef enum { E_NOT_INITIALIZED, E_INVALID_PARAMETER, E_UNCAUGHT_EXCEPTION } JPS_ErrorCodeEnum;

typedef void (*JPS_ErrorCallback)(JPS_ErrorCodeEnum, const char *);

/**
 * Adds a new agent to the pedestrian simulation.
 *
 * @param [in] callback The function pointer called in case of error.
 * @return              Returns 1 if the callback was set successful
 **/
int JPS_set_error_callback(JPS_ErrorCallback callback);

#endif /* _ERROR_H_ */
