#ifndef REPORTDOCUMENT_HPP
#define REPORTDOCUMENT_HPP

#include <string>
#include <vector>
#include <iostream>

class reportDocument{
    private:
    public:
        
        virtual ~reportDocument() {
      //      std::cout << "reportDoc desc" <<std::endl;
        }/// aici e ceva aiurea
       
        virtual std::string getDocumentName() = 0;
        virtual std::string getDocumentType() = 0;
        virtual void print() = 0;
        virtual bool newDoc(const std::string name) = 0;
        virtual bool open(const std::string file_name) = 0;
        virtual bool save(const std::string save_path=".") = 0;
        //virtual void addParagraf(const std::wstring paragraf) = 0;
        //virtual void setCurrentStyle(const std::wstring style) = 0;
        //virtual void addTitleRow(std::vector<std::wstring> cells) = 0;
        //virtual void addRow(std::vector<std::wstring> cells) = 0;
        //virtual void addText(const std::wstring text) = 0;
        //virtual void setPageType(const std::wstring pg_type) = 0;
        //virtual void setPageOrientation(const std::wstring pg_orient) = 0;

        virtual void setContent(const std::wstring text) = 0;
        virtual std::wstring getContent() = 0;

        virtual void replace(std::wstring str_to_replace, std::wstring str_with_replace) = 0;
        virtual reportDocument* clone() const = 0;
        virtual void setDocumentName(const std::string name) = 0;

        

};


#endif
