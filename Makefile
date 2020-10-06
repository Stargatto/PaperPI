#Prefix
PREFIX = .

#OBJS specifies which files to compile as part of the project
OBJS = file_man.o poppler.o AsciiLib.o bmp.o paperpi.o main.o IT8951.o miniGUI.o

#CC specifies which compiler we're using
CC = cc

#CFLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
CFLAGS = -Wall -O3 -I$(PREFIX)/include

#LDFLAGS specifies the libraries we're linking against
LDFLAGS = -L$(PREFIX)/lib `pkg-config --libs poppler-cpp` -lbcm2835


#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = paperpi

#This is the target that compiles our executable

all : $(OBJS)
	$(CXX) $(OBJS) $(CFLAGS) $(LDFLAGS) -o $(OBJ_NAME)

poppler.o: poppler.c
	$(CXX) -c $(CFLAGS) $(LDFLAGS) $<

clean:
	-rm -f *.o paperpi

