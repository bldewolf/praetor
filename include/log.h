/*
* This source file is part of praetor, a free and open-source IRC bot,
* designed to be robust, portable, and easily extensible.
*
* Copyright (c) 2015-2017 David Zero
* All rights reserved.
*
* The following code is licensed for use, modification, and redistribution
* according to the terms of the Revised BSD License. The text of this license
* can be found in the "LICENSE" file bundled with this source distribution.
*/

#ifndef PRAETOR_LOG
#define PRAETOR_LOG

#include <stdbool.h>
#include <stdio.h>
#include <syslog.h>

/**
 * The logging facility to be used for messages being logged to the syslog.
 */
#define LOG_FACILITY LOG_DAEMON

/**
 * If this variable is set to true, logs will include debug messages.
 */
extern bool debug;

/**
 * If this variable is set to true, logs will be written to stdout instead of
 * the syslog.
 */
extern bool foreground;

/**
 * Logs messages to the foreground. This function should not be used directly
 * under normal circumstances. Use the logmsg() macro instead.
 *
 * \param loglevel One of the set of log priorities defined in <syslog.h>. Do
 *                 \b not include the logging facility. This function will use the logging
 *                 facility defined by LOG_FACILITY by default.
 * \param msg      A printf-style format string for the message to be logged
 */
void logprintf(int loglevel, char* msg, ...);

/**
 * This macro defines the logic for choosing whether to log messages to the
 * syslog, or to stdout. Use this macro wherever a message needs to be logged.
 */
#define logmsg(loglevel, ...) do{\
    if(!debug){\
        if(loglevel == LOG_DEBUG || loglevel == LOG_INFO || loglevel == LOG_NOTICE){\
            break;\
        }\
    }\
    if(!foreground){\
        syslog(LOG_FACILITY | loglevel, __VA_ARGS__);\
    }\
    else{\
        logprintf(loglevel, __VA_ARGS__);\
    }\
} while(0);

#endif
