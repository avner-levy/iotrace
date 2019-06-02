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

enum tErrorLevel {ePrint, eReport, eInfo, eWarning, eError, eFatal};

#define LOG(logLevelMacroParam) COutput::stream(logLevelMacroParam)

class COutput final {
public:
	static std::ostream & stream(tErrorLevel);
	static COutput& get() {
	  static COutput instance;
	  return instance;
	}
	bool openReportFile(const std::string & filename);
	void closeReportFile();
	tErrorLevel level();
	void setLevel(tErrorLevel level);
	bool warningEnabled() {return m_eLoggingLevel<=eWarning;}
protected:
	static const int ERROR_LEVELS=6;
	static const char *ERROR_LEVEL_STRINGS_ARR[ERROR_LEVELS];
	std::ofstream 	m_oReportFile;
	tErrorLevel 	m_eLoggingLevel;

	class NullBuffer : public std::streambuf
	{
	public:
	  int overflow(int c) { return c; }
	};
	NullBuffer m_oNullBuffer;
	std::ostream m_oNullStream;
private:
	COutput();
	virtual ~COutput();
};

#endif /* COUTPUT_H_ */
