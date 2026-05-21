#ifndef VDBAPP_HPP
#define VDBAPP_HPP

#include "vApp.hpp"
#include "../dbConnection.hpp" // Asigură-te că calea e corectă
#include <map>
#include <string>
#include <memory>

class vDbApp : public vApp {
public:
    // Constructor.
    vDbApp(HINSTANCE hInstance);

    // Adaugă o conexiune la baza de date în colecție.
    void addDbConnection(const std::wstring& name, std::unique_ptr<dbConnection> conn);

    // Obține un pointer brut la o conexiune după numele său.
    dbConnection* getDbConnection(const std::wstring& name);

    // Inițializează aplicația și conexiunile la baze de date.
    bool initGui() override;

private:
    // Folosește un map pentru a asocia un nume cu fiecare conexiune, asigurând proprietatea prin unique_ptr.
    std::map<std::wstring, std::unique_ptr<dbConnection>> m_dbConnections;
};

#endif // VDBAPP_HPP