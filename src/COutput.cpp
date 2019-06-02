/*
 * COutput.cpp
 *
 *  Created on: Jun 1, 2019
 *      Author: avner
 */

#include <iostream>
#include "COutput.h"
using namespace std;
COutput::COutput() : m_eLoggingLevel(eError), m_oNullBuffer(), m_oNullStream(&m_oNullBuffer){
	// TODO Auto-generated constructor stub
}

COutput::~COutput() {
	// TODO Auto-generated destructor stub
}

tErrorLevel COutput::level()
{
	return m_eLoggingLevel;
}
void COutput::setLevel(tErrorLevel level)
{
	m_eLoggingLevel=level;
}
void COutput::closeReportFile()
{
	if (get().m_oReportFile.is_open())
		get().m_oReportFile.close();
}
bool COutput::openReportFile(const string & filename)
{
	m_oReportFile.open(filename);
	return m_oReportFile.is_open();
}
const char *COutput::ERROR_LEVEL_STRINGS_ARR[]= {"", "",  "Info: ", "Warning: ", "Error: ", "Fatal: "};
ostream & COutput::stream(tErrorLevel level)
{
	if (level<eInfo || (level>eReport && level>=COutput::get().level()))
	{
		switch (level)
		{
			case eFatal:
			case eError:
			case eWarning:
			case eInfo:
				cerr << ERROR_LEVEL_STRINGS_ARR[level];
				return cerr;

			case ePrint:
				return cout;

			case eReport:
				return get().m_oReportFile.is_open()?get().m_oReportFile:cout;
		}
	}
	else
		return get().m_oNullStream;
	return cerr;
}
