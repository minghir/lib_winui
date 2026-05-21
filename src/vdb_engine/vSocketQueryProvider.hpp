#ifndef VSOKETQUERYPROVIDER
#define VSOKETQUERYPROVIDER

#include "IQueryProvider.hpp"

class vSocketQueryProvider : public IQueryProvider {
private:
    vEngine& m_engine;
public:
    //vSocketQueryProvider() {}

    QueryResult execute(const std::wstring& sql) override {
        return {};
    }

    QueryResult getSchemaCatalog() override {
        return {};
    }

    QueryResult describeTable(const std::wstring& tableName) override {
        return {};
    }

    bool saveState(const std::wstring& filePath) override {
        return false;
    }

    bool loadState(const std::wstring& filePath) override {
        return false;
    }

    void inspectState(const std::wstring& filePath) override {
        return;
    }
};
#endif