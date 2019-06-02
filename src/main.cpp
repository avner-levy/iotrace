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
	iotrace->printReport();
	COutput::get().closeReportFile();
    exit(0);
}

int main(int argc, const char *argv[] )
{
    const char *l_sCommand = 		getParam("-cmd", argv, argc);
    const char *l_sReportFile = 	getParam("-report", argv, argc);
    const char *l_sAttachProcess = 	getParam("-p", argv, argc);
    bool l_bIncomplete = 			getParam("-i", argv, argc, true);
    bool l_bFollowFork = 			getParam("-f", argv, argc, true);
    bool l_bDebug   = 				getParam("-d",   argv, argc, true);
    bool l_bReportOnline = 			getParam("-o",   argv, argc, true);

	if (argc==1 || (!l_sCommand && !l_sAttachProcess))
	{
		usage();
	    return EXIT_ERROR;
	}
	tProcessId attachedProcessId=0;
	if (l_sAttachProcess!=NULL)
    	attachedProcessId=std::stoi(l_sAttachProcess);
    if (l_bDebug)
		COutput::get().setLevel(eInfo);
	if (l_sReportFile!=NULL && !COutput::get().openReportFile(l_sReportFile)) {
		LOG(eFatal)<<"Failed to open report file "<< l_sReportFile << ", exiting..." << endl;
		return EXIT_ERROR;
	}
	iotrace= new CIOTrace(l_bReportOnline, l_bIncomplete, l_bFollowFork, attachedProcessId, l_sCommand?l_sCommand:"");
    signal(SIGINT, handler);
    if (!iotrace->monitor())
    	LOG(eFatal)<< "Failed to run strace and monitor process" << endl;
    LOG(ePrint) << "Finished monitoring process"<<endl;
    iotrace->printReport();
    COutput::get().closeReportFile();

	return 0;
}

