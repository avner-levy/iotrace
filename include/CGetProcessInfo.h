/*
 * CommandLineFromPid.h
 *
 *  Created on: Jun 1, 2019
 *      Author: avner
 */

#ifndef COMMANDLINEFROMPID_H_
#define COMMANDLINEFROMPID_H_
#include <string>
#include <vector>
#include <sys/types.h>
#include "types.h"


class CGetProcessInfo {
public:
	static std::string getCommandLine(tProcessId pid);
	struct tFile {
		tFile(tFileNum 	a_nFileNum, const std::string & a_sFileName) : m_nFileNum(a_nFileNum), m_sFileName(a_sFileName){}
		tFileNum 	m_nFileNum;
		std::string m_sFileName;
	};
	static std::vector<tFile> getProcessOpenFiles(tProcessId pid);
};

#endif /* COMMANDLINEFROMPID_H_ */
