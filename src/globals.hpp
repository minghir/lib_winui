#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include "reportTemplate.hpp"
#include "reportVar.hpp"
//#include "reportHandler.hpp"

#include <map>
#include <string>
#include <memory>


extern std::map<std::string,reportTemplate> report_tpls;
extern std::map<std::string, reportVar> global_vars;
extern int _report_counter;
extern std::map<std::string, int> _rows_counter;
extern std::map<std::wstring, std::wstring> cfg_file_vars;

class reportHandler;
extern std::shared_ptr<reportHandler> _current_report;

void setCurrentReport(std::shared_ptr<reportHandler> _cur_report);
std::shared_ptr<reportHandler> getCurrentReport();


reportTemplate* getTemplateByName(std::string tpl_name);

std::wstring getGlobalVarValue(std::string var_name);

void setGlobalVarValue(std::string var_name, std::wstring var_value);

void clean_templates();

std::wstring report_counter();
void reset_report_counter();

void setGlobalReportPath(std::string path);
std::string getGlobalReportPath();
void setGVarByGVars(std::string gvar_name);


std::wstring rows_counter(std::string str = "default");
void reset_rows_counter(std::string str = "default");

#endif
