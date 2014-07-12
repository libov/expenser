#include "TClass.h"
#include "Riostream.h"
#include "TSystem.h"
#include "TEnv.h"
#include "TGExpenserTableInterface.h"
#include "TGResourcePool.h"
#include "TError.h"

ClassImp(TGExpenserTableInterface)

TGExpenserTableInterface::TGExpenserTableInterface (unsigned ncolumns) 
   : TVirtualTableInterface(),
     fNColumns(ncolumns)
{}

TGExpenserTableInterface::~TGExpenserTableInterface() {}

const char *TGExpenserTableInterface::GetRowHeader(UInt_t row) {
    return StrDup(TString::Format("Expense %d", row+1));
}

void  TGExpenserTableInterface::addCell(unsigned row, TString value) { 
    fData[row].push_back(value);
    fNRows = fData.size();
}

const char *TGExpenserTableInterface::GetValueAsString(UInt_t row, UInt_t column) {
    return fData[row][column].Data();
}

const char *TGExpenserTableInterface::GetColumnHeader(UInt_t column) {
    return fColumnNames[column].Data();
}

void TGExpenserTableInterface::setColumnNames(TString *columns) {
    fColumnNames = new TString[fNColumns];
    for (unsigned i=0; i<fNColumns; i++) {
	fColumnNames[i] = columns[i];
        cout << fColumnNames[i] << endl;
    }
}
