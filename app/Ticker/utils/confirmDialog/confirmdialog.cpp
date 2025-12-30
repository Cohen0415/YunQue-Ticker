// ConfirmDialog.cpp
#include "confirmdialog.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>

ConfirmDialog::ConfirmDialog(const QString &text, QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);   // 无边框对话框
    setAttribute(Qt::WA_TranslucentBackground);             // 背景透明

    QWidget *content = new QWidget(this);
    content->setObjectName("content");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(content);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *vLayout = new QVBoxLayout(content);
    vLayout->setContentsMargins(0, 15, 0, 10);        // left top right bottom
    vLayout->setSpacing(18);

    m_label = new QLabel(text, content);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setWordWrap(true);

    m_btnCancel = new QPushButton("取消", content);
    m_btnOk = new QPushButton("确定", content);
    m_btnCancel->setFocusPolicy(Qt::NoFocus);
    m_btnOk->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addStretch();
    hLayout->addWidget(m_btnCancel);
    hLayout->addSpacing(18);        // 按钮间距
    hLayout->addWidget(m_btnOk);
    hLayout->addStretch();
    hLayout->setContentsMargins(0, 0, 0, 0);

    vLayout->addWidget(m_label);
    vLayout->addLayout(hLayout);

    content->setStyleSheet(R"(
        QWidget#content {
            background: white;
        }
        QLabel { color:#222; font-size:18px; }
        QPushButton {
            min-width: 145px; min-height: 45px; border-radius: 14px; font-size: 18px;
        }
        QPushButton#btnCancel {
            background-color: #238dfa; color: #fff; border: none;
        }
        QPushButton#btnOk {
            background-color: #fa5555; color: #fff; border: none;
        }
        QPushButton#btnCancel:hover { background-color: #096dd9; }
        QPushButton#btnOk:hover     { background-color: #c32222; }
    )");

    m_btnCancel->setObjectName("btnCancel");
    m_btnOk->setObjectName("btnOk");

    auto *effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(20);
    effect->setOffset(0, 4);
    effect->setColor(QColor(0,0,0,40));
    content->setGraphicsEffect(effect);

    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_btnOk, &QPushButton::clicked, this, &QDialog::accept);

    resize(350, 140);
}

bool ConfirmDialog::ask(QWidget *parent, const QString &text)
{
    ConfirmDialog dlg(text, parent);
    dlg.setModal(true);
    return dlg.exec() == QDialog::Accepted;
}
