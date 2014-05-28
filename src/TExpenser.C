#include<TGFrame.h>
#include<TGNumberEntry.h>
#include <TGButton.h>
#include <TApplication.h>
#include <TGComboBox.h>

#include<TExpenser.h>

#include<iostream>
using namespace std;

TExpenser::TExpenser(const TGWindow *p,UInt_t w,UInt_t h):
TGMainFrame(p,w,h)
{
    // create a frame holding all widgets
    TGHorizontalFrame *hframe = new TGHorizontalFrame(this, 500, 40);

    // expense amount entry field
    fAmountEntry = new TGNumberEntryField(hframe, 0, 0, TGNumberFormat::kNESRealTwo,       //style
                                                        TGNumberFormat::kNEAAnyNumber);    //input value filter    
    hframe->AddFrame(fAmountEntry, new TGLayoutHints(kLHintsCenterX,5,5,3,4));
   
    // date entry field
    fDateEntry = new TGNumberEntry(hframe, 20140418, 10, -1, TGNumberFormat::kNESDayMYear,       //style
                                                             TGNumberFormat::kNEAAnyNumber,      //input value filter
                                                             TGNumberFormat::kNELLimitMinMax,    //specify limits
                                                             20090101.,20200101);                //limit values
    hframe->AddFrame(fDateEntry, new TGLayoutHints(kLHintsCenterX,5,5,3,4));

    // add-button
    TGTextButton * add_button = new TGTextButton(hframe,"&Add");
    add_button -> Connect("Clicked()", "TExpenser", this, "add()");
    hframe -> AddFrame(add_button, new TGLayoutHints(kLHintsCenterX,5,5,3,4));

    //
    char tmp[20];
// combo box layout hints
TGLayoutHints * fLcombo = new TGLayoutHints(kLHintsTop | kLHintsLeft,5,5,5,5);
// combo box widget
TGComboBox *fCombo = new TGComboBox(hframe,100);
for (unsigned i = 0; i < 10; i++) {
 sprintf(tmp, "Entry%i", i+1);
 fCombo->AddEntry(tmp, i+1);
}
fCombo->Resize(150, 20);
// Entry3 is selected as current
fCombo->Select(2);
hframe->AddFrame(fCombo, fLcombo);
    
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
}

TExpenser::~TExpenser() {
    // Clean up used widgets: frames, buttons, layouthints
    Cleanup();
    gApplication->Terminate(0);
}

void TExpenser::add() {
    Double_t amount = fAmountEntry -> GetNumber();
    cout << "amount: " << amount << endl;
    
    Int_t year, month, day;
    fDateEntry -> GetDate(year, month, day);
    cout << "year: " << year << " month: " << month << " day: " << day << endl;

}