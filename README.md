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

### Tool usage:
The tool has two main running modes like strace does, attaching to process or running a new command.
You should use either the -p or -cmd for attaching or monitoring a new process.
```
iotrace [-cmd command] [-p pid] [-o] [-d] [-report filename] [-i] [-f]
-cmd command: command to trace
-p pid: process id to trace
-o : report online IO activity
-d : debug
-i : include files descriptors which their file names are unknown
-f : follow forked processes
-report report-file : write the IO report into report-file
```

### Tool output
The tool output is in CSV format and looks like the following:
```
Filename, Read bytes, Written bytes, Opened, Read op, Write op
/dev/pts/1,1,526512,0,1,8904
socket_127.0.0.1:47948->127.0.0.1:22,1781764,396,0,8905,11
myfile.txt,65,0,9,10,0
pipe:[3339],0,0,0,1,0
```

