#include "reportTemplate.hpp"
#include "rtfDocument.hpp"
#include "xhtmlDocument.hpp"
#include "fileUtils.hpp"
#include "stringUtils.hpp"
#include "globals.hpp"
#include "ui\\ConsoleManager.hpp" // Adjust the path
#include <iostream>
//#include <afxwin.h>


reportTemplate::reportTemplate(const std::string& name, const std::string& file_path, const std::string& tpl_type)
            : name(name), file_path(file_path), tpl_type(tpl_type) {
                 if(tpl_type == "rtf"){
                        tpl = new rtfDocument();
                        tpl_parsed = new rtfDocument();
                         
                 }
                 else if(tpl_type == "xhtml"){
                     tpl = new xhtmlDocument();
                     tpl_parsed = new xhtmlDocument();
                 }

                loadTplFile();
         }

reportTemplate::~reportTemplate() {
    //if(tpl)  delete tpl;
    //if(tpl_parsed) delete tpl_parsed;
}

void reportTemplate::clean() {
    delete tpl;
    delete tpl_parsed;
    vars.clear();
}

void reportTemplate::loadTplFile() {
    /*
    if(tpl_type == "rtf"){
        tpl = new rtfDocument();
        tpl->addText(wstr_read_RTF_file(file_path));
        //tpl->addParagraf(wstr_read_RTF_file(tpl_file));
    }
    */

    //tpl->addText(wstr_read_RTF_file(file_path));
    tpl->setContent(wstr_read_RTF_file(file_path));

}

void reportTemplate::replace(std::wstring str_to_replace, std::wstring str_with_replace){


    //std::wcout << L"Inlocuiesc in:" << str_to_wstr(file_path) << L":"<<str_to_replace << "-" << str_with_replace << std::endl;

    tpl->replace(str_to_replace, str_with_replace);
}


bool reportTemplate::save(const std::string& filePath)  {
  //  std::cout << "Salvez cu numele:" << name << std::endl;

    ConsoleManager::getInstance().log(L"[LOG] reportTemplate::save: Salvez cu numele"+ str_to_wstr(filePath));


    parseVars();   
    //tpl_parsed->setDocumentName(name);
    tpl_parsed->setDocumentName(saveFileName);
    return tpl_parsed->save(filePath);
}

std::wstring reportTemplate::getContent(){
    return tpl->getContent();
}

void reportTemplate::setDocumentName(std::string doc_name){
        tpl->setDocumentName(doc_name);

}

void reportTemplate::assign(std::wstring var_name, std::wstring var_value){
    if(var_name.size()==0) return;

  //  if(var_value.size() == 0) return;

//    if(var_name == L"$fsna_oper_total"){

//        std::wcout<<"Asignez la tpl:"<<str_to_wstr(name) <<" var:" << var_name << ":" << var_value <<std::endl;
//    }
    
    if (var_name.empty()) {
        //AfxMessageBox("Eroare: var_name este gol!");
        return;
    }

    // Asigură-te că mapa este validă înainte de acces
   //AfxMessageBox(CString(var_name.c_str()));
    if (vars.find(var_name) == vars.end()) {
        //AfxMessageBox("Noua cheie adăugată: ");
    }
    vars[var_name] = var_value;
} 


std::wstring reportTemplate::fetch() {
    parseVars(); 
    return tpl_parsed->getContent();
}


void reportTemplate::parseVars()  {
//std::cout <<"AICICIC:"<<report_tpls.size() << std::endl;
//std::cout << "CRAP la :" << name << std::endl;
 tpl_parsed = tpl->clone();

    for(auto var : vars){
        tpl_parsed->replace(var.first, stripQuotes(var.second));
    }

}

void reportTemplate::printVars(bool value){
    if(value){
        for(auto var : vars){
            std::wcout<<var.first << " :" << var.second << std::endl;
        }
    }else{
         for(auto var : vars){
            std::wcout<<var.first << " :" << "--"<< std::endl;


        }   

    }
}

void reportTemplate::setDocumentSaveName(const std::string& nm) {
    ConsoleManager::getInstance().log(L"[LOG] reportTemplate->setDocumentSaveName : setez save document:" + str_to_wstr(nm));
    saveFileName = nm;
}

const std::string& reportTemplate::getDocumentSaveName() const {
    return saveFileName;
}
