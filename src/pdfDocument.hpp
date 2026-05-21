#ifndef PDFDOCUMENT_HPP
#define PDFDOCUMENT_HPP

#include "reportDocument.hpp"


class pdfDocument : public reportDocument {

private:
    std::string document_name;
    std::wstring content;


public:
    
    std::string getDocumentName() override;
    std::string getDocumentType() override;

    bool newDoc(const std::string name) override;
    void print() override;
    bool open(const std::string file_name) override;
    bool save(const std::string save_path = ".") override;



    
    ~pdfDocument() {
        //      std::cout << "reportDoc desc" <<std::endl;
    }/// aici e ceva aiurea
   
    
    //void addParagraf(const std::wstring paragraf) override {};
    //void setCurrentStyle(const std::wstring style) override {};

    //void addTitleRow(std::vector<std::wstring> cells) override {};
    //void addRow(std::vector<std::wstring> cells) override {};
    void setContent(const std::wstring text) override {};

    //void setPageType(const std::wstring pg_type) override{};
    //void setPageOrientation(const std::wstring pg_orient) override {};

    std::wstring getContent() override { return content; };

    void replace(std::wstring str_to_replace, std::wstring str_with_replace) override {};

    reportDocument* clone() const override {
        return new pdfDocument(*this);  // Copiere profundă
    }

    void setDocumentName(const std::string name) override {};


    void printTable();

};

#endif