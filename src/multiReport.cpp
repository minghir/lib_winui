#include "ui/ConsoleManager.hpp"
#include "multiReport.hpp"

multiReport::multiReport() {

}

multiReport::multiReport(const std::string& xmlDescFile) : xmlDescFile(xmlDescFile) {
    parseXmlFile();
}

void multiReport::setGlobalVarValue(std::string var_name, std::wstring var_value) {
    return ::setGlobalVarValue(var_name, var_value);
}

bool multiReport::parseXmlFile() {
    pugi::xml_document doc;
   // AfxMessageBox("Parsez multirep:" + CString(xmlDescFile.c_str()));
    // Încarcă fișierul XML
    LOG_DEBUG(L"Incep parsarea xml");
    pugi::xml_parse_result result = doc.load_file(xmlDescFile.c_str());
    if (!result) {
        LOG_ERROR(L"Eroare la încărcarea fisierului XML: " + str_to_wstr(result.description()));
        LOG_ERROR(L"Linia: "+ to_wstring<int>(result.offset) ); // Linie/offset unde s-a întâlnit problema
        return false;
    }
    pugi::xml_node vreport = doc.child("multi_report");
    if (!vreport) {
//        AfxMessageBox("Nu am gasit multirep");
        LOG_ERROR(L"Lipsește nodul <vreport> în fișierul XML.");
        return false;
    }
    name = vreport.attribute("name").value();
    templateFile = vreport.attribute("tpl_file").value();
    templateType = vreport.attribute("tpl_type").value();

   // AfxMessageBox("Multirep:" + CString(name.c_str()) );

    for (pugi::xml_node rtemplate : vreport.children("template")) {

        std::string tpl_name = rtemplate.attribute("name").value();
        std::string tpl_file_path = rtemplate.attribute("file_path").value();
        LOG_DEBUG(L"Incarc:" + str_to_wstr(tpl_name) + L"| Cale:" + str_to_wstr(tpl_file_path));
        report_tpls[tpl_name] = reportTemplate(tpl_name, tpl_file_path, templateType);
       // AfxMessageBox("Am incarcart" + CString(tpl_name.c_str()));
        LOG_DEBUG(L"Am incarcat template:" + str_to_wstr(tpl_name) + L"| Cale:" + str_to_wstr(tpl_file_path));
    }

    for (pugi::xml_node vrep : vreport.children("vreport")) {
        std::string vrep_name = vrep.attribute("name").value();
        std::string vrep_path = vrep.attribute("report_path").value();
       // AfxMessageBox("Adaug rep:" + CString(vrep_name.c_str()));
        addReport(vrep_name, vrep_path);
    }

    doc.reset();
    return true;
}

void multiReport::addReport(const std::string& rep_name, const std::string& rep_path){
    //reports.emplace(rep_name, reportHandler(rep_path));
    //reports.insert(std::make_pair(rep_name, reportHandler(rep_path)));
    //AfxMessageBox(CString(rep_path.c_str()));
    reportHandler rp(rep_path);
    reports[rep_name] = rp;
}




void multiReport::clean() {
    
    for (auto& report : reports) {
        report.second.clean();
    }
    reports.clear();
    activeReports.clear();
}

void multiReport::setSaveFileName(const std::string& nm) {
    saveFileName = nm;
}

const std::string& multiReport::getSaveFileName() const {
    return saveFileName;
}


bool multiReport::generateReport() {

    for (auto& report : reports) {
        if (std::find(activeReports.begin(), activeReports.end(), report.first) != activeReports.end()) {
            //AfxMessageBox("AICIAAAAAAAAA1"+ CString(report.first.c_str()));
            //AfxMessageBox("Continut:"+CString(templateFile.c_str()));
            report.second.generateReport();
            getTemplateByName(templateFile)->assign(L"$" + str_to_wstr(report.first), report.second.fetch());
        }
        else {
           // AfxMessageBox("AICI2");
            getTemplateByName(templateFile)->assign(L"$" + str_to_wstr(report.first), L"");
        }

        reset_report_counter();
    }

    for (const auto& g_var : global_vars) {
        getTemplateByName(templateFile)->assign(str_to_wstr("$" + g_var.first), g_var.second.getValue());
    }
//    return  reports[rep_name].generateReport();
    return true;
}

void multiReport::setActiveReport(const std::string& rep_name) {
  /*  if (reports.count(rep_name)) {
        return;
    }*/
    activeReports.push_back(rep_name);
}


bool multiReport::saveReport(const std::string& filePath) const {
    //report_tpls[templateFile].setDocumentName(name);
    if (saveFileName.size() == 0) {
        report_tpls[templateFile].setDocumentSaveName(name);
    }
    else {
        report_tpls[templateFile].setDocumentSaveName(saveFileName);
    }
    return report_tpls[templateFile].save(filePath);
}


reportHandler multiReport::getReport(const std::string& rep_name) {
    for (auto& report : reports) {
        if (report.second.getName() == rep_name)
            return report.second;
    }
    return reportHandler();
}
//#include "pugixml.hpp"
//#include "dbConnection.hpp"
//#include "stringUtils.hpp"
//#include "fileUtils.hpp"
//#include "reportTemplate.hpp"

//#include <iostream>
//#include <fstream>
//#include <sstream>
/*
// Constructor
reportHandler::reportHandler(const std::string& xmlDescFile)
    : xmlDescFile(xmlDescFile) {
        parseXmlFile();
    //    loadTemplateFile();
        connectAllDataSources();
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
    std::cout << "Adaug data sourcs:"<<source.getName()<<std::endl;
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

dataSource reportHandler::parseDataSource(const pugi::xml_node& data_source, const dbConnection& con ) {
        
    std::cout << "\nSursa de date: " << data_source.attribute("name").value() << std::endl;
    std::cout << "Tip: " << data_source.attribute("type").value() << std::endl;
    std::cout << "Query: " << data_source.attribute("value").value() << std::endl;

    std::string data_source_name = data_source.attribute("name").value();
    std::string data_source_type = data_source.attribute("type").value();
    std::wstring query_value = str_to_wstr(std::string(data_source.attribute("value").value()));

    dataSource ds(data_source_name,data_source_type,query_value, std::make_shared<dbConnection>(con));

  for (pugi::xml_node ds_variable = data_source.child("data_source_variable"); ds_variable; ds_variable = ds_variable.next_sibling("data_source_variable")) {
                std::string var_name = ds_variable.attribute("name").value();
                std::string var_tpl_name = ds_variable.attribute("tpl_name").value();
                std::wstring var_value = str_to_wstr(ds_variable.attribute("value").value());
                std::string var_type = ds_variable.attribute("type").value();
                std::string var_function = ds_variable.attribute("function").value();

                std::cout << "Data_source_Variable - Name: " << var_name
                    << ", Value: " << wstr_to_str(var_value)
                    << ", Type: " << var_type << std::endl;
                if(var_name.size()>0){
                    reportVar var(var_name,var_tpl_name,var_value,var_type,var_function);
                    ds.addDataSourceVar(var);
                }
                std::cout<<"Adaug variabila data sursa"<<std::endl;
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

                std::cout << "Variable - Name: " << var_name
                    << ", Value: " << wstr_to_str(var_value)
                    << ", Type: " << var_type << std::endl;

                reportVar var(var_name,var_tpl_name,var_value,var_type,var_function);
                row.addVariable(var);
             }

                for (pugi::xml_node nested_ds : data_source.child("rows").children("data_source")) {
                  
                   dataSource n_ds = parseDataSource(nested_ds,con);
                   row.addDataSource( std::make_shared<dataSource>(n_ds));

                   //addDataSource(n_ds);

                    //row.addDataSource( std::make_shared<dataSource>(parseDataSource(nested_ds, con)));

                }


            
            ds.addRow(row); 

    }
 
    // Parcurgem variabilele
    for (pugi::xml_node row : data_source.child("rows").children("variable")) {
        std::cout << "Variabilă: " << row.attribute("name").value() 
            << " - Template: " << row.attribute("tpl_name").value() 
            << " - Valoare: " << row.attribute("value").value() 
            << " - Tip: " << row.attribute("type").value() << std::endl;
    }

//    dataSource ds;
  
    // Verificăm dacă există un nou `data_source` imbricat și îl procesăm recursiv
//    for (pugi::xml_node nested_ds : data_source.child("rows").children("data_source")) {
//        parseDataSource(nested_ds, con);
//    }
   return ds;
}

bool reportHandler::parseXmlFile(){
    pugi::xml_document doc;
    std::cout<<xmlDescFile<<std::endl;

    // Încarcă fișierul XML
    pugi::xml_parse_result result = doc.load_file(xmlDescFile.c_str());

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



    std::cout << "Report Name: " << name << std::endl;
    std::cout << "Template File: " << templateFile << std::endl;


// for (pugi::xml_node rtemplate = vreport.child("template"); rtemplate; rtemplate = rtemplate.next_sibling("template")) {
  for (pugi::xml_node rtemplate : vreport.children("template")) {

      std::string tpl_name = rtemplate.attribute("name").value();
      std::string tpl_file_path = rtemplate.attribute("file_path").value();

      report_tpls[tpl_name] = reportTemplate(tpl_name,tpl_file_path, templateType);

      std::cout << "Template de incarcat:"<<tpl_name << "| Cale:" << tpl_file_path << std::endl;
 }


 std::cout << "Am incarcata:" << report_tpls.size() << " templateuri" << std::endl;


  for (pugi::xml_node rgvar : vreport.children("global_variable")) {

      std::string gvar_name = rgvar.attribute("name").value();
      std::string gvar_value = rgvar.attribute("value").value();

      setGlobalVarValue(gvar_name, str_to_wstr( gvar_value ));

      std::cout << "Variabila globala:"<<gvar_name << "=" << gvar_value << std::endl;
 }


 std::cout << "Am incarcata:" << global_vars.size() << " variabile globale" << std::endl;






    std::string connection_type;// = connection.attribute("type").value();
    std::string dsn;// = connection.attribute("dsn").value();

    pugi::xml_node connection = vreport.child("conection");

    if (connection) {
        connection_type = connection.attribute("type").value();
        dsn = connection.attribute("dsn").value();
        std::cout << "Connection Type: " << connection_type << std::endl;
        std::cout << "DSN: " << dsn << std::endl;

        //          &conn = new dbConnection(&connection_type,&dsn);

    } else {
        std::cerr << "Lipsește nodul <conection> în fișierul XML." << std::endl;
    }
    dbConnection conn(connection_type,str_to_wstr(dsn));
    

    

    // Accesează nodul <data_source>
   // pugi::xml_node data_source = vreport.child("data_source");
 for (pugi::xml_node data_source = vreport.child("data_source"); data_source; data_source = data_source.next_sibling("data_source")) {

//  if (data_source) {

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

        addDataSource(parseDataSource(data_source, conn));
    } //else {
      //  std::cerr << "Lipsește nodul <data_source> în fișierul XML." << std::endl;
//    }



    std::cout << result <<std::endl;
    std::cout<<"GATA PARSAREA XML"<<std::endl;
    doc.reset();
    return true;
}

const std::string& reportHandler::getTemplateFileName() const {
    return templateFile;
}


bool reportHandler::generateReport(){


    for ( auto& source : dataSources) {

        //if(source.getName() == "fsna_oper") continue;
//                std::cout << "Generez pt sursa de date:" << source.getName() << std::endl;
                
        source.getDbData();
       // report_tpls[templateFile].assign(L"$"+str_to_wstr(source.getName()), source.fetch());
        getTemplateByName(templateFile)->assign(L"$"+str_to_wstr(source.getName()), source.fetch());


//      report_tpls[templateFile].replace(L"$"+str_to_wstr(source.getName()), source.fetch());
    }
   

//report_tpls[templateFile].printVars(true);
    //  std::cout << "XXXX:"<<
  
    for ( auto& source : dataSources) {
        //source.getDbData();
        //report_tpls[templateFile].replace(L"$"+str_to_wstr(source.getName()), source.fetch());
        if(source.getName() == "fsna_oper"){
            for( auto& d_row: source.getDataRows())
                for( auto& var_r : d_row.getVars())
            std::cout<< "AAAAAAAAAAAAAAAAuuuuuuuuuu:"<<var_r.second.getName()<<" - " <<var_r.second.getTplName() << ":" << wstr_to_str(var_r.second.getValue())  <<std::endl;
        }
    }

    for(const auto& g_var : global_vars){
         getTemplateByName(templateFile)->assign(L"$"+str_to_wstr(g_var.first),g_var.second);
    }


    return true;
}

bool reportHandler::saveReport(const std::string& filePath) const {
    //report_tpls[templateFile].setDocumentName(name);
    if (saveFileName.size() == 0) {
        report_tpls[templateFile].setDocumentSaveName(name);
    }
    else {
        report_tpls[templateFile].setDocumentSaveName(saveFileName);
    }
    return report_tpls[templateFile].save(filePath);
}


std::wstring reportHandler::getGlobalVarValue(std::string var_name){
    return ::getGlobalVarValue(var_name);
}


void reportHandler::setGlobalVarValue(std::string var_name, std::wstring var_value){

    return ::setGlobalVarValue(var_name, var_value);
}

std::wstring reportHandler::fetch(){
    return report_tpls[templateFile].fetch();
}

void reportHandler::clean() {
    clean_templates();
    report_tpls.clear();
    global_vars.clear();
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
*/
