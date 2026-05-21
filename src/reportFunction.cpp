#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>


#include "reportFunction.hpp"
reportFunction::reportFunction(reportHandler *rep, std::string funcDesc, std::wstring out) {
	funcionDesc = funcDesc;
	report = rep;
	outStr = out;
}

std::wstring reportFunction::getFuncValue() {

	//std::cout << "IN FUNCTIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII" << report->getName() << std::endl;
	std::string aa = getStrBetween(funcionDesc, '(', ')');
	
	std::string datasource;
	std::string rows;
	std::string var;
	std::string function = sleft_of(funcionDesc,'(');


	auto funcPath = explode(aa, '.');

	if (!funcPath[0].empty()) {
		datasource = funcPath[0];
	} else {
		return L"";
	}

	if (!funcPath[1].empty()) {
		rows = funcPath[1];
	}
	else {
		return L"";
	}

	if (!funcPath[2].empty()) {
		var = funcPath[2];
	}
	else {
		return L"";
	}

	std::cout << "PATH:" << datasource << "." << rows << "." << var << std::endl;


	std::vector<std::wstring> rowsVars;
	for (dataSource ds : report->getDataSources()) {
		//std::cout << "DATTTTTTTTTTTTTTTAAAAAAAAAAAAAAAAAAAAAA:" << ds.getName() << std::endl;
		if (ds.getName() == datasource) {
			
			//std::cout << "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ:" << ds.getName() << std::endl;
			//std::cout << "WWWWWWWWWWWWWWSIZE2:" << ds.getDataRows().size()<< std::endl;
			
			for (auto row : ds.getDataRows()) {
				//std::cout << "CAUT ROW:" << row.getName() << std::endl;
				if (row.getName() == rows) {
					
					rowsVars.push_back(row.getVarByName(var).getValue());
				}
			}
		}
		std::cout << ds.getName() << std::endl;
	}

	//if (rowsVars.size() == 0)
	//	return L"pppp";

	if (function == "fsum") {
		return func_fsum(rowsVars);
	}

	if (function == "ifnotempty" && rowsVars.size() > 0) {
		return func_ifnotempty(rowsVars);
	}

	return L"";
	//return str_to_wstr(funcPath[2]);
	//return str_to_wstr(aa);
}

std::wstring reportFunction::func_fsum(const std::vector<std::wstring>& vect) {
	if (vect.empty())
		return L"0.00"; 

	double suma = 0.0;
	for (const auto& elem : vect) {
		std::wstringstream ss(elem);
		double numar = 0.0;
		if (ss >> numar) {
			suma += numar;
		}
		// altfel: ignoram valorile invalide
	}

	std::wstringstream rezultat;
	rezultat << std::fixed << std::setprecision(2) << suma;
	return rezultat.str();
}


std::wstring reportFunction::func_ifnotempty(const std::vector<std::wstring>& vect) {
	if (!vect[0].empty()) {
		//std::cout << "SUNNNNNNNNNNNNNNNNNNNNNNNN" << std::endl;
		return outStr;
	}
	return L"";
}

