#ifndef LOGGING_H
#define LOGGING_H

/**
 *  @file logging.h
 *
 *  @author  Telematics Group
 *
 *  @date    08/08/2006
 */

/******** include files **********************************************/
#include <stdio.h>

/******** macro definitions ******************************************/
#ifdef TRACE_LOGGING

#define dtrace(...)                         \
    (_printDebugLine(__FILE__, __LINE__),   \
    fprintf(stderr, __VA_ARGS__),           \
    (void) fprintf(stderr, "\n"))

#define derror(...)                         \
    (_printDebugLine(__FILE__, __LINE__),   \
    fprintf(stderr, "ERROR: "),             \
    fprintf(stderr, __VA_ARGS__)            \
    )

#else

#define dtrace(...)    ((void) 0)

#define derror(...)                         \
    (fprintf(stderr, __VA_ARGS__)           \
    )

#endif /* TRACE_LOGGING */

#ifdef TRACE_LOGGING
void _printDebugLine(const std::string& fileName, int lineNumber);
#endif

#endif /* LOGGING_H */
/******** end of file logging.h **************************************/
