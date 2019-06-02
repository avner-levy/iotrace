/*
 * CIOTrace.cpp
 *
 *  Created on: Jun 1, 2019
 *      Author: avner
 */

#include <string>
#include <string.h>
#include <sstream>
#include "CIOTrace.h"
#include "CGetProcessInfo.h"
#include "constants.h"
using namespace std;

// strace output constants
const char * const	OPEN_LINE          = "openat(";
const unsigned int  OPEN_CMD_LENGTH    = strlen(OPEN_LINE);
const char * const	READ_LINE          = "read(";
const unsigned int  READ_CMD_LENGTH    = strlen(READ_LINE);
const char * const	UNFINISHED_LINE    = "<unfinished ...>";
const unsigned int 	UNFINISHED_LENGTH  = strlen(UNFINISHED_LINE);
const char * const  WRITE_LINE         = "write(";
const unsigned int  WRITE_CMD_LENGTH   = strlen(WRITE_LINE);
const char * const  CLOSE_LINE         = "close(";
const unsigned int	CLOSE_CMD_LENGTH   = strlen(CLOSE_LINE);

CIOTrace::CIOTrace(bool reportOnline, bool incomplete, bool followFork, tProcessId attachProcessId, const std::string command) :
	m_bReportOnline(reportOnline),
	m_bIncludeIncomplete(incomplete),
	m_bFollowFork(followFork),
	m_nAttachProcess(attachProcessId),
	m_sCommand(command)
{
	// TODO Auto-generated constructor stub

}

CIOTrace::~CIOTrace() {
	// TODO Auto-generated destructor stub
}
void CIOTrace::printReport(){
	LOG(eReport)<< "Filename, Read bytes, Written bytes, Opened, Read op, Write op"<<endl;

	tFileInfoMap::iterator l_oIter=m_oFilesInfoMap.begin();
	while (l_oIter!=m_oFilesInfoMap.end())
	{
		LOG(eReport)<<
			l_oIter->first.c_str()<<CSV_SEP<<
			l_oIter->second.getRead()<<CSV_SEP<<
			l_oIter->second.getWritten()<<CSV_SEP<<
			l_oIter->second.getOpened()<<CSV_SEP<<
			l_oIter->second.getReadOp()<<CSV_SEP<<
			l_oIter->second.getWriteOp()<<endl;
		++l_oIter;
	}
}

bool CIOTrace::monitor()
{
    int status;
    char l_sLine[LINE_MAX_BUFF];

    const char *TRACE_CMD= "strace -o \\!cat -e read,write,open,openat,close ";

    string l_sStraceCommand= string(TRACE_CMD) + string(m_bFollowFork ? " -f  " : "") +
        (m_nAttachProcess!=0 ? string("-p ") + to_string(m_nAttachProcess) : m_sCommand);

    LOG(eInfo) << l_sStraceCommand << endl;
    FILE*  fp = popen(l_sStraceCommand.c_str() , "r");
    if (fp == NULL)
    {
        LOG(eFatal) << "Failed to execute strace, make sure strace is installed" << endl;
        return EXIT_ERROR;
    }

    while (fgets(l_sLine, LINE_MAX_BUFF, fp) != NULL)
    {
    	processLine(l_sLine);
    }

    status = pclose(fp);
    if (status == -1) {
        LOG(eFatal) << "Failed to close strace process" << endl;
        return false;
    }
    return true;

}

// remove newline from end of line
unsigned int trimNewLine(char *a_sLine)
{
	unsigned int n=strlen(a_sLine)-1;
    a_sLine[n]=0;
    return n;
}
// returns process ID while moving line pointer after process info, update line len
tProcessId getPid(char * & a_sLine, unsigned int & a_nLen)
{
    int i=0;
    int l_nNum=0;
    while (a_sLine[i]>='0' && a_sLine[i]<='9')
    {
        l_nNum=l_nNum*10+(a_sLine[i]-'0');
        ++i;
    }
    a_sLine+=i;
    a_nLen-= i;
    while (a_sLine[0]==' ')
    {
        a_sLine+=1;
        a_nLen-= 1;
    }
    return l_nNum;
}
void CIOTrace::addProcessFilesAndSocketsToActiveFiles(tProcessId a_nPid)
{
	vector<CGetProcessInfo::tFile> activeFiles=  CGetProcessInfo::getProcessOpenFiles(a_nPid);
	for (CGetProcessInfo::tFile file : activeFiles)
		AddActiveFile(a_nPid, file.m_nFileNum, file.m_sFileName, false);
}

const char *g_saStdPrefixes[] =
{
    "stdin_",
    "stdout_",
    "stderr_"
};

void CIOTrace::RemoveActiveFile(tProcessId a_nPid, tFileNum a_nFileNum)
{
	if (!m_oActiveFilesMap.erase(CActiveFileId(a_nPid, a_nFileNum)))
	{
		LOG(eError)<< "Didn't find file id "<< a_nFileNum <<endl;
	}
	else
	{
		LOG(eInfo)<< "file id removed from active files maps" << a_nFileNum <<endl;
	}
}

void CIOTrace::AddActiveFile(tProcessId a_nPid, tFileNum a_nFileNum, const std::string & a_sFileName, bool a_bOpenOp)
{
    string l_sFullName;
    if (a_nFileNum < 3)
        l_sFullName = string(g_saStdPrefixes[a_nFileNum]) + a_sFileName;
    else
        l_sFullName = a_sFileName;

    tActiveFiles::iterator l_oIter=m_oActiveFilesMap.find(CActiveFileId(a_nPid,a_nFileNum));
    if (l_oIter!=m_oActiveFilesMap.end())
    {
        LOG(eFatal) << "Trying to add file while file num already open pid=" << a_nPid << " desc id=" <<a_nFileNum << " replacing with "<< a_sFileName << endl;
        RemoveActiveFile(a_nPid, a_nFileNum);
    }
    LOG(eInfo) << "added file " << a_sFileName << " desc id="<< a_nFileNum << " pid="<<a_nPid<< endl;
    pair<tActiveFiles::iterator, bool> l_oIterNewActiveFile;
    l_oIterNewActiveFile=m_oActiveFilesMap.insert(tActiveFiles::value_type(CActiveFileId(a_nPid, a_nFileNum), CActiveFileInfo(l_sFullName)));

    tFileInfoMap::iterator l_oFileInfoIter = m_oFilesInfoMap.find(l_sFullName);
    if (l_oFileInfoIter==m_oFilesInfoMap.end())
    {
        pair<tFileInfoMap::iterator, bool> l_oFileInfoIter;
        l_oFileInfoIter=m_oFilesInfoMap.insert(
            tFileInfoMap::value_type(l_sFullName, CFileInfo()));
        l_oIterNewActiveFile.first->second.setFileInfoIter(l_oFileInfoIter.first);
    }
    else
    {
        if (a_bOpenOp)
            l_oFileInfoIter->second.open();
        l_oIterNewActiveFile.first->second.setFileInfoIter(l_oFileInfoIter);
    }

}

string CIOTrace::genIncompleteFilenameName(tProcessId a_nPid, tFileNum a_nFileID)
{
    ostringstream commandStream;

    switch (a_nFileID)
    {
        case 0:
        	commandStream << "descriptor_pid_"<<a_nPid<<"_stdin_desc_id_"<<a_nFileID;
            break;
        case 1:
        	commandStream << "descriptor_pid_"<<a_nPid<<"_stdout_desc_id_"<<a_nFileID;
            break;
        case 2:
        	commandStream << "descriptor_pid_"<<a_nPid<<"_stderr_desc_id_"<<a_nFileID;
            break;
        default:
            addProcessFilesAndSocketsToActiveFiles(a_nPid);
            CActiveFileInfo *l_opFile=GetActiveFile(a_nPid, a_nFileID);
            // In case the file was added to active list, we return empty string
            if (!l_opFile)
            	commandStream <<"descriptor_pid_"<< a_nPid << "_desc_id_" << a_nFileID;
    };
    return commandStream.str();

}

void CIOTrace::processLine(char *a_sLine)
{
    unsigned int l_nLineLen=trimNewLine(a_sLine);
	LOG(eInfo)<<"Processing: "<<a_sLine<< endl;

    // if this line came from a sub processes in the form "[pid  xxxx]"

    // In case the flag -f wasn't added to strace, the pid for each line isn't reported
    // so we need to use the pid we got for the tracing
	tProcessId l_nPid;
    if (m_bFollowFork)
        l_nPid = getPid(a_sLine, l_nLineLen);
    else
        l_nPid=m_nAttachProcess;

    if (l_nPid>0)
    {
        tPidSet::iterator l_oIter= m_oPidSet.find(l_nPid);
        if (l_oIter==m_oPidSet.end())
        {
            string l_sProcName=CGetProcessInfo::getCommandLine(l_nPid);
            LOG(ePrint)<< "new pid "<<l_nPid<<" process "<< l_sProcName;
            // Add the files already opened into the active files list
            m_oPidSet.insert(tPidSet::value_type(l_nPid));
            addProcessFilesAndSocketsToActiveFiles(l_nPid);
        }
        else
        {
            LOG(eInfo) << "Handling pid="<< l_nPid;
        }
    }

    switch (a_sLine[0])
    {
        case 'o': // open
            if (strncmp(OPEN_LINE,a_sLine, OPEN_CMD_LENGTH)==0)
            {
            	handleOpen(l_nPid, a_sLine, l_nLineLen, "");
            	return;
            }
            break;
        case 'r': // read
        	if (strncmp(READ_LINE,a_sLine,READ_CMD_LENGTH)==0)
        	{
        		handleRead(l_nPid, a_sLine, l_nLineLen, UNKNOWN_NUM);
        		return;
        	}
            break;
        case 'w': // write
        	if (strncmp(WRITE_LINE,a_sLine,WRITE_CMD_LENGTH)==0)
        	{
        		handleWrite(l_nPid, a_sLine, l_nLineLen, UNKNOWN_NUM);
        		return;
        	}
            break;
        case 'c': // close
        	if (strncmp(CLOSE_LINE,a_sLine,CLOSE_CMD_LENGTH)==0)
        	{
        		handleClose(l_nPid, a_sLine, l_nLineLen);
        		return;
        	}
            break;
        case '<': // pending io
        	if (strncmp(UNFINISHED_LINE,a_sLine,UNFINISHED_LENGTH)==0)
        	{
        		handleUnfinished(l_nPid, a_sLine, l_nLineLen);
        		return;
        	}
            break;
    }
	LOG(ePrint)<< a_sLine<< endl;
}

bool findFirstParameterOffset(const char *a_sLine, unsigned int & offset)
{
	const char *l_sSpace=strchr(a_sLine, ' ');
	if (l_sSpace==NULL)
	{
        LOG(eFatal)<< "failed to find space in line "<<a_sLine<<endl;
		return false;
	}
	else
	{
		offset=l_sSpace-a_sLine+1;
		LOG(eInfo)<< "findFirstParameterOffset "<<offset << a_sLine+offset << " line: "<<a_sLine<<endl;
		return true;
	}
}
const char *removeQuotes(char *a_sBuff)
{
    if (a_sBuff[0]!='"')
        return NULL;
    int i;
    for (i=1; a_sBuff[i] && a_sBuff[i]!='"'; ++i);
    if (a_sBuff[i]=='"')
    {
        a_sBuff[i]=0;
        return a_sBuff+1;
    }
    else
        return NULL;
}


// Check in the end of the line if it ends with the unfinished string.
// If yes the rest of the data will apear in the following lines
bool isOperationNotComplete(const char *a_sLine, unsigned int a_nLen)
{
    bool bRet= a_nLen>=UNFINISHED_LENGTH && strcmp(UNFINISHED_LINE, &a_sLine[a_nLen-UNFINISHED_LENGTH])==0;
    if (bRet)
        LOG(eInfo)<< "Found incomplete operation"<<endl;
    return bRet;
}

unsigned int getNumFromEndOfLine(const char *a_sLine, unsigned int a_nLineLen)
{
    int l_nMul=1;
    unsigned int l_nNum=0;
    unsigned int  i=a_nLineLen-1;
    if(!(a_sLine[i]>='0' && a_sLine[i]<='9'))
    {
        return UNKNOWN_NUM;
    }
    while (a_sLine[i]>='0' && a_sLine[i]<='9')
    {
        l_nNum=l_nNum+(a_sLine[i]-'0')*l_nMul;
        l_nMul*=10;
        --i;
    }
    if (a_sLine[i]!='-')
    {
        LOG(eInfo)<< "found number " <<  l_nNum << endl;
        return l_nNum;
    }
    else
        return UNKNOWN_NUM;
}

void CIOTrace::addPendingIoOperation(
		tProcessId            	a_nPid,
		tIoOp     				a_nOp,
		tFileNum                a_nFileId,
		std::string             a_sFilename)
{
	switch (a_nOp)
	{
	case tIoOp::eRead:
	case tIoOp::eWrite:
	case tIoOp::eClose:
		m_oPedningIoOperations.insert(
			tFilePendingInfoMap::value_type(a_nPid, CPendingIoOp(a_nOp, a_nFileId)));
		LOG(eInfo)<< "Added incomplete operation data pid="<< a_nPid << ", op=" << a_nOp << "fileid=" << a_nFileId<<endl;
		break;
	case tIoOp::eOpen:
		m_oPedningIoOperations.insert(
			tFilePendingInfoMap::value_type(a_nPid, CPendingIoOp(a_nOp, a_sFilename)));
		LOG(eInfo) << "Added incomplete operation data pid="<< a_nPid << ", op=" << a_nOp << "file name=" << a_sFilename<<endl;
	   break;
	}
}

CActiveFileInfo *CIOTrace::GetActiveFile(tProcessId a_nPid, tFileNum a_nFileNum)
{
	tActiveFiles::iterator l_oIter;
	l_oIter=m_oActiveFilesMap.find(CActiveFileId(a_nPid, a_nFileNum));
	if (l_oIter==m_oActiveFilesMap.end())
		return NULL;
	else
		return &(l_oIter->second);
}

void CIOTrace::handleOpen(tProcessId a_nPid, char *a_sLine, int a_nLen, const string & a_sFilename)
{
    const char *l_sFilename=NULL;
    if (a_sFilename.length()!=0)
        l_sFilename=a_sFilename.c_str();
    else
    {
        unsigned int l_nCmdLen=0;
		if (!findFirstParameterOffset(a_sLine, l_nCmdLen))
		{
			LOG(eFatal)<< "Failed to extract filename from line " <<  a_sLine << endl;
			return;
		}
        l_sFilename=removeQuotes(a_sLine+l_nCmdLen);
		LOG(eInfo) << "Found open of file " << l_sFilename << endl;
    }
    if (l_sFilename)
    {
        if (isOperationNotComplete(a_sLine, a_nLen))
        {
            addPendingIoOperation(a_nPid, eOpen, UNKNOWN_NUM, l_sFilename);
            return;
        }
        else
        {
            tFileNum l_nFileNum=getNumFromEndOfLine(a_sLine, a_nLen);
            if (l_nFileNum==UNKNOWN_NUM)
            {
                LOG(eInfo)<< "Failed to find open descriptor num from "<< a_sLine << endl;
                return;
            }

            AddActiveFile(a_nPid, l_nFileNum, l_sFilename);
            CActiveFileInfo *l_oFile=GetActiveFile(a_nPid, l_nFileNum);
            if (l_oFile) l_oFile->open();
        }
    }
    else
        LOG(eFatal) << "Failed to get filename" << endl;
}

tFileNum getFileNum(const char *a_sLine)
{
    int i=0;
    unsigned int l_nNum=0;
    if (a_sLine[i]=='-')
        return UNKNOWN_NUM;

    while (a_sLine[i]>='0' && a_sLine[i]<='9')
    {
        l_nNum=l_nNum*10+(a_sLine[i]-'0');
        ++i;
    }
    return l_nNum;
}

void CIOTrace::UpdateRead(CActiveFileInfo *a_opActiveFile, const char *a_sLine, unsigned int a_nLineLen)
{
	unsigned int l_nReadBytes = getNumFromEndOfLine(a_sLine, a_nLineLen);
    if (l_nReadBytes==UNKNOWN_NUM)
    {
        LOG(eError) << "failed to identify how much was read" << endl;
    }
    else
    {
        if (m_bReportOnline)
            LOG(ePrint)<<a_opActiveFile->getName()<<","<<l_nReadBytes<<","<<0<<endl;
        a_opActiveFile->read(l_nReadBytes);
    }
}

void CIOTrace::handleRead(tProcessId a_nPid, char *a_sLine, unsigned int  a_nLineLen, tFileNum a_nFileId)
{
    tFileNum l_nFileNum;
    if (a_nFileId!=UNKNOWN_NUM)
        l_nFileNum = a_nFileId;
    else
        l_nFileNum=getFileNum(a_sLine+READ_CMD_LENGTH);

    if (l_nFileNum==UNKNOWN_NUM)
    {
        LOG(eError) << "Failed to get file id for read operation" << endl;
    }
    else
    {
        if (isOperationNotComplete(a_sLine, a_nLineLen))
        {
            addPendingIoOperation(a_nPid, eRead, l_nFileNum, "");
            return;
        }
        else
        {
            CActiveFileInfo *l_opActiveFile=GetActiveFile(a_nPid, l_nFileNum);
            if (l_opActiveFile)
            {
                UpdateRead(l_opActiveFile, a_sLine, a_nLineLen);
            }
            else
            {
                if (m_bIncludeIncomplete)
                {
                    string l_sIncompleteName = genIncompleteFilenameName(a_nPid, l_nFileNum);

                    // If the name returns empty it means that the file name was found already and added
                    // to the active file list
                    if (l_sIncompleteName!="")
                        AddActiveFile(a_nPid, l_nFileNum, l_sIncompleteName.c_str());
                    LOG(eInfo) << "Added active file for descriptor " << l_nFileNum << "pid " << a_nPid<<endl;
                    CActiveFileInfo *l_opActiveFile=GetActiveFile(a_nPid, l_nFileNum);
                    if (l_opActiveFile)
                    {
                        UpdateRead(l_opActiveFile, a_sLine, a_nLineLen);
                    }
                    else
                    {
                    	LOG(eError) << "couldn't find file details for " << l_nFileNum<< endl;
                    }
                }
                else {
                	LOG(eError)<< "couldn't find file details for "<< l_nFileNum <<endl;
                }
            }
        }
    }
}

void CIOTrace::updateWrite(CActiveFileInfo *a_opActiveFile, const char *a_sLine, unsigned int a_nLineLen)
{
	unsigned int l_nWriteBytes = getNumFromEndOfLine(a_sLine, a_nLineLen);
    if (l_nWriteBytes==UNKNOWN_NUM)
    {
        LOG(eError) << "failed to identify how much was write"<<endl;
    }
    else
    {
        if (m_bReportOnline)
            LOG(ePrint)<<a_opActiveFile->getName()<<0<<l_nWriteBytes<<endl;

        a_opActiveFile->write(l_nWriteBytes);
    }
}

void CIOTrace::handleWrite(tProcessId a_nPid, char *a_sLine, unsigned int a_nLineLen, tFileNum a_nFileId)
{
    tFileNum l_nFileNum;
    if (a_nFileId!=UNKNOWN_NUM)
        l_nFileNum = a_nFileId;
    else
        l_nFileNum=getFileNum(a_sLine+WRITE_CMD_LENGTH);

    if (l_nFileNum==UNKNOWN_NUM)
    {
        LOG(eError) << "bad file id for write" << endl;
    }
    else
    {
        if (isOperationNotComplete(a_sLine, a_nLineLen))
        {
            addPendingIoOperation(a_nPid, eWrite, l_nFileNum, "");
            return;
        }
        else
        {
            CActiveFileInfo *l_opActiveFile=GetActiveFile(a_nPid, l_nFileNum);
            if (l_opActiveFile)
            {
                updateWrite(l_opActiveFile, a_sLine, a_nLineLen);
            }
            else
            {
                if (m_bIncludeIncomplete)
                {
                    string l_sIncompleteName = genIncompleteFilenameName(a_nPid, l_nFileNum);

                    // If the name returns empty it means that the file name was found already and added
                    // to the active file list
                    if (l_sIncompleteName!="")
                        AddActiveFile(a_nPid, l_nFileNum, l_sIncompleteName.c_str());

                    LOG(eInfo) << "Added active file for descriptor " << l_nFileNum << endl;
                    CActiveFileInfo *l_opActiveFile=GetActiveFile(a_nPid, l_nFileNum);
                    if (l_opActiveFile)
                    {
                        updateWrite(l_opActiveFile, a_sLine, a_nLineLen);
                    }
                    else
                    	LOG(eError)<< "coultn't find file details, file num "<< l_nFileNum <<endl;
                }
                else
                	LOG(eError) << "couldn't find file details, file num  " << l_nFileNum << endl;
            }
        }
    }

}
void CIOTrace::handleClose(tProcessId a_nPid, char *a_sLine, unsigned int a_nLineLen)
{
    tFileNum l_nFileNum=getFileNum(a_sLine+CLOSE_CMD_LENGTH);
    if (l_nFileNum==UNKNOWN_NUM)
    {
        LOG(eError) <<  "Closed bad handle" << endl;
    }
    else
    {
    	LOG(eInfo) << "closing file " << l_nFileNum << endl;
        if (isOperationNotComplete(a_sLine, a_nLineLen))
        {
            addPendingIoOperation(a_nPid, eClose, l_nFileNum, "");
        }
        CActiveFileInfo *l_opActiveFile=GetActiveFile(a_nPid, l_nFileNum);
        if (l_opActiveFile)
        {
        	LOG(eInfo) << "closing active file << "<< l_opActiveFile->getName()<<endl;
            RemoveActiveFile(a_nPid, l_nFileNum);
        }
        else
        	LOG(eInfo)<< "closing an unknown active file " <<l_nFileNum << endl;
    }
}

void CIOTrace::handleUnfinished(tProcessId a_nPid, char *a_sLine, int a_nLen)
{
	tFilePendingInfoMap::iterator l_oIter;
	l_oIter=m_oPedningIoOperations.find(a_nPid);
	if (l_oIter==m_oPedningIoOperations.end())
	{
		LOG(eFatal) << "couldn't find pending operation data" << endl;
		return;
	}
	CPendingIoOp &PendOp = l_oIter->second;
	switch (PendOp.op())
	{
		case eOpen :
			handleOpen(a_nPid, a_sLine, a_nLen, PendOp.filename());
			break;
		case eClose:
			handleClose(a_nPid, a_sLine, a_nLen);
			break;
		case eRead:
			handleRead(a_nPid, a_sLine, a_nLen, PendOp.fileId());
			break;
		case eWrite:
			handleWrite(a_nPid, a_sLine, a_nLen, PendOp.fileId());
			break;
	}
	m_oPedningIoOperations.erase(l_oIter);
}
