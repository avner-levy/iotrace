/*
 * COutput.h
 *
 *  Created on: Jun 1, 2019
 *      Author: avner
 */

#ifndef COUTPUT_H_
#define COUTPUT_H_
#include <string>
#include <ostream>
#include <fstream>

enum tErrorLevel {eFatal=0, eError, eWarning, eInfo, ePrint, eReport};
class COutput final {
public:
	static std::ostream & stream(tErrorLevel);
	static COutput& get() {
	  static COutput instance;
	  return instance;
	}
	bool openReportFile(const std::string & filename);
protected:
	static const int ERROR_LEVELS=6;
	static const char *ERROR_LEVEL_STRINGS_ARR[ERROR_LEVELS];
	std::ofstream m_oReportFile;
private:
	COutput();
	virtual ~COutput();
};

#endif /* COUTPUT_H_ */
