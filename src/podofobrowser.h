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

#ifndef PODOFOBROWSER_H
#define PODOFOBROWSER_H

// Sledge hammer approach to getting all the required definitions
// for Qt4
#include <QtCore>
#include <QtGui>

#include "ui_podofobrowserbase.h"
#include <podofo/podofo.h>
#include <qstring.h>

class PdfObjectModel;
class QModelIndex;

class PoDoFoBrowser: public QMainWindow, private Ui::PoDoFoBrowserBase
{
    Q_OBJECT

 public:
    PoDoFoBrowser();
    ~PoDoFoBrowser();

 private slots:
    void fileNew();

    void fileOpen();
    void fileOpen( const QString & filename );

    bool fileSave();
    bool fileSave( const QString & filename );
    bool fileSaveAs();

    void fileExit();

    void toolsToHex();
    void toolsFromHex();

    void treeSelectionChanged( const QModelIndex & current, const QModelIndex & previous );

    void slotImportStream();
    void slotExportStream();

    void helpAbout();

    void editInsertBefore();
    void editInsertAfter();
    void editInsertKey();
    void editInsertChildBelow();
    void editRemoveItem();
    void editCreateMissingObject();

 private:
    void ModelChange(PdfObjectModel* newModel);
    void UpdateMenus();

    inline QModelIndex GetSelectedItem();

    void loadConfig();
    void saveConfig();

    void parseCmdLineArgs();
    void clear();

    bool saveObject();

    bool trySave();

    // Insert an element at row `row' under `parent'.
    // Just a helper for editInsertBlah()
    void insertElement(int row, const QModelIndex& parent);

 private:

    QTreeWidgetItem*      m_lastItem; //XXX
    QString               m_filename;

    PoDoFo::PdfObject*    m_pCurObject;
    PoDoFo::PdfDocument*  m_pDocument;
};

QModelIndex PoDoFoBrowser::GetSelectedItem()
{
    return listObjects->selectionModel()->currentIndex();
}

#endif
