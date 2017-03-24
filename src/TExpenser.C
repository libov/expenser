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

// ========== constants ========== //
const unsigned NCATEGORIES = 19;
TString CATEGORIES[]  = {"Food", "Restaurant", "Flat", "Cash", "Car", "Music equipment", "Books", "Clothes", "Entertainment", "Presents", "Travel", "Transport HH", "Business trip", "Degussa", "Piano lessons", "Category unknown", "No record", "Lend", "Other"};
const TString MONTHS[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
const unsigned FIRST_YEAR = 2009;
const unsigned LAST_YEAR = 2017;
unsigned NROWSTABLE = 30;
unsigned WIDTH = 800;
unsigned HEIGHT = 800;

struct expense {
    TString amount;
    TString category;
    TString description;
    TString withdrawn;
    TString date;
    TString dateForCell;
    TString id;
};

bool compare(expense a, expense b) { return (a.date.Atoi() > b.date.Atoi()); }

TExpenser::TExpenser(const TGWindow *p, UInt_t w, UInt_t h):
TGMainFrame(p,w,h)
{
    // open the xml file
    fXMLParser = new TXMLParser("data/expenses.xml");
    fBalanceXMLParser = new TXMLParser("data/balance.xml");
    fIncomeXMLParser = new TXMLParser("data/incomes.xml");
    fFilterActive = false;

    create_ui();
}

TExpenser::~TExpenser() {
    // Clean up used widgets: frames, buttons, layouthints
    Cleanup();
    gApplication->Terminate(0);
}

void TExpenser::create_ui() {
    // create tab manager and add to main window
    fTab = new TGTab(this, 300, 300);
    AddFrame(fTab, new TGLayoutHints(kLHintsCenterX,2,2,2,2));

    drawExpensesTab();
    drawStatisticsTab();
    drawStatisticsYearTab();
    drawCategoryTab();
    drawBalanceTab();
    drawIncomeTab();

    // Set a name to the main frame
    SetWindowName("Expenser");

    // Map all subwindows of main frame
    MapSubwindows();

    // Initialize the layout algorithm
    Resize(GetDefaultSize());

    // Map main frame
    MapWindow();
}

void TExpenser::createExpensesTableInterface() {

    // create table interface
    const unsigned ncolumns =  6;
    fTableInterface = new TGExpenserTableInterface(ncolumns);
    TString columns[ncolumns]={"amount", "category", "description", "withdrawn","date", "id"};
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
        ex.id  = fXMLParser -> getNodeContent("id");

        fXMLParser->selectNextNode("expense");

        if (fFilterActive) {
            if ( fFilterCategory != "any" && ex.category != fFilterCategory) continue;
            if ( fFilterMonth != "any" && MONTHS[month.Atoi()-1] != fFilterMonth) continue;
            if ( fFilterYear != "any" && year != fFilterYear) continue;
            if ( fFilterWithdrawn != "any" && fFilterWithdrawn != ex.withdrawn ) continue;
        }

        expenses.push_back(ex);
    }

    sort(expenses.begin(), expenses.end(), compare);

    for (unsigned i=0; i<expenses.size(); i++) {
        fTableInterface -> addCell (i, expenses[i].amount);
        fTableInterface -> addCell (i, expenses[i].category);
        fTableInterface -> addCell (i, expenses[i].description);
        fTableInterface -> addCell (i, expenses[i].withdrawn);
        fTableInterface -> addCell (i, expenses[i].dateForCell);
        fTableInterface -> addCell (i, expenses[i].id);
    }
    fTableEntries = expenses.size();

}

void TExpenser::drawExpensesTab() {

    fExpensesTab = fTab->AddTab("Expenses");
    fExpensesTab -> SetLayoutManager(new TGHorizontalLayout(fExpensesTab));

    // Create the table
    createExpensesTableInterface();
    fTable = new TGTable(fExpensesTab, 999, fTableInterface, NROWSTABLE, fTableInterface->GetNColumns());
    fExpensesTab -> AddFrame(fTable, new TGLayoutHints(kLHintsCenterY,2,2,2,2));

    // create a frame holding all widgets on the right of the table
    TGVerticalFrame *hframe = new TGVerticalFrame(fExpensesTab, 500, 40);
    fExpensesTab -> AddFrame(hframe, new TGLayoutHints(kLHintsCenterX,2,2,2,2));

    // --------- New expense group --------- //
    TGGroupFrame *frame_new_expense = new TGGroupFrame(hframe, "New expense");
    frame_new_expense->SetTitlePos(TGGroupFrame::kLeft);
    frame_new_expense->SetTextFont("-adobe-helvetica-bold-r-*-*-12-*-*-*-*-*-iso8859-1"); // when font too large, collides with group content....
    // frame_new_expense->SetTextColor(kBlue); // doesn't work unfortunately
    hframe->AddFrame(frame_new_expense, new TGLayoutHints(kLHintsExpandX));

    // expense amount entry field
    fAmountEntry = new TGNumberEntryField(frame_new_expense, 0, 0, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEAAnyNumber);
    frame_new_expense->AddFrame(fAmountEntry, new TGLayoutHints(kLHintsLeft,5,5,3,4));

    // date entry field
    TDatime time;
    fDateEntry = new TGNumberEntry(frame_new_expense, time.GetDate(), 10, -1, TGNumberFormat::kNESDayMYear, TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELLimitMinMax, 20090101., 20200101);
    frame_new_expense->AddFrame(fDateEntry, new TGLayoutHints(kLHintsLeft,5,5,3,4));

    // withdrawn or not
    fWithdrawn = new TGComboBox(frame_new_expense,30);
    fWithdrawn->AddEntry("Not Withdrawn", 1);
    fWithdrawn->AddEntry("Withdrawn", 2);
    fWithdrawn->Select(1);
    fWithdrawn->Resize(150, 20);
    frame_new_expense->AddFrame(fWithdrawn, new TGLayoutHints(kLHintsLeft,5,10,5,5));

    // category selector
    fCategoryBox = new TGComboBox(frame_new_expense,100);
    for (unsigned i = 0; i < NCATEGORIES; i++) {
        fCategoryBox->AddEntry(CATEGORIES[i], i+1);
    }
    fCategoryBox->Resize(150, 20);
    fCategoryBox->Select(1);
    frame_new_expense->AddFrame(fCategoryBox, new TGLayoutHints(kLHintsLeft,5,10,5,5));

    // description field
    fDescription = new TGTextEntry(frame_new_expense, "");
    fDescription -> SetToolTipText("Description");
    fDescription -> Resize(200, fDescription->GetDefaultHeight());
    frame_new_expense -> AddFrame(fDescription, new TGLayoutHints(kLHintsLeft, 5,5,5,5));

    // add-button
    TGTextButton * add_button = new TGTextButton(frame_new_expense,"&Add Expense");
    add_button -> Connect("Clicked()", "TExpenser", this, "add()");
    add_button  ->  SetFont("-*-times-bold-r-*-*-28-*-*-*-*-*-*-*");
    frame_new_expense -> AddFrame(add_button, new TGLayoutHints(kLHintsLeft,5,5,3,4));

    // --------- Withdrawing group --------- //
    TGGroupFrame *frame_withdrawn = new TGGroupFrame(hframe, "Withdrawing");
    frame_withdrawn->SetTitlePos(TGGroupFrame::kLeft);
    frame_withdrawn->SetTextFont("-adobe-helvetica-bold-r-*-*-12-*-*-*-*-*-iso8859-1"); // when font too large, collides with group content....
    hframe->AddFrame(frame_withdrawn, new TGLayoutHints(kLHintsExpandX));

    // withdrawn id entry field and withdrawn button
    TGHorizontalFrame *hframe1 = new TGHorizontalFrame(frame_withdrawn, 500, 40);
    frame_withdrawn -> AddFrame(hframe1,new TGLayoutHints(kLHintsLeft,5,5,3,4));
    fWithdrawnIdEntry = new TGNumberEntryField(hframe1, 0, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEAAnyNumber);
    hframe1->AddFrame(fWithdrawnIdEntry, new TGLayoutHints(kLHintsLeft|kLHintsExpandY,5,5,3,4));
    TGTextButton * withdrawn_button = new TGTextButton(hframe1,"&Withdrawn");
    withdrawn_button ->  SetFont("-*-times-bold-r-*-*-28-*-*-*-*-*-*-*");
    hframe1->AddFrame(withdrawn_button, new TGLayoutHints(kLHintsLeft,5,5,3,4));
    withdrawn_button -> Connect("Clicked()", "TExpenser", this, "set_withdrawn()");

    // commit-button
    TGTextButton * commit_button = new TGTextButton(hframe,"&Commit");
    commit_button -> Connect("Clicked()", "TExpenser", this, "commit()");
    commit_button -> SetFont("-*-times-bold-r-*-*-28-*-*-*-*-*-*-*");
    hframe -> AddFrame(commit_button, new TGLayoutHints(kLHintsLeft,5,5,3,4));

    // --------- Filter group --------- //
    TGGroupFrame *frame_filter = new TGGroupFrame(hframe, "Filter");
    frame_filter->SetTitlePos(TGGroupFrame::kLeft);
    frame_filter->SetTextFont("-adobe-helvetica-bold-r-*-*-12-*-*-*-*-*-iso8859-1"); // when font too large, collides with group content....
    hframe->AddFrame(frame_filter, new TGLayoutHints(kLHintsExpandX));

    // filter category selector
    fFilterCategoryBox = new TGComboBox(frame_filter, 100);
    fFilterCategoryBox->AddEntry("Select Category", 1);
    for (unsigned i = 1; i < NCATEGORIES+1; i++) {
        fFilterCategoryBox->AddEntry(CATEGORIES[i], i+1);
    }
    fFilterCategoryBox->Resize(150, 20);
    fFilterCategoryBox->Select(1);
    frame_filter->AddFrame(fFilterCategoryBox, new TGLayoutHints(kLHintsLeft,5,10,5,5));

    // month selector
    fFilterMonthBox = new TGComboBox(frame_filter);
    fFilterMonthBox -> AddEntry("Select Month", 1);
    for (unsigned i = 0; i < 12; i++) {
        fFilterMonthBox->AddEntry(MONTHS[i], i+2);
    }
    fFilterMonthBox->Select(1);
    fFilterMonthBox->Resize(150, 20);
    frame_filter->AddFrame(fFilterMonthBox, new TGLayoutHints(kLHintsLeft,5,10,5,5));

    // year selector
    fFilterYearBox = new TGComboBox(frame_filter);
    fFilterYearBox -> AddEntry("Select Year", 1);
    for (unsigned i = FIRST_YEAR; i <= LAST_YEAR; i++) {
        fFilterYearBox->AddEntry(toStr(i), i+2-FIRST_YEAR);
    }
    fFilterYearBox->Resize(100, 20);
    fFilterYearBox->Select(1);
    frame_filter->AddFrame(fFilterYearBox, new TGLayoutHints(kLHintsLeft,5,10,5,5));

    // withdrawn/not withdrawn selector
    fFilterWithdrawnBox = new TGComboBox(frame_filter);
    fFilterWithdrawnBox -> AddEntry("Select Withdrawn", 1);
    fFilterWithdrawnBox->AddEntry("Withdrawn", 2);
    fFilterWithdrawnBox->AddEntry("Not Withdrawn", 3);
    fFilterWithdrawnBox->Resize(100, 20);
    fFilterWithdrawnBox->Select(1);
    frame_filter->AddFrame(fFilterWithdrawnBox, new TGLayoutHints(kLHintsLeft,5,10,5,5));

    TGHorizontalFrame *hframe2 = new TGHorizontalFrame(frame_filter, 500, 40);
    frame_filter -> AddFrame(hframe2,new TGLayoutHints(kLHintsLeft,5,5,3,4));

    TGTextButton * filter_button = new TGTextButton(hframe2,"&Filter");
    filter_button -> Connect("Clicked()", "TExpenser", this, "filter_expense_table()");
    filter_button -> SetFont("-*-times-bold-r-*-*-28-*-*-*-*-*-*-*");
    hframe2 -> AddFrame(filter_button, new TGLayoutHints(kLHintsLeft,5,5,3,4));

    TGTextButton * undo_filter_button = new TGTextButton(hframe2,"&Undo Filters");
    undo_filter_button -> Connect("Clicked()", "TExpenser", this, "undo_filters_expense_table()");
    undo_filter_button -> SetFont("-*-times-bold-r-*-*-28-*-*-*-*-*-*-*");
    hframe2 -> AddFrame(undo_filter_button, new TGLayoutHints(kLHintsLeft,15,5,3,4));
}

void TExpenser::drawStatisticsTab() {

    fStatisticsTab = fTab->AddTab("Statistics (Month)");
    fStatisticsTab -> SetLayoutManager(new TGHorizontalLayout(fStatisticsTab));

    TRootEmbeddedCanvas * fEcanvas = new TRootEmbeddedCanvas("Ecanvas",fStatisticsTab, WIDTH, HEIGHT);
    fStatisticsTab -> AddFrame(fEcanvas, new TGLayoutHints(kLHintsCenterX, 10,10,10,1));
    fCanvas = fEcanvas->GetCanvas();
    fCategoriesHistogram = new TH1F("fCategoriesHistogram", "Expenses for each category for a given month", NCATEGORIES, 0, NCATEGORIES);
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
    TGTextButton * update_button = new TGTextButton(vframe,"&Apply");
    update_button -> SetFont("-*-times-bold-r-*-*-28-*-*-*-*-*-*-*");
    update_button -> Connect("Clicked()", "TExpenser", this, "calculate_monthly()");
    vframe -> AddFrame(update_button, new TGLayoutHints(kLHintsLeft,5,5,3,4));

    calculate_monthly();
}

void TExpenser::drawStatisticsYearTab() {

    fStatisticsYearTab = fTab->AddTab("Statistics (Year)");
    fStatisticsYearTab -> SetLayoutManager(new TGHorizontalLayout(fStatisticsYearTab));

    TRootEmbeddedCanvas * StatisticsYearCanvas = new TRootEmbeddedCanvas("StatisticsYearCanvas",fStatisticsYearTab, WIDTH, HEIGHT);
    fStatisticsYearTab -> AddFrame(StatisticsYearCanvas, new TGLayoutHints(kLHintsCenterX, 10,10,10,1));
    fCanvasYear = StatisticsYearCanvas->GetCanvas();
    fMonthsHistogram = new TH1F("fMonthsHistogram", "Expenses for each month for a given category", 12, 0, 12);
    fMonthsHistogram -> SetStats(0);

    TGVerticalFrame *vframe = new TGVerticalFrame(fStatisticsYearTab, 60, 40);
    fStatisticsYearTab -> AddFrame(vframe, new TGLayoutHints(kLHintsCenterX,2,2,2,2));

    //  selector
    fStatisticsCategory = new TGComboBox(vframe);
    for (unsigned i = 0; i < NCATEGORIES; i++) {
        fStatisticsCategory->AddEntry(CATEGORIES[i], i+1);
    }
    fStatisticsCategory->Resize(100, 20);
    TDatime time;
    fStatisticsCategory->Select(1);
    vframe->AddFrame(fStatisticsCategory, new TGLayoutHints(kLHintsLeft,5,10,5,5));

    // year selector
    fStatisticsYear2 = new TGComboBox(vframe);
    for (unsigned i = FIRST_YEAR; i <= LAST_YEAR; i++) {
        fStatisticsYear2->AddEntry(toStr(i), i+1-FIRST_YEAR);
    }
    fStatisticsYear2->Resize(100, 20);
    fStatisticsYear2->Select(time.GetYear()-FIRST_YEAR+1);
    vframe->AddFrame(fStatisticsYear2, new TGLayoutHints(kLHintsLeft,5,10,5,5));

    // update-button
    TGTextButton * update_button = new TGTextButton(vframe,"&Update");
    update_button -> Connect("Clicked()", "TExpenser", this, "calculate_yearly()");
    vframe -> AddFrame(update_button, new TGLayoutHints(kLHintsLeft,5,5,3,4));

    calculate_yearly();
}

void TExpenser::drawCategoryTab(){
    fCategoryTab = fTab->AddTab("Category");
    fCategoryTab -> SetLayoutManager(new TGHorizontalLayout(fCategoryTab));
}

void TExpenser::drawBalanceTab() {

    fBalanceTab = fTab->AddTab("Balance");

    TColor *color = gROOT->GetColor(kBlue);
    TGFont *font = gClient->GetFont("-*-times-bold-r-*-*-46-*-*-*-*-*-*-*");

    fLastStatusLabel = new TGLabel(fBalanceTab, "");
    fBalanceTab->AddFrame(fLastStatusLabel, new TGLayoutHints(kLHintsNormal, 5, 5, 3, 4));
    fLastStatusLabel->SetTextColor(color);
    fLastStatusLabel -> SetTextFont(font);

    fCurrentStatusLabel = new TGLabel(fBalanceTab, "");
    fBalanceTab->AddFrame(fCurrentStatusLabel, new TGLayoutHints(kLHintsNormal, 5, 5, 3, 4));
    fCurrentStatusLabel->SetTextColor(color);
    fCurrentStatusLabel -> SetTextFont(font);

    // update-button
    TGTextButton * update_button = new TGTextButton(fBalanceTab,"&Update");
    update_button -> Connect("Clicked()", "TExpenser", this, "calculate_balance()");
    fBalanceTab -> AddFrame(update_button, new TGLayoutHints(kLHintsNormal,5,5,3,4));

    calculate_balance();
}

void TExpenser::drawIncomeTab() {

    // create the income tab
    fIncomeTab = fTab->AddTab("Income");
    fIncomeTab -> SetLayoutManager(new TGHorizontalLayout(fIncomeTab));

    // create table interface
    const unsigned ncolumns =  3;
    TGExpenserTableInterface * income_table = new TGExpenserTableInterface(ncolumns);
    TString columns[ncolumns]={"amount", "description", "date"};
    income_table -> setColumnNames(columns);

    fIncomeXMLParser->selectMainNode();
    fIncomeXMLParser->selectNode("entry");
    vector<expense> incomes;
    while (fIncomeXMLParser->getCurrentNode() != 0) {
        XMLNodePointer_t current_node = fIncomeXMLParser->getCurrentNode();

        fIncomeXMLParser -> selectNode("date");
        TString year = fIncomeXMLParser -> getNodeContent("year");
        TString month = fIncomeXMLParser -> getNodeContent("month");
        fIncomeXMLParser -> setCurrentNode(current_node);

        if (month.Atoi()<10) month="0"+month;

        expense in;
        in.amount = fIncomeXMLParser -> getNodeContent("amount");
        in.description = fIncomeXMLParser -> getNodeContent("description");
        in.date = year+month;
        in.dateForCell = month+"/"+year;
        incomes.push_back(in);

        fIncomeXMLParser->selectNextNode("entry");
    }

    sort(incomes.begin(), incomes.end(), compare);

    for (unsigned i=0; i<incomes.size(); i++) {
        income_table -> addCell (i, incomes[i].amount);
        income_table -> addCell (i, incomes[i].description);
        income_table -> addCell (i, incomes[i].dateForCell);
    }
    unsigned entries = incomes.size();

    // Create the table
    TGTable * fInTable = new TGTable(fIncomeTab, 999, income_table, NROWSTABLE, income_table->GetNColumns());
    fIncomeTab -> AddFrame(fInTable, new TGLayoutHints(kLHintsCenterY,2,2,2,2));

    /*
    TGVerticalFrame *vframe = new TGVerticalFrame(fIncomeTab, 60, 40);
    fIncomeTab -> AddFrame(vframe, new TGLayoutHints(kLHintsCenterX,2,2,2,2));

    // year selector
    TDatime time;
    TGComboBox * income_tab_year = new TGComboBox(vframe);
    for (unsigned i = FIRST_YEAR; i <= LAST_YEAR; i++) {
        income_tab_year->AddEntry(toStr(i), i+1-FIRST_YEAR);
    }
    income_tab_year->Resize(100, 20);
    income_tab_year->Select(time.GetYear()-FIRST_YEAR+1);
    vframe->AddFrame(income_tab_year, new TGLayoutHints(kLHintsLeft,5,10,5,5));

    TGTextButton * apply_button = new TGTextButton(vframe,"&Apply");
    apply_button -> Connect("Clicked()", "TExpenser", this, "calculate_monthly()");
    vframe -> AddFrame(apply_button, new TGLayoutHints(kLHintsLeft,5,5,3,4));
    */
}

void TExpenser::add() {

    // first get total number of entries in order to assign a new id
    fXMLParser -> selectMainNode();
    unsigned last_expense_id = fXMLParser -> getNodeContent("last_expense_id").Atoi();
    XMLNodePointer_t last_expense_id_node = fXMLParser -> getNode("last_expense_id");
    fXMLParser->SetNodeContent(last_expense_id_node, toStr(last_expense_id+1));

    fXMLParser -> selectMainNode();
    XMLNodePointer_t expense = fXMLParser -> NewChild(fXMLParser->getCurrentNode(), 0, "expense");

    fXMLParser -> NewChild(expense, 0, "id", toStr( last_expense_id+1 ) );

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

    // update the table
    delete fTableInterface;
    createExpensesTableInterface();
    fTable -> Update();
}

void TExpenser::commit() {
    system ("git add data/expenses.xml");
    system ("git add data/incomes.xml");
    system ("git add data/balance.xml");
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
        TString category = fXMLParser -> getNodeContent("category");
        fXMLParser->selectNextNode("expense");

        if (year != selected_year) continue;
        if (month != selected_month) continue;

        monthly_sum[category] += amount;
    }

    for (unsigned i=0; i<NCATEGORIES; i++) {
        fCategoriesHistogram -> Fill(CATEGORIES[i], monthly_sum[CATEGORIES[i]]);
    }
    fCanvas -> cd();
    fCategoriesHistogram -> Draw();
    fCanvas -> Update();
}

void TExpenser::calculate_yearly(){

    unsigned selected_category = fStatisticsCategory -> GetSelected ();
    unsigned selected_year =  fStatisticsYear2 -> GetSelected () + FIRST_YEAR - 1;

    map<TString, Float_t> monthly_sum;
    for (unsigned i=0; i<12; i++) {
        monthly_sum[i]=0;
        fMonthsHistogram -> SetBinContent(i+1, 0);
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
        TString category = fXMLParser -> getNodeContent("category");
        fXMLParser->selectNextNode("expense");

        if (year != selected_year) continue;
        if (category != CATEGORIES[selected_category-1]) continue;

        monthly_sum[month-1] += amount;
    }

    for (unsigned i=0; i<12; i++) {
        fMonthsHistogram -> Fill(MONTHS[i], monthly_sum[i]);
    }
    fCanvasYear -> cd();
    fMonthsHistogram -> Draw();
    fCanvasYear -> Update();
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

void TExpenser::calculate_balance() {

    fBalanceXMLParser -> selectMainNode();
    fBalanceXMLParser -> selectNode("entry");
    TString balance = fBalanceXMLParser -> getNodeContent("amount");
    fBalanceXMLParser -> selectNode("date");
    TString balance_year = fBalanceXMLParser -> getNodeContent("year");
    TString balance_month = fBalanceXMLParser -> getNodeContent("month");

    fLastStatusLabel -> SetText(balance_month+"/"+balance_year+": " + balance + " eur");

    // now calculate the current balance (last - expenses since the last)
    TDatime time;

    fXMLParser->selectMainNode();
    fXMLParser->selectNode("expense");
    Double_t expenses_since_last_status = 0;
    while (fXMLParser->getCurrentNode() != 0) {
        XMLNodePointer_t current_node = fXMLParser->getCurrentNode();

        fXMLParser -> selectNode("date");
        TString year = fXMLParser -> getNodeContent("year");
        TString month = fXMLParser -> getNodeContent("month");
        fXMLParser -> setCurrentNode(current_node);

        bool year_more_recent = (year.Atoi() > balance_year.Atoi());
        bool year_same = (year.Atoi() == balance_year.Atoi());
        bool month_more_recent = (month.Atoi()>=balance_month.Atoi());
        bool expense_more_recent_than_balance = (year_more_recent || (year_same && month_more_recent));
        if (  expense_more_recent_than_balance && fXMLParser -> getNodeContent("withdrawn") == "Yes" ) {
            expenses_since_last_status += fXMLParser -> getNodeContent("amount").Atof();
        }

        fXMLParser->selectNextNode("expense");
    }

    // calculate total income since last balance
    fIncomeXMLParser->selectMainNode();
    fIncomeXMLParser->selectNode("entry");
    Double_t income_since_last_status = 0;
    while (fIncomeXMLParser->getCurrentNode() != 0) {
        XMLNodePointer_t current_node = fIncomeXMLParser->getCurrentNode();

        fIncomeXMLParser -> selectNode("date");
        TString year = fIncomeXMLParser -> getNodeContent("year");
        TString month = fIncomeXMLParser -> getNodeContent("month");
        fIncomeXMLParser -> setCurrentNode(current_node);

        if ( ( (month.Atoi()>=balance_month.Atoi()) && (year.Atoi()==balance_year.Atoi()) ) || (year.Atoi()>balance_year.Atoi()) ) {
            income_since_last_status += fIncomeXMLParser -> getNodeContent("amount").Atof();
        }

        fIncomeXMLParser->selectNextNode("entry");
    }

    Double_t new_balance = balance.Atof() - expenses_since_last_status + income_since_last_status;
    fCurrentStatusLabel -> SetText(toStr(time.GetDay())+"/"+toStr(time.GetMonth())+"/"+toStr(time.GetYear())+": " + toStr(new_balance,2) + " eur");
}

void TExpenser::filter_expense_table() {

    fFilterActive = true;

    // get selected category
    unsigned selected_entry = fFilterCategoryBox -> GetSelected ();
    if (selected_entry==1) {
        fFilterCategory = "any";
    } else {
        fFilterCategory = CATEGORIES[selected_entry-1];
    }

    // get selected month
    selected_entry = fFilterMonthBox -> GetSelected ();
    if (selected_entry==1) {
         fFilterMonth = "any";
    } else {
         fFilterMonth = MONTHS[selected_entry-2];
    }

    // get selected year
    selected_entry = fFilterYearBox -> GetSelected ();
    if (selected_entry==1) {
         fFilterYear = "any";
    } else {
        fFilterYear = toStr(FIRST_YEAR + selected_entry - 2);
    }

    // check whether withdrawn
    if (fFilterWithdrawnBox -> GetSelected () == 1) {
        fFilterWithdrawn = "any";
    } else if (fFilterWithdrawnBox -> GetSelected () == 2) {
        fFilterWithdrawn = "Yes";
    } else if (fFilterWithdrawnBox -> GetSelected () == 3) {
        fFilterWithdrawn = "No";
    }

    // update the table
    delete fTableInterface;
    createExpensesTableInterface();
    fTable -> Update();
}

void TExpenser::undo_filters_expense_table() {

    fFilterActive = false;

    fFilterCategoryBox -> Select(1);
    fFilterMonthBox -> Select(1);
    fFilterYearBox -> Select(1);

    delete fTableInterface;
    createExpensesTableInterface();
    fTable -> Update();
}
