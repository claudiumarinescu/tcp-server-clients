CC=gcc
LIBSOCKET=-lnsl
CCFLAGS=-Wall -g
SRV=server
SEL_SRV=selectserver
CLT=client

PORT = 8003
IP_SERVER = 127.0.0.1

all: $(SEL_SRV) $(CLT)

$(SEL_SRV):$(SEL_SRV).c
	$(CC) -o $(SEL_SRV) $(LIBSOCKET) $(SEL_SRV).c

$(CLT):	$(CLT).c
	$(CC) -o $(CLT) $(LIBSOCKET) $(CLT).c

# Ruleaza serverul
run_server:
	./${SEL_SRV} ${PORT}

# Ruleaza clientul 	
run_client:
	./${CLT} ${IP_SERVER} ${PORT}

clean:
	rm -f *.o *~
	rm -f $(SEL_SRV) $(CLT)


