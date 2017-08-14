#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <matlabhack.h>

int exit_matlab(int *wfds, int *rfds, int pid) {
  char buf[4096];
  int rc;

  sprintf(buf, "EXIT");
  //  printf("writing exit...\n");
  rc = write(wfds[1], buf, 4096);
  
  //  printf("waiting for ready\n");
  rc = read(rfds[0], buf, 4096);

  close(wfds[0]);
  close(wfds[1]);
  close(rfds[0]);
  close(rfds[1]);
  
  //  printf("Waiting for child death\n");
  wait(pid);
  return(1);
}

char *exec_matlab(int *wfds, int *rfds, char *cmds) {
  char buf[4096], ofile[512], *ret;
  int rc, fd;

  sprintf(ofile, "/tmp/foogfile.%d", (int)(rand() % 32768));

  bzero(buf, 4096);
  sprintf(buf, "fd = fopen('%s', 'w');\n", ofile);
  rc = write(wfds[1], buf, 4096);
  rc = read(rfds[0], buf, 4096);

  rc = write(wfds[1], cmds, strlen(cmds));
  rc = read(rfds[0], buf, 4096);

  bzero(buf, 4096);
  sprintf(buf, "fprintf(fd, '%%s\\n', num2str(a));\nfclose(fd);\n\n\n");
  rc = write(wfds[1], buf, 4096);

  bzero(buf, 4096);
  rc = read(rfds[0], buf, 4096);

  if (!strcmp(buf, "READY")) {
    fd = -1;
    while(fd < 0) {
      fd = open(ofile, O_RDONLY);
    }
    ret = malloc(sizeof(char)*4096);
    bzero(ret, 4096);
    rc = 0;
    while(rc <= 0) {
      rc = read(fd, ret, 4096);
      //      printf("rc: %d\n", rc);
    }
    close(fd);
    unlink(ofile);
  } else {
    ret = NULL;
  }
    
  return(ret);

}

int fork_matlab(int *wfds, int *rfds) {
  FILE *PFH;
  int pid, rc;
  char buf[4096];
  
  srand(time(NULL));
  
  bzero(buf, 4096);
  rc = pipe(wfds);
  rc = pipe(rfds);
  
  pid = fork();
  if (pid == 0) {
    PFH = popen("/home/nurmi/my/packages/matlab-14sp3/bin/matlab -nodesktop -nodisplay >/dev/null 2>/dev/null", "w");
    //    PFH = popen("/home/nurmi/my/packages/matlab-14sp3/bin/matlab -nodesktop -nodisplay", "w");
    fprintf(PFH, "addpath /home/nurmi/matlaby\n");
    bzero(buf, 4096);
    
    while(1) {
      //      printf("ready...\n");
      rc = read(wfds[0], buf, 4096);
      //      printf("SEND TO KERNEL: %d %s\n", rc, buf);
      if (!strcmp(buf, "EXIT")) {
	sprintf(buf, "READY");
	rc = write(rfds[1], buf, 4096);
	pclose(PFH);
	exit(0);
      }
      fprintf(PFH, "%s", buf);
      fflush(PFH);
      bzero(buf, 4096);
      sprintf(buf, "READY");
      rc = write(rfds[1], buf, 4096);
    }
  } else {
    sleep(3);
  }

  /*
  fprintf(PFH, "a = 10;\n");
  fprintf(PFH, "fd = fopen('/home/nurmi/foogfile', 'w');\n");
  fprintf(PFH, "fprintf(fd, '%%d\\n', a);\n");
  fprintf(PFH, "fclose(fd);\n");
  fprintf(PFH, "exit\n");
  pclose(PFH);
  */

  return(pid);
}
 
