/*
 * CPendingIoOp.h
 *
 *  Created on: Jun 1, 2019
 *      Author: avner
 */

#ifndef CPENDINGIOOP_H_
#define CPENDINGIOOP_H_

#include <string>
#include <map>
#include "types.h"


// Holds information for pending io operation

class CPendingIoOp final {
public:
	const tFileNum NULL_FILE_ID=-1;
    CPendingIoOp(tIoOp       a_nOp, tFileNum    	 a_nFileId)   : m_tOp(a_nOp), m_sFilename(), m_nFileId(a_nFileId)  {}
    CPendingIoOp(tIoOp       a_nOp, std::string      a_sFilename) : m_tOp(a_nOp), m_sFilename(a_sFilename), m_nFileId(NULL_FILE_ID) {}
    const std::string filename() const {return m_sFilename;}
    tIoOp op() const {return m_tOp;}
    tFileNum fileId() const {return m_nFileId;}
private:
    tIoOp       	m_tOp;
    std::string     m_sFilename;
    tFileNum    	m_nFileId;
};

// Keeping for each process its pending IO data
typedef std::map<tProcessId, CPendingIoOp> tFilePendingInfoMap;

#endif /* CPENDINGIOOP_H_ */
