#ifndef VDBFILTEREDGRID_HPP
#define VDBFILTEREDGRID_HPP

#include "vPanel.hpp"
#include "vDbGrid.hpp"
#include "vButton.hpp"
#include "vEdit.hpp"
#include "vLabel.hpp"
#include "vDbEditDialog.hpp"
#include "vStatusBar.hpp"
#include "vDbFormPanel.hpp"
#include "vPopupMenu.hpp"

#include <vector>
#include <memory>
#include <functional>
#include <atomic>   // <--- ADAUGĂ ACEASTĂ LINIE PENTRU std::atomic
#include <thread>   // <--- ADAUGĂ ACEASTĂ LINIE PENTRU std::thread

/*
enum class DbDialogMode {
    Update,
    Insert,
    Delete,
    View
};
*/
using OnContextMenuExtendCallback = std::function<void(vPopupMenu&, int, const std::wstring&)>;

class vDbFilteredGrid : public vPanel {
private:

    using OnCellDblClickCallback = std::function<void(int, int, const std::wstring&)>;
    OnCellDblClickCallback m_customCellDblClick = nullptr;

    using OnDbEditCallback = std::function<void(DbDialogMode mode, const std::wstring& idValue)>;
    OnDbEditCallback m_onDbEdit = nullptr;

    //using OnDbAddCallback = std::function<void(DbDialogMode mode, const std::wstring& idValue)>;
    using OnDbAddCallback = std::function<void(DbDialogMode mode)>;
    OnDbAddCallback m_onDbAdd = nullptr;

    using OnDbDelCallback = std::function<void(DbDialogMode mode, const std::wstring& idValue)>;
    OnDbDelCallback m_onDbDel = nullptr;

    using OnDbViewCallback = std::function<void(DbDialogMode mode, const std::wstring& idValue)>;
    OnDbViewCallback m_onDbView = nullptr;

    std::wstring m_title = L"";
    // Flag-uri de configurare
    bool m_isFilterable = true;
    bool m_isPaginable = true;
    bool m_isEditable = true;
    bool m_isExportable = true;
    bool m_hasStatusBar = false;
    bool m_hasActionBar = true;

    // Containere dedicate (Sub-panouri)
    vPanel* m_topPanel = nullptr;    // Schimbă din std::unique_ptr în pointer simplu
    vPanel* m_filterPanel = nullptr;
    vPanel* m_bottomPanel = nullptr;
    // m_grid și m_statusBar rămân direct în containerul principal


    std::atomic<bool> m_shuttingDown = false;
    std::vector<std::thread> m_threads;

    OnContextMenuExtendCallback m_onContextMenuExtend = nullptr;

public:

   

    void setOnContextMenuExtend(OnContextMenuExtendCallback callback) {
        m_onContextMenuExtend = callback;
    }

    explicit vDbFilteredGrid(
        HINSTANCE hInstance,
        const std::string& id,
        int x, int y, int width, int height,
        EventDispatcher& dispatcher,
        dbConnection* db
        //,const std::map<std::wstring, std::wstring>& columnLabels
    );

    virtual ~vDbFilteredGrid();

    void setOnCellDblClick(OnCellDblClickCallback callback) {
        m_customCellDblClick = callback;
    }

    void setOnDbEdit(OnDbEditCallback callback) {
        m_onDbEdit = callback;
    }

    void setOnDbAdd(OnDbAddCallback callback) {
        m_onDbAdd = callback;
    }

    void setOnDbDel(OnDbDelCallback callback) {
        m_onDbDel = callback;
    }


    bool populate(const std::wstring& query, const std::string uniqueIdField = "id");
    void setUniqueIdField(std::string str) { m_uniqueIdField = str; }
    void createControls(HWND parent);
    //void createFilterControls(); // Noua metodă
    
    void setTitle(const std::wstring title);
    void setColumnLabels(const std::map<std::wstring, std::wstring>& labels);
    void setColumnLabel(const std::wstring& dbColumnName, const std::wstring& label) {
        m_grid->setColumnLabel(dbColumnName, label);
    }
    void setColumnWidth(const std::wstring& dbColumnName, int width);
    unsigned long int getRowsCount() const;
    void resize();
    void create(HWND parent);

    vDbGrid* getDbGrid() {
        return m_grid;
    }

    int getSelectedRow() {
        return m_grid->getSelectedRow();
    }

    std::wstring getCellValueByFieldName(int rowIndex, const std::string& fieldName) {
        return m_grid->getCellValueByFieldName(rowIndex, fieldName);
    }

    

    unsigned long int getTotalRrecords() const;
    /**
     * @brief Setează numărul de înregistrări de afișat pe pagină.
     * @param recordsPerPage Noul număr de înregistrări per pagină.
     */
    void setRecordsPerPage(int recordsPerPage);
   
    void setEditable(bool editable); 
    void setExportable(bool exportable);
    void setPaginable(bool paginable);
    void setFilterable(bool filterable);

    void applyFilters();
    void clearFilters();
    void resetSort();

    std::wstring getLastQuery() {
        return m_lastQuery;
    }

    std::wstring getLastFinalQuery() {
        return m_lastFinalQuery;
    }

    std::wstring getLastFilteredQuery() {
        return m_lastFilteredQuery;
    }

protected:
    virtual void onCellDblClick(int rowIndex, int colIndex, const std::wstring& content);
private:

    dbConnection* m_db;
    vDbGrid* m_grid = nullptr; // Use a raw pointer to reference the child control
    std::unique_ptr<vButton> m_applyButton; // This should be a raw pointer as well
    std::vector<vEdit*> m_filterEdits; // Use raw pointers
//    std::vector<vLabel*> m_filterLabels; // Use raw pointers

    std::wstring m_lastQuery;
    std::wstring m_lastFilteredQuery;
    std::wstring m_lastFinalQuery; // cu filtrare si sortare

    std::string m_uniqueIdField;
    std::wstring m_uniqueIdValue;
   
    
   

    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

    std::map<std::wstring, std::wstring> m_columnLabels;


    int m_recordsPerPage = 100; // Numărul de înregistrări pe pagină
    int m_currentPage = 0;             // Pagina curentă (începe de la 0)
    unsigned long int m_totalRecords = 0; // Numărul total de înregistrări

    // Pointeri către butoanele de paginare
    vButton* m_nextBtn = nullptr;
    vButton* m_prevBtn = nullptr;
    vButton* m_firstBtn = nullptr;
    vButton* m_lastBtn = nullptr;
    vLabel* m_paginationLabel = nullptr;
    vLabel* m_titleLabel = nullptr;

    std::unique_ptr<vWindow> m_activeDetailWin;

//   vButton* m_clearFiltersBtn = nullptr;
//    vButton* m_applyFiltersBtn = nullptr;
//    vButton* m_addRecordBtn = nullptr;

    vStatusBar* m_statusBar = nullptr;
    
    //vDbFormPanel* m_DbFormPanel = nullptr;
    //vDbEditDialog* m_EditDialog = nullptr;
    //vWindow* m_customEditWindow = nullptr;

    
    int m_sortedColumn = -1;
    bool m_sortAscending = true;

    void updatePaginationButtonsState();
    void updateStatusWithSelection(int rowIndex);

    // În vDbFilteredGrid.cpp
    void goToFirstPage();
    void goToPrevPage();
    void goToNextPage();
    void goToLastPage();
    void runCountAsync(dbConnection* db, const std::wstring& countQuery, EventDispatcher* dispatcher, const std::string& controlId);

    void setupPagingControls();
    void setupActionControls();
    void setupFilterControls();
    void syncFilterScroll();

    void updateSortArrow(int columnIndex, bool ascending);
    void handleGridMenu(const std::string& argument);

    bool editRecord();
    bool insertRecord();
    bool deleteRecord();
    bool viewRecord();

    bool exportToCsv();

    void showCellContent(int rowIndex, int colIndex, const std::wstring& content);
};

#endif // VDBFILTEREDGRID_HPP