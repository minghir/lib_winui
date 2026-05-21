//#include <afxwin.h>
#include "dataSource.hpp"
#include "stringUtils.hpp"
#include "globals.hpp"
#include "ui\ConsoleManager.hpp" // Adjust the path
#include <iostream>
#include <algorithm>



// Constructor
//dataSource::dataSource(const std::string& name, const std::string& type, const std::wstring& query, std::shared_ptr<dbConnection> conn)
dataSource::dataSource(const std::string& name, const std::string& type, const std::wstring& query, dbConnection *conn)
    : name(name), type(type), query(query), conn(conn) {

        //connectToSource();
    }

// Getters
const std::string& dataSource::getName() const {
    return name;
}

const std::string& dataSource::getType() const {
    return type;
}

const std::wstring& dataSource::getQuery() const {
    return query;
}

const std::wstring& dataSource::getRunQuery() const {
    return run_query;
}


const std::vector<reportRow> dataSource::getRows() const {
    return rows;
}

std::vector<reportRow> dataSource::getDataRows() {
    return data_rows;
}


// Accesare membru connection
//std::shared_ptr<dbConnection> dataSource::getConnection() {
dbConnection* dataSource::getConnection() {
    return conn;
}

// Adaugă o variabilă
void dataSource::addRow(const reportRow& row) {
    rows.push_back(row);
}

// Afișează variabilele
void dataSource::print()  {
    std::cout << "Sursa de date: " << name << "(" << type << "):";
    std::wcout<<query << std::endl;

    for (auto& row : rows) {
        row.print();
    }
}

// Conectare folosind membrul `connection`
bool dataSource::connectToSource() {

    for (const auto& g_var : global_vars) {
       // std::cout << "Caut var:" << g_var.second.getTplName() << " in " << wstr_to_str(conn->getConnectionDSN()) << std::endl;
        if (conn->getConnectionDSN().find(str_to_wstr(g_var.second.getTplName())) != std::string::npos) {
            conn->setConnectionDSN(rpl_wstr_in_wstr(conn->getConnectionDSN(), str_to_wstr(g_var.second.getTplName()), g_var.second.getValue()));
            //std::wcout << L"AM SETAT DSN:" << conn->getConnectionDSN() << L"|" << L"$" + g_var.first << "=" << wstr_to_str(g_var.second.getValue()) << std::endl;
        }
    }
    

    if(!conn->openDatabase())
        std::cout << "EEEroare la conectare" << std::endl;

    //getDbData();

    if (conn->isConnected()) {
        //std::cout << "Conectat la sursa: " << name << " prin conexiunea de tip " << conn->getType() << std::endl;
        ConsoleManager::getInstance().log(L"Conectat la sursa: " + str_to_wstr(name) + L" prin conexiunea de tip " + str_to_wstr(conn->getConnectionType()) );
        return true;
    } else {
        std::cerr << "Eroare la conectarea sursei: " << name << std::endl;
        return false;
    }


}

void dataSource::disconnectFromSource() {
    return conn->closeDatabase();
 }

bool dataSource::getDbData(){

    data_rows.clear();

    if(type == "table"){
        query = L"SELECT * FROM " + query;
    }
    //std::wcout << "Incerc query:" << query << std::endl;
    if(conn->isConnected()){

       // std::cout<<name<<"-:conectat"<<std::endl;
    }
    else{
        //std::cout<<name<<"!!!!!!!:Neconectat"<<wstr_to_str(conn->getConnectionDSN())<<std::endl;
        conn->openDatabase();

    }
    
    prepareQuery();
    //std::wcout << L"Am sa rulez "<<str_to_wstr(name) << ":" << run_query << std::endl<<std::endl;


    if(!conn->execQuery(run_query,name)){
        return false;
     }
    reset_rows_counter(rows.front().getName());
    //std::cout << "START: " <<conn.getRowCount(name) <<"-" << rows.size()<< std::endl;
    std::map<std::wstring,std::wstring> map_tmp;// = conn.fetchMap(name);
    //while( !(map_tmp = conn->fetchMap(name)).empty()){
    while (conn->fetchNextRow(name)) {
        map_tmp = conn->fetchMap(name);

        reportRow tmp_row;
        for (reportRow& row : rows) {

            if (!map_tmp.empty()) {

                for(const auto& col : map_tmp){

                    auto firstKey = col.first;
                  
                    //if (col.first == L"tara") std::wcout << col.second << std::endl;
                    //else  std::wcout << L"CAUT :"<< col.first << std::endl;

                   row.setVarValue(wstr_to_str(firstKey),wstr_trim(col.second));
 

                    tmp_row = row;
//                    std::wcout<<"stezt la var:"<<firstKey<<" :"<<col.second <<std::endl;

                   // tmp_row.setVarValue(wstr_to_str(firstKey),wstr_trim(col.second));

    //              std::wcout<<"Am stezt la var:"<<str_to_wstr(tmp_row.getVarByName("nume_operator").getName())<< tmp_row.getVarByName("nume_operator").getValue()<< std::endl;


//std::cout << "aaaaaaaaaaaaaaaaaaaaaaa" << std::endl;

                   // reportVar tmp_var = row.getVarByName(wstr_to_str(firstKey));
                   // tmp_var.setValue(wstr_trim(col.second));
                   // tmp_row.addVariable(tmp_var);
                }
            }
             data_rows.push_back(tmp_row);

         }
        
         //data_rows.push_back(row);
    }
 //   std::cout << "END--------" <<      data_rows.size() << std::endl;
    
    return true; 
}


//void dataSource::setConnection(std::shared_ptr<dbConnection> con){
void dataSource::setConnection(dbConnection *con) {
    conn = con;
}


std::wstring dataSource::fetch(){
    std::wstring wstr;

     for(reportRow& row : data_rows){
  //       std::cout<<"ADAUG LA fetc sursa:"<<getName() << " :" <<row.getName() << std::endl;
            wstr += row.fetch();
     }

    return wstr;
}


void dataSource::addDataSourceVar(reportVar ds_var){
//  data_source_vars.push_back(ds_var);
//    std::cout<<"fac insert in data_source_vars"<<std::endl;
//std::cout << "AICIIII:"<< data_source_vars.size()<<std::endl;

    data_source_vars.insert({ds_var.getName(), ds_var});
//std::cout << "AICIIII22:"<< data_source_vars.size()<<std::endl;


}

void dataSource::setDataSourceVarVal(std::string nm, std::wstring val){
    if (data_source_vars.find(nm) != data_source_vars.end()){
        std::wcout<<"Setez datasource var la :"<<str_to_wstr(nm)<<":"<<val<<std::endl;
        data_source_vars[nm].setValue(val);
    }
}


/*
void dataSource::prepareQuery(){

  //  std::cout << "AICIIII:"<< data_source_vars.size()<<std::endl;
    int i = 0;
    LOG_WARNING(query);
    if(data_source_vars.empty()){
        run_query = query;
    }else{
        for(const auto& ds_var : data_source_vars){
            auto firstKey = ds_var.first;
            if(ds_var.second.getType() != "query_var") continue;

            //std::cout<<"Inlocuiesc in query:" <<data_source_vars.size()<< ds_var.second.getName() << " :" << wstr_to_str(ds_var.second.getValue() )<< std::endl;
            std::wstring var_val_to_change = ds_var.second.getValue();
            
            //if (var_val_to_change.find(L',') != std::wstring::npos) {

           var_val_to_change = stripQuotes(var_val_to_change);
           
            if(i == 0){
                run_query = rpl_wstr_in_wstr(query, str_to_wstr(ds_var.second.getTplName()), var_val_to_change);
            }else{
                run_query = rpl_wstr_in_wstr(run_query, str_to_wstr(ds_var.second.getTplName()), var_val_to_change );
            }
            
            i++;
            //std::wcout<<L"AM COMPUS QUERY:"<<run_query <<std::endl;
        }
    }

    for(const auto& g_var : global_vars){
        LOG_DEBUG(L"[dataSource::prepareQuery()] global var:" + str_to_wstr(g_var.first) + L" si val: "+ g_var.second.getValue());
        if(i==0){
             run_query = rpl_wstr_in_wstr(query, L"$"+str_to_wstr(g_var.first), g_var.second.getValue() ); 
        }else{
             run_query = rpl_wstr_in_wstr(run_query, L"$"+str_to_wstr(g_var.first), g_var.second.getValue() ); 
        }

        i++;
    }
    //AfxMessageBox(CString(wstr_to_str(run_query).c_str()));
    LOG_DEBUG( L"[dataSource::prepareQuery()] QUERY dupa replace:" +  run_query );
}
*/

void dataSource::prepareQuery() {
    run_query = query;
    int i = 0;

    // 1. Variabilele din sursa de date (XML)
    for (const auto& ds_var : data_source_vars) {
        if (ds_var.second.getType() != "query_var") continue;
        std::wstring tplName = str_to_wstr(ds_var.second.getTplName());
        if (tplName.empty() || tplName == L"$") continue; // PROTECȚIE

        run_query = rpl_wstr_in_wstr(run_query, tplName, stripQuotes(ds_var.second.getValue()));
        i++;
    }

    // 2. Variabilele globale - Colectăm și sortăm pentru a evita înlocuirile parțiale
    std::vector<std::pair<std::wstring, std::wstring>> sortedVars;
    for (const auto& g_var : global_vars) {
        std::wstring name = str_to_wstr(g_var.first);
        if (!name.empty()) {
            sortedVars.push_back({ L"$" + name, g_var.second.getValue() });
        }
    }

    // Sortăm descrescător după lungimea numelui (ex: $total_where_clz înaintea $where_cls)
    std::sort(sortedVars.begin(), sortedVars.end(), [](const auto& a, const auto& b) {
        return a.first.length() > b.first.length();
        });

    for (const auto& v : sortedVars) {
        LOG_DEBUG(L"[dataSource::prepareQuery()] Inlocuiesc " + v.first + L" cu valoare.");
        run_query = rpl_wstr_in_wstr(run_query, v.first, v.second);
    }

    LOG_DEBUG(L"[dataSource::prepareQuery()] QUERY FINAL: " + run_query);
}

std::map<std::string, reportVar> dataSource::getDataSourceVars(){
    return data_source_vars;
}

reportVar dataSource::getDataSourceVarByName(const std::string& var_name){
  auto it = data_source_vars.find(var_name);
  if (it != data_source_vars.end()) {
        return it->second;  // Returnează obiectul găsit
    }
    throw std::runtime_error("Variabila nu a fost găsită!");  // Aruncă o excepție dacă nu există

}

void dataSource::clean() {
    data_rows.clear();  // Lista variabilelor
    rows.clear();  // Lista variabilelor
    conn->closeDatabase();
   // delete conn;                   // Membru pentru conexiune
                                                          //
     data_source_vars.clear();
}

void dataSource::setGlobalVars() {
    for (reportRow& row : data_rows) {
        //       std::cout<<"ADAUG LA fetc sursa:"<<getName() << " :" <<row.getName() << std::endl;
        row.setGlobalVars();
    }
}