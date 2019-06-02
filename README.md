# iotrace
Monitor and generate reports on Linux process's files/sockets IO.

The tool uses the strace command in order to monitor the process's file activity.
By going over strace output and keeping information on each file operation it is able to provide a final report with the following data for each tcp socket/file:
1. Filename
2. Bytes read
3. Bytes written
4. How many times the file was opened
5. How many read operations were done
6. How many write operations were done

# Tool usage
iotrace [-cmd command] [-p pid] [-o] [-d] [-report filename] [-i] [-f]
-cmd command: command to trace
-p pid: process id to trace
-o : report online IO activity
-d : debug
-i : include files descriptors which their file names are unknown
-f : follow forked processes
-report report-file : write the IO report into report-file




