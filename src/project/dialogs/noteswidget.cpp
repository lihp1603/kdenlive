/***************************************************************************
 *   Copyright (C) 2008 by Jean-Baptiste Mardelle (jb@kdenlive.org)        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/


#include "noteswidget.h"

#include <klocalizedstring.h>
#include <QDebug>
#include <QMenu>
#include <QMouseEvent>


NotesWidget::NotesWidget(QWidget * parent) :
        QTextEdit(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(slotFillNotesMenu(const QPoint &)));
    setMouseTracking(true);
}

void NotesWidget::slotFillNotesMenu(const QPoint &pos)
{
    QMenu *menu = createStandardContextMenu();
    QAction *a = new QAction(i18n("Insert current timecode"), menu);
    connect(a, SIGNAL(triggered(bool)), this, SIGNAL(insertNotesTimecode()));
    menu->insertAction(menu->actions().at(0), a);
    menu->exec(viewport()->mapToGlobal(pos));
}

void NotesWidget::mouseMoveEvent( QMouseEvent * e )
{
    const QString anchor = anchorAt(e->pos());
    if (anchor.isEmpty())
        viewport()->setCursor(Qt::IBeamCursor);
    else
        viewport()->setCursor(Qt::PointingHandCursor);
    QTextEdit::mouseMoveEvent(e);
}

void NotesWidget::mousePressEvent( QMouseEvent * e )
{
    QString anchor = anchorAt(e->pos());
    if (anchor.isEmpty()) {
        QTextEdit::mousePressEvent(e);
        return;
    }
    //qDebug()<<"+++++++++\nCLICKED NACHOR: "<<anchor;
    emit seekProject(anchor.toInt());
    e->setAccepted(true);
}

NotesWidget::~NotesWidget()
{
}




