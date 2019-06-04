/*
 * CStraceOutputParser.h
 *
 *  Created on: Jun 2, 2019
 *      Author: avner
 */

#ifndef CSTRACEOUTPUTPARSER_H_
#define CSTRACEOUTPUTPARSER_H_
#include <string>
#include <memory>
#include "types.h"
#include "constants.h"

/*
 * Utility class to parse strace output
 *
 * Exmaple for strace output:
 *
 * openat(AT_FDCWD, "t.txt", O_RDONLY)     = 3
 * read(3, "test\n", 131072)               = 5
 * write(1, "test\n", 5)                   = 5
 * read(3, "", 131072)                     = 0
 * close(3)                                = 0
 *
 */

/** Strace file operation state
 * Strace may report each file io operation in 3 forms
 * 1. Operation start (line will include input params), 					example: 			[pid 26817] 12:48:22.972737 close(449 <unfinished ...>
 * 2. Operation resumed (line will include operation result), 				example:			[pid 26817] 12:48:22.972808 <... close resumed> ) = 0
 * 3. Operation reported in one line (both input and output data included), example:			[pid 26817] 12:48:22.972737 close(449) = 0
 */
enum tStraceOpState {eStraceComplete, eStraceUnfinished, eStraceResumed};

class CStraceOutputParser {
public:
	// Parsing output messages

	// Strace io operation base class
	class CStraceOperation
	{
	public:
		CStraceOperation(tIoOp a_eOp, tStraceOpState a_eOpState, tProcessId a_nPid, tFileNum a_nFileNum) :
			m_eIoOp(a_eOp), m_eOpState(a_eOpState), m_nPid(a_nPid), m_nFileNum(a_nFileNum){}
		virtual ~CStraceOperation() {}
		virtual tIoOp opType() const 					{return m_eIoOp;}
		virtual tStraceOpState opState() const 			{return m_eOpState;}
		virtual tProcessId pid() const 					{return m_nPid;}
		virtual tFileNum fileNum() const 				{return m_nFileNum;}
		virtual bool unfinished() const 				{return m_eOpState==eStraceUnfinished;}
		virtual bool resumed() const 					{return m_eOpState==eStraceResumed;}
		virtual bool complete() const 					{return m_eOpState==eStraceResumed;}
	protected:
		tIoOp 				m_eIoOp;
		tStraceOpState		m_eOpState;
		tProcessId 			m_nPid;
		tFileNum 			m_nFileNum;
	};

	class CStraceOpenOperation : public CStraceOperation
	{
	public:
		CStraceOpenOperation(tStraceOpState a_eOpState, tProcessId a_nPid, const std::string & a_sfilename, tFileNum a_nFileNum) :
			CStraceOperation(eOpen, a_eOpState, a_nPid, a_nFileNum), m_sFilename(a_sfilename) {}
		virtual ~CStraceOpenOperation() {}
		virtual const std::string filename() const {return m_sFilename;}
	protected:
		std::string m_sFilename;
	};

	class CStraceCloseOperation : public CStraceOperation
	{
	public:
		CStraceCloseOperation(tStraceOpState a_eOpState, tProcessId a_nPid, tFileNum a_nFileNum) :
			CStraceOperation(eClose, a_eOpState, a_nPid, a_nFileNum) {}
		virtual ~CStraceCloseOperation() {}
	};

	// Base class for data transfer messages like read/write
	class CStraceDataTransferOperation : public CStraceOperation
	{
	public:
		CStraceDataTransferOperation(tIoOp a_eOp, tStraceOpState a_eOpState, tProcessId a_nPid, tFileNum a_nFileNum, UInt64 a_nBytes) :
			CStraceOperation(a_eOp, a_eOpState, a_nPid, a_nFileNum),m_nBytes(a_nBytes) {}
		virtual ~CStraceDataTransferOperation() {}
		virtual UInt64 bytes() const {return m_nBytes;}
	protected:
		UInt64 m_nBytes;
	};

	class CStraceReadOperation : public CStraceDataTransferOperation
	{
	public:
		CStraceReadOperation(tStraceOpState a_eOpState, tProcessId a_nPid, tFileNum a_nFileNum, UInt64 a_nBytes) :
			CStraceDataTransferOperation(eRead, a_eOpState, a_nPid, a_nFileNum, a_nBytes) {}
		virtual ~CStraceReadOperation() {}
	};

	class CStraceWriteOperation : public CStraceDataTransferOperation
	{
	public:
		CStraceWriteOperation(tStraceOpState a_eOpState, tProcessId a_nPid, tFileNum a_nFileNum, UInt64 a_nBytes) :
			CStraceDataTransferOperation(eWrite, a_eOpState, a_nPid, a_nFileNum, a_nBytes) {}
		virtual ~CStraceWriteOperation() {}
	};

	static std::unique_ptr<CStraceOperation> parseStraceLine(char * a_sLine, bool a_bIncludesProcessId);
private:
	CStraceOutputParser(){}
};

#endif /* CSTRACEOUTPUTPARSER_H_ */
