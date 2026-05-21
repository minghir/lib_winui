#ifndef REPORTTEMPLATE_HPP
#define REPORTTEMPLATE_HPP

#include <string>
#include <map>
#include "reportDocument.hpp"

class reportTemplate{

    private:
        std::string name;
        std::string file_path;
        std::string tpl_type; 

        reportDocument *tpl;
        reportDocument *tpl_parsed;

        std::string saveFileName;

        std::map<std::wstring, std::wstring> vars;

    public:
        reportTemplate() = default;

        reportTemplate(const std::string& name, const std::string& file_path, const std::string& tpl_type);

        ~reportTemplate();

        std::string getName() const { return name; }
        std::string getFilePath() const { return file_path; }
        //reportDocument getTemplate() const { return tpl; }

        void setName(const std::string& newName) { name = newName; }
        void setFilePath(const std::string& newFilePath) { file_path = newFilePath; }
//      void setTemplate(const reportDocument& newTpl) { tpl = newTpl; }
        void loadTplFile();
        
        void replace(std::wstring str_to_replace, std::wstring str_with_replace);
        bool save(const std::string& filePath)  ;
        std::wstring getContent();

        void setDocumentName(std::string doc_name);

        void assign(std::wstring var_name,std::wstring var_value);

        

        std::wstring fetch();

        void parseVars() ;

        void printVars(bool value);

        void clean();

      
        
        void setDocumentSaveName(const std::string& nm);

        const std::string& getDocumentSaveName() const;


};
#endif
