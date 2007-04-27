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
class BackgroundLoader;
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
    void fileReload();

    void fileExit();

    void toolsToHex();
    void toolsFromHex();
    void toolsDisplayCodeForSelection();

    void treeSelectionChanged( const QModelIndex & current, const QModelIndex & previous );

    void slotImportStream();
    void slotExportStream();

    void helpAbout();

    void editInsertBefore();
    void editInsertAfter();
    void editInsertKey();
    void editInsertChildBelow();
    void editRemoveItem();
    void editCreateNewObject();
    void editFind();
    void editFindNext();
    void editFindPrevious();
    void editReplace();
    void editGotoObject();

    void viewRefreshView();


 private:
    void ModelChange(PdfObjectModel* newModel);
    void DocChange(PoDoFo::PdfDocument* doc);
    void UpdateMenus();

    // Find the object m_gotoReference
    // in the current tree and select it in the list view.
    void GotoObject();

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

    QString               m_filename;

    PoDoFo::PdfDocument*  m_pDocument;
    BackgroundLoader*     m_pBackgroundLoader;
    QProgressBar*         m_pDelayedLoadProgress;

    // Members for find, findNext and findPrevious
    bool                  m_bHasFindText;
    QString               m_sFindText;

    PoDoFo::PdfReference  m_gotoReference;

    QTextDocument::FindFlags m_findFlags;
};

QModelIndex PoDoFoBrowser::GetSelectedItem()
{
    return listObjects->selectionModel()->currentIndex();
}

#endif
