#ifndef REPORTHANDLER_HPP
#define REPORTHANDLER_HPP


#include "pugixml.hpp"
#include "dataSource.hpp"

//#include "reportDocument.hpp"
#include "globals.hpp"
#include "stringUtils.hpp"

#include <string>
#include <vector>


class reportHandler {
private:
    std::string xmlDescFile;
    std::string name;                      // Numele raportului
    std::string title;
    std::string templateFile;              // Fișierul template asociat (ex. exemplu.rtf)
    std::vector<dataSource> dataSources;   // Lista cu sursele de date
    std::wstring templateFileContent;	//Continutul fisierului template
    std::string templateType;
    std::wstring generatedReport;
    std::string saveFileName;
    //std::string pathToReport;

    //reportDocument* tpl;    

bool parseXmlFile();
dataSource parseDataSource(const pugi::xml_node& data_source, dbConnection *con);

public:
    // Constructor
    reportHandler();

    reportHandler(const std::string& xmlDescFile);


    // Metode de setare și accesare
    const std::string& getName() const;
    const std::string& getTemplateFile() const;

    // Gestionarea surselor de date
    void addDataSource(const dataSource& source);
    const std::vector<dataSource>& getDataSources() const;
    void connectAllDataSources(); // Conectează toate sursele de date
    void disconnectAllDataSources(); // Conectează toate sursele de date
//    bool loadTemplateFile();

    // Afișarea informațiilor despre raport
    void printReportInfo() ;
    const std::string& getTemplateFileName() const;
    bool generateReport();
    bool saveReport(const std::string& filePath) const;

    std::wstring getGlobalVarValue(std::string var_name);
    void setGlobalVarValue(std::string var_name, std::wstring var_value);
    void addGlobalVar(std::string var_name, reportVar gvar);

    std::wstring fetch();

    void clean();
    void setSaveFileName(const std::string& nm);
    const std::string& getSaveFileName() const;

    const std::wstring& getDataSourceQuery(const std::string ds_name) const;
    const std::wstring& getDataSourceRunQuery(const std::string ds_name) const;
    void setReportGlobalPath(const std::string& path);


};

#endif // REPORTHANDLER_HPP
