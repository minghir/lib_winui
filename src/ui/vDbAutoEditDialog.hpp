#pragma once

#include "vDbEditDialog.hpp"
#include "../sql/SqlQueryAnalyzer.hpp"

class vDbAutoEditDialog : public vDbEditDialog {
public:
	explicit vDbAutoEditDialog(HINSTANCE hInstance, const std::string& id, EventDispatcher& dispatcher, dbConnection* db);
	void setupEditControls() override;
};