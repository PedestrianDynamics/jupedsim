#ifndef _ERROR_H_
#define _ERROR_H_

typedef enum { E_NOT_INITIALIZED, E_INVALID_PARAMETER, E_UNCAUGHT_EXCEPTION } ErrorCodeEnum;

typedef void (*ErrorCallback)(ErrorCodeEnum, const char *);

/**
 * Adds a new agent to the pedestrian simulation.
 *
 * @param [in] callback The function pointer called in case of error.
 * @return              Returns 1 if the callback was set successful
 **/
int set_error_callback(ErrorCallback callback);

#endif /* _ERROR_H_ */
