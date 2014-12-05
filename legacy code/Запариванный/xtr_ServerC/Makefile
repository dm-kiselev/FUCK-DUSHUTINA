DEBUG = -g
CC = qcc
PH_FLAG = -Bstatic -lph -lfont


BINS = m_server m_client m_proc m_procm m_monitor m_p_monitor m_killServer m_clientP
 
all: $(BINS)
	

m_server: 
	$(CC) -o server xtr_Server.cc
	
m_client: 
	$(CC) -o client xtr_Client.cc
	
m_clientP: 
	$(CC) -o clientPhoton xtr_ClientPhoton.cc $(PH_FLAG)
	
m_proc: 
	$(CC) -o proc xtr_ProcP.cc IMSL.cc
	
m_procm: 
	$(CC) -o procm xtr_ProcMulti.cc IMSL.cc
	
m_monitor: 
	$(CC) -o monitor xtr_Monitor.cc

m_p_monitor: 
	$(CC) -o monitor_p xtr_PhotonMonitor.cc	 $(PH_FLAG)
	
m_killServer: 
	$(CC) -o killserver xtr_killServer.cc
	

clean:
	rm -f *.o *.tmp *.so $(BINS)