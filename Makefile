_DEPS = pseudo_terminal.h helpers.h tsh.h session.h connection.h ssh_server.h ssh_client.h
_OBJ = pseudo_terminal.o helpers.o tsh.o session.o connection.o ssh_server.o ssh_client.o
_SOBJ = server_main.o
_COBJ = client_main.o
_TOBJ = test.o

HOSTAPPBIN = server
CLIENTAPPBIN = client
TESTBIN = budget_ssh_test

DEBUG = -DDEBUGMODE

IDIR = include
CC = g++
CFLAGS = -I$(IDIR) -Wall $(DEBUG) -Wextra -g -pthread
ODIR = obj
SSDIR = server_src
CSDIR = client_src
HDIR = helpers
LDIR = lib
TDIR = test
LIBS = -lm
XXLIBS = $(LIBS) -lstdc++ -lgtest -lgtest_main -lpthread
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))
SOBJ = $(patsubst %,$(ODIR)/%,$(_SOBJ))
COBJ = $(patsubst %,$(ODIR)/%,$(_COBJ))
TOBJ = $(patsubst %,$(ODIR)/%,$(_TOBJ)) 

$(ODIR)/%.o: $(HDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(SSDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(CSDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(TDIR)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(HOSTAPPBIN) $(CLIENTAPPBIN) $(TESTBIN) submission

$(HOSTAPPBIN): $(OBJ) $(SOBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(CLIENTAPPBIN): $(OBJ) $(COBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(TESTBIN): $(TOBJ) $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(XXLIBS)

submission:
	find . -name "*~" -exec rm -rf {} \;
	zip -r submission client_src server_src lib include


.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
	rm -f $(HOSTAPPBIN) $(CLIENTAPPBIN) $(TESTBIN)
	rm -f submission.zip
