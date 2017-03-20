#include <TGFrame.h>
#include <TGNumberEntry.h>
#include <TGComboBox.h>
#include<TGTextEntry.h>
#include <TGTable.h>
#include <TGTab.h>
#include <TH1F.h>
#include <TCanvas.h>

#include <TXMLParser.h>

#include <map>
#include <vector>
using namespace std;

class TGExpenserTableInterface;

class TExpenser : public TGMainFrame {

    public:

        TExpenser(const TGWindow *p, UInt_t w, UInt_t h);
        virtual ~TExpenser();
        void create_ui();
        void add();
        void drawExpensesTable();
        void drawExpensesTab();
        void drawStatisticsTab();
        void drawStatisticsYearTab();
        void drawCategoryTab();
        void drawBalanceTab();
        void drawIncomeTab();
        void commit();
        void calculate_monthly();
        void calculate_yearly();
        void set_withdrawn();
        void calculate_balance();
        void filter_expense_table();
        ClassDef(TExpenser, 0)

    private:

        TGNumberEntryField          * fAmountEntry;
        TGNumberEntry               * fDateEntry;
        TGComboBox                  * fCategoryBox;
        TGTextEntry                 * fDescription;
        TGComboBox                  * fWithdrawn;
        TGExpenserTableInterface    * fTableInterface;
        TGTable                     * fTable;
        TXMLParser                  * fXMLParser;
        unsigned                      fTableEntries;
        TGTab                       * fTab;
        TGCompositeFrame            * fExpensesTab;
        TGCompositeFrame            * fStatisticsTab;
        TGCompositeFrame            * fStatisticsYearTab;
        TGCompositeFrame            * fCategoryTab;
        TGCompositeFrame            * fBalanceTab;
        TGCompositeFrame            * fIncomeTab;
        TH1F                        * fCategoriesHistogram;
        TH1F                        * fMonthsHistogram;
        TCanvas                     * fCanvas;
        TCanvas                     * fCanvasYear;
        TGComboBox                  * fStatisticsMonth;
        TGComboBox                  * fStatisticsYear;
        TGComboBox                  * fStatisticsYear2;
        TGComboBox                  * fStatisticsCategory;
        TXMLParser                  * fBalanceXMLParser;
        TGNumberEntryField          * fWithdrawnIdEntry;
        TXMLParser                  * fIncomeXMLParser;
        TGLabel                     * fCurrentStatusLabel;
        TGLabel                     * fLastStatusLabel;

};
