#include <TGFrame.h>
#include <TGNumberEntry.h>

class TExpenser : public TGMainFrame {

    public:

        TExpenser(const TGWindow *p, UInt_t w, UInt_t h);
        virtual ~TExpenser();
        void add();
        ClassDef(TExpenser, 0)

    private:
        
        TGNumberEntry * fDateEntry;
        TGNumberEntry * fAmountEntry;

};