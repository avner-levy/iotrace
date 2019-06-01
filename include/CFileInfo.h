/*
 * CFileInfo.h
 *
 *  Created on: Jun 1, 2019
 *      Author: avner
 */

#ifndef INCLUDE_CFILEINFO_H_
#define INCLUDE_CFILEINFO_H_

#include <map>
#include <string>
#include "types.h"

class CFileInfo final
{
public:
    CFileInfo() : m_nReadBytes(0), m_nWrittenBytes(0), m_nOpened(0), m_nReadOp(0), m_nWriteOp(0) {}
    void open() {++m_nOpened;}
    void read(UInt64  a_nReadBytes)   {m_nReadBytes+=a_nReadBytes; ++m_nReadOp;}
    void write(UInt64 a_nWriteBytes)  {m_nWrittenBytes+=a_nWriteBytes; ++m_nWriteOp;}
    UInt64 getRead()     {return m_nReadBytes;}
    UInt64 getWritten()  {return m_nWrittenBytes;}
    UInt64 getReadOp()   {return m_nReadOp;}
    UInt64 getWriteOp()  {return m_nWriteOp;}
    UInt64 getOpened()   {return m_nOpened;}
private:
    UInt64     m_nReadBytes;
    UInt64     m_nWrittenBytes;
    UInt64     m_nOpened;
    UInt64     m_nReadOp;
    UInt64     m_nWriteOp;
};

typedef std::map<std::string, CFileInfo> tFileInfoMap;

#endif /* INCLUDE_CFILEINFO_H_ */
