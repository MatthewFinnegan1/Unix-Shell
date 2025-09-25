_DEPS = process.h parser.h executor.h 
_OBJ = executor.o parser.o process.o
_MOBJ = main.o
_TOBJ = test.o

APPBIN = shell_app
TESTBIN = shell_test

DEBUG = -DDEBUGMODE


IDIR = include
CC = g++
CFLAGS = -I$(IDIR) -Wall $(DEBUG) -Wextra -g -pthread
ODIR = obj
SDIR = src
LDIR = lib
TDIR = test
LIBS = -lm
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))
MOBJ = $(patsubst %,$(ODIR)/%,$(_MOBJ))
TOBJ = $(patsubst %,$(ODIR)/%,$(_TOBJ)) 

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(TDIR)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(APPBIN) $(TESTBIN) submission

$(APPBIN): $(OBJ) $(MOBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(TESTBIN): $(TOBJ) $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(XXLIBS)

submission:
	find . -name "*~" -exec rm -rf {} \;
	zip -r submission.zip .


.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
	rm -f $(APPBIN) $(TESTBIN)
	rm -f submission.zip
