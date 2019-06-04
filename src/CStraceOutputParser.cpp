/*
 * CStraceOutputParser.cpp
 *
 *  Created on: Jun 2, 2019
 *      Author: avner
 */

#include <iostream>
#include "string.h"
#include "CStraceOutputParser.h"
#include "COutput.h"
using namespace std;

// strace output constants
const char * const	OPEN_LINE          = "open";
const unsigned int  OPEN_CMD_LENGTH    = strlen(OPEN_LINE);
const char * const	READ_LINE          = "read(";
const unsigned int  READ_CMD_LENGTH    = strlen(READ_LINE);
const char * const	UNFINISHED_LINE    = "<unfinished ...>";
const unsigned int 	UNFINISHED_LENGTH  = strlen(UNFINISHED_LINE);
const char * const	RESUMED_LINE 		= "<... ";
const unsigned int 	RESUMED_LENGTH  = strlen(RESUMED_LINE);
const char * const  WRITE_LINE         = "write(";
const unsigned int  WRITE_CMD_LENGTH   = strlen(WRITE_LINE);
const char * const  CLOSE_LINE         = "close(";
const unsigned int	CLOSE_CMD_LENGTH   = strlen(CLOSE_LINE);


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

// remove newline from end of line
unsigned int trimNewLine(char *a_sLine)
{
	unsigned int n=strlen(a_sLine)-1;
    a_sLine[n]=0;
    return n;
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

// Check in the end of the line if it ends with the unfinished string.
// If yes the rest of the data will apear in the following lines
bool isOperationNotComplete(const char *a_sLine, unsigned int a_nLen)
{
    bool bRet= a_nLen>=UNFINISHED_LENGTH && strcmp(UNFINISHED_LINE, &a_sLine[a_nLen-UNFINISHED_LENGTH])==0;
    if (bRet)
        LOG(eInfo)<< "Found incomplete operation"<<endl;
    return bRet;
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

unique_ptr<CStraceOutputParser::CStraceOperation>  CStraceOutputParser::parseStraceLine(char * a_sLine, bool a_bIncludesProcessId)
{
    unsigned int l_nLineLen=trimNewLine(a_sLine);
	LOG(eInfo)<<"Processing: "<<a_sLine<< endl;
	tProcessId l_nPid = a_bIncludesProcessId ? getPid(a_sLine, l_nLineLen) : 0;
	tFileNum   l_nFileNum=UNKNOWN_NUM;
	tStraceOpState l_eOpState=eStraceComplete;

    switch (a_sLine[0])
    {
        case 'o': // open
            if (strncmp(OPEN_LINE,a_sLine, OPEN_CMD_LENGTH)==0)
            {
                unsigned int l_nCmdLen=0;
        		if (!findFirstParameterOffset(a_sLine, l_nCmdLen))
        		{
        			LOG(eFatal)<< "Failed to extract filename from line " <<  a_sLine << endl;
        			return nullptr;
        		}
                const char * l_sFilename=removeQuotes(a_sLine+l_nCmdLen);
                if (!l_sFilename)
        		{
        			LOG(eFatal)<< "Failed to remove quotes from line " <<  a_sLine << endl;
        			return nullptr;
        		}
        		LOG(eInfo) << "Found open of file " << l_sFilename << endl;
        		if (isOperationNotComplete(a_sLine, l_nLineLen))
        			l_eOpState=eStraceUnfinished;
        		else
        		{
        			l_nFileNum=getNumFromEndOfLine(a_sLine, l_nLineLen);
					if (l_nFileNum==UNKNOWN_NUM)
					{
						LOG(eInfo)<< "Failed to find open descriptor number from "<< a_sLine << endl;
						return nullptr;
					}
        		}
        		return std::make_unique<CStraceOpenOperation>(l_eOpState, l_nPid, l_sFilename, l_nFileNum);
            }
            break;
        case 'r': // read
        case 'w': // write
        	if (strncmp(READ_LINE,a_sLine,READ_CMD_LENGTH)==0 || strncmp(WRITE_LINE,a_sLine,WRITE_CMD_LENGTH)==0)
        	{
        		UInt64 l_nBytes=0;
        		l_nFileNum=getFileNum(a_sLine+READ_CMD_LENGTH);
            	if (l_nFileNum==UNKNOWN_NUM)
				{
					LOG(eInfo)<< "Failed to find read descriptor number from "<< a_sLine << endl;
					return nullptr;
				}
        		if (isOperationNotComplete(a_sLine, l_nLineLen))
        			l_eOpState=eStraceUnfinished;
        		else {
        			l_nBytes=getNumFromEndOfLine(a_sLine, l_nLineLen);
        			if (l_nBytes==UNKNOWN_NUM)
					{
						LOG(eError) << "failed to identify how much was read" << endl;
						return nullptr;
					}
        		}
        		if (strncmp(READ_LINE,a_sLine,READ_CMD_LENGTH)==0)
        			return std::make_unique<CStraceReadOperation>(l_eOpState, l_nPid, l_nFileNum, l_nBytes);
        		else
					return std::make_unique<CStraceWriteOperation>(l_eOpState, l_nPid, l_nFileNum, l_nBytes);
        	}
            break;
        case 'c': // close
        	if (strncmp(CLOSE_LINE,a_sLine,CLOSE_CMD_LENGTH)==0)
        	{
            	l_nFileNum=getFileNum(a_sLine+CLOSE_CMD_LENGTH);
            	if (l_nFileNum==UNKNOWN_NUM)
				{
					LOG(eInfo)<< "Failed to find close descriptor number from "<< a_sLine << endl;
					return nullptr;
				}
        		if (isOperationNotComplete(a_sLine, l_nLineLen))
        			l_eOpState=eStraceUnfinished;
        		return std::make_unique<CStraceCloseOperation>(l_eOpState, l_nPid, l_nFileNum);
        	}
            break;
        case '<': // resumed io operation
        	if (strncmp(RESUMED_LINE,a_sLine,RESUMED_LENGTH)==0)
        	{
        		l_eOpState=eStraceResumed;
        		a_sLine+=RESUMED_LENGTH;
        		l_nLineLen-=RESUMED_LENGTH;
        		if (strncmp(READ_LINE,a_sLine,READ_CMD_LENGTH)==0 || strncmp(WRITE_LINE,a_sLine,WRITE_CMD_LENGTH)==0)
        		{
            		UInt64 l_nBytes=0;
            		l_nBytes=getNumFromEndOfLine(a_sLine, l_nLineLen);
					if (l_nBytes==UNKNOWN_NUM)
					{
						LOG(eError) << "failed to identify how much was read from " << a_sLine << endl;
						return nullptr;
					}
	        		if (strncmp(READ_LINE,a_sLine,READ_CMD_LENGTH)==0)
	        			return std::make_unique<CStraceReadOperation>(l_eOpState, l_nPid, l_nFileNum, l_nBytes);
	        		else
						return std::make_unique<CStraceWriteOperation>(l_eOpState, l_nPid, l_nFileNum, l_nBytes);
        		}
        		else if (strncmp(OPEN_LINE,a_sLine, OPEN_CMD_LENGTH)==0)
        		{
					l_nFileNum=getNumFromEndOfLine(a_sLine, l_nLineLen);
					if (l_nFileNum==UNKNOWN_NUM)
					{
						LOG(eError) << "failed to identify open file descriptor from " << a_sLine << endl;
						return nullptr;
					}
					return std::make_unique<CStraceOpenOperation>(l_eOpState, l_nPid, "", l_nFileNum);
        		}
        		else if (strncmp(CLOSE_LINE,a_sLine,CLOSE_CMD_LENGTH)==0)
            	{
        			return std::make_unique<CStraceCloseOperation>(l_eOpState, l_nPid, l_nFileNum);
            	}
        		else return nullptr;
        	}
            break;
    }
    return nullptr;
}
