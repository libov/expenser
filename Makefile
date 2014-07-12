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

expenser.exe: expenser.o TExpenser.o TExpenseDict.o  TExpenserDict.o TGExpenserTableInterface.o TGExpenserTableInterfaceDict.o TExpense.o
	g++ -o $@ $(OBJ)/expenser.o $(OBJ)/TExpenser.o $(OBJ)/TExpense.o $(OBJ)/TExpenseDict.o $(OBJ)/TExpenserDict.o $(OBJ)/TGExpenserTableInterface.o $(OBJ)/TGExpenserTableInterfaceDict.o $(FLAGS) -lMLP -lTXMLParser -lXMLIO -L$(TXMLPARSER)/lib 

expenser.o: expenser.cxx TExpenser.h
	g++ -o $(OBJ)/$@ -c $< -I inc $(FLAGS_OBJ) -I$(TXMLPARSER)/inc
 
TGExpenserTableInterface.o: TGExpenserTableInterface.cxx TGExpenserTableInterface.h
	g++ -o $(OBJ)/$@ -c $< -I inc $(FLAGS_OBJ) -I$(TXMLPARSER)/inc

TGExpenserTableInterfaceDict.C: TGExpenserTableInterface.h
	rootcint -f $(OBJ)/$@ -c -I$(TXMLPARSER)/inc $< inc/TGExpenserTableInterfaceLinkDef.h

TGExpenserTableInterfaceDict.o: TGExpenserTableInterfaceDict.C
	g++ -o $(OBJ)/$@ -c $(OBJ)/$< $(FLAGS_OBJ) -I. -I$(TXMLPARSER)/inc

TExpense.o: TExpense.C TExpense.h
	g++ -o $(OBJ)/$@ -c $< -I inc $(FLAGS_OBJ)

TExpenseDict.C: TExpense.h
	rootcint -f $(OBJ)/$@ -c  $< inc/TExpenseLinkDef.h

TExpenseDict.o: TExpenseDict.C
	g++ -o $(OBJ)/$@ -c $(OBJ)/$< $(FLAGS_OBJ) -I.

TExpenser.o: TExpenser.C TExpenser.h
	g++ -o $(OBJ)/$@ -c $< -I inc $(FLAGS_OBJ) -I$(TXMLPARSER)/inc

TExpenserDict.C: TExpenser.h
	rootcint -f $(OBJ)/$@ -c -I$(TXMLPARSER)/inc $< inc/TExpenserLinkDef.h

TExpenserDict.o: TExpenserDict.C
	g++ -o $(OBJ)/$@ -c $(OBJ)/$< $(FLAGS_OBJ) -I. -I inc/ -I$(TXMLPARSER)/inc


.PHONY: clean
clean:
	rm -rf $(OBJ)/*.o
	rm -rf $(OBJ)/*.C
	rm -rf $(OBJ)/*.h
