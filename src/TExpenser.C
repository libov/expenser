#include<TGFrame.h>
#include<TGNumberEntry.h>
#include <TGButton.h>
#include <TApplication.h>

#include<TExpenser.h>

#include<iostream>
using namespace std;

TExpenser::TExpenser(const TGWindow *p,UInt_t w,UInt_t h):
TGMainFrame(p,w,h)
{
    // create a frame holding all widgets
    TGHorizontalFrame *hframe = new TGHorizontalFrame(this, 500, 40);

    // date entry field
    fDateEntry = new TGNumberEntry(hframe, 20140418, 10, -1, TGNumberFormat::kNESDayMYear,       //style
                                                        TGNumberFormat::kNEAAnyNumber,      //input value filter
                                                        TGNumberFormat::kNELLimitMinMax,    //specify limits
                                                        20090101.,20200101);                //limit values
    hframe->AddFrame(fDateEntry, new TGLayoutHints(kLHintsCenterX,5,5,3,4));

    // add button
    TGTextButton * add_button = new TGTextButton(hframe,"&Add");
    add_button -> Connect("Clicked()", "TExpenser", this, "add()");
    hframe -> AddFrame(add_button, new TGLayoutHints(kLHintsCenterX,5,5,3,4));
    
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
    Int_t year, month, day;
    fDateEntry -> GetDate(year, month, day);
    cout << "year: " << year << " month: " << month << " day: " << day << endl;

}