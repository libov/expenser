#ifndef ROOT_TGExpenserTableInterface
#define ROOT_TGExpenserTableInterface

#ifndef ROOT_TVirtualTableInterface
#include "TVirtualTableInterface.h"
#endif

#include<map>
#include<vector>
using namespace std;

class TGExpenserTableInterface : public TVirtualTableInterface {

private:
   map<unsigned, vector<TString> >  fData;
   UInt_t                           fNRows;
   UInt_t                           fNColumns;
   TString                          *fColumnNames;

protected:

public:
   TGExpenserTableInterface(unsigned ncolumns);
   virtual ~TGExpenserTableInterface();

   virtual Double_t    GetValue(UInt_t row, UInt_t column) { return 0; };
   virtual const char *GetValueAsString(UInt_t row, UInt_t column);
   virtual const char *GetRowHeader(UInt_t row);
   virtual const char *GetColumnHeader(UInt_t column);
   virtual UInt_t      GetNRows() { return fNRows; }
   virtual UInt_t      GetNColumns() { return fNColumns; }
   void                addCell(unsigned row, TString value);
   void                setColumnNames(TString *columns);

   ClassDef(TGExpenserTableInterface, 0) // Interface to data in a 2D array of Double_t
};

#endif
