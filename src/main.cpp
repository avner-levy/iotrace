/*
 * main.cpp
 *
 *  Created on: Jun 1, 2019
 *      Author: avner
 */
#include <iostream>
#include <string.h>
#include  <signal.h>

#include "constants.h"
#include "CIoTrace.h"

using namespace std;

void usage()
{
    cerr << "iotrace [-cmd command] [-p pid] [-o] [-d] [-report filename] [-i] [-f]\n"
            "-cmd command: command to trace\n"
            "-p pid: process id to trace\n"
            "-o : report online IO activity\n"
            "-d : debug\n"
            "-i : include files descriptors which their file names are unknown\n"
            "-f : follow forked processes\n"
            "-report report-file : write the IO report into report-file" << endl;
}

const char *getParam(const char *a_sParamName, const char **argv, int argc, bool a_bNoVal=false)
{
    int i;
    for (i=0; i<argc; ++i)
        if (strcmp(a_sParamName, argv[i])==0)
            break;
    if (a_bNoVal && i<argc)
        return argv[i];
    if (i+1<argc)
        return argv[i+1];
    else
        return NULL;
}

CIOTrace *iotrace=NULL;

void handler(int sig)
{
	COutput::stream(eInfo)<<"Got signal " << sig << " printing summary and exit..."<< endl;
	//CioTrace::instance().printSummary();
	//CioTrace::instance().CloseReportFile();
    exit(0);
}
int main(int argc, const char *argv[] )
{
    const char *l_sCommand = 		getParam("-cmd", argv, argc);
    const char *l_sDebug   = 		getParam("-d",   argv, argc, true);
    const char *l_sReportOnline = 	getParam("-o",   argv, argc, true);
    const char *l_sReportFile = 	getParam("-report", argv, argc);
    const char *l_sAttachProcess = 	getParam("-p", argv, argc);
    const char *l_sIncomplete = 	getParam("-i", argv, argc, true);
    const char *l_sFollowFork = 	getParam("-f", argv, argc, true);

	if (argc==1 || (!l_sCommand && !l_sAttachProcess))
	{
		usage();
	    return EXIT_ERROR;
	}

	if (!COutput::get().openReportFile(l_sReportFile)) {
		COutput::stream(eFatal)<<"Failed to open report file "<< l_sReportFile << ", exiting..." << endl;
		return EXIT_ERROR;
	}
	iotrace= new CIOTrace(l_sDebug!=NULL, l_sReportOnline!=NULL, l_sIncomplete!=NULL, l_sFollowFork!=NULL);
    signal(SIGINT, handler);

	return 0;
}

