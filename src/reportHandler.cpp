//#include <afxwin.h>

#include "reportHandler.hpp"
#include "odbcConnection.hpp"
#include "csvConnection.hpp"
#include "reportFunction.hpp"
#include "fileUtils.hpp"

#include "ui\\ConsoleManager.hpp" // Adjust the path

//#include "pugixml.hpp"
//#include "dbConnection.hpp"
//#include "stringUtils.hpp"
//#include "fileUtils.hpp"
//#include "reportTemplate.hpp"

//#include <iostream>
//#include <fstream>
//#include <sstream>

// 

reportHandler::reportHandler(){
    //AfxMessageBox("Aici 2");
}

reportHandler::reportHandler(const std::string& xmlDescFile)
    : xmlDescFile(xmlDescFile) {
    //AfxMessageBox("Aici 1");
        parseXmlFile();
         //    loadTemplateFile();
        
    }

// Getters
const std::string& reportHandler::getName() const {
    return name;
}

const std::string& reportHandler::getTemplateFile() const {
    return templateFile;
}

// Adaugă o sursă de date
void reportHandler::addDataSource(const dataSource& source) {
    //std::cout << "Adaug data sourcs:"<<source.getName()<<std::endl;
    ConsoleManager::getInstance().log(L"[LOG] reportHandler::parseDataSource: Adaug data source : " + str_to_wstr(source.getName()));
        dataSources.push_back(source);
}

// Obține toate sursele de date
const std::vector<dataSource>& reportHandler::getDataSources() const {
    return dataSources;
}

// Conectează toate sursele de date
void reportHandler::connectAllDataSources() {

    

    for (auto& source : dataSources) {
        source.connectToSource(); // Utilizează metoda de conectare din dataSource
    }
}

void reportHandler::disconnectAllDataSources() {
    for (auto& source : dataSources) {
        source.disconnectFromSource(); // Utilizează metoda de conectare din dataSource
    }
}

// Afișează informațiile raportului
void reportHandler::printReportInfo()  {
    std::cout << "Nume raport: " << name << std::endl;
    std::cout << "Fișier template: " << templateFile << std::endl;
    std::cout << "Tip template: " << templateType << std::endl;


    std::cout << "Surse de date asociate:" << std::endl;
    for ( auto& source : dataSources) {
        std::cout << "- " << source.getName() << " (" << source.getType() << ")" << std::endl;
        source.print(); // Afișează variabilele pentru fiecare sursă
    }
}
/*
bool reportHandler::loadTemplateFile(){
    if(templateType == "rtf"){
        tpl = new rtfDocument();
        tpl->addText(wstr_read_RTF_file(templateFile));
    }else{
        return false;
    }

//    templateFileContent =  wstr_read_RTF_file(templateFile);
    return true; // Returnează true dacă citirea a avut succes
}
*/

//dataSource reportHandler::parseDataSource(const pugi::xml_node& data_source, const dbConnection& con ) {
dataSource reportHandler::parseDataSource(const pugi::xml_node & data_source, dbConnection *con) {
        
//    std::cout << "\nSursa de date: " << data_source.attribute("name").value() << std::endl;
//    std::cout << "Tip: " << data_source.attribute("type").value() << std::endl;
//    std::cout << "Query: " << data_source.attribute("value").value() << std::endl;
    
    ConsoleManager::getInstance().log(L"[LOG] reportHandler::parseDataSource: Sursa de date : " + str_to_wstr(data_source.attribute("name").value()));
    ConsoleManager::getInstance().log(L"[LOG] reportHandler::parseDataSource: Tip : " + str_to_wstr(data_source.attribute("type").value()));
    ConsoleManager::getInstance().log(L"[LOG] reportHandler::parseDataSource: Query: " + str_to_wstr(data_source.attribute("value").value()));

    std::string data_source_name = data_source.attribute("name").value();
    std::string data_source_type = data_source.attribute("type").value();
    std::wstring query_value = str_to_wstr(std::string(data_source.attribute("value").value()));
    //std::wcout << "AM CITI QUERY DIN XML:" << query_value << std::endl;
    //dataSource ds(data_source_name,data_source_type,query_value, std::make_shared<dbConnection>(con));
    dataSource ds(data_source_name, data_source_type, query_value, con);

  for (pugi::xml_node ds_variable = data_source.child("data_source_variable"); ds_variable; ds_variable = ds_variable.next_sibling("data_source_variable")) {
                std::string var_name = ds_variable.attribute("name").value();
                std::string var_tpl_name = ds_variable.attribute("tpl_name").value();
                std::wstring var_value = str_to_wstr(ds_variable.attribute("value").value());
                std::string var_type = ds_variable.attribute("type").value();
                std::string var_function = ds_variable.attribute("function").value();

                LOG_DEBUG(L"Data_source_Variable - Name: " + str_to_wstr(var_name) + L", Value: " + var_value
                    + L", Type: " + str_to_wstr(var_type));
                if(var_name.size()>0){
                    reportVar var(var_name,var_tpl_name,var_value,var_type,var_function);
                    ds.addDataSourceVar(var);
                }
                LOG_DEBUG(L"Adaug variabila data sursa");
   }





    pugi::xml_node rows = data_source.child("rows");
        if (rows) {
//            std::string name = row.attribute("name").value();
//            std
            reportRow row;
            row.setName(rows.attribute("name").value());
            row.setTplType(rows.attribute("tpl_type").value());
            row.setTplFile(rows.attribute("tpl_file").value());



            for (pugi::xml_node variable = rows.child("variable"); variable; variable = variable.next_sibling("variable")) {
                std::string var_name = variable.attribute("name").value();
                std::string var_tpl_name = variable.attribute("tpl_name").value();
                std::wstring var_value = str_to_wstr(variable.attribute("value").value());
                std::string var_type = variable.attribute("type").value();
                std::string var_function = variable.attribute("function").value();

//                std::cout << "Variable - Name: " << var_name
//                    << ", Value: " << wstr_to_str(var_value)
//                    << ", Type: " << var_type << std::endl;

                ConsoleManager::getInstance().log(L"[LOG] reportHandler::parseDataSource: Variable - Name: " + str_to_wstr(var_name)
                    + L", Value: " + var_value + L", Type: " + str_to_wstr(var_type));


                reportVar var(var_name,var_tpl_name,var_value,var_type,var_function);
                row.addVariable(var);
             }

                for (pugi::xml_node nested_ds : data_source.child("rows").children("data_source")) {
                   //auuto ds = std::make_shared<dataSource>(/* inițializare */); 
                   dataSource n_ds = parseDataSource(nested_ds,con);
                   row.addDataSource( std::make_shared<dataSource>(n_ds));

                   //addDataSource(n_ds);

                    //row.addDataSource( std::make_shared<dataSource>(parseDataSource(nested_ds, con)));

                }


            
            ds.addRow(row); 

    }
/*
    // Parcurgem variabilele
    for (pugi::xml_node row : data_source.child("rows").children("variable")) {
        std::cout << "Variabilă: " << row.attribute("name").value() 
            << " - Template: " << row.attribute("tpl_name").value() 
            << " - Valoare: " << row.attribute("value").value() 
            << " - Tip: " << row.attribute("type").value() << std::endl;
    }

//    dataSource ds;
  */  
    // Verificăm dacă există un nou `data_source` imbricat și îl procesăm recursiv
//    for (pugi::xml_node nested_ds : data_source.child("rows").children("data_source")) {
//        parseDataSource(nested_ds, con);
//    }
   return ds;
}

bool reportHandler::parseXmlFile(){
    pugi::xml_document doc;
    //std::cout<<xmlDescFile<<std::endl;

    // Încarcă fișierul XML
    if (!fileExists(str_to_wstr(getGlobalReportPath()+xmlDescFile))) {
        ConsoleManager::getInstance().log(L"[ERROR] reportHandler::parseXmlFile: Fisierul:"+ str_to_wstr(xmlDescFile) +L" nu exista!!!");
        return false;
    }
    // ConsoleManager::getInstance().log(L"[INFO] reportHandler::parseXmlFile: Incerc sa incarc Fisierul:" + str_to_wstr(getGlobalReportPath()) + str_to_wstr(xmlDescFile) + L" nu exista!!!");
    pugi::xml_parse_result result = doc.load_file((getGlobalReportPath()+xmlDescFile).c_str());

    if (!result) {
        std::cerr << "Eroare la încărcarea fisierului XML: " << result.description() << std::endl;
        std::cerr << "Linia: " << result.offset << std::endl; // Linie/offset unde s-a întâlnit problema

        return false;
    }
    // Accesează rădăcina <vreport>
    pugi::xml_node vreport = doc.child("vreport");
    if (!vreport) {
        std::cerr << "Lipsește nodul <vreport> în fișierul XML." << std::endl;
        return false;
    }

    // Extrage atributele din <vreport>
    name = vreport.attribute("name").value();
    templateFile = vreport.attribute("tpl_file").value();
    title = vreport.attribute("title").value();
    templateType = vreport.attribute("tpl_type").value();



   // std::cout << "Report Name: " << name << std::endl;
   // std::cout << "Template File: " << templateFile << std::endl;
    ConsoleManager::getInstance().log(L"[LOG] reportHandler::parseXmlFile: Report Name:" + str_to_wstr(name) + L" ");
    ConsoleManager::getInstance().log(L"[LOG] reportHandler::parseXmlFile: Template File:" + str_to_wstr(templateFile) + L" ");

// for (pugi::xml_node rtemplate = vreport.child("template"); rtemplate; rtemplate = rtemplate.next_sibling("template")) {
  for (pugi::xml_node rtemplate : vreport.children("template")) {

      std::string tpl_name = rtemplate.attribute("name").value();
      std::string tpl_file_path = rtemplate.attribute("file_path").value();
      
      LOG_DEBUG(L"Incerc sa incarc template:" + str_to_wstr(tpl_name) + L" path:" + str_to_wstr(tpl_file_path) + L" type:" + str_to_wstr(templateType));
      report_tpls[tpl_name] = reportTemplate(tpl_name,tpl_file_path, templateType);

      //std::cout << "Template de incarcat:"<<tpl_name << "| Cale:" << tpl_file_path << std::endl;
      ConsoleManager::getInstance().log(L"[LOG] reportHandler::parseXmlFile: Template de incarcat : "+ str_to_wstr(tpl_name) + L" | Cale : " + str_to_wstr(tpl_file_path));
 }


 //std::cout << "Am incarcata:" << report_tpls.size() << " templateuri" << std::endl;
 ConsoleManager::getInstance().log(L"[LOG] reportHandler::parseXmlFile: Am incarcat:" + to_wstring<int>(report_tpls.size())+ L" templateuri");


  for (pugi::xml_node rgvar : vreport.children("global_variable")) {

      std::string gvar_name = rgvar.attribute("name").value();
      std::wstring gvar_value = str_to_wstr(rgvar.attribute("value").value());
      std::string gvar_tpl_name = rgvar.attribute("tpl_name").value();
      std::string gvar_type = rgvar.attribute("type").value();
      std::string gvar_function = rgvar.attribute("function").value();

      reportVar gvar(gvar_name, gvar_tpl_name, gvar_value, gvar_type, gvar_function);
      addGlobalVar(gvar_name,gvar);


      //setGlobalVarValue(gvar_name, str_to_wstr( gvar_value ));

      //std::cout << "Variabila globala:"<<gvar_name << "=" << gvar_value << std::endl;
 }


 //std::cout << "Am incarcata:" << global_vars.size() << " variabile globale" << std::endl;
 ConsoleManager::getInstance().log(L"[LOG] reportHandler::parseXmlFile: Am incarcat:" + to_wstring<int>(global_vars.size()) + L" variabile globale");





    std::string connection_type;// = connection.attribute("type").value();
    std::string dsn;// = connection.attribute("dsn").value();

    pugi::xml_node connection = vreport.child("conection");

    if (connection) {
        connection_type = connection.attribute("type").value();
        dsn = connection.attribute("dsn").value();

        //std::cout << "Connection Type: " << connection_type << std::endl;
        //std::cout << "DSN: " << dsn << std::endl;
        ConsoleManager::getInstance().log(L"[LOG] reportHandler::parseXmlFile: Connection Type: " + str_to_wstr(connection_type) + L" DNS:" + str_to_wstr(dsn));
        //          &conn = new dbConnection(&connection_type,&dsn);

    } else {
        std::cerr << "Lipsește nodul <conection> în fișierul XML." << std::endl;
    }
    dbConnection *conn = nullptr;

    if(connection_type == "odbc")
        conn = new odbcConnection(connection_type,str_to_wstr(dsn));
    if (connection_type == "csv")
         conn = new csvConnection(connection_type, str_to_wstr(dsn));
    

    

    // Accesează nodul <data_source>
   // pugi::xml_node data_source = vreport.child("data_source");
 for (pugi::xml_node data_source = vreport.child("data_source"); data_source; data_source = data_source.next_sibling("data_source")) {

//  if (data_source) {
/*
        // Accesează nodul <conection>
        //        dbConnection conn(std::string(""),std::string(""));
        //

        std::string data_source_name = data_source.attribute("name").value();
        std::string data_source_type = data_source.attribute("type").value();
        std::wstring query_value = str_to_wstr(std::string(data_source.attribute("value").value()));
        std::cout << "Data Source Name: " << data_source_name << std::endl;
        std::cout << "Data Source Type: " << data_source_type << std::endl;
        std::wcout << "Query Value: " << query_value << std::endl;

        dataSource ds(data_source_name,data_source_type,query_value, conn);

        // Iterează prin <variable> pentru a extrage informațiile
        pugi::xml_node rows = data_source.child("rows");
        if (rows) {
            reportRow row;
            for (pugi::xml_node variable = rows.child("variable"); variable; variable = variable.next_sibling("variable")) {
                std::string var_name = variable.attribute("name").value();
                std::string var_tpl_name = variable.attribute("tpl_name").value();
                std::wstring var_value = str_to_wstr(variable.attribute("value").value());
                std::string var_type = variable.attribute("type").value();
                std::cout << "Variable - Name: " << var_name
                    << ", Value: " << wstr_to_str(var_value)
                    << ", Type: " << var_type << std::endl;

                reportVar var(var_name,var_tpl_name,var_value,var_type);
                row.addVariable(var);
            }
            ds.addRow(row); 
        }
        dataSources.push_back(ds);
*/
        addDataSource(parseDataSource(data_source, conn));
    } //else {
      //  std::cerr << "Lipsește nodul <data_source> în fișierul XML." << std::endl;
//    }


    
    //std::cout << result <<std::endl;
    //std::cout<<"GATA PARSAREA XML"<<std::endl;

    ConsoleManager::getInstance().log(L"[LOG] reportHandler::parseXmlFile: Loaded: " + result );
    ConsoleManager::getInstance().log(L"[LOG] reportHandler::parseXmlFile: GATA PARSAREA XML");

    doc.reset();
    return true;
}

const std::string& reportHandler::getTemplateFileName() const {
    return templateFile;
}

/*
bool reportHandler::generateReport(){

    connectAllDataSources();

    for ( auto& source : dataSources) {

        //if(source.getName() == "fsna_oper") continue;
//                std::cout << "Generez pt sursa de date:" << source.getName() << std::endl;
                
        source.getDbData();
       // report_tpls[templateFile].assign(L"$"+str_to_wstr(source.getName()), source.fetch());
        source.setGlobalVars();
        getTemplateByName(templateFile)->assign(L"$"+str_to_wstr(source.getName()), source.fetch());
        

//      report_tpls[templateFile].replace(L"$"+str_to_wstr(source.getName()), source.fetch());
    }
   

    for(const auto& g_var : global_vars){
        setGVarByGVars(g_var.second.getName());
        //getTemplateByName(templateFile)->assign(str_to_wstr("$" + g_var.first),g_var.second.getValue());
        if (g_var.second.getType() == "fnc_var") {
            //function="sum(csv_fsna.fsna.Total)"
            //reportFunction repFunc(this, g_var.second.getFunction());
            getTemplateByName(templateFile)->assign(str_to_wstr(g_var.second.getTplName()), reportFunction(this, g_var.second.getFunction(), g_var.second.getValue()).getFuncValue());
        }
        else {
            getTemplateByName(templateFile)->assign(str_to_wstr(g_var.second.getTplName()), g_var.second.getValue());
        }
    }


    return true;
}
*/


bool reportHandler::generateReport() {


    for (auto& source : dataSources) {

        //if(source.getName() == "fsna_oper") continue;
//                std::cout << "Generez pt sursa de date:" << source.getName() << std::endl;

        source.getDbData();
        // report_tpls[templateFile].assign(L"$"+str_to_wstr(source.getName()), source.fetch());
        getTemplateByName(templateFile)->assign(L"$" + str_to_wstr(source.getName()), source.fetch());


        //      report_tpls[templateFile].replace(L"$"+str_to_wstr(source.getName()), source.fetch());
    }


    //report_tpls[templateFile].printVars(true);
        //  std::cout << "XXXX:"<<
      /*
        for ( auto& source : dataSources) {
            //source.getDbData();
            //report_tpls[templateFile].replace(L"$"+str_to_wstr(source.getName()), source.fetch());
            if(source.getName() == "fsna_oper"){
                for( auto& d_row: source.getDataRows())
                    for( auto& var_r : d_row.getVars())
                std::cout<< "AAAAAAAAAAAAAAAAuuuuuuuuuu:"<<var_r.second.getName()<<" - " <<var_r.second.getTplName() << ":" << wstr_to_str(var_r.second.getValue())  <<std::endl;
            }
        }
    */

    for (const auto& g_var : global_vars) {
        getTemplateByName(templateFile)->assign(str_to_wstr("$" + g_var.first), g_var.second.getValue());
    }


    return true;
}

bool reportHandler::saveReport(const std::string& filePath) const {
    //report_tpls[templateFile].setDocumentName(name);

    if (saveFileName.size() == 0) {
        //std::cout << "VERIFIC1:" << name << std::endl;
        ConsoleManager::getInstance().log(L"[LOG] reportHandler::saveReport: VERIFIC1:" + str_to_wstr(name));
        report_tpls[templateFile].setDocumentSaveName(name);
    }
    else {
        //std::cout << "VERIFIC2:" << VERIFIC2: << std::endl;
        ConsoleManager::getInstance().log(L"[LOG] reportHandler::saveReport: VERIFIC2:" + str_to_wstr(saveFileName));
        report_tpls[templateFile].setDocumentSaveName(saveFileName);
    }
    //std::cout << "SALVEZ RAPORT:" << report_tpls[templateFile].save(getGlobalReportPath() + filePath) << std::endl;
    //return report_tpls[templateFile].save(getGlobalReportPath()+filePath);
//    std::cout << "SALVEZ RAPORT:" << report_tpls[templateFile].save(filePath) << std::endl;
    ConsoleManager::getInstance().log(L"[LOG] reportHandler::saveReport: SALVEZ RAPORT:");
    return report_tpls[templateFile].save(filePath);
}




std::wstring reportHandler::getGlobalVarValue(std::string var_name){
    return ::getGlobalVarValue(var_name);
}


void reportHandler::setGlobalVarValue(std::string var_name, std::wstring var_value){

    return ::setGlobalVarValue(var_name, var_value);
}

void reportHandler::addGlobalVar(std::string var_name, reportVar gvar) {

    global_vars[var_name] = gvar;


    //return ::setGlobalVarValue(var_name, var_value);
}

std::wstring reportHandler::fetch(){
   // AfxMessageBox("Continut:");
    return report_tpls[templateFile].fetch();
}

void reportHandler::clean() {
    clean_templates();
    report_tpls.clear();
    global_vars.clear();
    reset_report_counter();
    disconnectAllDataSources();
    for (auto& source : dataSources) {
        source.clean(); 
    }
}

void reportHandler::setSaveFileName(const std::string& nm){
    saveFileName = nm;
}

const std::string& reportHandler::getSaveFileName() const {
    return saveFileName;
}


const std::wstring& reportHandler::getDataSourceQuery(const std::string ds_name) const {
    for (auto& source : dataSources) {
        if (source.getName() == ds_name)
            return source.getQuery();
    }

    return L"";
}

const std::wstring& reportHandler::getDataSourceRunQuery(const std::string ds_name) const {
    for (auto& source : dataSources) {
        if (source.getName() == ds_name)
            return source.getQuery();
    }

    return L"";
}

void reportHandler::setReportGlobalPath(const std::string& path) {
    setGlobalReportPath(path);
}