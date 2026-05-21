#ifndef REPORTFUNCTION_HPP
#define REPORTFUNCTION_HPP

#include<string>

#include"reportHandler.hpp"

class reportFunction{
    std::string funcionDesc;
    std::wstring outStr;
    reportHandler *report;

public:

    std::wstring func_fsum(const std::vector<std::wstring>& vect);
    std::wstring func_ifnotempty(const std::vector<std::wstring>& vect);

    reportFunction(reportHandler *rep, std::string funcDesc, std::wstring out = L"");
    std::wstring getFuncValue();
};
#endif
