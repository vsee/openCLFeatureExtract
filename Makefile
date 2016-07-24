OBJS = oclFeatureExt.o
CC = g++
DEBUG = -g
CPPFLAGS = -Wall -c $(DEBUG)
LDFLAGS = -Wall $(DEBUG)

CPPFLAGS = -Wall -c `llvm-config --cppflags` -std=c++11
LDFLAGS = -Wall `llvm-config --ldflags`
LIBS = `llvm-config --libs --system-libs`


EXEC = oclFeatureExt.out

all : $(EXEC)

$(EXEC) : $(OBJS)
	$(CC) $(OBJS) -o $(EXEC) $(LDFLAGS) $(LIBS) 

$(OBJS) : %.o : %.cpp
	$(CC) $(CPPFLAGS) $< -o $@

clean:
	\rm *.o *~ $(EXEC)
