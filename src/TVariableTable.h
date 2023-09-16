#pragma once

#include "error_type.h"

#include <vector>
#include <unordered_map>

class TVariableTable
{
public:
	TVariableTable();
	~TVariableTable();
	//bool Has(const std::string &varstr);
	//void Define(std::ostream *pStr, std::string vars, std::string input_num = "",bool bIgnoreReDef = false);
	//void DefineOne(std::ostream *pStr,std::string var, double value,bool bIgnoreReDef=false);//定义单变量
	//void Output(std::ostream *pStr);
	//void OutputValue(std::ostream *pStr);//输出 x=0 形式
	//void RemoveOne(std::ostream *pStr, std::string var, bool bIgnoreUnExisted = false);//同时清除变量和数据
	//double GetValueFromVarPoint(const std::string pVar);
	//void SetValueFromVarStr(std::string VarStr, double value);
	//void SetValueByVarTable(TVariableTable &VarTable);
private:
	std::unordered_map<std::string, double> VariableTable;

	//bool isLegalName(std::string s);
};

