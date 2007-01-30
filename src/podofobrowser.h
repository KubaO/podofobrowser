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
#include <Qt3Support>

#include "ui_podofobrowserbase.h"
#include <podofo/podofo.h>
#include <qstring.h>

class Q3ListViewItem;
class PdfObjectModel;

class PoDoFoBrowser: public Q3MainWindow, private Ui::PoDoFoBrowserBase
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

    void editInsertKey();
    void editInsertObject();

    void editDeleteKey();
    void editDeleteObject();

    void toolsToHex();
    void toolsFromHex();

    void loadAllObjects();

    void objectChanged( QTreeWidgetItem* );
    void slotImportStream();
    void slotExportStream();
    void slotTableChanged();

    void helpAbout();

 private:

    void loadConfig();
    void saveConfig();

    void parseCmdLineArgs();
    void clear();

    void podofoError( const PoDoFo::PdfError & eCode );

    bool saveObject();

    void streamChanged( PoDoFo::PdfObject* );

    bool trySave();

 private:

    QTreeWidgetItem*      m_lastItem; //XXX
    QString               m_filename;

    PdfObjectModel*	  m_pObjectModel;

    PoDoFo::PdfObject*    m_pCurObject;
    PoDoFo::PdfDocument*  m_pDocument;

    bool                  m_bEditableStream;
    bool                  m_bChanged;
    bool                  m_bObjectChanged;
};


#endif
