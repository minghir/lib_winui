
#include "ui\vDbApp.hpp"
/*
#include "ui\vMenu.hpp"
#include "ui\vPanel.hpp"
#include "ui\vLabel.hpp"
#include "ui\vButton.hpp"
#include "ui\vComboBox.hpp"
#include "ui\vSpacer.hpp"
#include "ui\vDbFormPanel.hpp"
#include "ui\vDbEditDialog.hpp"
#include "ui\vDbFilteredGrid.hpp"
#include "ui\vDbSortableGrid.hpp"
#include "ui\Layouts\Layouts.hpp"
#include "ui\vDbAutoEditDialog.hpp"
#include "ui\vTabControl.hpp"
#include "ui\vCheckBox.hpp"
#include "ui\vGroupBox.hpp"
#include "ui\vRadioButton.hpp"
#include "ui\vDatePicker.hpp"
#include "ui\vSeparator.hpp"
*/
#include "dbConnection.hpp"
#include "odbcConnection.hpp"
#include "dbfConnection.hpp"
#include "csvConnection.hpp"

#include "vdb_engine\vLocalQueryProvider.hpp"
#include "vdb_engine\vSqlShell.hpp"
#include "vdb_engine\vEngine.hpp"
#include "vdb_engine\vDataLoader.hpp"

#include<iostream>
#include <sstream>







class myApp : public vApp {
public:
    myApp(HINSTANCE hInstance) :vApp(hInstance) { setRunMode(RunMode::CONSOLE); };
    ~myApp() {};

    bool initConsole() override {
        vEngine engine;
        engine.init();
        
        engine.getDB()->addSchema(L"aviation");

        vDataLoader loader(&engine);
        
        // Deschizi o conexiune DBF
        //auto myDb = std::make_unique<dbfConnection>("dbf", );
        dbfConnection dbfConn("DBF_NATIVE", L"./dbf/251228AL.dbf");
        if (dbfConn.openDatabase()) {

            // Folosești DataLoader pentru a "pompa" datele în Engine
            
            //loader.loadIntoTable(&dbfConn, L"Zboruri", L"aviation");
            if (loader.loadIntoTable(&dbfConn, L"zboruri"))
                dbfConn.closeDatabase();
        }

        dbfConnection dbfConn2("DBF_NATIVE", L"./dbf/ev20249.dbf");
        if (dbfConn2.openDatabase()) {

            // Folosești DataLoader pentru a "pompa" datele în Engine

            if(loader.loadIntoTable(&dbfConn2, L"ev20249", L"aviation"))
                dbfConn2.closeDatabase();
        }
        




        /*
        csvConnection csvConn("csv", L"./csv");
        if (csvConn.openDatabase()) {

            // Folosești DataLoader pentru a "pompa" datele în Engine

            loader.loadIntoTable(&csvConn, L"csv_test", L"aviation");
        }
        */

        // Dacă vrei LOCAL:
        
        
        
        // În viitor, dacă vrei prin SOCKET (fără să schimbi restul aplicației):
        /*
        vSocketQueryProvider socketProv("127.0.0.1", 8080);
        vSqlShell shell(socketProv);
        shell.run();
        */

        // Acum poți interoga tabelul prin Engine
        //std::wstring query = L"SELECT ARCID, ADEP, ADES FROM aviation.Zboruri";
        //std::wstring query = L"SELECT * FROM aviation.Zboruri";
        //std::wstring query = L"SELECT * FROM aviation.ev20249";
        //QueryResult res = engine.executeQuery(query);

        vLocalQueryProvider localProv(engine);
        vSqlShell shell(localProv);
        shell.executeShellCommand(L"/load ./vdb/date.vdb");
        shell.run(); // Această metodă blochează până la 'exit'
        return true;
    }
};
  


int main(int argc, char* argv[]) {
    // Obținem HINSTANCE pentru vApp (main nu îl primește ca argument)
    HINSTANCE hInstance = GetModuleHandle(NULL);

    // Inițializăm aplicația în modul Consola
    myApp app(hInstance);

    // ConsoleManager::initialize() va detecta că Subsystem e deja Console
    app.startConsole();

    // Rulăm logica (nCmdShow e de obicei SW_SHOW la consolă)
    return app.run(SW_SHOW);
}