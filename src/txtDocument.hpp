#ifndef TXTDOCUMENT_HPP
#define TXTDOCUMENT_HPP

#include "reportDocument.hpp"
#include "stringUtils.hpp"


class txtDocument : public reportDocument{

    private:
        std::string document_name;
        std::wstring content;

    public: 
        std::string getDocumentName() override;
        std::string getDocumentType() override;

        bool newDoc(std::string name) override;
        void print() override;
        bool open(const std::string file_name) override;
        bool save(const std::string save_path=".") override;

        void setContent(const std::wstring text) override {
            content = text; 
        }

        virtual std::wstring getContent() override {
            return content;
        };

        void replace(std::wstring str_to_replace, std::wstring str_with_replace) override {
            content = rpl_wstr_in_wstr(content, str_to_replace, str_with_replace);
        }

        reportDocument* clone() const override {
            return new txtDocument(*this);  // Copiere profundă
        }

        void setDocumentName(const std::string name) override { document_name = name; }

};

#endif


