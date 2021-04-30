OBJS = oclFeatureExt.o
CC = g++
DEBUG = -g -DDEBUG
LLVM_CONFIG=llvm-config

CPPFLAGS = -Wall -c `$(LLVM_CONFIG) --cppflags` -std=c++17 ${DEBUG}
LDFLAGS = -Wall `$(LLVM_CONFIG) --ldflags`
LIBS = `$(LLVM_CONFIG) --libs --system-libs`


EXEC = oclFeatureExt.out

all : $(EXEC)

$(EXEC) : $(OBJS)
	$(CC) $(OBJS) -o $(EXEC) $(LDFLAGS) $(LIBS) 

$(OBJS) : %.o : %.cpp
	$(CC) $(CPPFLAGS) $< -o $@

clean:
	rm -f *.o *~ $(EXEC)
