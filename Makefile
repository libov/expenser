FLAGS=`root-config --glibs --cflags` -g
FLAGS_OBJ=`root-config --cflags` -Wall -g

OBJ := tmp
SRC := src
INC := inc

vpath %.o   $(OBJ)
vpath %.cxx $(SRC)
vpath %.C   $(SRC)
vpath %.h   $(INC)

all: expenser.exe

expenser.exe: expenser.o TExpenser.o TExpenserDict.o
	g++ -o $@ $(OBJ)/expenser.o $(OBJ)/TExpenser.o $(OBJ)/TExpenserDict.o  $(FLAGS)

expenser.o: expenser.cxx TExpenser.h
	g++ -o $(OBJ)/$@ -c $< -I inc $(FLAGS_OBJ)

TExpenser.o: TExpenser.C TExpenser.h
	g++ -o $(OBJ)/$@ -c $< -I inc $(FLAGS_OBJ)
	
TExpenserDict.C: TExpenser.h
	rootcint -f $(OBJ)/$@ -c $< inc/TExpenserLinkDef.h

TExpenserDict.o: TExpenserDict.C
	g++ -o $(OBJ)/$@ -c $(OBJ)/$< $(FLAGS_OBJ) -I.


.PHONY: clean
clean:
	rm -rf $(OBJ)/*.o
	rm -rf $(OBJ)/*.C
	rm -rf $(OBJ)/*.h
