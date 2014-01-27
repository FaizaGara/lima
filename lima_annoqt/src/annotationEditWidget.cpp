/*
 *    Copyright 2002-2013 CEA LIST
 * 
 *    This file is part of LIMA.
 * 
 *    LIMA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Affero General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 * 
 *    LIMA is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Affero General Public License for more details.
 * 
 *    You should have received a copy of the GNU Affero General Public License
 *    along with LIMA.  If not, see <http://www.gnu.org/licenses/>
 */
/***************************************************************************
 *   Copyright (C) 2007 by CEA LIST / LVIC *
 *   Gael.de-Chalendar@cea.fr   *
 ***************************************************************************/

#include "annotationEditWidget.h"
#include "annoqt.h"

#include <QDebug>

AnnotationEditWidget::AnnotationEditWidget(Annoqt* parent) :
  QTextEdit(parent),
  m_parent(parent)
{
}

void AnnotationEditWidget::mousePressEvent ( QMouseEvent * event )
{
  QTextEdit::mousePressEvent(event);
  qDebug() << "AnnotationEditWidget::mousePressEvent cursor position: " << textCursor().position();
  if (event->button() ==  Qt::LeftButton
    && event->modifiers() != Qt::ControlModifier)
  {
    m_parent->selectEventAt(textCursor().position(), event->globalPos());
  }
}
