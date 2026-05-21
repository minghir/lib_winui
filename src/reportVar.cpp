#include "stringUtils.hpp"
#include "reportVar.hpp"

// Constructor
reportVar::reportVar(const std::string& name, const std::string& tpl_name, const std::wstring& value, const std::string& type, const std::string& function)
    : name(name), tpl_name(tpl_name), value(value), type(type), function(function) {}
    
reportVar::reportVar(const reportVar& cp){
    this->name = cp.name;
    this->tpl_name = cp.tpl_name;
    this->value = cp.value;
    this->type = cp.type;
    this->function = cp.function;

}


// Getters
const std::string& reportVar::getName() const {
    return name;
}

const std::string& reportVar::getTplName() const {
    return tpl_name;
}


const std::wstring& reportVar::getValue() const {
    return value;
}

const std::string& reportVar::getType() const {
    return type;
}

const std::string& reportVar::getFunction() const {
    return function;
}


// Setters
void reportVar::setValue(const std::wstring& newValue) {
    value = newValue;
}

// Helper methods
void reportVar::print() const {
    std::cout << name << " => " << wstr_to_str(value) << " (" << type << ")" << std::endl;
}

bool reportVar::isAppVar() const {
    return type == "app_var";
}

bool reportVar::isRowVar() const {
    return type == "row_var" || type == "row_field";
}

bool reportVar::isTxtVar() const {
    return type == "txt_var";// || type == "row_field";
}

bool reportVar::isFunctionVar() const {
    return type == "fnc_var";// || type == "row_field";
}



 reportVar& reportVar::operator=(const reportVar& cp) {
        if (this != &cp) {  // Evită auto-atribuirea
            this->name = cp.name;  // Copierea datelor
            this->tpl_name = cp.tpl_name;
            this->value = cp.value;
            this->type = cp.type;
            this->function = cp.function;

        }
        return *this;
    }
