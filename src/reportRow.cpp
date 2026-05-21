#include "reportRow.hpp"
#include "dataSource.hpp"
//#include "fileUtils.hpp"
#include "stringUtils.hpp"
#include "globals.hpp"
#include "reportFunction.hpp"

#include <regex>

reportRow::reportRow(){
} 

reportRow::reportRow(const reportRow& other) 
    : name(other.name), 
      tpl_type(other.tpl_type), 
      tpl_file(other.tpl_file), 
      variables(other.variables) { 

    // Copierea shared_ptr (se va incrementa contorul de referință)
    data_source = other.data_source;

 /*   if (other.tpl) {
        tpl = other.tpl->clone();
    } else {
        tpl = nullptr;
    }   
*/

    //tpl = other.tpl ? other.tpl->clone() : nullptr;


}


reportRow& reportRow::operator=(const reportRow& other) {  // Trebuie să fie membru al clasei
    if (this != &other) {  // Evită auto-atribuirea
        name = other.name;
        tpl_type = other.tpl_type;
        tpl_file = other.tpl_file;
        variables = other.variables;

        // Copierea shared_ptr
        data_source = other.data_source;

        // Șterge obiectul existent și copiază noul `reportDocument`
        /*
        if(tpl){
           //std::cout<<"incerc delete tpl"<<std::endl;
            
           delete tpl;
           tpl = nullptr; 
          // std::cout<<"ttttttttttttttttttttttt"<<std::endl;
           tpl = other.tpl ? other.tpl->clone() : nullptr;
           //
        }
        */
    }
    return *this;
}

void reportRow::print() {

  

    std::cout<<"Nume rows:"<<name<<"("<<tpl_type<<"):"<<tpl_file<<std::endl;
    for(const auto& var : variables){
            var.second.print();
    }
    if(data_source)    data_source->print();


    std::wcout << fetch() <<std::endl;

}


void reportRow::addVariable(const reportVar& var) {
//  variables[var.getName()] = reportVar(var);

variables.insert({var.getName(), var});

}

reportVar reportRow::getVarByName(const std::string& var_name) {
    //std::cout << "CAUT VARIABILAAAAA:" << var_name << std::endl;
    auto it = variables.find(var_name);
    if (it != variables.end()) {
        return it->second;  // Returnează obiectul găsit
    }
    throw std::runtime_error("Variabila nu a fost găsită!");  // Aruncă o excepție dacă nu există
}

std::map<std::string,reportVar> reportRow::getVars() const {
    return variables;
}

void reportRow::addDataSource(const std::shared_ptr<dataSource>& ds){
    data_source = ds;
//     data_source = std::make_shared<dataSource>(ds); 
}

void reportRow::setName(const std::string& n){
    name = n;
}

void reportRow::setTplType(const std::string& t){
    tpl_type = t;

}

void reportRow::setTplFile(const std::string& f){
    tpl_file = f;
  //  loadTplFile();
}

std::shared_ptr<dataSource> reportRow::getDataSource() const{
    return data_source;
}
/*
void reportRow::loadTplFile() {
    if(tpl_type == "rtf"){
        tpl = new rtfDocument();
        tpl->addText(wstr_read_RTF_file(tpl_file));
        //tpl->addParagraf(wstr_read_RTF_file(tpl_file));
    }
}
*/

void reportRow::setGlobalVars() {
    for (auto gvar : global_vars) {
        if (gvar.second.getType() != "ds_var") continue;

        for (auto var : variables) {
            if (var.first == gvar.first) {
                //std::wcout << L"SETEZ:" << str_to_wstr(gvar.first) << L" = " << var.second.getValue() << std::endl;
                gvar.second.setValue(var.second.getValue());
                setGlobalVarValue(gvar.first, var.second.getValue());
            }
        }
    }
}


 
std::wstring reportRow::fetch() {
    
//print();
   for( auto& var : variables){

       if(var.second.getType() == "row_var"){
            getTemplateByName(tpl_file)->assign(str_to_wstr(var.second.getTplName()),var.second.getValue());
       }

       if(data_source){
           data_source->setDataSourceVarVal(var.first,var.second.getValue());
       }
   }

  if(data_source){
      data_source->getDbData();
      getTemplateByName(tpl_file)->assign(L"$"+str_to_wstr(data_source->getName()), data_source->fetch()); 
  }

  for( auto& var : variables){
      //std::cout << "SUNNNNNNNNNNNNNNNNN AIIIIIIICI!" <<var.first << "|" << var.second.getType() << std::endl;
       if(var.second.getType() == "fnc_var"){
           //std::cout << "Am gasit func_var!!!!!!!!!!!!" << var.first <<std::endl;
           if (var.second.getFunction() == "report_counter()") {
               getTemplateByName(tpl_file)->assign(str_to_wstr(var.second.getTplName()), report_counter());
               continue;
           }

           if (var.second.getFunction() == "rows_counter()") {
               getTemplateByName(tpl_file)->assign(str_to_wstr(var.second.getTplName()), rows_counter(getName()));
               continue;
           }
           if (strExtractPrefix(var.second.getFunction()) == "page_counter(") {
               getTemplateByName(tpl_file)->assign(str_to_wstr(var.second.getTplName()), to_wstring_precise((double)_rows_counter["fsna"] , 0));
               continue;
           }
           
           /*
           if (strExtractPrefix(var.second.getFunction()) == "fsum(") {
               std::cout << "CALCULEZ FSUM" << std::endl;
               reportFunction rep_func(getCurrentReport().get(), var.second.getFunction(), var.second.getValue());
               getTemplateByName(tpl_file)->assign(str_to_wstr(var.second.getTplName()), rep_func.getFuncValue());
               //getTemplateByName(tpl_file)->assign(str_to_wstr(var.second.getTplName()), L"1111.00");
               continue;
           }
           */
           //std::map<std::string,reportVar> vars_to_count; 
           std::vector<reportRow> rows_to_count;
           int i = 0;

            if(data_source){
                //data_source->setDataSourceVarVal(var.first,var.second.getValue());
                rows_to_count = data_source->getDataRows();
                for(auto& crow :rows_to_count){
                     std::wstring value_to_count = crow.getVarByName(extractParameter(var.second.getFunction())).getValue(); 
                     if (!is_plain_decimal(value_to_count)) {
                         value_to_count = normalize_number(value_to_count, 2);
                     }
                        if(i == 0){
                            var.second.setValue( value_to_count );
                        }else{
                            long double old_value = from_wstring<long double>(var.second.getValue());
                            long double new_value = from_wstring<long double>(value_to_count) + old_value;
                            //std::wcout<<"ADUN :"<< value_to_count << "LA: "<< old_value  << "SI OBTIN:" << new_value << std::endl;
                            std::wstring n_val = to_wstring_precise(new_value, 2);
                            if (!is_plain_decimal(n_val)) {
                                n_val = normalize_number(n_val, 2);
                            }

                            var.second.setValue(n_val);
                            // var.second.setValue( value_to_count );

                        }

                        i++;
                }
            }else {
                std::cout << "Fac agregare pe curent dataset!!!!!!!!!!!!" << std::endl;
            }

            getTemplateByName(tpl_file)->assign(str_to_wstr(var.second.getTplName()),var.second.getValue());
       }
   }


 for(const auto& g_var : global_vars){
         getTemplateByName(tpl_file)->assign(L"$"+str_to_wstr(g_var.first),g_var.second.getValue());
    }


 //  return report_tpls[tpl_file].getContent();
 //setGlobalVars();
return getTemplateByName(tpl_file)->fetch();
   
}

bool reportRow::setVarValue(std::string var_name, std::wstring var_value){

// if (variables.find(var_name) != variables.end()) return false;


// std::wcout<<"---------------------Setez:"<<str_to_wstr(var_name)<<"="<<var_value<<std::endl;
    variables[var_name].setValue(var_value);

    return true;
}



std::string reportRow::extractParameter(const std::string& expression) {
    std::regex pattern(R"(sum\(([\w\.]+)\))");  // Caută `sum(...)`
    std::smatch match;

    if (std::regex_search(expression, match, pattern)) {
        std::string fullArgument = match[1];  // Ex: "detalii_operator.mtow"

        // Extrage ultimul element după `.`
        size_t pos = fullArgument.find_last_of('.');
        if (pos != std::string::npos) {
            return fullArgument.substr(pos + 1);  // Returnează "mtow"
        }
        return fullArgument;  // Dacă nu există `.`, returnează direct argumentul
    }

    throw std::invalid_argument("Expresia nu este validă!");
}


std::string reportRow::getName(){
    return name;
}

void reportRow::clean() {
    //data_source.clear();
    data_source->clean();
    variables.clear();
}