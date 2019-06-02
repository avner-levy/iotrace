/*
 * CommandLineFromPid.cpp
 *
 *  Created on: Jun 1, 2019
 *      Author: avner
 */

#include "../include/CGetProcessInfo.h"

#include <string.h>
#include <map>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <dirent.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <linux/limits.h>
#include "COutput.h"
using namespace std;

std::string CGetProcessInfo::getCommandLine(tProcessId pid)
{
	    char arg_list[16*1024];
	    size_t length;
	    char* next_arg;

	    /* Generate the name of the cmdline file for the process. */
	    ostringstream commandStream;
	    commandStream << "/proc/"<<pid<<"/cmdline";
	    /* Read the contents of the file. */
	    int fd = open (commandStream.str().c_str(), O_RDONLY);
	    if (fd==-1)
	    {
	        LOG(eError)<< "Failed to open "<<commandStream.str()<< " to get pid name";
	        if (COutput::get().warningEnabled()) perror( "Error opening file" );
	        return string("unknown");
	    }
	    length = read (fd, arg_list, sizeof (arg_list)-1);
	    close (fd);

	    /* read does not NUL-terminate the buffer, so do it here. */
	    arg_list[length] = '\0';

	    string l_sProcCmdLine;
	    /* Loop over arguments. Arguments are separated by NULs. */
	    next_arg = arg_list;
	    while (next_arg < arg_list + length) {
	        /* Print the argument. Each is NUL-terminated, so just treat it
	        like an ordinary string. */
	        l_sProcCmdLine+=next_arg+string(" ");
	        /* Advance to the next argument. Since each argument is
	        NUL-terminated, strlen counts the length of the next argument,
	        not the entire argument list. */
	        next_arg += strlen (next_arg) + 1;
	    }
	    return l_sProcCmdLine;

}

struct tSockInfo{
  struct in_addr local_addr;
  u_int local_port;
  struct in_addr remote_addr;
  u_int remote_port;
  u_char status;
  uid_t uid;
  ino_t inode;
  pid_t pid;
  short int protocol;
};
typedef map<ino_t, tSockInfo> tInod2SockInfo;

enum tSocketType {eTcp, eUdp, eRaw};
tInod2SockInfo readProtocol(tSocketType protocol)
{
	tInod2SockInfo results;
	FILE *fd=NULL;
	char line[1024];
	tSockInfo l_oSockInfo;
	switch (protocol) {
		case eTcp:
			if ((fd = fopen("/proc/net/tcp", "r")) == NULL)
				LOG(eFatal)<< "Failed to fopen(\"/proc/net/tcp\")"<<endl;
			break;
		case eUdp:
			if ((fd = fopen("/proc/net/udp", "r")) == NULL)
				LOG(eFatal)<< "fopen(\"/proc/net/udp\")"<<endl;
			break;
		case eRaw:
			if ((fd = fopen("/proc/net/raw", "r")) == NULL)
				LOG(eFatal)<< "fopen(\"/proc/net/raw\")"<<endl;
			break;
	}

	if (fd==NULL)
	{
		LOG(eFatal)<< "Failed to open protocol file" << endl;
		return results;
	}
	memset(line, 0x0, sizeof(line));

	while (fgets(line, sizeof(line)-1, fd) != NULL) {
	if (sscanf(line, "%*u: %lX:%x %lX:%x %hx %*X:%*X %*x:%*X %*x %u %*u %u",
				 (u_long *)&l_oSockInfo.local_addr, &l_oSockInfo.local_port,
		 (u_long *)&l_oSockInfo.remote_addr, &l_oSockInfo.remote_port,
				 (u_short *)&l_oSockInfo.status, (u_int *)&l_oSockInfo.uid,
				 (u_int *)&l_oSockInfo.inode) != 7)
		continue;
	switch (protocol) {
		case eTcp:	l_oSockInfo.protocol = eTcp; break;
		case eUdp:	l_oSockInfo.protocol = eUdp; break;
		case eRaw:	l_oSockInfo.protocol = eRaw; break;
	}
	results.insert(
		tInod2SockInfo::value_type(l_oSockInfo.inode, l_oSockInfo));
	}
	fclose(fd);
	return results;

}
vector<CGetProcessInfo::tFile> CGetProcessInfo::getProcessOpenFiles(tProcessId a_nPid){
	vector<tFile> results;
    LOG(ePrint) << "Scanning process "<< a_nPid << " table for files and sockets"<<endl;
    DIR *procDir = NULL;
    string folder="/proc/"+to_string(a_nPid)+"/fd";
    if ((procDir = opendir(folder.c_str())) == NULL)
    {
        LOG(eFatal) << "Failed to open "<<folder<<" to get active file list"<< endl;
        return results;
    }
    tInod2SockInfo l_oSocketInfo=readProtocol(eTcp); // we support tcp sockets for now
    struct dirent *fdent;
    while((fdent = readdir(procDir)) != NULL)
    {
        struct stat st;
	    ostringstream l_oFileName;
        if (fdent->d_name[0] != '.')
        {
        	l_oFileName<<"/proc/"<<a_nPid<<"/fd/"<<fdent->d_name;
        }
        else continue;

        if (stat(l_oFileName.str().c_str(), &st) < 0)
        {
            LOG(eFatal) << "Failed to stat file "<< l_oFileName.str()<<endl;
            continue;
        }
        if (S_ISSOCK(st.st_mode))
        {
            tInod2SockInfo::iterator l_oIter = l_oSocketInfo.find(st.st_ino);
            if (l_oIter!=l_oSocketInfo.end())
            {
                char l_sSockName[256];
                tSockInfo &l_SockData = l_oIter->second;
                string l_sLocalAddress=inet_ntoa(l_SockData.local_addr);
                string l_sRemoteAddress=inet_ntoa(l_SockData.remote_addr);

                sprintf(l_sSockName, "socket_%s:%d->%s:%d",
                    l_sLocalAddress.c_str(),
                    (int)l_SockData.local_port,
                    l_sRemoteAddress.c_str(),
                    (int)l_SockData.remote_port);
                results.push_back(tFile(stoi(fdent->d_name), l_sSockName));
            }
        }
        else
        {
            /* symbolic link */
            char targetName[PATH_MAX + 1];
            int l_nLinkLen;
            if( (l_nLinkLen=readlink( l_oFileName.str().c_str(), targetName, PATH_MAX)) != -1 )
            {
                targetName[l_nLinkLen]=0;
                results.push_back(tFile(stoi(fdent->d_name), targetName));
            } else {
                LOG(eFatal) << l_oFileName.str() << " -> (invalid symbolic link!)"<< endl;
            }
        }

    }
    closedir(procDir);
    return results;
}









