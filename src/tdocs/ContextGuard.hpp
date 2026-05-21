#pragma once

#include "RenderingContex.hpp"
// ... include-urile necesare, cum ar fi RenderingContext.hpp

// Presupunând că RenderingContext este accesibil

class ContextGuard {
private:
    // Referință la contextul pe care îl protejăm
    RenderingContext& m_contextRef;

    // Copie a stării vechi (părinte) a contextului
    RenderingContext m_savedContext;

public:
    /**
     * @brief Constructorul ContextGuard salvează starea curentă a contextului.
     * @param context Referință la obiectul RenderingContext de protejat.
     */
    ContextGuard(RenderingContext& context)
        : m_contextRef(context), m_savedContext(context) {
        // La construcție, m_savedContext devine o copie exactă a stării curente m_context.
    }

    // NU PERMITE COPIEREA SAU ASIGNAREA
    // (Un ContextGuard nu ar trebui să fie copiat sau mutat, deoarece gestionează starea)
    ContextGuard(const ContextGuard&) = delete;
    ContextGuard& operator=(const ContextGuard&) = delete;

    /**
     * @brief Destructorul ContextGuard restaurează starea salvată.
     */
    ~ContextGuard() {
        // La ieșirea din scope, starea curentă (m_contextRef) este suprascrisă cu starea salvată.
        m_contextRef = m_savedContext;
    }
};