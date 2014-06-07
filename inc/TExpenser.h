#include <TGFrame.h>
#include <TGNumberEntry.h>
#include <TGComboBox.h>
#include<TGTextEntry.h>

#include <TXMLParser.h>

class TExpenser : public TGMainFrame {

    public:

        TExpenser(const TGWindow *p, UInt_t w, UInt_t h);
        virtual ~TExpenser();
        void add();
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
};
