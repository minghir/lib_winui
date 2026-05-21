#ifndef DATASOURCE_HPP
#define DATASOURCE_HPP

#include <string>
#include <vector>
#include <memory>
#include "reportRow.hpp"
#include "dbConnection.hpp" // Include pentru conexiune


class reportRow;

class dataSource {
private:
    std::string name;                  // Numele sursei
    std::string type;                  // Tipul sursei (table, query, etc.)
    std::wstring query;                 // Query SQL asociat
    std::wstring run_query;
    std::vector<reportRow> data_rows;  // Lista variabilelor
    std::vector<reportRow> rows;  // Lista variabilelor

    //std::shared_ptr<dbConnection> conn;                   // Membru pentru conexiune
    dbConnection *conn;                   // Membru pentru conexiune
                                                          //
    std::map<std::string, reportVar> data_source_vars;

public:
    // Constructor
    //dataSource(const std::string& name, const std::string& type, const std::wstring& query, std::shared_ptr<dbConnection> conn);
    dataSource(const std::string& name, const std::string& type, const std::wstring& query, dbConnection *conn);

    // Getters
    const std::string& getName() const;
    const std::string& getType() const;
    const std::wstring& getQuery() const;
    const std::wstring& getRunQuery() const;
    const std::vector<reportRow> getRows() const;
    std::vector<reportRow> getDataRows();

    //std::shared_ptr<dbConnection> getConnection();
    dbConnection* getConnection();

    // Metode pentru gestionarea variabilelor
    void addRow(const reportRow& row);
    void print();

    // Funcționalități conexe
    //void setConnection(std::shared_ptr<dbConnection> con);
    void setConnection(dbConnection *con);
    bool connectToSource();  // Utilizează membrul `conn` pentru conectare
    void disconnectFromSource();
    bool getDbData();

    std::wstring fetch();


    void addDataSourceVar(reportVar ds_var);
    void setDataSourceVarVal(std::string nm, std::wstring val);
    std::map<std::string, reportVar> getDataSourceVars();
    reportVar getDataSourceVarByName(const std::string& var_name); 

    void prepareQuery();

    void clean();
    void setGlobalVars();
};

#endif // DATASOURCE_HPP

