#ifndef REPORTROW_HPP
#define REPORTROW_HPP


#include <vector>
#include <map>
#include <memory>

//#include "dataSource.hpp"
#include "reportVar.hpp"
#include "rtfDocument.hpp"


class dataSource;

class reportRow{
    private:
        //variables
//      std::vector<reportVar> variables;  // Lista variabilelor
        std::string name;
        std::string tpl_type;
        std::string tpl_file;

        std::shared_ptr<dataSource> data_source;      
        std::map<std::string,reportVar> variables;

        //reportDocument* tpl;

  

    public:
       reportRow();
       reportRow(const reportRow& other);
       void print();
       void addVariable(const reportVar& var);
       //..portVar getVarByName(const std::string var_name);
       reportVar getVarByName(const std::string& var_name);
       std::map<std::string,reportVar> getVars() const;
      // void addDataSource(const dataSource& ds);
      void addDataSource(const std::shared_ptr<dataSource>& ds);


      void setName(const std::string& n);
      void setTplType(const std::string& t);
      void setTplFile(const std::string& f);

      std::shared_ptr<dataSource> getDataSource() const;


      //void loadTplFile();

     bool setVarValue(std::string var_name, std::wstring var_value);

      std::wstring fetch();

    reportRow& operator=(const reportRow& other); 


    std::string extractParameter(const std::string& expression);

    std::string getName();
    void clean();

    void setGlobalVars();
    

};


#endif
