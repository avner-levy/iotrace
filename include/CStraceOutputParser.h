/*
 * CStraceOutputParser.h
 *
 *  Created on: Jun 2, 2019
 *      Author: avner
 */

#ifndef CSTRACEOUTPUTPARSER_H_
#define CSTRACEOUTPUTPARSER_H_
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

enum tStraceOpState {eStraceComplete, eStraceUnfinished, eStraceResumed};
class CStraceOutputParser {
public:
	// Result of strace line parsing
	class CStraceOperation final
	{
	public:
		tIoOp  ioOp() 			const {return m_eIoOp;}
		UInt64 readBytes() 		const {return m_nReadBytes;}
		UInt64 writtenBytes() 	const {return m_nWrittenBytes;}
		bool   incomplete() 	const {return m_eOpState==eStraceUnfinished;}
		bool   resumed() 		const {return m_eOpState==eStraceResumed;}

		tProcessId pid()		const {return m_nPid;}
		tFileNum fileNum()		const {return m_nFileNum;}
		const char * filename() const {return m_sFilename;}

	private:
		void reset() {m_eIoOp=eRead; m_nReadBytes=0; m_nWrittenBytes=0; m_eOpState=eStraceComplete; m_nPid=0; m_nFileNum=UNKNOWN_NUM;m_sFilename=nullptr;}
		tIoOp m_eIoOp;
		UInt64 m_nReadBytes;
		UInt64 m_nWrittenBytes;
		tStraceOpState m_eOpState;
		tProcessId m_nPid;
		tFileNum m_nFileNum;
		const char * m_sFilename;

		friend class CStraceOutputParser;
	};

	static bool parseStraceLine(char * a_sLine, bool a_bIncludesProcessId, CStraceOperation & a_oStraceOp);
private:
	CStraceOutputParser(){}
};

#endif /* CSTRACEOUTPUTPARSER_H_ */
