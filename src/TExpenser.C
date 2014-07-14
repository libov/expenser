#include<TGFrame.h>
#include<TGNumberEntry.h>
#include<TGTextEntry.h>
#include <TGButton.h>
#include <TApplication.h>
#include <TGComboBox.h>
#include <TDatime.h>
#include <TGTable.h>
#include <TRootEmbeddedCanvas.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TH1F.h>
#include <TGLabel.h>
#include <TColor.h>
#include <TGFont.h>

#include<TExpenser.h>
#include<TGExpenserTableInterface.h>

#include<iostream>
#include<fstream>
#include <cstdlib>
#include <map>
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
const unsigned NCATEGORIES = 8;
TString CATEGORIES[]  = {"Food", "Restaurant", "Flat", "Cash", "Music equipment", "Books", "Clothes", "Other"};

TExpenser::TExpenser(const TGWindow *p, UInt_t w, UInt_t h):
TGMainFrame(p,w,h)
{
    // open the xml file
    fXMLParser = new TXMLParser("data/expenses.xml");

    // create tab manager and add to main window
    fTab = new TGTab(this, 300, 300);
    AddFrame(fTab, new TGLayoutHints(kLHintsCenterX,2,2,2,2));

    drawExpensesTab();
    drawStatisticsTab();
    drawBalanceTab();

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

void TExpenser::drawExpensesTable() {

    // create table interface
    const unsigned ncolumns =  5;
    fTableInterface = new TGExpenserTableInterface(ncolumns);
    TString columns[ncolumns]={"amount", "category", "description", "withdrawn","date"};
    fTableInterface -> setColumnNames(columns);

    fXMLParser->selectMainNode();
    fXMLParser->selectNode("expense");
    fTableEntries = 0;
    while (fXMLParser->getCurrentNode() != 0) {
        XMLNodePointer_t current_node = fXMLParser->getCurrentNode();

        fXMLParser -> selectNode("date");
        TString year = fXMLParser -> getNodeContent("year");
        TString month = fXMLParser -> getNodeContent("month");
        TString day = fXMLParser -> getNodeContent("day");
        fXMLParser -> setCurrentNode(current_node);

        fTableInterface -> addCell (fTableEntries, fXMLParser -> getNodeContent("amount"));
        fTableInterface -> addCell (fTableEntries, fXMLParser -> getNodeContent("category"));
        fTableInterface -> addCell (fTableEntries, fXMLParser -> getNodeContent("description"));
        fTableInterface -> addCell (fTableEntries, fXMLParser -> getNodeContent("withdrawn"));
        fTableInterface -> addCell (fTableEntries, day+"/"+month+"/"+year);

        fXMLParser->selectNextNode("expense");
        fTableEntries++;
    }

    // Create the table
    fTable = new TGTable(fExpensesTab, 999, fTableInterface, 15, fTableInterface->GetNColumns());

    fExpensesTab -> AddFrame(fTable, new TGLayoutHints(kLHintsCenterY,2,2,2,2));
}

void TExpenser::drawExpensesTab() {

    fExpensesTab = fTab->AddTab("Expenses");

    // create a frame holding all widgets
    TGHorizontalFrame *hframe = new TGHorizontalFrame(fExpensesTab, 500, 40);
    fExpensesTab -> AddFrame(hframe, new TGLayoutHints(kLHintsCenterX,2,2,2,2));

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

    drawExpensesTable();
}

void TExpenser::drawStatisticsTab() {

    fStatisticsTab = fTab->AddTab("Statistics");

    TRootEmbeddedCanvas * fEcanvas = new TRootEmbeddedCanvas("Ecanvas",fStatisticsTab,200,200);
    fStatisticsTab -> AddFrame(fEcanvas, new TGLayoutHints(kLHintsExpandX| kLHintsExpandY, 10,10,10,1));
    // TCanvas *fCanvas = fEcanvas->GetCanvas();
    // fCanvas->cd();
    TH1F * fCategoriesHistogram = new TH1F("fCategoriesHistogram", "Categories", NCATEGORIES, 0, NCATEGORIES);
    fCategoriesHistogram -> SetStats(0);

    map<TString, Float_t> monthly_sum;
    for (unsigned i=0; i<NCATEGORIES; i++) {
        monthly_sum[CATEGORIES[i]]=0;
    }
    fXMLParser->selectMainNode();
    fXMLParser->selectNode("expense");
    while (fXMLParser->getCurrentNode() != 0) {
        XMLNodePointer_t current_node = fXMLParser->getCurrentNode();
        fXMLParser -> selectNode("date");
        unsigned year = fXMLParser -> getNodeContent("year").Atoi();
        unsigned month = fXMLParser -> getNodeContent("month").Atoi();
        unsigned day = fXMLParser -> getNodeContent("day").Atoi();
        fXMLParser -> setCurrentNode(current_node);
        Float_t amount = fXMLParser -> getNodeContent("amount").Atof();
        fXMLParser->selectNextNode("expense");
        if (year != 2014) continue;
        if (month != 7) continue;

        monthly_sum[fXMLParser -> getNodeContent("category")] += amount;
    }

    for (unsigned i=0; i<NCATEGORIES; i++) {
        fCategoriesHistogram -> Fill(CATEGORIES[i], monthly_sum[CATEGORIES[i]]);
    }
    fCategoriesHistogram -> Draw();
}

void TExpenser::drawBalanceTab() {

    fBalanceTab = fTab->AddTab("Balance");

    TGLabel * status_label = new TGLabel(fBalanceTab, "0 eur");
    fBalanceTab->AddFrame(status_label, new TGLayoutHints(kLHintsNormal, 5, 5, 3, 4));
    TColor *color = gROOT->GetColor(kBlue);
    status_label->SetTextColor(color);
    TGFont *font = gClient->GetFont("-*-times-bold-r-*-*-46-*-*-*-*-*-*-*");
    status_label -> SetTextFont(font);
}

void TExpenser::add() {

    // first get total number of entries in order to assign a new id
    fXMLParser -> selectMainNode();
    unsigned n_expenses = fXMLParser -> getNodeContent("n_expenses").Atoi();
    XMLNodePointer_t n_expenses_node = fXMLParser -> getNode("n_expenses");
    fXMLParser->SetNodeContent(n_expenses_node, toStr(n_expenses+1));

    fXMLParser -> selectMainNode();
    XMLNodePointer_t expense = fXMLParser -> NewChild(fXMLParser->getCurrentNode(), 0, "expense");

    fXMLParser -> NewChild(expense, 0, "id", toStr( n_expenses+1 ) );

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

    // save the file!
    fXMLParser -> SaveDoc(fXMLParser->getDocument(), "data/expenses.xml");

    // add new entry to the table
    fTableInterface -> addCell(fTableEntries, toStr( fAmountEntry -> GetNumber(), 2));
    fTableInterface -> addCell(fTableEntries, CATEGORIES[selected_entry-1] );
    fTableInterface -> addCell(fTableEntries, fDescription -> GetText() );
    fTableInterface -> addCell(fTableEntries, withdrawn );
    fTableInterface -> addCell(fTableEntries, toStr(day)+"/"+ toStr(month)+"/"+toStr(year) );
    fTableEntries++;
    fTable -> Update();
}

void TExpenser::commit() {
    system ("git add data/expenses.xml");
    system ("git commit -m \"XML database file update\"");
    system ("git push");
}
