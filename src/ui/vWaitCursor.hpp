#ifndef VWAITCURSOR
#define VWAITCURSOR

#include <windows.h>

class vWaitCursor {
public:
    // Constructorul setează clepsidra
    vWaitCursor() {
        m_oldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
        // În Win32, uneori e nevoie să forțăm desenarea cursorului
        ShowCursor(TRUE); 
    }

    // Destructorul restaurează automat cursorul anterior
    ~vWaitCursor() {
        if (m_oldCursor) {
            SetCursor(m_oldCursor);
        }
    }

    // Prevenim copierea obiectului (nu vrem două instanțe care să se bată pe cursor)
    vWaitCursor(const vWaitCursor&) = delete;
    vWaitCursor& operator=(const vWaitCursor&) = delete;

private:
    HCURSOR m_oldCursor;
};

#endif