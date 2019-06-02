/*
 * CIOTrace.h
 *
 *  Created on: Jun 1, 2019
 *      Author: avner
 */

#ifndef CIOTRACE_H_
#define CIOTRACE_H_
#include <string>
#include "CFileInfo.h"
#include "CActiveFileInfo.h"
#include "CPendingIoOp.h"

class CIOTrace final{
public:
	CIOTrace(bool reportOnline, bool incomplete, bool followFork, tProcessId attachProcessId, const std::string command);
	bool monitor();
	void printReport();
	virtual ~CIOTrace();
protected:
	void processLine(char *straceLine);
	void addProcessFilesAndSocketsToActiveFiles(tProcessId a_nPid);

	void AddActiveFile(tProcessId a_nPid, tFileNum a_nFileNum, const std::string & a_sFileName, bool a_bOpenOp=true);
	void RemoveActiveFile(tProcessId a_nPid, tFileNum a_nFileNum);


	std::string genIncompleteFilenameName(tProcessId a_nPid, tFileNum a_nFileID);


	// handle different strace file operations
	void handleOpen(tProcessId a_nPid, char *a_sLine, int a_nLen, const std::string &a_sFilename);

	void UpdateRead(CActiveFileInfo *a_opActiveFile, const char *a_sLine, unsigned int a_nLineLen);
	void handleRead(tProcessId a_nPid, char *a_sLine, unsigned int a_nLineLen, tFileNum a_nFileId);

	void updateWrite(CActiveFileInfo *a_opActiveFile, const char *a_sLine, unsigned int a_nLineLen);
	void handleWrite(tProcessId a_nPid, char *a_sLine, unsigned int a_nLineLen, tFileNum a_nFileId);
	void handleClose(tProcessId a_nPid, char *a_sLine, unsigned int a_nLineLen);
	void handleUnfinished(tProcessId a_nPid, char *a_sLine, int a_nLen);

	void addPendingIoOperation(
						tProcessId            	a_nPid,
						tIoOp     				a_nOp,
						tFileNum                a_nFileId,
						std::string             a_sFilename);


	CActiveFileInfo *GetActiveFile(tProcessId a_nPid, tFileNum a_nFileNum);

	bool m_bReportOnline;
	bool m_bIncludeIncomplete;
	bool m_bFollowFork;
	tProcessId m_nAttachProcess; 	// Traced pid number
	std::string m_sCommand; 		// Command to run and monitor
	tActiveFiles m_oActiveFilesMap;
	tFileInfoMap m_oFilesInfoMap;
	tPidSet		 m_oPidSet;
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
