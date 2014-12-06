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
        void add();
        void drawExpensesTable();
        void drawExpensesTab();
        void drawStatisticsTab();
        void drawStatisticsYearTab();
        void drawBalanceTab();
        void commit();
        void calculate_monthly();
        void set_withdrawn();
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
        TGCompositeFrame            * fBalanceTab;
        TH1F                        * fCategoriesHistogram;
        TCanvas                     * fCanvas;
        TGComboBox                  * fStatisticsMonth;
        TGComboBox                  * fStatisticsYear;
        TXMLParser                  * fBalanceXMLParser;
        TGNumberEntryField          * fWithdrawnIdEntry;
        TXMLParser                  * fIncomeXMLParser;

};
