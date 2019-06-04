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
#include "CStraceOutputParser.h"

/*
 * The class responsible for running strace utility and parsing its output while doing bookkeeping of the following information per file:
 * 1. How many times opened
 * 2. How many bytes read
 * 3. How many bytes written
 * 4. How many read operations
 * 5. How many write operations
 *
 * By keeping the filename from the open operations and then accumulating the read/write operations which contains just the file descriptor id we are able
 * to generate a report with the above information summary at the end.
 * We also support online reporting which means we will report in read-time each time a file is read or written including the filename.
 * The online report format is: filename, read bytes, written bytes
 * Example: myfile.txt,0,150   => means 150 bytes were written to myfile.txt file. This makes it easier to monitor the process IO since in strace you would see
 * write(5,"asdasd")=6 which doesn't mean much since you don't know the file pointed by file descriptor 5.
 *
 */
class CIOTrace final{
public:
	/**
	 * Constructor
	 * Set the tracing main settings
	 * @param reportOnline - Print to console file IO on real-time including the file name
	 * @param incomplete   - Report file descriptors which were opened before we started monitoring and we may not have filename information for them.
	 * 						 We use /proc/pid/fd and /proc/net/tcp to try and complete missing infomration
	 * @param followFork   - Follow process child processes
	 * @param attachProcessId - Monitor an already running process with the supplied pid. Pass 0 to avoid attaching to process.
	 * @param command 		  - Run the supplied command and monitor its IO (you should pass only one of the attachProcessId / command options )
	 */
	CIOTrace(bool reportOnline, bool incomplete, bool followFork, tProcessId attachProcessId, const std::string command);
	/**
	 *
	 * @return true if strace was run and monitoring succeeded or false if some failure happened
	 */
	bool monitor();
	/**
	 * Print to cout or report file (if set in COutput) the summary of all process IO operations (files/sockets).
	 * The report is in the following CSV format:
	 * 		Filename, Read bytes, Written bytes, Opened, Read op, Write op
	 * 		/dev/pts/1,1,526512,0,1,8904
	 * 		socket_127.0.0.1:47948->127.0.0.1:22,1781764,396,0,8905,11
	 * 		myfile.txt,65,0,9,10,0
	 */
	void printReport();
	~CIOTrace();
private:
	/**
	 * Analyze an output line of strace. each line is a file operation.
	 * @param straceLine - strace output line
	 */
	void processLine(char *straceLine);
	/**
	 * Load files information such as filenames and socket complete data (source/port, destination/port) for running processes to complete missing data
	 * of proccesses which were running before we attached to them. Uses /proc/pid/fd and /proc/net/tcp
	 * @param a_nPid - process ID to load its files data into our process files data
	 */
	void addProcessFilesAndSocketsToActiveFiles(tProcessId a_nPid);
	/**
	 * Each file / socket can be opened and closed several times (sequentially and concurrently).
	 * Each time the process opens a new file/socket we add its data to the active opened files map.
	 * Then when we will encounter read/write operations we will be able to inrich the operation data and know the filename/socket data.
	 * Once the file is closed the active file data is removed.
	 * The active file points to a file info instance which is kept even after the file is closed.
	 * So when the file is opened again we continue to accumulate the file data between open/close operations.
	 * @param a_nPid - process pid
	 * @param a_nFileNum - file descriptor
	 * @param a_sFileName - filename or socket complete data
	 * @param a_bOpenOp - true for file added because we encountered an open operation or false in case we loaded it from /proc/fd for files which were already opened.
	 */
	void AddActiveFile(tProcessId a_nPid, tFileNum a_nFileNum, const std::string & a_sFileName, bool a_bOpenOp=true);
	/**
	 * Remove active file info when we encounter a close operation on the file descriptor
	 * @param a_nPid - process id
	 * @param a_nFileNum - file descriptor
	 */
	void RemoveActiveFile(tProcessId a_nPid, tFileNum a_nFileNum);

	/**
	 * Try to find the file descriptor information by loading data from /proc/fd, /proc/net/tcp for files we didn't encounter the open operation
	 * @param a_nPid - process id
	 * @param a_nFileID - file descriptor number
	 * @return the filename or socket details (src/port, dst/port). If we fail to find the information we will use the process/file descriptor to uniquely identify the file descriptor.
	 */
	std::string genIncompleteFilenameName(tProcessId a_nPid, tFileNum a_nFileID);


	// handle different strace file operations
	/**
	 * Handle file open operation. Add the filename to files data and add an active file entry for usage in following read/write/close operations (where we have only file descriptor)
	 * @param a_nPid - process id
	 * @param a_oStraceOp - Strace operation data, will contain all needed data in a complete strace operations. Will include file descriptor only in case of split strace operation report.
	 * @param a_sFilename - In case the strace split the operations into two separate lines (unfinished/resumed) we pass the filename we got from the first operation report.
	 */
	void handleOpen(const CStraceOutputParser::CStraceOpenOperation & a_oStraceOp);

	void UpdateRead(CActiveFileInfo *a_opActiveFile, const CStraceOutputParser::CStraceReadOperation & a_oStraceOp);
	void handleRead(const CStraceOutputParser::CStraceReadOperation & a_oStraceOp);
	void updateWrite(CActiveFileInfo *a_opActiveFile, const CStraceOutputParser::CStraceWriteOperation & a_oStraceOp);
	void handleWrite(const CStraceOutputParser::CStraceWriteOperation & a_oStraceOp);
	void handleClose(const CStraceOutputParser::CStraceCloseOperation & a_oStraceOp);
	void handleUnfinished(CStraceOutputParser::CStraceOperation & a_oStraceOp);

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
