OBJS = oclFeatureExt.o
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

EXEC = oclFeatureExt

all : $(EXEC)

$(EXEC) : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $(EXEC)

$(OBJS) : %.o : %.cpp
	$(CC) $(CFLAGS) $< -o $@

clean:
	\rm *.o *~ $(EXEC)
