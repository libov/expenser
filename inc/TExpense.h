#include <TROOT.h>

class TExpense {

    public:

	TExpense(TString fCategory, Float_t fAmount, unsigned fYear, unsigned fMonth, unsigned fDay);
	~TExpense();

	TString getCategory() {return fCategory;}
	Float_t getAmount() {return fAmount;}
	unsigned getYear() {return fYear;}
	unsigned getMonth() {return fMonth;}
	unsigned getDay() {return fDay;}

    private:

        TString		fCategory;
	Float_t		fAmount;
	unsigned	fYear;
	unsigned	fMonth;
	unsigned	fDay;

};
