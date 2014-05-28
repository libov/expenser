#include<TGFrame.h>
#include<TGNumberEntry.h>
#include<TGTextEntry.h>
#include <TGButton.h>
#include <TApplication.h>
#include <TGComboBox.h>
#include <TDatime.h>

#include<TExpenser.h>

#include<iostream>
using namespace std;

// ========== helping functions ========== //
TString toStr(Double_t arg, Int_t decimals);
TString toStr(Int_t arg);

TString toStr(Double_t arg, Int_t decimals) {
    char tmp[256];
    TString format = "%."+toStr(decimals)+"f";
    sprintf(tmp, format, arg);
    TString result(tmp);
    return result;
}

TString toStr(Int_t arg) {
    TString result;
    result += arg;
    return result;
}

// constants
const unsigned NCATEGORIES = 3;
TString CATEGORIES[]  = {"Food", "Restaurant", "Flat"};

TExpenser::TExpenser(const TGWindow *p, UInt_t w, UInt_t h):
TGMainFrame(p,w,h)
{
    // create a frame holding all widgets
    TGHorizontalFrame *hframe = new TGHorizontalFrame(this, 500, 40);

    // expense amount entry field
    fAmountEntry = new TGNumberEntryField(hframe, 0, 0, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEAAnyNumber);
    hframe->AddFrame(fAmountEntry, new TGLayoutHints(kLHintsLeft,5,5,3,4));

    // date entry field
    TDatime time;
    fDateEntry = new TGNumberEntry(hframe, time.GetDate(), 10, -1, TGNumberFormat::kNESDayMYear, TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELLimitMinMax, 20090101., 20200101);
    hframe->AddFrame(fDateEntry, new TGLayoutHints(kLHintsLeft,5,5,3,4));

    // withdrawn or not
    fWithdrawn = new TGComboBox(hframe,30);
    fWithdrawn->AddEntry("Not Withdrawn", 1);
    fWithdrawn->AddEntry("Withdrawn", 2);
    fWithdrawn->Select(1);
    fWithdrawn->Resize(150, 20);
    hframe->AddFrame(fWithdrawn, new TGLayoutHints(kLHintsLeft,5,10,5,5));

    // category selector
    fCategoryBox = new TGComboBox(hframe,100);
    for (unsigned i = 0; i < NCATEGORIES; i++) {
        fCategoryBox->AddEntry(CATEGORIES[i], i+1);
    }
    fCategoryBox->Resize(150, 20);
    fCategoryBox->Select(1);
    hframe->AddFrame(fCategoryBox, new TGLayoutHints(kLHintsLeft,5,10,5,5));

    // description field
    fDescription = new TGTextEntry(hframe, "");
    fDescription -> SetToolTipText("Description");
    fDescription -> Resize(200, fDescription->GetDefaultHeight());
    hframe -> AddFrame(fDescription, new TGLayoutHints(kLHintsLeft, 5,5,5,5));

    // add-button
    TGTextButton * add_button = new TGTextButton(hframe,"&Add");
    add_button -> Connect("Clicked()", "TExpenser", this, "add()");
    hframe -> AddFrame(add_button, new TGLayoutHints(kLHintsLeft,5,5,3,4));

    // add frame to main frame
    AddFrame(hframe, new TGLayoutHints(kLHintsCenterX,2,2,2,2));

    // Set a name to the main frame
    SetWindowName("Expenser");

    // Map all subwindows of main frame
    MapSubwindows();

    // Initialize the layout algorithm
    Resize(GetDefaultSize());

    // Map main frame
    MapWindow();

    // open the xml file
    fXMLParser = new TXMLParser("data/expenses.xml");
}

TExpenser::~TExpenser() {
    // Clean up used widgets: frames, buttons, layouthints
    Cleanup();
    gApplication->Terminate(0);
}

void TExpenser::add() {

    fXMLParser -> selectMainNode();
    XMLNodePointer_t expense = fXMLParser -> NewChild(fXMLParser->getCurrentNode(), 0, "expense");

    fXMLParser -> NewChild(expense, 0, "amount", toStr( fAmountEntry -> GetNumber(), 2) );

    Int_t year, month, day;
    fDateEntry -> GetDate(year, month, day);
    XMLNodePointer_t date = fXMLParser -> NewChild(expense, 0, "date");
    fXMLParser -> NewChild(date, 0, "day", toStr(day) );
    fXMLParser -> NewChild(date, 0, "month", toStr(month) );
    fXMLParser -> NewChild(date, 0, "year", toStr(year) );

    TString withdrawn;
    if (fWithdrawn -> GetSelected () == 1) withdrawn = "No";
    if (fWithdrawn -> GetSelected () == 2) withdrawn = "Yes";
    fXMLParser->NewChild(expense, 0, "withdrawn", withdrawn);

    unsigned selected_entry = fCategoryBox -> GetSelected ();
    fXMLParser->NewChild(expense, 0, "category", CATEGORIES[selected_entry-1]);

    fXMLParser->NewChild(expense, 0, "description", fDescription -> GetText());

    fXMLParser -> SaveDoc(fXMLParser->getDocument(), "data/expenses.xml");
}
