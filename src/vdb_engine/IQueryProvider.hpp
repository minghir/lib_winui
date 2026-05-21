#ifndef IQUERYPROVIDER
#define IQUERYPROVIDER

#include "vEngine.hpp"

class IQueryProvider {
public:
    virtual ~IQueryProvider() = default;
    virtual QueryResult execute(const std::wstring& sql) = 0;
    virtual QueryResult getSchemaCatalog() = 0;
    virtual QueryResult describeTable(const std::wstring& tableName) = 0;
    virtual bool saveState(const std::wstring& filePath) = 0;
    virtual bool loadState(const std::wstring& filePath) = 0;
    virtual void inspectState(const std::wstring& filePath) = 0;

    virtual bool importData(const std::wstring& type, const std::wstring& path, const std::wstring& target) = 0;
    
};

#endif