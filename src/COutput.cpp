/*
 * COutput.cpp
 *
 *  Created on: Jun 1, 2019
 *      Author: avner
 */

#include <iostream>
#include "COutput.h"
using namespace std;
COutput::COutput() {
	// TODO Auto-generated constructor stub

}

COutput::~COutput() {
	// TODO Auto-generated destructor stub
}

bool COutput::openReportFile(const string & filename)
{
	m_oReportFile.open(filename);
	return m_oReportFile.is_open();
}
const char *COutput::ERROR_LEVEL_STRINGS_ARR[]= {"Fatal: ", "Error: ", "Warning: ", "Info: ", "", ""};
ostream & COutput::stream(tErrorLevel level)
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
			return cout;
	}
	return cerr;
}
