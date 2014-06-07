#include "TClass.h"
#include "Riostream.h"
#include "TSystem.h"
#include "TEnv.h"
#include "TGExpenserTableInterface.h"
#include "TGResourcePool.h"
#include "TError.h"

ClassImp(TGExpenserTableInterface)

TGExpenserTableInterface::TGExpenserTableInterface () 
   : TVirtualTableInterface()
{}

TGExpenserTableInterface::~TGExpenserTableInterface() {}

const char *TGExpenserTableInterface::GetRowHeader(UInt_t row) {
    return StrDup(TString::Format("Expense %d", row));
}

void  TGExpenserTableInterface::addCell(unsigned row, TString value) { 
    fData[row].push_back(value);
    fNRows = fData.size();
    fNColumns = 0;
    for (unsigned i=0; i<fNRows; i++) {
        if (fData[i].size() > fNColumns) fNColumns = fData[i].size();
    }
}

const char *TGExpenserTableInterface::GetValueAsString(UInt_t row, UInt_t column) {
    return fData[row][column].Data();
}

const char *TGExpenserTableInterface::GetColumnHeader(UInt_t column) {
    return fColumnNames[column];
}
