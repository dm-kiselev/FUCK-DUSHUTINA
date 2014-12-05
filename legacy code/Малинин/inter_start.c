#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/netmgr.h>
#include <fcntl.h>
#include <ha/ham.h>
#include <process.h>
#include <signal.h>
#include <spawn.h>
#include <sys/types.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <spawn.h>
int ham = 0;
int main(int argc, char *argv[])
{
int status;
  char *inetdpath;
    ham_entity_t *ehdl;
    ham_condition_t *chdl;
    ham_action_t *ahdl;
    pid_t pid1, pid2;
    int pid;

	int nodeid = ND_LOCAL_NODE;
    printf("nd %d\n", nodeid);
    /*system("ham");
    sleep(2);*/
    inetdpath = strdup("/home/gr10/inter/inter_server -l");
    if (ham){
    ham_connect_nd(nodeid, 0);
    ehdl = ham_attach("inter_server", nodeid, 0, inetdpath, 0);
    if (ehdl != NULL)
    {
      chdl = ham_condition(ehdl,CONDDEATH, "death", HREARMAFTERRESTART);
    if (chdl != NULL) {
        ahdl = ham_action_restart(chdl, "restart", inetdpath,
                              HREARMAFTERRESTART);
          if (ahdl == NULL)
              printf("add action failed\n");
          else
              printf("action added\n");
          }
        else
            printf("add condition failed\n");
    }
    else
        printf("add entity failed\n");
    }
    else
    {
    	spawnl(P_NOWAITO, "/home/gr10/inter/inter_server", "/home/gr10/inter/inter_server","-l");
    }
    pid2 = spawnl(P_NOWAITO, "/home/gr10/inter/inter_cont", "/home/gr10/inter/inter_cont","-l");
    sleep(1);
    pid1 = spawnl(P_NOWAITO, "/home/gr10/inter/inter_proc", "/home/gr10/inter/inter_proc","-l");
    printf("All started  %d, %d\n", pid1, pid2);
    if (ham){
    sleep(5);
    system ("slay -f inter_server");
    printf("Server dawn\n");
    sleep(5);
    system ("slay -f inter_proc");
    printf("Proc dawn\n");
    sleep(5);
    system ("slay -f inter_cont");
    printf("cont  dawn\n");
    ham_condition_remove(  chdl, NULL );
    ham_disconnect_nd(nodeid,0);
    //ham_stop();
    system ("hamctrl -stop");
    sleep(5);
    printf("Ham  dawn\n");
    }
    else
    	sleep(15);
    //ham_stop_nd(nodeid);
    system ("slay -f inter_server");
    system ("slay -f inter_proc");
    system ("slay -f inter_cont");
    printf("All  dawn\n");
    sleep(2);
    return 0;
}
