#include "ui\vDbApp.hpp"
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
#include "ui\vDbXmlEditDialog.hpp"
#include "ui\vTabControl.hpp"
#include "ui\vCheckBox.hpp"
#include "ui\vGroupBox.hpp"
#include "ui\vRadioButton.hpp"
#include "ui\vDatePicker.hpp"
#include "ui\vSeparator.hpp"
#include "ui\vMessageDialog.hpp"
#include "ui\vXmlDbDialog.hpp"
#include "ui\vDbGridPicker.hpp"
#include "ui\vDbComboBox.hpp"
#include "ui\vCodeView.hpp"

#include "dbConnection.hpp"
#include "odbcConnection.hpp"
#include "dbfConnection.hpp"
#include "ui/FontManager.hpp"


#include "XmlTestDialog.hpp"
#include "CodeDialog.hpp"

//#include "vdb_engine\vEngine.hpp"
//#include "vdb_engine\vDataLoader.hpp"

#include<iostream>
#include <sstream>

#include <ShellScalingApi.h>
#pragma comment(lib, "Shcore.lib")


class myDbApp : public vDbApp {
public:
    myDbApp(HINSTANCE hInstance) :vDbApp(hInstance) {};
    ~myDbApp() {};

      bool initGui() override {

          auto myDb2 = std::make_unique<odbcConnection>("odbc", L"local_ANC_Server");
          if (myDb2->openDatabase()) {
              this->addDbConnection(L"local_ANC_Server", std::move(myDb2));
              ConsoleManager::getInstance().log(L"Baza de date 'local_ANC_Config_Server' a fost deschisă și adăugată.");
          }
          else {
              ConsoleManager::getInstance().log(L"[ERROR] Nu s-a putut deschide baza de date 'local_ANC_Config_Server'.");
          }
          dbConnection* db2 = getDbConnection(L"local_ANC_Server");

        // 1. Fereastra Principală
        auto pMainWindow = std::make_unique<vWindow>(m_instance, "main", WindowType::StandardWindow, true, getEventDispatcher());
        pMainWindow->setLayoutStrategy(std::make_unique<AnchorLayout>());
        pMainWindow->create(L"VAppMainWindowClass", L"Test vTabControl", WS_OVERLAPPEDWINDOW , 100, 100, 1024, 768, nullptr, nullptr);
        addWindow("main", std::move(pMainWindow));
        auto mainWindow = getWindow("main");

        HWND hMain = mainWindow->getHandle();
        auto m_fileMenu = std::make_unique<vMenu>("file_menu", getEventDispatcher());
        m_fileMenu->addItem("file_open", L"&Autentificare");
        m_fileMenu->addSeparator("file_sep");
        m_fileMenu->addItem("file_exp", L"E&xportă CSV");
        m_fileMenu->addSeparator("file_sep_e");
        m_fileMenu->addItem("file_open", L"Deschide cod");
        m_fileMenu->addItem("file_exit", L"I&eșire");
        m_fileMenu->create(hMain);

        auto m_helpMenu = std::make_unique<vMenu>("help_menu", getEventDispatcher());
        m_helpMenu->addItem("help", L"&Ajutor");
        m_helpMenu->addItem("about", L"&Despre");
        m_helpMenu->create(hMain);

        auto m_mainMenu = std::make_unique<vMenu>("main_menu", getEventDispatcher());
        m_mainMenu->create(hMain);
        m_mainMenu->addSubMenu(L"&Fișier", m_fileMenu.get());
        m_mainMenu->addSubMenu(L"&Ajutor", m_helpMenu.get());

        // Atașare la fereastră
        mainWindow->setMenu(m_mainMenu.get());

        getEventDispatcher().registerHandler("file_exit", [this]() {
            PostQuitMessage(0);
            });

        getEventDispatcher().registerHandler("file_open", [this]() {
            openCodeDialog();
            });

        getEventDispatcher().registerHandler("about", [this]() {
            vMessageDialog::Info(L"uiTest: testam interfata grafica.");
            });

        // 2. Panel Principal
        auto panel = std::make_unique<vPanel>(m_instance, "mainPanel", 0, 0, 800, 600, getEventDispatcher());
        panel->setLayoutStrategy(std::make_unique<VerticalStackLayout>());
        
        panel->setHeightMode(SizeMode::FILL);
        panel->setWidthMode(SizeMode::FILL);
        panel->setMargins(10, 10, 10, 10);

        vPanel* pMainPanel = panel.get();
        pMainPanel->setBackgroundColor(RGB(0, 0, 100));
        mainWindow->addChild("mainPanel", std::move(panel));

        // 3. TabControl
        auto tabControl = std::make_unique<vTabControl>(m_instance, "mainTabs", 0, 0, 600, 400, getEventDispatcher());
        tabControl->setHeightMode(SizeMode::FILL);
        tabControl->setWidthMode(SizeMode::FILL);
        //tabControl->setMargins(10, 10, 10, 10);
        vTabControl* pTabs = tabControl.get();
        pMainPanel->addChild("mainTabs", std::move(tabControl));

        // --- TAB 1: Configurare ---
        auto page1 = std::make_unique<vPanel>(m_instance, "page1", 0, 0, 100, 100, getEventDispatcher());
        page1->setHeightMode(SizeMode::FILL);
        page1->setWidthMode(SizeMode::FILL);
        page1->setBackgroundColor(RGB(0, 100, 100));
        page1->setLayoutStrategy(std::make_unique<VerticalStackLayout>());
        //page1->setMargins(20, 20, 20, 20);

        vPanel* pg1 = page1.get();
        pTabs->addTabPage(L"Configurare", std::move(page1));
        //pTabs->addChild("pConf", std::move(page1));
        

        pg1->addChild("lbl1", std::make_unique<vLabel>(m_instance, "lbl1", L"Acesta este Tab-ul de Configurare", 0, 0, 300, 30, getEventDispatcher()));
        pg1->addChild("btn1", std::make_unique<vButton>(m_instance, "btn1", L"Buton Test Tab 1", 0, 0, 150, 40, getEventDispatcher()));
        
        vLabel* lbl1 = pg1->getChildAs<vLabel>("lbl1");
        lbl1->setBackgroundColor(RGB(2, 2, 200));
        lbl1->setTextColor(RGB(255, 255, 255));


        vButton* btn1 = dynamic_cast<vButton*>(pg1->getChild("btn1"));
        btn1->on("click", [this, hMain]() {
            LOG_DEBUG(L"Deschidem dialogul XML...");

            this->removeWindow("xml_dlg");

            // 1. Instanțiem dialogul (folosind calea către XML)
            // Transmitem hMain ca părinte pentru a-l centra față de fereastra principală


            auto myDialog = std::make_unique<XmlTestDialog>(
                m_instance,
                "xml_test_dlg",
                getEventDispatcher(),
                "xmldlg/test4.xml",
                getDbConnection(L"local_ANC_Server")
                );
            //myDialog->init();
            
            this->addWindow("xml_dlg", std::move(myDialog));
            this->getWindow("xml_dlg")->show();
            });

        auto check = std::make_unique<vCheckBox>(m_instance, "chkSave", L"Salvează automat", 0, 0, 350, 30, getEventDispatcher());
        vCheckBox* pCheck = check.get();
        pCheck->setChecked(true);
        check->setMargins(10, 50, 0, 0);
        pCheck->on("change", [pCheck]() {
            bool isChecked = pCheck->isChecked();
            LOG_DEBUG(L"Checkbox-ul " + str_to_wstr(pCheck->getId()) + L" este acum: " + (isChecked ? L"Bifat" : L"Debifat"));
            });
        
        auto check2 = std::make_unique<vCheckBox>(m_instance, "chkLoad", L"Încarcă automat", 0, 0, 350, 30, getEventDispatcher());
        vCheckBox* pCheck2 = check2.get();
        pCheck2->setChecked(false);
        check2->setMargins(10, 10, 0, 0);
        pCheck2->on("change", [pCheck2]() {
            bool isChecked = pCheck2->isChecked();
            LOG_DEBUG(L"Checkbox-ul " + str_to_wstr(pCheck2->getId()) + L" este acum: " + (isChecked ? L"Bifat" : L"Debifat"));
            });



        auto group = std::make_unique<vGroupBox>(m_instance, "grp1", L"Opțiuni", 0, 0, 600, 300, getEventDispatcher());
        group->setMargins(10, 30, 10, 10); // 25px sus oferă spațiu pentru titlu
        group->setBackgroundColor(RGB(2, 200, 200));
        //group->setTextColor(RGB(255, 255, 255));
        group->setTextColor(RGB(0, 0, 0));
        //group->setBackgroundColor(RGB(0, 0, 0));
        //vSpacer::s_debugMode = true;
        group->setLayoutStrategy(std::make_unique<VerticalStackLayout>());
        vGroupBox* grp = group.get();
        pg1->addChild("grp1", std::move(group));

        //grp->addChild("spacerTop", std::make_unique<vSpacer>("spacerTop", 0, 0, 20, 60, getEventDispatcher()));
        grp->addChild("chkSave", std::move(check));
        grp->addChild("chkLoad", std::move(check2));

        grp->addChild("spacer1", std::make_unique<vSpacer>( "spacer1", 0, 0, 350, 10, getEventDispatcher()));
        grp->addChild("rbM", std::make_unique<vRadioButton>(m_instance,"rbM", L"Masculin", 0, 0, 350, 30, getEventDispatcher()));

        vRadioButton* rbM = dynamic_cast<vRadioButton*>(grp->getChild("rbM"));
        rbM->setMargins(20, 0, 0, 0);
        rbM->setChecked(true);
        rbM->setGroupName("gender");
        rbM->setFont(L"Calibri (Body)", 11, fontWeight(FontWeight::Bold), false, false);
        grp->addChild("spacer2", std::make_unique<vSpacer>("spacer2", 0, 0, 350, 5, getEventDispatcher()));
        grp->addChild("rbF", std::make_unique<vRadioButton>(m_instance, "rbF", L"Feminin", 0, 0, 350, 30, getEventDispatcher()));
        vRadioButton* rbF = dynamic_cast<vRadioButton*>(grp->getChild("rbF"));
        rbF->setMargins(20, 0, 0, 0);
        rbF->setGroupName("gender");

        grp->addChild("spacer3", std::make_unique<vSpacer>("spacer3", 0, 0, 350, 15, getEventDispatcher()));
        grp->addChild("datePick", std::make_unique<vDatePicker>(m_instance, "datePick", 0, 0, 350, 30, getEventDispatcher()));
        vDatePicker* datePi = dynamic_cast<vDatePicker*>(grp->getChild("datePick"));
        datePi->setMargins(10, 0, 0, 0);
        datePi->on("dateChange", [datePi]() {
            LOG_SUCCESS(L"Utilizatorul a ales data: " + datePi->getDateString());
            });

        grp->applyLayout();
        // ADĂUGĂM TOTUL ÎNAINTE DE MOVE

        pg1->addChild("spacer1", std::make_unique<vSpacer>("spacer1", 0, 0, 350, 10, getEventDispatcher()));
        pg1->addChild("rbMa", std::make_unique<vRadioButton>(m_instance, "rbMa", L"Masculina", 0, 0, 350, 30, getEventDispatcher()));
        vRadioButton* rbMa = dynamic_cast<vRadioButton*>(pg1->getChild("rbMa"));
        rbMa->setMargins(20, 0, 0, 0);
        rbMa->setGroupName("other");
        
        pg1->addChild("spacer2", std::make_unique<vSpacer>("spacer2", 0, 0, 350, 5, getEventDispatcher()));
        pg1->addChild("rbFa", std::make_unique<vRadioButton>(m_instance, "rbFa", L"Feminina", 0, 0, 350, 30, getEventDispatcher()));
        vRadioButton* rbFa = dynamic_cast<vRadioButton*>(pg1->getChild("rbFa"));
        rbFa->setMargins(20, 0, 0, 0);
        rbFa->setGroupName("other");

        // --- ADAUGARE vDbGridPicker în TAB 1 ---
        // 1. Creăm obiectul. 
// 1. Label-ul separat
        pg1->addChild("lblClient", std::make_unique<vLabel>(m_instance, "lbl_cli", L"Selectați Clientul:", 5, 0, 200, 20, getEventDispatcher()));
        auto lbl_cli = pg1->getChildAs<vLabel>("lblClient");
        if(lbl_cli) lbl_cli->setTextColor(RGB(255,255,255));
        // 2. Picker-ul
        auto picker = std::make_unique<vDbGridPicker>(m_instance, "pickerClienti", 0, 0, 100, 30, getEventDispatcher(), db2);
        picker->setLookupInfo(L"select registration from date_aviatie.fleet", L"registration");
        picker->setBaseHeight(30);
        picker->setWidthMode(SizeMode::FILL);
        picker->setBackgroundColor(pg1->getBackgroundColor());
        picker->setMargins(10, 5, 10, 10);
        pg1->addChild("pickerClienti", std::move(picker));

            //pg1->addChild("dbCombo", std::make_unique<vDbComboBox>(m_instance, "dbCombo",  5, 0, 200, 20, getEventDispatcher(),db2));
        auto dbCombo = std::make_unique<vDbComboBox>(m_instance, "dbCombo", 0, 0, 500, 30, getEventDispatcher(), db2);
        dbCombo->setMargins(10, 5, 10, 10);
        dbCombo->setWidthMode(SizeMode::FILL);
        vDbComboBox* dbCombobox = dbCombo.get();
        pg1->addChild("dbCombo", std::move(dbCombo));
        dbCombobox->populate(L"SELECT codexcept_id, concat(codexp,' - ', denexp) as except FROM date_aviatie.codexcept_terminal");
        //dbCombobox->setSelectedByValue(LPARAM(18));
        dbCombobox->setSelectedIndex(-1);
        dbCombobox->addItem(L"None above", 999);
        //dbCombobox->populate(L"SELECT concat(codexp,' - ', denexp) as except FROM date_aviatie.codexcept_terminal");
        
            


        pg1->applyLayout();

        // Acum mutăm pagina în TabControl. addTabPage va apela create() care va crea și copiii de mai sus.
        
        
        // --- TAB 2: Statistici ---
        auto page2 = std::make_unique<vPanel>(m_instance, "page2", 0, 0, 100, 100, getEventDispatcher());
        page2->setHeightMode(SizeMode::FILL);
        page2->setWidthMode(SizeMode::FILL);
        page2->setBackgroundColor(RGB(100, 100, 100));
        page2->setLayoutStrategy(std::make_unique<VerticalStackLayout>());
        //page2->setMargins(20, 20, 20, 20);

        vPanel* pg2 = page2.get();
        pTabs->addTabPage(L"Statistici", std::move(page2));
        // ADĂUGĂM TOTUL ÎNAINTE DE MOVE
        pg2->addChild("lbl2", std::make_unique<vLabel>(m_instance, "lbl2", L"Aici vor fi afișate statisticile", 0, 0, 400, 30, getEventDispatcher()));
        vLabel* lbl2 = pg2->getChildAs<vLabel>("lbl2");
        lbl2->setBackgroundColor(RGB(100, 2, 22));
        lbl2->setTextColor(RGB(255, 255, 255));
        auto combo = std::make_unique<vComboBox>(m_instance, "cmbTest", 0, 0, 200, 30, getEventDispatcher(), 100);
        vComboBox* pComboRef = combo.get(); // Păstrăm o referință dacă vrem să adăugăm itemi după move
        pg2->addChild("cmbTest", std::move(combo));

        pComboRef->addItem(L"Opțiunea 1");
        pComboRef->addItem(L"Opțiunea 2");
        
        pComboRef->on("selectionChange", [pg2]() {
            vComboBox* pCombo = dynamic_cast<vComboBox*>(pg2->getChild("cmbTest"));
            int idx = pCombo->getSelectedIndex();
            std::wstring item = pCombo->getSelectedText();
            vLabel* pLabel = dynamic_cast<vLabel*>(pg2->getChild("lbl2"));
            pLabel->setText(L"Aici vor fi afișate statisticile pentru:" + item);
            
            LOG_SUCCESS(L"ComboBox schimbat! Selecție: " + item + L" (Index: " + std::to_wstring(idx) + L")");
            });
        

        pg2->addChild("sep1", std::make_unique<vSeparator>(m_instance, "sep1", 0, 0, 100, SeparatorOrientation::Horizontal, getEventDispatcher()));
        vSeparator* pSep = dynamic_cast<vSeparator*>(pg2->getChild("sep1"));
        pSep->setWidthMode(SizeMode::FILL);
        pSep->setMargins(10, 20, 10, 20); // Margini generoase ca să-l vezi clar
        pg2->applyLayout();


        // --- TAB 3: DBF ---
        auto page3 = std::make_unique<vPanel>(m_instance, "page3", 0, 0, 100, 100, getEventDispatcher());
        page3->setHeightMode(SizeMode::FILL);
        page3->setWidthMode(SizeMode::FILL);
        //page3->setBackgroundColor(RGB(100, 100, 100));
        page3->setLayoutStrategy(std::make_unique<VerticalStackLayout>());
        vPanel* pg3 = page3.get();
        pTabs->addTabPage(L"Test cu DBF", std::move(page3));
       
        auto dbfDb = std::make_unique<dbfConnection>("DBF_NATIVE", L"dbfs");
        auto db = dbfDb.get();
        if (db->openDatabase()) {
            this->addDbConnection(L"DBF_Test", std::move(dbfDb));
            LOG_INFO(L"Baza de date 'DBF_Test' a fost deschisă și adăugată.");
        }
        else {
            LOG_ERROR(L"Nu s-a putut deschide baza de date 'DBF_Test'.");
        }

        if (db && db->isConnected()) {

            db->execQuery(L"SELECT * from persoane");
            if (db->fetchNextRow()) {
                std::vector<std::wstring> firstRow = db->fetchRow();

                print_wstr_map(db->fetchMap());

                LOG_SUCCESS(L"Primul rand citit are " + std::to_wstring(firstRow.size()) + L" coloane.");
                for (size_t i = 0; i < firstRow.size(); ++i) {
                    LOG_DEBUG(L"Col [" + std::to_wstring(i) + L"]: " + firstRow[i]);
                }
            }
            else {
                LOG_ERROR(L"Nu s-a putut citi niciun rand. Fisierul e gol sau header-ul e gresit.");
            }

            auto grid = std::make_unique<vGrid>(m_instance, "dataGrid", 0, 0, 800, 300, getEventDispatcher());
            //grid->setHeightMode(SizeMode::FILL);
            grid->setWidthMode(SizeMode::FILL);
            grid->setMargins(5, 5, 5, 2);
            vGrid* pGrid = grid.get();
            pg3->addChild("dataGrid", std::move(grid));

            // Populare coloane
            const auto& columns = db->getColumnNames();
            for (const auto& colName : columns) {
                pGrid->addColumn(colName, 100); // Nume coloană și lățime default
            }

            // Populare rânduri
            db->execQuery(L"SELECT * from persoane");
            while (db->fetchNextRow()) {
                pGrid->addRow(db->fetchRow());
            }

           


            auto vDgrid = std::make_unique<vDbSortableGrid>(m_instance, "rudeGrid", 0, 0, 100, 400, getEventDispatcher(), getDbConnection(L"DBF_Test"));
            vDgrid->setHeightMode(SizeMode::FILL);
            vDgrid->setWidthMode(SizeMode::FILL);
            vDgrid->setMargins(5, 3, 5, 5);
            vDbSortableGrid* gridRude = vDgrid.get();
            pg3->addChild("rudeGrid", std::move(vDgrid));
            gridRude->populate(L"SELECT * FROM rude");
            pg3->applyLayout();
        }
        
        // TAB 4
        auto page4 = std::make_unique<vPanel>(m_instance, "page4", 0, 0, 100, 100, getEventDispatcher());
        page4->setHeightMode(SizeMode::FILL);
        page4->setWidthMode(SizeMode::FILL);
        //page3->setBackgroundColor(RGB(100, 100, 100));
        page4->setLayoutStrategy(std::make_unique<VerticalStackLayout>());
        vPanel* pg4 = page4.get();
        pTabs->addTabPage(L"Test cu ODBC", std::move(page4));
        pg4->setBackgroundColor(RGB(255, 255, 255));
       
        
        
        if (db2 && db2->isConnected()) {


            // Grid
            auto grid2 = std::make_unique<vDbFilteredGrid>(m_instance, "testGrid2", 0, 0, 100, 100, getEventDispatcher(), db2);
            grid2->setHeightMode(SizeMode::FILL);
            grid2->setWidthMode(SizeMode::FILL);
            grid2->setMargins(10, 0, 10, 0);
            vDbFilteredGrid* pGrid2 = grid2.get();
            pg4->addChild("testGrid2", std::move(grid2));

            //pGrid->populate(L"SELECT fpl_id,nr_fsna,adep_dep,ades_dep FROM fsna.fsna_2025","nr_fsna");
            std::wstring queryANC = L"SELECT * FROM fsna.fsna_2025 ";

            //std::wstring queryANC = L"SELECT fpl_id, adep, ades, eobt, eobd, reg FROM fsna.fpl";
            pGrid2->populate(queryANC);
            pGrid2->setUniqueIdField("fpl_id");

            pGrid2->setOnDbEdit([&](DbDialogMode mode, const std::wstring& id) {
                // Aici creezi ce tip de dialog vrei tu!
                
                auto dbDialog = new vXmlDbDialog(
                    m_instance,
                    "dynamic_edit_dialog",
                    getEventDispatcher(),
                    "xmldlg/test4.xml",
                    getDbConnection(L"local_ANC_Server")
                );
                // Configurezi dialogul XML
                dbDialog->setDbConfig(mode, L"USERS", L"ID", id);

                // Îl inițializezi (asta va rula injectDbRecursively și loadRecordFromDb)
                dbDialog->init();

                // Îl afișezi
                dbDialog->show();
                
                // OPȚIONAL: Poți să te abonezi la un eveniment din dbDialog 
                // ca să dai Refresh la Grid după ce se închide fereastra de editare.
                });

        }
        

        // --- TAB 5: Test Manual vGrid ---
        auto page5 = std::make_unique<vPanel>(m_instance, "page5", 0, 0, 100, 100, getEventDispatcher());
        page5->setHeightMode(SizeMode::FILL);
        page5->setWidthMode(SizeMode::FILL);
        page5->setLayoutStrategy(std::make_unique<VerticalStackLayout>());
        vPanel* pg5 = page5.get();

        // 1. Adăugăm tab-ul în control (Atenție: ordinea contează pentru vizibilitate)
        pTabs->addTabPage(L"Manual Grid", std::move(page5));

        // 2. Creăm vGrid-ul (fără DB, doar UI)
        auto manualGrid = std::make_unique<vGrid>(m_instance, "manualGrid", 0, 0, 400, 200, getEventDispatcher());
        manualGrid->setHeightMode(SizeMode::FILL);
        manualGrid->setWidthMode(SizeMode::FILL);
        manualGrid->setMargins(20, 20, 20, 20);

        vGrid* pManGrid = manualGrid.get();
        pg5->addChild("manualGrid", std::move(manualGrid));

        // IMPORTANT: În vGrid-ul tău, coloanele TREBUIE adăugate după ce 
        // obiectul a fost creat și are un HWND valid (lucru garantat în interiorul framework-ului tău la acest pas)

        // 3. Adăugare manuală coloane (Text, Lățime)
        pManGrid->addColumn(L"Coloana A", 150);
        pManGrid->addColumn(L"Coloana B", 150);

        // 4. Adăugare manuală rânduri (std::vector de std::wstring)
        pManGrid->addRow({ L"Rând 1, Celula 1", L"Rând 1, Celula 2" });
        pManGrid->addRow({ L"Rând 2, Celula 1", L"Rând 2, Celula 2" });

        pg5->applyLayout();
      
        // Forțăm refresh pe primul tab
        pTabs->switchPage(0);
        
        
        return true;
    }

    void openCodeDialog() {

        HWND hMain = vApp::getAppInstance()->getMainWindow();

        auto myDialog = std::make_unique<CodeDialog>(
            m_instance,
            "code_test_dlg",
            getEventDispatcher(),
            "xmldlg/test4.xml"
            );
        myDialog->create(L"VRegWindowClass", L"Code viewer",
            WS_OVERLAPPEDWINDOW,
            200, 200, 1000, 600,
            hMain, // <--- SCHIMBĂ nullptr cu hMain
            nullptr);
        myDialog->setLayoutStrategy(std::make_unique<VerticalStackLayout>());
        
        CodeDialog* pDlg = myDialog.get();

        myDialog->init();


        myDialog->applyLayout();
        this->addWindow("code_dlg", std::move(myDialog));
        this->getWindow("code_dlg")->show();
        
        pDlg->applayColors();

        
    }

};

//int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) 
int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow)
{
   
    myDbApp app(hInstance);
    app.startConsole();
    return app.run(nCmdShow);
}