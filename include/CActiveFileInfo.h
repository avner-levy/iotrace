/*
 * CActiveFileInfo.h
 *
 *  Created on: Jun 1, 2019
 *      Author: avner
 */

#ifndef CACTIVEFILEINFO_H_
#define CACTIVEFILEINFO_H_
#include <string>
#include "COutput.h"
#include "types.h"
#include "CFileInfo.h"
#include "CActiveFileId.h"

// Holds the information of an open file
class CActiveFileInfo final
{
public:
    CActiveFileInfo(const char *a_sFilename)    		{m_sName=a_sFilename;}
    CActiveFileInfo(const std::string & a_sFilename) 	{m_sName=a_sFilename;}
    const std::string &getName()                  	 	{return m_sName;}
    void setFileInfoIter(tFileInfoMap::iterator a_oFileInfoIter)
        {m_oFileInfoIter=a_oFileInfoIter;}
    void read(UInt64 a_nRead)
        {
    		COutput::stream(eInfo)<<"Added read of " << a_nRead<< " bytes on file "<< m_sName.c_str();
            m_oFileInfoIter->second.read(a_nRead);
        }
    void write(UInt64 a_nWrite)
        {
    		COutput::stream(eInfo)<<"Added write of " << a_nWrite<< " bytes on file "<< m_sName.c_str();
            m_oFileInfoIter->second.write(a_nWrite);
        }
    void open() {m_oFileInfoIter->second.open();}
private:
    std::string       		m_sName;
    tFileInfoMap::iterator 	m_oFileInfoIter;
};

typedef std::map<CActiveFileId, CActiveFileInfo> tActiveFiles;

#endif /* CACTIVEFILEINFO_H_ */
