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

/*
 * Simple class to enable easy logging from code.
 * ePrint/eReport will go to cout on default.
 * If openReportFile is called, eReport will be written to report file.
 * eInfo-eFatal are written to cerr.
 * Decided to write to minimal "logging infra" to avoid external dependencies
 */
class COutput final {
public:
	// Static method to get relevant logging stream (using singleton)
	static std::ostream & stream(tErrorLevel);

	// Get singleton instance
	static COutput& get() {
	  static COutput instance;
	  return instance;
	}
	// Open report file which will be used by eReport
	bool openReportFile(const std::string & filename);
	void closeReportFile();
	// Get current logging level
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
