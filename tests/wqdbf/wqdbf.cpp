#include "qdbfConnection.hpp"
//#include "shell/vShell.hpp"
#include "shell/vSqlShellEngine.hpp"

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
#include "ui\vTabControl.hpp"
#include "ui\vCheckBox.hpp"
#include "ui\vGroupBox.hpp"
#include "ui\vRadioButton.hpp"
#include "ui\vDatePicker.hpp"
#include "ui\vSeparator.hpp"
#include "ui\vCanvas.hpp"
#include "ui\vWinConsole.hpp"
#include "ui\vCommandDialog.hpp"

#include "dbConnection.hpp"
#include "odbcConnection.hpp"
#include "dbfConnection.hpp"

#include<iostream>
#include <sstream>
#include <memory>

#include <ShellScalingApi.h>
#pragma comment(lib, "Shcore.lib")

class vLoginDialog : public vWindow {
private:
    vEdit* m_userEdit;
    vEdit* m_passEdit;
    vEdit* m_ipEdit;
    vEdit* m_portEdit; // Câmp nou
    std::function<void(std::wstring)> m_onSuccess; // Trimitem direct URI-ul final

public:
    vLoginDialog(HINSTANCE hInst, EventDispatcher& ed)
        : vWindow(hInst, "login_dlg", WindowType::DialogWindow, false, ed) {}

    void init(std::function<void(std::wstring)> callback) {
        m_onSuccess = callback;

        // 1. Crearea ferestrei
        this->create(L"VLoginClass", L"Conectare Server QDBF",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
            CW_USEDEFAULT, CW_USEDEFAULT, 350, 300, nullptr, nullptr);

        // 2. Configurare Layout (5 rânduri, 2 coloane, gap 10px)
        setLayoutStrategy(std::make_unique<GridLayout>(5, 2, 10));

        // Rând 0: Utilizator
        vControl* l1 = addChildWithReturn("l1", std::make_unique<vLabel>(m_hInstance, "l1", L"Utilizator:", 0, 0, 100, 25, getEventDispatcher()));
        l1->setGridPosition(0, 0);

        m_userEdit = static_cast<vEdit*>(addChildWithReturn("u", std::make_unique<vEdit>(m_hInstance, "u", 0, 0, 150, 25, getEventDispatcher())));
        m_userEdit->setGridPosition(0, 1);
        m_userEdit->setText(L"admin");

        // Rând 1: Parolă
        vControl* l2 = addChildWithReturn("l2", std::make_unique<vLabel>(m_hInstance, "l2", L"Parolă:", 0, 0, 100, 25, getEventDispatcher()));
        l2->setGridPosition(1, 0);

        m_passEdit = static_cast<vEdit*>(addChildWithReturn("p", std::make_unique<vEdit>(m_hInstance, "p", 0, 0, 150, 25, getEventDispatcher(), EditType::PASSWORD)));
        m_passEdit->setGridPosition(1, 1);

        // Rând 2: IP Server
        vControl* l3 = addChildWithReturn("l3", std::make_unique<vLabel>(m_hInstance, "l3", L"IP Server:", 0, 0, 100, 25, getEventDispatcher()));
        l3->setGridPosition(2, 0);

        m_ipEdit = static_cast<vEdit*>(addChildWithReturn("ip", std::make_unique<vEdit>(m_hInstance, "ip", 0, 0, 150, 25, getEventDispatcher())));
        m_ipEdit->setGridPosition(2, 1);
        m_ipEdit->setText(L"10.9.50.241");

        // Rând 3: Port
        vControl* l4 = addChildWithReturn("l4", std::make_unique<vLabel>(m_hInstance, "l4", L"Port:", 0, 0, 100, 25, getEventDispatcher()));
        l4->setGridPosition(3, 0);

        m_portEdit = static_cast<vEdit*>(addChildWithReturn("port", std::make_unique<vEdit>(m_hInstance, "port", 0, 0, 80, 25, getEventDispatcher())));
        m_portEdit->setGridPosition(3, 1);
        m_portEdit->setText(L"3519");

        // Rând 4: Spacer și Buton
        vControl* s1 = addChildWithReturn("s1", std::make_unique<vSpacer>("s1", 0, 0, 10, 10, getEventDispatcher()));
        s1->setGridPosition(4, 0);

        auto btnContainer = std::make_unique<vButton>(m_hInstance, "btn_login", L"Conectare", 0, 0, 100, 35, getEventDispatcher());
        vButton* btn = btnContainer.get(); // Referință temporară pentru configurare
        btn->setGridPosition(4, 1);

        btn->setOnClick([this]() {
            if (m_onSuccess) {
                std::wstring user = m_userEdit->getText();
                std::wstring pass = m_passEdit->getText();

                // Debug Log
                ConsoleManager::getInstance().log(L"[DEBUG] Login cu User: " + user);

                std::wstring fullUri = user + L":" + pass + L"@" +
                    m_ipEdit->getText() + L" " + m_portEdit->getText();
                m_onSuccess(fullUri);
            }
            });

        // Mutăm proprietatea butonului la final
        addChild("btn_login", std::move(btnContainer));

        // 3. Finalizare Vizuală
        centerWindow();
        applyLayout();
    }
};


class QdbfClient : public vSqlShellEngine {
public:
    QdbfClient()
        : vSqlShellEngine(std::make_unique<qdbfConnection>(L"10.9.50.241", 3519)) {}

    // Implementăm metoda cerută de părinte pentru a satisface compilatorul
    bool handleConnect(const ShellCommand& cmd) override {
        // Dacă totuși cineva apelează prin shell /connect
        if (cmd.args.empty()) return false;

        std::wstring ip = cmd.args[0];
        int port = (cmd.args.size() > 1) ? std::stoi(cmd.args[1]) : 8080;

        return connectToServer(ip, port);
    }

    // Noua ta metodă simplificată
    bool connectToServer(const std::wstring& ip, int port = 8080) {
        auto oldCon = std::move(con);
        con = std::make_unique<qdbfConnection>(ip, port);

        if (con->openDatabase()) {
            LOG_SUCCESS(L"Conectat la: " + ip + L":" + std::to_wstring(port));
            return true;
        }
        else {
            LOG_ERROR(L"Eroare: " + con->getError());
            con = std::move(oldCon);
            return false;
        }
    }

    vConResult getLastResult() {
        return this->result;
    }

};

class wqdbfApp : public vDbApp {
private :
    std::unique_ptr<QdbfClient> m_sqlEngine;
public:
    wqdbfApp(HINSTANCE hInstance) :vDbApp(hInstance) {
        m_sqlEngine = std::make_unique<QdbfClient>();
    };

    ~wqdbfApp() {};
    

    bool initGui() override {
        // În loc să creăm fereastra principală direct, afișăm dialogul de login
        auto loginDlg = std::make_unique<vLoginDialog>(m_instance, getEventDispatcher());

        // Configurăm ce se întâmplă când utilizatorul apasă "Conectare"
        loginDlg->init([this](std::wstring uri) {
            LOG_INFO(L"Încercare conectare la: " + uri);

            // Presupunem că aici ai logica de conectare...
            //if (m_sqlEngine->connectToServer(L"10.9.50.241", 3519)) {
            if (m_sqlEngine->connectToServer(uri, 3519)) {
                LOG_SUCCESS(L"Login reușit!");

                // REZOLVARE: Accesăm direct m_windowManager din clasa părinte vApp
                //this->m_windowManager.remove("login_dlg");
                HWND hLogin = this->m_windowManager.get("login_dlg")->getHandle();
                PostMessage(hLogin, WM_CLOSE, 0, 0);

                // Acum deschidem fereastra principală
                this->showMainWindow();
            }
            else {
                LOG_ERROR(L"Conectare eșuată. Verificați datele.");
            }
            });

        loginDlg->show();
        addWindow("login_dlg", std::move(loginDlg));

        return true;
    }

    void showMainWindow() {
        auto mainWindow = std::make_unique<vWindow>(m_instance, "main", WindowType::StandardWindow, true, getEventDispatcher());
        mainWindow->setLayoutStrategy(std::make_unique<AnchorLayout>());
        mainWindow->create(L"VAppMainWindowClass", L"Win Query DBF", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 1024, 768, nullptr, nullptr);

        // Consola de logare
        auto consoleCanvas = std::make_unique<vWinConsole>(m_instance, "log_console", 0, 0, 400, 300, getEventDispatcher());
        consoleCanvas->setWidthMode(SizeMode::FILL);
        consoleCanvas->setHeightMode(SizeMode::FILL);
        consoleCanvas->setBackgroundColor(RGB(30, 30, 30));

        mainWindow->addChild("my_canvas", std::move(consoleCanvas));

        addWindow("main", std::move(mainWindow));

        // Deschidem și fereastra de comenzi
        openCommandWindow();

        LOG_SUCCESS(L"Sistem pregătit.");
    }

    void openCommandWindow() {
        auto cmdDlg = std::make_unique<vCommandDialog>(m_instance, getEventDispatcher());
        cmdDlg->init();

        getEventDispatcher().registerHandler("command_executed", "cmd_modal_window", [this](const std::string& command) {
            // 1. Trimm și curățare
            std::string cleanCmd = command;
            cleanCmd.erase(0, cleanCmd.find_first_not_of(" \n\r\t"));
            cleanCmd.erase(cleanCmd.find_last_not_of(" \n\r\t") + 1);

            if (cleanCmd.empty()) return;

            // 2. DEBOUNCE: Verificăm dacă nu cumva e aceeași comandă trimisă repetat (lost focus după enter)
            static std::string lastExecuted = "";
            static uint64_t lastTime = 0;
            uint64_t now = GetTickCount64();

            if (cleanCmd == lastExecuted && (now - lastTime) < 500) {
                return; // Ignorăm duplicatul
            }

            lastExecuted = cleanCmd;
            lastTime = now;

            // 3. Execuția propriu-zisă
            if (cleanCmd == "brow" || cleanCmd == "browse") {
                this->browResult();
                return;
            }

            if (cleanCmd == "quit" || cleanCmd == "exit") {
                this->shutdown();
                return;
            }

            m_sqlEngine->execute(str_to_wstr(cleanCmd));
            });

        cmdDlg->show();
        this->addWindow("cmd_dialog", std::move(cmdDlg));
    }

    void browResult() {
        vConResult res = m_sqlEngine->getLastResult();
        vConTable& table = res.table;

        if (table.records.empty()) return;

        std::string uniqueId = "brow_window_" + std::to_string(GetTickCount64());

        // 1. Creăm fereastra
        auto browWindow = std::make_unique<vWindow>(m_instance, uniqueId, WindowType::StandardWindow, false, getEventDispatcher());
        browWindow->create(L"VAppBrowWindowClass", L"Browse Data", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 150, 150, 900, 600, nullptr, nullptr);
        browWindow->setLayoutStrategy(std::make_unique<AnchorLayout>());
      
        auto manualGrid = std::make_unique<vGrid>(m_instance, "manualGrid", 0, 0, 400, 200, getEventDispatcher());
        manualGrid->setHeightMode(SizeMode::FILL);
        manualGrid->setWidthMode(SizeMode::FILL);
        manualGrid->setMargins(20, 20, 20, 20);

        vGrid* pManGrid = manualGrid.get();
        browWindow->addChild("manualGrid", std::move(manualGrid));

      
        // 3. Adăugare manuală coloane (Text, Lățime)
        for (size_t i = 0; i < table.columns.size(); ++i) {
            pManGrid->addColumn(table.columns[i], 120);
            
        }

        for (const auto& row : table.records) {
            pManGrid->addRow(row);
        }

        browWindow->applyLayout();
        this->addWindow(uniqueId, std::move(browWindow));
    }
};



//int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) 
int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow)
{

    
    wqdbfApp app(hInstance);
    //app.startConsole();
    return app.run(nCmdShow);
}