/*
 * CIOTrace.cpp
 *
 *  Created on: Jun 1, 2019
 *      Author: avner
 */

#include "CIOTrace.h"

CIOTrace::CIOTrace(bool debug, bool reportOnline, bool incomplete, bool followFork) :
	m_bDebug(debug),
	m_bReportOnline(reportOnline),
	m_bIncludeIncomplete(incomplete),
	m_bFollowFork(followFork),
	m_nAttachProcess(0)
{
	// TODO Auto-generated constructor stub

}

CIOTrace::~CIOTrace() {
	// TODO Auto-generated destructor stub
}

