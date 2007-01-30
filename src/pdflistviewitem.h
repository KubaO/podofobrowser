/***************************************************************************
 *   Copyright (C) 2005 by Dominik Seichter                                *
 *   domseichter@web.de                                                    *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _PDF_LIST_VIEW_ITEM_H_
#define _PDF_LIST_VIEW_ITEM_H_

// Sledge hammer approach to getting all the required definitions
// for Qt4
#include <QtCore>
#include <QtGui>
#include <Qt3Support>

#include "qlistview.h"

namespace PoDoFo {
    class PdfObject;
    class PdfDictionary;
    class PdfVariant;
};

class PdfListViewItem : public Q3ListViewItem {
 public:
    PdfListViewItem( Q3ListView* parent, PoDoFo::PdfObject* object );
    PdfListViewItem( Q3ListViewItem* parent, PoDoFo::PdfObject* object, const QString & key );
    ~PdfListViewItem();

    inline PoDoFo::PdfObject* object() const; 

    int compare( Q3ListViewItem* i, int col, bool ascending ) const;

    void init();

 private:
    PoDoFo::PdfObject*     m_pObject;
    QString                m_sText;
    QString                m_sType;

    bool                   m_bInitialized;
};

PoDoFo::PdfObject* PdfListViewItem::object() const
{
    return m_pObject;
}

#endif // _PDF_LIST_VIEW_ITEM_H_
