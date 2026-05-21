#ifndef VLOCALQUERYPROVIDER
#define VLOCALQUERYPROVIDER

#include "IQueryProvider.hpp"
#include "vDataLoader.hpp"

class vLocalQueryProvider : public IQueryProvider {
private:
    vEngine& m_engine;
public:
    vLocalQueryProvider(vEngine& engine) : m_engine(engine) {}

    QueryResult execute(const std::wstring& sql) override {
        return m_engine.executeQuery(sql);
    }

    QueryResult getSchemaCatalog() override {
        return m_engine.getSchemaCatalog();
    }

    QueryResult describeTable(const std::wstring& tableName) override {
        return m_engine.describeTable(tableName);
    }

    bool saveState(const std::wstring& filePath) override {
        return m_engine.saveState(filePath);
    }

    bool loadState(const std::wstring& filePath) override {
        return m_engine.loadState(filePath);
    }

    void inspectState(const std::wstring& filePath) override {
        return m_engine.inspectState(filePath);
    }

    bool importData(const std::wstring& type, const std::wstring& path, const std::wstring& target) override{
        // Tot ce face Engine-ul este să instanțieze Loader-ul și să îi dea comanda
        vDataLoader loader(&m_engine);
        return loader.importExternalData(type, path, target);
        
    }
};

#endif