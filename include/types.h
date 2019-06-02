/*
 * types.h
 *
 *  Created on: Jun 1, 2019
 *      Author: avner
 */

#ifndef INCLUDE_TYPES_H_
#define INCLUDE_TYPES_H_

#include <set>

// define unsigned int 64 to make source more readable
typedef unsigned long long UInt64;

// File Descriptor number (32 bit is more than enough)
typedef unsigned int tFileNum;

typedef unsigned int tProcessId;
typedef std::set<tProcessId> tPidSet;


// Supported strace file operations
enum tIoOp {eRead, eWrite, eOpen, eClose};





#endif /* INCLUDE_TYPES_H_ */
