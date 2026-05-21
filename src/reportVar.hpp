#ifndef REPORTVAR_HPP
#define REPORTVAR_HPP

#include <string>
#include <iostream>

class  reportVar{
private:
    std::string name;  // Numele variabilei (de ex. $fpl_id)
	std::string tpl_name;  // Numele variabilei (de ex. $fpl_id)
    std::wstring value; // Valoarea variabilei (de ex. "1234")
    std::string type;  // Tipul variabilei (app_var, row_var, row_field)
	std::wstring data;  // Tipul variabilei (app_var, row_var, row_field)
    std::string function; // optional
                          //
    bool data_var = false;

public:
    // Constructor
    reportVar() = default;
    reportVar(const reportVar& cp);
    reportVar(const std::string& name, const std::string& tpl_name, const std::wstring& value, const std::string& type, const std::string& function);

    reportVar& operator=(const reportVar& cp); 
    
    // Getters
    const std::string& getName() const;
    const std::string& getTplName() const;

    const std::wstring& getValue() const;
    const std::string& getType() const;
    const std::string& getFunction() const;

    // Setters
    void setValue(const std::wstring& newValue);

    // Helper methods
    void print() const;
    bool isAppVar() const;
    bool isRowVar() const;
    bool isTxtVar() const;
    bool isFunctionVar() const;

    void setDataVar(){ data_var = true; }
    void unsetDataVar(){ data_var = false; }
    bool isDataVar() { return data_var;}

    void setVarValue(const std::wstring val) { value = val; }

};

#endif // REPORTVAR_HPP
