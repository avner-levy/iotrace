/*
 * CIOTrace.h
 *
 *  Created on: Jun 1, 2019
 *      Author: avner
 */

#ifndef CIOTRACE_H_
#define CIOTRACE_H_

#include "CFileInfo.h"
#include "CActiveFileInfo.h"
#include "CPendingIoOp.h"

class CIOTrace final{
public:
	CIOTrace(bool debug, bool reportOnline, bool incomplete, bool followFork);
	virtual ~CIOTrace();
protected:
	bool m_bDebug;
	bool m_bReportOnline;
	bool m_bIncludeIncomplete;
	bool m_bFollowFork;
	// Traced pid number
	tProcessId m_nAttachProcess;
	tActiveFiles m_oActiveFilesMap;
	tFileInfoMap m_oFilesInfoMap;
	/* In some cases when tracing multiple processes when an operation takes a long
	   time the io operation is split into 2 lines in the form of:

	[pid 14123] open("/opt/lib/libstdc++.so.5", O_RDONLY <unfinished ...>
	[pid 14122] read(14,  <unfinished ...>
	[pid 14123] <... open resumed> )        = -1 ENOENT (No such file or directory)
	[pid 14122] <... read resumed> "\0\0\0\17", 4) = 4

	This is why we need the following map in order to hold incomplete requests */
	tFilePendingInfoMap m_oPedningIoOperations;
};

#endif /* CIOTRACE_H_ */
