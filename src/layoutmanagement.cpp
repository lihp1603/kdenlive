/*
Copyright (C) 2012  Jean-Baptiste Mardelle <jb@kdenlive.org>
Copyright (C) 2014  Till Theato <root@ttill.de>
This file is part of kdenlive. See www.kdenlive.org.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "layoutmanagement.h"
#include "core.h"
#include "mainwindow.h"
#include <QMenu>
#include <QInputDialog>
#include <KXMLGUIFactory>

LayoutManagement::LayoutManagement(QObject* parent) :
    QObject(parent)
{
    // Prepare layout actions
    KActionCategory *layoutActions = new KActionCategory(i18n("Layouts"), pCore->window()->actionCollection());
    m_loadLayout = new KSelectAction(i18n("Load Layout"), pCore->window()->actionCollection());
    for (int i = 1; i < 5; ++i) {
        KAction *load = new KAction(KIcon(), i18n("Layout %1", i), this);
        load->setData('_' + QString::number(i));
        layoutActions->addAction("load_layout" + QString::number(i), load);
        m_loadLayout->addAction(load);
        KAction *save = new KAction(KIcon(), i18n("Save As Layout %1", i), this);
        save->setData('_' + QString::number(i));
        layoutActions->addAction("save_layout" + QString::number(i), save);
    }
    // Required to enable user to add the load layout action to toolbar
    layoutActions->addAction("load_layouts", m_loadLayout);
    connect(m_loadLayout, SIGNAL(triggered(QAction*)), SLOT(slotLoadLayout(QAction*)));

    
    QMenu *saveLayout = static_cast<QMenu*>(pCore->window()->factory()->container("layout_save_as", pCore->window()));
    if (saveLayout) {
        connect(saveLayout, SIGNAL(triggered(QAction*)), SLOT(slotSaveLayout(QAction*)));
    }

    // Load layout names from config file
    loadLayouts();
}

void LayoutManagement::loadLayouts()
{
    QMenu *saveLayout = static_cast<QMenu*>(pCore->window()->factory()->container("layout_save_as", pCore->window()));
    if (m_loadLayout == NULL || saveLayout == NULL) return;
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup layoutGroup(config, "Layouts");
    QStringList entries = layoutGroup.keyList();
    QList<QAction *> loadActions = m_loadLayout->actions();
    QList<QAction *> saveActions = saveLayout->actions();
    for (int i = 1; i < 5; ++i) {
        // Rename the layouts actions
        foreach(const QString & key, entries) {
            if (key.endsWith(QString("_%1").arg(i))) {
                // Found previously saved layout
                QString layoutName = key.section('_', 0, -2);
                for (int j = 0; j < loadActions.count(); ++j) {
                    if (loadActions.at(j)->data().toString().endsWith('_' + QString::number(i))) {
                        loadActions[j]->setText(layoutName);
                        loadActions[j]->setData(key);
                        break;
                    }
                }
                for (int j = 0; j < saveActions.count(); ++j) {
                    if (saveActions.at(j)->data().toString().endsWith('_' + QString::number(i))) {
                        saveActions[j]->setText(i18n("Save as %1", layoutName));
                        saveActions[j]->setData(key);
                        break;
                    }
                }
            }
        }
    }
}

void LayoutManagement::slotLoadLayout(QAction *action)
{
    if (!action) {
        return;
    }

    QString layoutId = action->data().toString();
    if (layoutId.isEmpty()) {
        return;
    }

    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup layouts(config, "Layouts");
    QByteArray state = QByteArray::fromBase64(layouts.readEntry(layoutId).toAscii());
    pCore->window()->restoreState(state);
}

void LayoutManagement::slotSaveLayout(QAction *action)
{
    QString originallayoutName = action->data().toString();
    int layoutId = originallayoutName.section('_', -1).toInt();

    QString layoutName = QInputDialog::getText(pCore->window(), i18n("Save Layout"), i18n("Layout name:"), KLineEdit::Normal,
                                               originallayoutName.section('_', 0, -2));
    if (layoutName.isEmpty()) {
        return;
    }
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup layouts(config, "Layouts");
    layouts.deleteEntry(originallayoutName);

    QByteArray st = pCore->window()->saveState();
    layoutName.append('_' + QString::number(layoutId));
    layouts.writeEntry(layoutName, st.toBase64());
    loadLayouts();
}

#include "layoutmanagement.moc"
