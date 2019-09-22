CC = g++
CCC = g++
CFLAGS = -c -g
CCFLAGS = $(CFLAGS)
LDFLAGS = -g

O = parser.o source.o interpreter.o genssa.o

interpreter: $O
	$(CCC) $(LDFLAGS) $O

parser.o:
	$(CCC) $(CCFLAGS) parser.cpp

source.o:
	$(CCC) $(CCFLAGS) source.cpp

interpreter.o:
	$(CCC) $(CCFLAGS) interpreter.cpp

genSSA.o:
	$(CCC) $(CCFLAGS) genssa.cpp

clean:
	rm *.o	
