#ifndef MULTIREPORT_HPP
#define MULTIREPORT_HPP

#include "reportHandler.hpp"
#include <vector>


class multiReport {
private :
        
        std::string xmlDescFile;
        std::string templateFile;
        std::string templateType;
        std::string saveFileName;
        std::string name;
        std::map<std::string, reportHandler> reports;
        std::vector<std::string> activeReports;

        bool parseXmlFile();
public:
    multiReport();
    multiReport(const std::string& xmlDescFile);
    void setGlobalVarValue(std::string var_name, std::wstring var_value);
    void setSaveFileName(const std::string& nm);
    const std::string& getSaveFileName() const;
    void addReport(const std::string& rep_name, const std::string& rep_path);
    bool generateReport();
    void setActiveReport(const std::string& rep_name);
    reportHandler getReport(const std::string& rep_name);
    void clean();
    bool saveReport(const std::string& filePath) const;

};

#endif // MULTIREPORT_HPP
