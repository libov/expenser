
#include <TExpense.h>

TExpense::TExpense(TString category, Float_t amount, unsigned year, unsigned month, unsigned day):
fCategory(category),
fAmount(amount),
fYear(year),
fMonth(month),
fDay(day)
{}

TExpense::~TExpense() {}
