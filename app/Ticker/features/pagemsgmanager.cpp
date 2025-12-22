#include "pagemsgmanager.h"

PageMsgManager *PageMsgManager::m_instance = nullptr;

PageMsgManager::PageMsgManager(QObject *parent)
    : QObject{parent}
{}

PageMsgManager *PageMsgManager::getInstance()
{
    if (m_instance == nullptr)
    {
        m_instance = new PageMsgManager();
    }
    return m_instance;
}


