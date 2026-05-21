#ifndef RTFDOCUMENT_HPP
#define RTFDOCUMENT_HPP

#include "reportDocument.hpp"


class rtfDocument : public reportDocument{

    private:
        std::string document_name;
        std::wstring content;
        std::wstring current_style;
        std::wstring page_height = L"\\paperh16840";
        std::wstring page_width = L"\\paperw11907";
        std::wstring page_orientation = L"\\sectd";


        std::wstring getFontTable();


    public: 
        ~rtfDocument() override {
            //std::cout<<"Desc rtfDoc"<<std::endl;
        };
        std::string getDocumentName() override;
        std::string getDocumentType() override;

        bool newDoc(std::string name) override;
        void print() override;
        bool open(const std::string file_name) override;
        bool save(const std::string save_path=".") override;


        
        void addParagraf(const std::wstring paragraf);// override;
        void setCurrentStyle(const std::wstring style);// override;
        void addTitleRow(std::vector<std::wstring> cells);// override;
        void addRow(std::vector<std::wstring> cells);// override;
        void setContent(const std::wstring text) override;


        void setPageType(const std::wstring pg_type);// override;
        void setPageOrientation(const std::wstring pg_orient);// override;
        std::wstring getContent() override;

        void replace(std::wstring str_to_replace, std::wstring str_with_replace) override;

        reportDocument* clone() const override {
            return new rtfDocument(*this);  // Copiere profundă
        }

        void setDocumentName(const std::string name) override;



};

#endif


