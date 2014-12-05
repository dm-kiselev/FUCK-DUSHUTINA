DEBUG = -g
CC = qcc



BINS = m_server m_client

all: $(BINS)
	

m_server: 
	$(CC) -o server xtr_ServerX.cc
	
m_client: 
	$(CC) -o client xtr_Client.cc
	
m_proc: 
	$(CC) -o proc xtr_ProcP.cc IMSL.cc
	
m_procm: 
	$(CC) -o procm xtr_ProcMulti.cc IMSL.cc
	
m_monitor: 
	$(CC) -o monitor xtr_Monitor.cc
	

clean:
	rm -f *.o *.tmp *.so $(BINS)