#ifndef XHTMLDOCUMENT_HPP
#define XHTMLDOCUMENT_HPP

#include "reportDocument.hpp"


class xhtmlDocument : public reportDocument {

private:
    std::string document_name;
    std::wstring content;

public:
    std::string getDocumentName() override;
    std::string getDocumentType() override;

    bool newDoc(std::string name) override;
    void print() override;
    bool open(const std::string file_name) override;
    bool save(const std::string save_path = ".") override;

      

    void setContent(const std::wstring text) override;
    std::wstring getContent() override;

    void replace(std::wstring str_to_replace, std::wstring str_with_replace) override;
    reportDocument* clone() const override {
        return new xhtmlDocument(*this);  // Copiere profundă
    }
    void setDocumentName(const std::string name) override;

};

#endif