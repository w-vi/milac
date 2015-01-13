/*
 * Comment/uncomment TRACE_ENABLED in trace.h to disable/enable trace
 *
 *  FIX: Add another macro to trace just strings without any other value - only 1 parameter.
 * TODO: Add hexdump here.
 * TODO: Add the posibility to write to log file
 * TODO: Add the possibility to send the logs over TCP/IP
 * TODO: Use GNU fasttime library instead of gettimeofday.
 * TODO: Set some limits of TLVL.
 */

#include "trace.h"

#ifdef TRACE_ENABLED
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>

//pid_t PID;

int TLVL = 0;
int PID = 0;

// Time displayed in TRACE messages. 
// As time information is not critical for us, there are no mutex.
char * 
gettime() 
{
    struct timeval tv;
    const int timebufsiz = 9;
    char buf[timebufsiz];
    static char time[] = "18:34:22.701780";

    if (gettimeofday(&tv, 0) < 0) {
        perror("gettimeofday() error");
        return false;
    }

    if (strftime(buf, timebufsiz, "%H:%M:%S", localtime(&tv.tv_sec))
            != timebufsiz - 1) {
        fprintf(stderr, "strftime() error\n");
        return false;
    }
    sprintf(time, "%s.%06lu", buf, tv.tv_usec);
    return time;
}

int 
trace_init(char *argv0)
{
    char *str;
    char *prog_name = basename (argv0);
    char TLVL_name[128];
    PID=getpid();

    fprintf(stderr, "Process %s (pid=%d) started, Compiled with TRACE functionality.\n",
            prog_name, PID);
    sprintf (TLVL_name,"TLVL_%s", prog_name);

    str = getenv (TLVL_name);
    if (str != NULL)
    {
        TLVL=atoi(str);
        fprintf (stdout, "TRACE messages initialised to level %s=%i.\n",TLVL_name,  TLVL);
        TRACE (1, "TRACE messages initialised to level %s=%i.", TLVL_name, TLVL);
    }
    else fprintf (stderr,"To enable TRACE messages set envirovment variable %s to some integer number >0.\n",
            TLVL_name);

    TRACE (0, "To increase TRACE level use command kill -USR1 %d", PID);
    TRACE (0, "To decrease TRACE level use command kill -USR2 %d", PID);


    // Init the signals to control TLVL
    signal(SIGUSR1, catch_TLVL_inc);
    signal(SIGUSR2, catch_TLVL_dec);

    return 0;

}

// Handling of USER singnals
void catch_TLVL_inc(int sig) {
    signal(SIGUSR1, catch_TLVL_inc);
    TLVL++;
    TRACE (0, "TLVL increased TLVL=%i, sig=%i", TLVL, sig);
    return ;
}

void catch_TLVL_dec(int sig) {
    if (TLVL >0 ) TLVL--;
    TRACE (0, "TLVL decreased: TLVL=%i", TLVL);
    signal(SIGUSR2, catch_TLVL_dec);
    return ;
}


#endif /* #ifdef TRACE_ENABLED */

