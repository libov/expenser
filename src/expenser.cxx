#include <TApplication.h>
#include <TGClient.h>

#include<TExpenser.h>

int main(int argc, char **argv) {

    TApplication theApp("App",&argc,argv);
    new TExpenser(gClient->GetRoot(), 500, 100);
    theApp.Run();

    return 0;
}