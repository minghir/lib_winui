#include "globals.hpp"
#include "stringUtils.hpp"
//#include <afxwin.h>
std::map<std::string,reportTemplate> report_tpls;
//std::map<std::string,std::wstring> global_vars;
std::map<std::string, reportVar> global_vars;
std::string _global_report_path = "";
int _report_counter = 0;
//int _rows_counter = 0;
std::map<std::string, int> _rows_counter;


std::shared_ptr<reportHandler> _current_report;
void setCurrentReport(std::shared_ptr<reportHandler> _cur_report) {
    _current_report = _cur_report;
}

std::shared_ptr<reportHandler> getCurrentReport() {
    return _current_report;
}



void clean_templates() {
    for (auto& tpls : report_tpls) {
        tpls.second.clean();
    }
}

reportTemplate* getTemplateByName(std::string tpl_name){
    //AfxMessageBox(CString("caut:")+CString(tpl_name.c_str()));
    auto it = report_tpls.find(tpl_name);
    if (it != report_tpls.end()) {
//        std::cout<<"AM GASIT TEMPLATEUL:" << tpl_name << " SUCCES!!!" <<std::endl;
        return &(it->second);  // Returnează pointer către obiectul găsit
    }else{
        std::cout << "TEMPLATEUL:" << tpl_name << " NU EXISTA!"<<std::endl;
    }
    return nullptr;
}


std::wstring getGlobalVarValue(std::string var_name){
    if (global_vars.find(var_name) != global_vars.end()) {
        return global_vars[var_name].getValue();
    }else{
        return L"";
    }
}


void setGlobalVarValue(std::string var_name, std::wstring var_value){

    if(var_name.size() == 0) return;

    global_vars[var_name].setValue( var_value );
}


std::wstring report_counter() {
    return to_wstring<int>(++_report_counter);
}

void reset_report_counter() {
    _report_counter = 0;
}

std::wstring rows_counter(std::string str) {
    return to_wstring<int>(++_rows_counter[str]);
}

void reset_rows_counter(std::string str) {
    _rows_counter[str] = 0;
}


void setGlobalReportPath(std::string path) {
    _global_report_path = path;
}

std::string getGlobalReportPath() {
    return _global_report_path;
}

void setGVarByGVars(std::string gvar_name) {
    for (const auto& g_var : global_vars) {
        if (g_var.second.getName() == gvar_name) {
            continue; //nu fac cum ea insasi in spatele ei fiind ea insasi
        }
        else {
            global_vars[gvar_name].setVarValue(rpl_wstr_in_wstr(global_vars[gvar_name].getValue(), str_to_wstr(g_var.second.getTplName()), g_var.second.getValue()));
        }
    }
}
