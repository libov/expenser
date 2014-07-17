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
#include <algorithm>
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
const unsigned NCATEGORIES = 9;
TString CATEGORIES[]  = {"Food", "Restaurant", "Flat", "Cash", "Music equipment", "Books", "Clothes", "Presents", "Other"};
const TString MONTHS[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
const unsigned FIRST_YEAR = 2009;
const unsigned LAST_YEAR = 2015;

struct expense {
    TString amount;
    TString category;
    TString description;
    TString withdrawn;
    TString date;
    TString dateForCell;
};

bool compare(expense a, expense b) { return (a.date.Atoi() > b.date.Atoi()); }

TExpenser::TExpenser(const TGWindow *p, UInt_t w, UInt_t h):
TGMainFrame(p,w,h)
{
    // open the xml file
    fXMLParser = new TXMLParser("data/expenses.xml");
    fBalanceXMLParser = new TXMLParser("data/balance.xml");

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
    vector<expense> expenses;
    while (fXMLParser->getCurrentNode() != 0) {
        XMLNodePointer_t current_node = fXMLParser->getCurrentNode();

        fXMLParser -> selectNode("date");
        TString year = fXMLParser -> getNodeContent("year");
        TString month = fXMLParser -> getNodeContent("month");
        TString day = fXMLParser -> getNodeContent("day");
        fXMLParser -> setCurrentNode(current_node);

        if (month.Atoi()<10) month="0"+month;
        if (day.Atoi()<10) day="0"+day;

        expense ex;
        ex.amount = fXMLParser -> getNodeContent("amount");
        ex.category = fXMLParser -> getNodeContent("category");
        ex.description = fXMLParser -> getNodeContent("description");
        ex.withdrawn = fXMLParser -> getNodeContent("withdrawn");
        ex.date = year+month+day;
        ex.dateForCell  = day+"/"+month+"/"+year;
        expenses.push_back(ex);

        fXMLParser->selectNextNode("expense");
    }

    sort(expenses.begin(), expenses.end(), compare);

    for (unsigned i=0; i<expenses.size(); i++) {
        fTableInterface -> addCell (i, expenses[i].amount);
        fTableInterface -> addCell (i, expenses[i].category);
        fTableInterface -> addCell (i, expenses[i].description);
        fTableInterface -> addCell (i, expenses[i].withdrawn);
        fTableInterface -> addCell (i, expenses[i].dateForCell);
    }
    fTableEntries = expenses.size();

    // Create the table
    fTable = new TGTable(fExpensesTab, 999, fTableInterface, 15, fTableInterface->GetNColumns());

    fExpensesTab -> AddFrame(fTable, new TGLayoutHints(kLHintsCenterY,2,2,2,2));
}

void TExpenser::drawExpensesTab() {

    fExpensesTab = fTab->AddTab("Expenses");
    fExpensesTab -> SetLayoutManager(new TGHorizontalLayout(fExpensesTab));

    drawExpensesTable();

    // create a frame holding all widgets
    TGVerticalFrame *hframe = new TGVerticalFrame(fExpensesTab, 500, 40);
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

    // withdrawn id entry field and withdrawn button
    TGHorizontalFrame *hframe1 = new TGHorizontalFrame(hframe, 500, 40);
    hframe -> AddFrame(hframe1,new TGLayoutHints(kLHintsLeft,5,5,3,4));
    fWithdrawnIdEntry = new TGNumberEntryField(hframe1, 0, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEAAnyNumber);
    hframe1->AddFrame(fWithdrawnIdEntry, new TGLayoutHints(kLHintsLeft,5,5,3,4));
    TGTextButton * withdrawn_button = new TGTextButton(hframe1,"&Withdrawn");
    hframe1->AddFrame(withdrawn_button, new TGLayoutHints(kLHintsLeft,5,5,3,4));
    withdrawn_button -> Connect("Clicked()", "TExpenser", this, "set_withdrawn()");

    // commit-button
    TGTextButton * commit_button = new TGTextButton(hframe,"&Commit");
    commit_button -> Connect("Clicked()", "TExpenser", this, "commit()");
    hframe -> AddFrame(commit_button, new TGLayoutHints(kLHintsLeft,5,5,3,4));
}

void TExpenser::drawStatisticsTab() {

    fStatisticsTab = fTab->AddTab("Statistics");
    fStatisticsTab -> SetLayoutManager(new TGHorizontalLayout(fStatisticsTab));

    TRootEmbeddedCanvas * fEcanvas = new TRootEmbeddedCanvas("Ecanvas",fStatisticsTab,600,600);
    fStatisticsTab -> AddFrame(fEcanvas, new TGLayoutHints(kLHintsCenterX, 10,10,10,1));
    fCanvas = fEcanvas->GetCanvas();
    fCategoriesHistogram = new TH1F("fCategoriesHistogram", "Expenses for each category for in a given month", NCATEGORIES, 0, NCATEGORIES);
    fCategoriesHistogram -> SetStats(0);

    TGVerticalFrame *vframe = new TGVerticalFrame(fStatisticsTab, 60, 40);
    fStatisticsTab -> AddFrame(vframe, new TGLayoutHints(kLHintsCenterX,2,2,2,2));

    // month selector
    fStatisticsMonth = new TGComboBox(vframe);
    for (unsigned i = 0; i < 12; i++) {
        fStatisticsMonth->AddEntry(MONTHS[i], i+1);
    }
    fStatisticsMonth->Resize(100, 20);
    TDatime time;
    fStatisticsMonth->Select(time.GetMonth());
    vframe->AddFrame(fStatisticsMonth, new TGLayoutHints(kLHintsLeft,5,10,5,5));

    // year selector
    fStatisticsYear = new TGComboBox(vframe);
    for (unsigned i = FIRST_YEAR; i <= LAST_YEAR; i++) {
        fStatisticsYear->AddEntry(toStr(i), i+1-FIRST_YEAR);
    }
    fStatisticsYear->Resize(100, 20);
    fStatisticsYear->Select(time.GetYear()-FIRST_YEAR+1);
    vframe->AddFrame(fStatisticsYear, new TGLayoutHints(kLHintsLeft,5,10,5,5));

    // update-button
    TGTextButton * update_button = new TGTextButton(vframe,"&Update");
    update_button -> Connect("Clicked()", "TExpenser", this, "calculate_monthly()");
    vframe -> AddFrame(update_button, new TGLayoutHints(kLHintsLeft,5,5,3,4));

    calculate_monthly();
}

void TExpenser::drawBalanceTab() {

    fBalanceTab = fTab->AddTab("Balance");

    fBalanceXMLParser -> selectMainNode();
    fBalanceXMLParser -> selectNode("entry");
    TString balance = fBalanceXMLParser -> getNodeContent("amount");
    fBalanceXMLParser -> selectNode("date");
    TString balance_year = fBalanceXMLParser -> getNodeContent("year");
    TString balance_month = fBalanceXMLParser -> getNodeContent("month");
    TString balance_day = fBalanceXMLParser -> getNodeContent("day");
    TString balance_date = balance_year+balance_month+balance_day;

    TColor *color = gROOT->GetColor(kBlue);
    TGFont *font = gClient->GetFont("-*-times-bold-r-*-*-46-*-*-*-*-*-*-*");

    TGLabel * last_status_label = new TGLabel(fBalanceTab, balance_day+"/"+balance_month+"/"+balance_year+": " + balance + " eur");
    fBalanceTab->AddFrame(last_status_label, new TGLayoutHints(kLHintsNormal, 5, 5, 3, 4));
    last_status_label->SetTextColor(color);
    last_status_label -> SetTextFont(font);

    // now calculate the current balance (last - expenses since the last)
    TDatime time;

    fXMLParser->selectMainNode();
    fXMLParser->selectNode("expense");
    Float_t expenses_since_last_status = 0;
    while (fXMLParser->getCurrentNode() != 0) {
        XMLNodePointer_t current_node = fXMLParser->getCurrentNode();

        fXMLParser -> selectNode("date");
        TString year = fXMLParser -> getNodeContent("year");
        TString month = fXMLParser -> getNodeContent("month");
        TString day = fXMLParser -> getNodeContent("day");
        fXMLParser -> setCurrentNode(current_node);
        fXMLParser->selectNextNode("expense");

        TString date = year+month+day;
        //if (date.Atoi()<=balance_date.Atoi()) continue;
        if ( month.Atoi()!=balance_month.Atoi() || fXMLParser -> getNodeContent("withdrawn") != "Yes" ) continue;
        expenses_since_last_status += fXMLParser -> getNodeContent("amount").Atof();
    }

    Float_t new_balance = balance.Atof() - expenses_since_last_status;
    TGLabel * current_status_label = new TGLabel(fBalanceTab, toStr(time.GetDay())+"/"+toStr(time.GetMonth())+"/"+toStr(time.GetYear())+": " + toStr(new_balance,2) + " eur");
    fBalanceTab->AddFrame(current_status_label, new TGLayoutHints(kLHintsNormal, 5, 5, 3, 4));
    current_status_label->SetTextColor(color);
    current_status_label -> SetTextFont(font);
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

void TExpenser::calculate_monthly() {

    unsigned selected_month = fStatisticsMonth -> GetSelected ();
    unsigned selected_year =  fStatisticsYear -> GetSelected () + FIRST_YEAR - 1;

    map<TString, Float_t> monthly_sum;
    for (unsigned i=0; i<NCATEGORIES; i++) {
        monthly_sum[CATEGORIES[i]]=0;
        fCategoriesHistogram -> SetBinContent(i+1, 0);
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

        if (year != selected_year) continue;
        if (month != selected_month) continue;

        monthly_sum[fXMLParser -> getNodeContent("category")] += amount;
    }

    for (unsigned i=0; i<NCATEGORIES; i++) {
        fCategoriesHistogram -> Fill(CATEGORIES[i], monthly_sum[CATEGORIES[i]]);
    }
    fCanvas -> cd();
    fCategoriesHistogram -> Draw();
    fCanvas -> Update();
}

void TExpenser::set_withdrawn() {

    // id of the entry to be changed
    unsigned id = fWithdrawnIdEntry -> GetNumber();

    fXMLParser->selectMainNode();
    fXMLParser->selectNode("expense");
    while ( fXMLParser->getCurrentNode() != 0 ) {
        XMLNodePointer_t expense_node = fXMLParser->getCurrentNode();
        XMLNodePointer_t withdrawn_node = fXMLParser->getNode("withdrawn");
        if ( fXMLParser -> getNodeContent("id").Atoi() == id ) {
            fXMLParser->SetNodeContent(withdrawn_node, "Yes");
            break;
        }

        fXMLParser->selectNextNode("expense");
    }

    fXMLParser -> SaveDoc(fXMLParser->getDocument(), "data/expenses.xml");
}
