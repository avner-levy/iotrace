/*
 * CActiveFileId.h
 *
 *  Created on: Jun 1, 2019
 *      Author: avner
 */

#ifndef CACTIVEFILEID_H_
#define CACTIVEFILEID_H_

#include "types.h"

/*
 * ID of an open file in a process. Since we can monitor multiple processes which were forked from the main process we need
 * both process ID and descriptor ID in order to identify uniquely an active file.
 * This class is used as the key in the active files map
 */
class CActiveFileId final {
public:
    CActiveFileId(UInt64 a_nPid, tFileNum a_nFileId) : m_nPid(a_nPid), m_nId(a_nFileId){}
    bool operator < (const CActiveFileId & a_oOther) const
    {
        return this->m_nPid < a_oOther.m_nPid ||
              (this->m_nPid == a_oOther.m_nPid && this->m_nId < a_oOther.m_nId);
    }
private:
    UInt64       m_nPid;
    tFileNum     m_nId;
};

#endif /* CACTIVEFILEID_H_ */
