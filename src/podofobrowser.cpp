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

#include "podofobrowser.h"
#include "podofoinfodlg.h"
#include "podofoutil.h"
#include "backgroundloader.h"
#include "ui_podofoaboutdlg.h"
#include "ui_podofofinddlg.h"
#include "ui_podofogotodlg.h"
#include "ui_podofogotopagedlg.h"
#include "ui_podoforeplacedlg.h"
#include "pdfobjectmodel.h"
#include "hexwidget/QHexView.h"

#include <QApplication>
#include <QDockWidget>
#include <QCursor>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QProgressDialog>
#include <QPushButton>
#include <QSettings>
#include <QSplitter>
#include <QStatusBar>
#include <QTextCodec>
#include <QTextEdit>
#include <QBuffer>

#include <cassert>
#include <iostream>
#include <algorithm>

using namespace PoDoFo;

class ObjectsComperator { 
public:
    ObjectsComperator( unsigned long lObject, unsigned long lGeneration )
        {
            m_lObject     = lObject;
            m_lGeneration = lGeneration;
        }
    
    bool operator()(const PdfObject* p1) { 
        return (p1->Reference().ObjectNumber() == m_lObject && p1->Reference().GenerationNumber() == m_lGeneration );
    }
private:
    unsigned long m_lObject;
    unsigned long m_lGeneration;
};

PoDoFoBrowser::PoDoFoBrowser( const QString & filename )
    : QMainWindow(),
      PoDoFoBrowserBase(),
      m_pDocument( NULL ),
      m_pBackgroundLoader( NULL ),
      m_pDelayedLoadProgress( NULL ),
      m_bHasFindText( false ),
      m_pByteArray( NULL ),
      m_pByteArrayIO (NULL)
{
    setupUi(this);
    setObjectName(QString::fromUtf8("PoDoFoBrowser"));
    setAttribute(Qt::WA_DeleteOnClose);

    // tree view
    dockObjects = new QDockWidget(tr("Objects"),this);
    listObjects = new QTreeView(0);
    dockObjects->setWidget(listObjects);
    addDockWidget(Qt::TopDockWidgetArea, dockObjects);

    // stream edition
    slotSetStreamEditable(false);

    m_pDelayedLoadProgress = new QProgressBar( statusBar() );
    m_pDelayedLoadProgress->setFormat( tr("%p% of objects loaded") );
    statusBar()->addPermanentWidget(m_pDelayedLoadProgress);

    clear();

    connect( buttonImport, SIGNAL( clicked() ), this, SLOT( slotImportStream() ) );
    connect( buttonExport, SIGNAL( clicked() ), this, SLOT( slotExportStream() ) );
    connect( fileReloadAction, SIGNAL( triggered() ), this, SLOT( fileReload() ) );
    connect( actionInformations, SIGNAL( triggered() ), this, SLOT( fileInfo() ) );
    connect( actionInsert_Before, SIGNAL( triggered() ), this, SLOT( editInsertBefore() ) );
    connect( actionInsert_After,  SIGNAL( triggered() ), this, SLOT( editInsertAfter() ) );
    connect( actionInsert_Key,    SIGNAL( triggered() ), this, SLOT( editInsertKey() ) );
    connect( actionInsert_Child,  SIGNAL( triggered() ), this, SLOT( editInsertChildBelow() ) );
    connect( actionRemove_Item,   SIGNAL( triggered() ), this, SLOT( editRemoveItem()) );
    connect( actionRefreshView,   SIGNAL( triggered() ), this, SLOT( viewRefreshView()) );
    connect( actionCatalogView,   SIGNAL( triggered() ), this, SLOT( viewRefreshView()) );
    connect( actionCreateNewObject, SIGNAL( triggered() ), this, SLOT( editCreateNewObject()) );
    connect( actionToolsDisplayCodeForSelection, SIGNAL( triggered() ), this, SLOT( toolsDisplayCodeForSelection()) );
    connect( actionFind,          SIGNAL( triggered() ), this, SLOT( editFind() ) );
    connect( actionFindNext,      SIGNAL( triggered() ), this, SLOT( editFindNext() ) );
    connect( actionFindPrevious,  SIGNAL( triggered() ), this, SLOT( editFindPrevious() ) );
    connect( actionReplace,       SIGNAL( triggered() ), this, SLOT( editReplace() ) );
    connect( actionGotoObject,    SIGNAL( triggered() ), this, SLOT( editGotoObject() ) );
    connect( actionGotoPage,      SIGNAL( triggered() ), this, SLOT( editGotoPage() ) );

    connect( checkEditable, SIGNAL(toggled(bool)), this, SLOT(slotSetStreamEditable(bool)) );
    connect( commitButton, SIGNAL(clicked()), this, SLOT(slotCommitStream()) );

    show();
    statusBar()->showMessage( tr("Ready"), 2000 );

    loadConfig();

    if( !filename.isNull() )
        fileOpen( filename );
}

void PoDoFoBrowser::ModelChange(PdfObjectModel* newModel)
{
    PdfObjectModel* oldModel = static_cast<PdfObjectModel*>(listObjects->model());
    if (oldModel)
    {
        disconnect( listObjects->selectionModel(), SIGNAL( currentChanged (QModelIndex, QModelIndex) ),
                    this, SLOT( treeSelectionChanged(QModelIndex, QModelIndex) ) );
    }
    listObjects->setModel(newModel);
    if (newModel)
    {
        connect( listObjects->selectionModel(), SIGNAL( currentChanged (QModelIndex, QModelIndex) ),
                 this, SLOT( treeSelectionChanged(QModelIndex, QModelIndex) ) );
    }
    UpdateMenus();
    delete oldModel;
}

void PoDoFoBrowser::DocChange(PdfMemDocument* newDoc)
{
    delete m_pBackgroundLoader;
    m_pBackgroundLoader = NULL;
    m_pDelayedLoadProgress->reset();
    if (newDoc)
    {
        // create a background loader and hook it up to the progress bar
        m_pBackgroundLoader = new BackgroundLoader(newDoc);
        m_pDelayedLoadProgress->setMaximum( m_pDocument->GetObjects().GetSize() );
        connect( m_pBackgroundLoader, SIGNAL(progress(int)), m_pDelayedLoadProgress, SLOT(setValue(int)) );
        connect( m_pBackgroundLoader, SIGNAL(done()), m_pDelayedLoadProgress, SLOT(reset()) );

        // then start loading
        m_pBackgroundLoader->start();
    }
}

PoDoFoBrowser::~PoDoFoBrowser()
{
    ModelChange(NULL);
    DocChange(NULL);

    saveConfig();

    delete m_pDocument;
    delete m_pByteArrayIO;
    delete m_pByteArray;
}

void PoDoFoBrowser::loadConfig()
{
    int w,h;

    QList<int> list;
    QSettings       settings;

    settings.setPath( QSettings::IniFormat,
		      QSettings::UserScope,
		      QString::fromUtf8("podofobrowser") );

    w = settings.value( QString::fromUtf8("/geometry/width"), width() ).toInt();
    h = settings.value( QString::fromUtf8("/geometry/height"), height() ).toInt();

    m_codecForStream = QTextCodec::codecForName(settings.value( QString::fromUtf8("/Stream/Codec"),QString::fromUtf8("ISO 8859-1") ).toString().toUtf8());

    list << width()/3;
    list << (width()/3 * 2);

    splitter2->setSizes( list );
    splitter3->setSizes( list );

    this->resize( w, h );

    actionCatalogView->setChecked( settings.value(QString::fromUtf8("/view/catalog"), actionCatalogView->isChecked() ).toBool() );
}

void PoDoFoBrowser::saveConfig()
{

    QSettings settings;

    settings.setPath( QSettings::IniFormat,
		      QSettings::UserScope,
		      QString::fromUtf8("podofobrowser") );

    settings.setValue(QString::fromUtf8("/geometry/width"), width() );
    settings.setValue(QString::fromUtf8("/geometry/height"), height() );
    settings.setValue(QString::fromUtf8("/view/catalog"), actionCatalogView->isChecked() );

    settings.setValue(QString::fromUtf8("/Stream/Codec"), m_codecForStream->name());
}

void PoDoFoBrowser::clear()
{
    m_filename = QString();
    setWindowTitle( tr("PoDoFoBrowser") );

    labelStream->setText( QString::fromUtf8("") );
    textStream->clear();
    buttonImport->setEnabled( false );
    buttonExport->setEnabled( false );
    textStream->setEnabled(false);

    ModelChange(NULL);
    DocChange(NULL);

    delete m_pDocument;
    m_pDocument       = NULL;

    hexView->clear();
    delete m_pByteArrayIO;
    m_pByteArrayIO = NULL;
    delete m_pByteArray;
    m_pByteArray = NULL;
}

void PoDoFoBrowser::fileNew()
{
    PdfError         eCode;

    if( !trySave() ) 
        return;

    this->clear();

    PdfMemDocument* oldDoc = m_pDocument;
    m_pDocument = new PdfMemDocument();

    ModelChange( new PdfObjectModel(m_pDocument, listObjects, actionCatalogView->isChecked()) );
    DocChange(m_pDocument);

    delete oldDoc;

    hexView->clear();
    delete m_pByteArrayIO;
    m_pByteArrayIO = NULL;
    delete m_pByteArray;
    m_pByteArray = NULL;
}

void PoDoFoBrowser::fileOpen( const QString & filename )
{
    QStringList      lst;

    clear();

    QApplication::setOverrideCursor( Qt::WaitCursor );

    PdfMemDocument* oldDoc = m_pDocument;
    try {
        m_pDocument = new PdfMemDocument( filename.toLocal8Bit().data() );
        delete oldDoc;
    } catch( PdfError & e ) {
        QApplication::restoreOverrideCursor();
        podofoError( e );
        return;
    }

    ModelChange( new PdfObjectModel(m_pDocument, listObjects, actionCatalogView->isChecked()) );
    DocChange(m_pDocument);

    SetFileName( filename );

    QApplication::restoreOverrideCursor();
    statusBar()->showMessage( tr("Opened file %1 successfully").arg( filename ), 2000 );
}

bool PoDoFoBrowser::fileSave( const QString & filename )
{
    QApplication::setOverrideCursor( Qt::WaitCursor );

    try {
        m_pDocument->Write( filename.toLocal8Bit().data() );
    } catch( PdfError & e ) {
        QApplication::restoreOverrideCursor();
        podofoError( e );
        return false;
    }

    QApplication::restoreOverrideCursor();
    statusBar()->showMessage( tr("Wrote file %1 successfully").arg( filename ), 2000 );

    SetFileName( filename );

    return true;
}

void PoDoFoBrowser::UpdateMenus()
{
    PdfObjectModel * const model = static_cast<PdfObjectModel*>(listObjects->model());
    QModelIndex sel = GetSelectedItem();

    fileSaveAction->setEnabled(model != 0);
    fileSaveAsAction->setEnabled(model != 0);
    fileReloadAction->setEnabled(model != 0 && !m_filename.isEmpty() && model->DocChanged() );
    actionRefreshView->setEnabled(model != 0);
    actionCatalogView->setEnabled(model != 0);
    actionGotoObject->setEnabled(model != 0 && !actionCatalogView->isChecked() );

    // Can add a child to any array or dictionary
    actionInsert_Child->setEnabled( sel.isValid() && (model->IndexIsDictionary(sel) || model->IndexIsArray(sel)) );
    // Can create a new indirect object in place of any value
    actionCreateNewObject->setEnabled( sel.isValid() );

    QModelIndex parent = sel.parent();
    bool enableInsertBeforeAfter = parent.isValid() && model->IndexIsArray(parent);
    actionInsert_Before->setEnabled(enableInsertBeforeAfter);
    actionInsert_After->setEnabled(enableInsertBeforeAfter);
    actionInsert_Key->setEnabled(parent.isValid() && model->IndexIsDictionary(parent));
    actionRemove_Item->setEnabled(sel.isValid() && !model->GetObjectForIndex(sel)->Reference().IsIndirect());

    // DS: parent.isValid() is never true, if not in catalog view mode
    //     and should not be needed anyways
    // bool enableFind = parent.isValid() && model->GetObjectForIndex(sel) 
    //    && model->GetObjectForIndex(sel)->HasStream();
    bool enableFind = model->GetObjectForIndex(sel) && model->GetObjectForIndex(sel)->HasStream();

    actionFind->setEnabled( enableFind );
    actionFindNext->setEnabled( enableFind && m_bHasFindText );
    actionFindPrevious->setEnabled( enableFind && m_bHasFindText );
    actionReplace->setEnabled( enableFind );

    actionToolsDisplayCodeForSelection->setEnabled( sel.isValid() );
}

 void PoDoFoBrowser::SetFileName(const QString& name)
 {
	m_filename = name;
	setWindowTitle( m_filename );
 }

// Triggered when the selected object in the list view changes
void PoDoFoBrowser::treeSelectionChanged( const QModelIndex & current, const QModelIndex & previous )
{
    Q_UNUSED(previous);
    UpdateMenus();

    textStream->clear();
    buttonImport->setEnabled( false );
    buttonExport->setEnabled( false );
    textStream->setEnabled(false);
        
    hexView->clear();
    delete m_pByteArrayIO;
    m_pByteArrayIO = NULL;
    delete m_pByteArray;
    m_pByteArray = NULL;

    // make keyboard navigation easier, especially in Catalog View mode
    // hopefully this does not slow down to much.
    // CR: We may need a more efficient approach to this as the tree grows,
    // since it does slow down a lot.
    // DS: Disabled because it is annyoing so that the column is always resized
    // even if you have set a manual columnwidth
    // listObjects->resizeColumnToContents( 0 );

    PdfObjectModel * const model = static_cast<PdfObjectModel*>(listObjects->model());
    if (!model)
    {
        labelStream->setText( tr("No document available") );
        return;
    }

    const PdfObject* object = model->GetObjectForIndex(current);
    if (!object)
    {
        labelStream->setText( tr("No object available") );
        return;
    }

    const PdfReference & ref (object->Reference());
    if (!ref.IsIndirect())
    {
        labelStream->setText( tr("Object is not indirect so it can not have a stream") );
        return;
    }

    try {
        if (!object->HasStream())
        {
            labelStream->setText( tr("Object %1 %2 obj has no stream").arg(ref.ObjectNumber()).arg(ref.GenerationNumber()) );
            return;
        }
    }
    catch ( PdfError & e ) {
        labelStream->setText( tr("Error while testing for object stream") );
        podofoError( e );
        return;
    }

    // If we get this far, we can safely import a new stream even if 
    // the current stream contents are invalid.
    buttonImport->setEnabled( true );

    // XXX this should be in the model
    const PdfStream * const stream = object->GetStream();
    char * pBuf = NULL;
    pdf_long lLen = -1;
    try {
        stream->GetFilteredCopy( &pBuf, &lLen );
    } catch( PdfError & e ) {
        labelStream->setText( tr("Unable to filter object stream") );
        podofoError( e );
        return;
    }

    assert(pBuf);
    assert(lLen >= 0);

    // Quick and dirty binary-ness test
    // TODO: sane encoding-safe approach to binary data
    // TODO: hex editor
    QString displayInfo;
    const bool isBinary = std::find(pBuf, pBuf+lLen, 0) != pBuf+lLen;
    if (!isBinary)
    {
        // TODO FIXME XXX AUUGH! Encoding assumption like nothing ever
        // seen before!
//        QString data = QString::fromAscii(pBuf, lLen);
	QByteArray pArray(pBuf, lLen);
	QString data = m_codecForStream->toUnicode(pArray);
        textStream->setEnabled(true);
        textStream->setText( data );
        displayInfo = tr("displayed in full");
        stackedWidget->setCurrentWidget( pageStream );
    }
    else
    {
	// Read the whole stream into memory and point the hex editor at it.
	// Later we'll support progressive I/O.
        m_pByteArray = new QByteArray( pBuf, lLen );
	m_pByteArrayIO = new QBuffer( m_pByteArray );
	// Open for random R/W I/O
	m_pByteArrayIO->open(QIODevice::ReadWrite);
	// and tell the hex editor to display the new data
        hexView->setData( m_pByteArrayIO, m_pByteArrayIO->size() );
        displayInfo = tr("contains binary data");
        stackedWidget->setCurrentWidget( pageHexView );
    }
    free( pBuf );

    labelStream->setText( tr("Stream from object %1 %2 obj of unfiltered length %3 bytes %4")
            .arg(ref.ObjectNumber())
            .arg(ref.GenerationNumber())
            .arg(lLen)
            .arg(displayInfo)
            );
    buttonImport->setEnabled( true );
    buttonExport->setEnabled( true );

}

void PoDoFoBrowser::fileOpen()
{
   if( !trySave() ) 
       return;

    QString filename = QFileDialog::getOpenFileName(
		    this, tr("Open PDF..."), QString(), tr("PDF File (*.pdf)"));
    if( !filename.isNull() )
        fileOpen( filename );
}

bool PoDoFoBrowser::fileSave()
{
    if( m_filename.isEmpty() )
        return fileSaveAs();
    else
    {
        // can't just overwrite existing file, since we might have file streams
        // reading from it as we write. Instead we write to a temp file and then
	// swap it with the original.
	    // lets try something else. pm
        const QString origFileName = m_filename;
	const QFileInfo origFileInfo(origFileName);
	QDir tmpDir = QDir::temp();
	const QString tmpFileName = QString::number(QCoreApplication::applicationPid())
                    + QStringLiteral("-")
				    + origFileInfo.fileName();
	tmpDir.remove(tmpFileName);
        bool success = false;
        QString message;

	if (!fileSave( tmpDir.absoluteFilePath(tmpFileName) ))
	{
		message = tr("Unable to write file \"%1\"").arg(tmpDir.absoluteFilePath(tmpFileName));
		tmpDir.remove(tmpFileName);
	}
	else
	{
		if(!QFile::remove(origFileName))
		{
			message = tr("Unable to remove original file.");
		}
		else if(!QFile::copy(tmpDir.absoluteFilePath(tmpFileName), origFileName))
		{
			message = tr("Unable to replace original file by the new file \"%1\".\nNote that the original file has been already removed.").arg(tmpDir.absoluteFilePath(tmpFileName));
		}
		else
			success = true;
	}
	SetFileName( origFileName );

        if (!success)
            QMessageBox::critical(this, tr("Saving failed"),
		    tr("Unable to save %1:\n%2").arg(origFileName).arg(message));
        return success;
    }
}

bool PoDoFoBrowser::fileSaveAs()
{
    QString filename = QFileDialog::getSaveFileName(
		    this, tr("Save As..."), QString(), tr("PDF File (*.pdf)"));
    if( !filename.isNull() )
        return fileSave( filename );
    else
        return false;
}

void PoDoFoBrowser::fileReload()
{
    if (m_filename.isEmpty()) return;
    QMessageBox::StandardButton result = QMessageBox::question(this, tr("Discard changes and re-load file?"),
            tr("Are you sure you want to discard all your changes and re-load the last saved version of this file?"),
            QMessageBox::Discard|QMessageBox::Cancel, QMessageBox::Cancel);
    if (result == QMessageBox::Discard)
    {
        const QString filename = m_filename;
        fileOpen(filename);
    }
}

void PoDoFoBrowser::fileInfo()
{
	if(m_pDocument)
	{
		PodofoInfoDialog infoDlg( m_filename , m_pDocument , this);
		infoDlg.exec();
	}
}

void PoDoFoBrowser::fileExit()
{
   if( !trySave() ) 
       return;

    this->close();
}

void PoDoFoBrowser::insertElement(int row, const QModelIndex& parent)
{
    PdfObjectModel * const model = static_cast<PdfObjectModel*>(listObjects->model());
    if (!model->insertElement(row, parent))
        qDebug("insertElement() failed!");
}

// Insert an array element before the selected item
void PoDoFoBrowser::editInsertBefore()
{
    QModelIndex idx = GetSelectedItem();
    if (!idx.isValid()) return; // shouldn't happen
    QModelIndex parent = idx.parent();
    insertElement(idx.row(), parent);
}

// Insert an array element after the selected item
void PoDoFoBrowser::editInsertAfter()
{
    QModelIndex idx = GetSelectedItem();
    if (!idx.isValid()) return; // shouldn't happen
    QModelIndex parent = idx.parent();
    insertElement(idx.row() + 1, parent);
}

// Insert a new dictionary key at the same level as the selected item
void PoDoFoBrowser::editInsertKey()
{
    QModelIndex idx = GetSelectedItem();
    if (!idx.isValid()) return; // shouldn't happen

    bool ok = false;
    QString keyName = QInputDialog::getText(
		    this, tr("Key Name"),
		    tr("Key Name (unescaped, without leading /)"),
                    QLineEdit::Normal, QString(), &ok);
    if (ok)
    {
        QModelIndex parent = idx.parent();
        PdfObjectModel * const model = static_cast<PdfObjectModel*>(listObjects->model());
        if (!model->insertKey( PdfName( keyName.toUtf8().data() ), parent))
            qDebug("editInsertKey() failed!");
    }
}

// Insert a new array element OR dictionary key as a child of the selected item.
void PoDoFoBrowser::editInsertChildBelow()
{
    QModelIndex parent = GetSelectedItem();
    if (!parent.isValid())
    {
        qDebug("%s: invalid selection", __FUNCTION__);
        return;
    }
    PdfObjectModel * const model = static_cast<PdfObjectModel*>(listObjects->model());

    if (model->IndexIsDictionary(parent))
    {
        bool ok = false;
        QString keyName = QInputDialog::getText(
			this, tr("Key Name"),
			tr("Key Name (unescaped, without leading /)"),
                        QLineEdit::Normal, QString(), &ok);
        if (ok)
        {
            model->insertKey( PdfName( keyName.toUtf8().data() ), parent);
        }
    }
    else if (model->IndexIsArray(parent))
        model->insertElement( model->IndexChildCount(parent), parent );
    else
        qDebug("editInsertChildBelow() on item that's not a multivalued container");
}

void PoDoFoBrowser::editRemoveItem()
{
    QModelIndex sel = GetSelectedItem();
    if (!sel.isValid())
    {
        qDebug("%s: invalid selection", __FUNCTION__);
        return;
    }
    PdfObjectModel * const model = static_cast<PdfObjectModel*>(listObjects->model());

    model->deleteIndex(sel);
}

void PoDoFoBrowser::editCreateNewObject()
{
    QModelIndex sel = GetSelectedItem();
    if (!sel.isValid())
    {
        qDebug("%s: invalid selection", __FUNCTION__);
        return;
    }
    PdfObjectModel * const model = static_cast<PdfObjectModel*>(listObjects->model());

    model->createNewObject(sel);
}

void PoDoFoBrowser::editFind()
{
    // TODO: Wrap this dialog into a nice interface
    Ui::PoDoFoFindDlg dlgui;
    QDialog dlg;
    dlgui.setupUi( &dlg );

    if( dlg.exec() == QDialog::Accepted ) 
    {
        m_bHasFindText = true;
        m_sFindText    = dlgui.comboBoxText->currentText();

        m_findFlags    = 0;
        if( dlgui.checkBoxCaseSensitive->isChecked() )
            m_findFlags |= QTextDocument::FindCaseSensitively;

        if( dlgui.checkBoxWholeWords->isChecked() )
            m_findFlags |= QTextDocument::FindWholeWords;

        if( dlgui.checkBoxFindBackwards->isChecked() )
            editFindPrevious();
        else
            editFindNext();

        UpdateMenus(); // to enable Find Next and Find Previous
    }
}

void PoDoFoBrowser::editFindNext()
{
    if( !textStream->find( m_sFindText, m_findFlags ) )
        QMessageBox::warning( this, tr("Find Next"), tr("The Text \"%1\" could not be found!").arg( m_sFindText ) );

    // TODO: continue at top
}

void PoDoFoBrowser::editFindPrevious()
{
    int  cursorPos = textStream->textCursor().position();
    bool bFound = textStream->find( m_sFindText, m_findFlags | QTextDocument::FindBackward );
    
    // we most likely found the previous text again:
    // so move the cursor before the selection and search again
    if( bFound && cursorPos == textStream->textCursor().position() && textStream->textCursor().hasSelection() )
    {
        QTextCursor cursor = textStream->textCursor();
        cursor.setPosition( cursor.selectionStart() );
        textStream->setTextCursor( cursor );

        bFound = textStream->find( m_sFindText, m_findFlags | QTextDocument::FindBackward );
    }

    if( !bFound )
        QMessageBox::warning( this, tr("Find Previous"), tr("The Text \"%1\" could not be found!").arg( m_sFindText ) );

    // TODO: continue at bottom
}

void PoDoFoBrowser::editReplace()
{
    Ui::PoDoFoReplaceDlg dlgui;
    QDialog dlg;
    dlgui.setupUi( &dlg );

    if( dlg.exec() == QDialog::Accepted ) 
    {
        QString sFind     = dlgui.comboBoxText->currentText();
        QString sReplace  = dlgui.comboBoxReplace->currentText();

        QTextDocument::FindFlags findFlags = 0;

        if( dlgui.checkBoxCaseSensitive->isChecked() )
            findFlags |= QTextDocument::FindCaseSensitively;

        if( dlgui.checkBoxWholeWords->isChecked() )
            findFlags |= QTextDocument::FindWholeWords;

        bool bFromCursor = dlgui.checkBoxFromCursor->isChecked();
        bool bBackwards  = dlgui.checkBoxFindBackwards->isChecked();
        bool bSelected   = dlgui.checkBoxSelectedText->isChecked();
        bool bPrompt     = dlgui.checkBoxPromptOnReplace->isChecked();
        Q_UNUSED(bFromCursor);
        Q_UNUSED(bSelected);

        int nCount = 0;

        while( true ) 
        {
            // Find the text
            // TODO: Reg exp
            bool bFound;
            if( !bBackwards ) 
                bFound = textStream->find( sFind, findFlags );
            else
            {
                int cursorPos = textStream->textCursor().position();
                bFound = textStream->find( sFind, findFlags | QTextDocument::FindBackward );
    
                // we most likely found the previous text again:
                // so move the cursor before the selection and search again
                if( bFound && cursorPos == textStream->textCursor().position() && textStream->textCursor().hasSelection() )
                {
                    QTextCursor cursor = textStream->textCursor();
                    cursor.setPosition( cursor.selectionStart() );
                    textStream->setTextCursor( cursor );

                    bFound = textStream->find( sFind, findFlags | QTextDocument::FindBackward );
                }
            }

            if( !bFound )
            {
                if( !nCount )
                    QMessageBox::warning( this, tr("Replace"), tr("The Text \"%1\" could not be found!").arg( sFind ) );
                break;
            }

            // Select the text
            textStream->textCursor().movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, sFind.length());
            
            // Prompt
            bool bPerformReplace = true;
            if( bPrompt ) 
            {
                QMessageBox msgBox;
                msgBox.setWindowTitle( tr("Replace confirmantion") );
                msgBox.setText( tr("Found an occurence of your search term. What do you want to do?") );
                QPushButton* buttonReplace    = msgBox.addButton(tr("Replace"), QMessageBox::ActionRole);
                QPushButton* buttonReplaceAll = msgBox.addButton(tr("Replace All"), QMessageBox::ActionRole);
                QPushButton* buttonFindNext   = msgBox.addButton(tr("Find Next"), QMessageBox::ActionRole);
                QPushButton* buttonClose      = msgBox.addButton(tr("Close"), QMessageBox::ActionRole);

                msgBox.exec();

                if (msgBox.clickedButton() == buttonReplace ) 
                {
                    bPerformReplace = true;
                } 
                else if (msgBox.clickedButton() == buttonReplaceAll) 
                {
                    bPrompt = false;
                    bPerformReplace = true;
                }
                else if (msgBox.clickedButton() == buttonFindNext) 
                {
                    bPerformReplace = false;
                }
                else if (msgBox.clickedButton() == buttonClose) 
                {
                    bPerformReplace = false;
                    break;
                }
            }

            if( bPerformReplace ) 
            {
                ++nCount;
                textStream->textCursor().removeSelectedText();
                textStream->textCursor().insertText( sReplace );
            }
        }
        

        QMessageBox::information( this, tr("Replace"), tr("Replaced %1 occurences of \"%2\".").arg( nCount ).arg( sFind ) );
    }
}

void PoDoFoBrowser::editGotoObject()
{
    QDialog dlg( this );
    Ui::PoDoFoGotoDlg dlgUi;
    dlgUi.setupUi( &dlg );

    dlgUi.spinObjectNumber->setValue( m_gotoReference.ObjectNumber() );
    dlgUi.spinGenerationNumber->setValue( m_gotoReference.GenerationNumber() );
    dlgUi.spinObjectNumber->selectAll();

    if( dlg.exec() == QDialog::Accepted ) 
    {
        m_gotoReference = PoDoFo::PdfReference( dlgUi.spinObjectNumber->value(), 
                                                dlgUi.spinGenerationNumber->value() );

        this->GotoObject();
    }
}

void PoDoFoBrowser::editGotoPage()
{
    QDialog dlg( this );
    Ui::PoDoFoGotoPageDlg dlgUi;
    dlgUi.setupUi( &dlg );

    dlgUi.spinPage->setValue( 0 );
    dlgUi.spinPage->setMinimum( 1 );
    dlgUi.spinPage->setMaximum( m_pDocument->GetPageCount() );
    dlgUi.spinPage->selectAll();

    if( dlg.exec() == QDialog::Accepted ) 
    {
        PdfPage* pPage = m_pDocument->GetPage( dlgUi.spinPage->value() - 1 );
        if( pPage ) 
        {
            m_gotoReference = pPage->GetObject()->Reference();

            this->GotoObject();
        }
    }
}

// For debugging: refresh the view
void PoDoFoBrowser::viewRefreshView()
{
    PdfObjectModel * const model = static_cast<PdfObjectModel*>(listObjects->model());
    if (!model)
        qDebug("can't refresh with no model");

    ModelChange(new PdfObjectModel(m_pDocument, listObjects, actionCatalogView->isChecked()));
}

void PoDoFoBrowser::slotImportStream()
{
    QModelIndex idx = GetSelectedItem();
    if (!idx.isValid()) return; // shouldn't happen

    PdfObjectModel * const model = static_cast<PdfObjectModel*>(listObjects->model());

    // TODO: if the stream is a file stream, convert it to a mem
    // stream while retaining all dictionary attributes.
    const PdfObject * obj = model->GetObjectForIndex(idx);
    if (!obj->HasStream())
        return;
    // dodgy!
    PdfStream * stream = const_cast<PdfObject*>(obj)->GetStream();

    QString fn = QFileDialog::getOpenFileName(this,
            tr("Import stream")
            );
    if (fn.isEmpty())
        return;

    QFile f(fn);
    if (!f.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical( this, tr("Saving failed"), QString( tr("Cannot open file %1 for reading.") ).arg( fn ) );
        return;
    }

    // Load the stream in streamChunkSize byte chunks.
    static const qint64 streamChunkSize=1024*64;
    char* pBuf = static_cast<char*>(malloc( streamChunkSize*sizeof(char) ));

    qint64 bytesRead = 0;
    try {
	// Clear the stream and begin appending, with all data being encoded according
	// to the current stream dictionary's filters.
        stream->BeginAppend( PdfFilterFactory::CreateFilterList(obj), true );
        do
        {
            bytesRead = f.read(pBuf, streamChunkSize);
            assert(bytesRead != -1); // error not handled properly
            if (bytesRead > 0)
                stream->Append(pBuf, bytesRead);
        }
        while (bytesRead > 0);
	stream->EndAppend();
    } catch (PdfError& e) {
        free(pBuf);
        podofoError( e );
        return;
    }
    free(pBuf);

    statusBar()->showMessage( tr("Stream imported from %1").arg( fn ), 2000 );

    // TODO: refresh stream
}

void PoDoFoBrowser::slotExportStream()
{
    QModelIndex idx = GetSelectedItem();
    if (!idx.isValid()) return; // shouldn't happen

    PdfObjectModel * const model = static_cast<PdfObjectModel*>(listObjects->model());

    // TODO: if the stream is a file stream, convert it to a mem
    // stream while retaining all dictionary attributes.
    const PdfObject * obj = model->GetObjectForIndex(idx);
    if (!obj->HasStream())
        return;

    QString fn = QFileDialog::getSaveFileName(this,
            tr("Import stream")
            );
    if (fn.isEmpty())
        return;

    // TODO: progressive reading of stream
    char* pBuf = 0;
    pdf_long lLen = 0;
    try {
        obj->GetStream()->GetFilteredCopy( &pBuf, &lLen );
    } catch( PdfError & e ) {
        podofoError( e );
        return;
    }

    QFile f(fn);
    if (!f.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical( this, tr("Saving failed"), QString( tr("Cannot open file %1 for writing.") ).arg( fn ) );
        return;
    }

    qint64 bytesWritten = f.write(pBuf, lLen);
    free(pBuf);
    f.close();
    if (bytesWritten != lLen)
    {
        QMessageBox::critical( this, tr("Saving failed"), QString( tr("Wrote only partial stream contents") ).arg( fn ) );
        return;
    }

    statusBar()->showMessage( tr("Stream exported to %1").arg( fn ), 2000 );
}

void PoDoFoBrowser::toolsToHex()
{
    std::unique_ptr<PdfFilter> hexfilter = std::unique_ptr<PdfFilter>(PdfFilterFactory::Create(ePdfFilter_ASCIIHexDecode));

    char* pBuffer = NULL;
    pdf_long  lLen    = 0;
    QString text  = QInputDialog::getText(
			    this, tr("PoDoFoBrowser: To Hex"),
			    tr("Please input a string (7-bit ASCII only):") );

    if( !text.isEmpty() )
    {
        try {
	    // FIXME XXX TODO big bad encoding assumption
            hexfilter->Encode( text.toLatin1().data(), text.length(), &pBuffer, &lLen );
        } catch( PdfError & e ) {
            podofoError( e );
            return;
        }

	// This ::fromAscii is safe since we're dealing with hex strings only.
        text = QString::fromLatin1( pBuffer, lLen );
        QMessageBox::information( this, tr("PoDoFoBrowser"), tr("The string converted to hex:<br>") + text );
    }
}

void PoDoFoBrowser::toolsFromHex()
{
    std::unique_ptr<PdfFilter> hexfilter = std::unique_ptr<PdfFilter>(PdfFilterFactory::Create(ePdfFilter_ASCIIHexDecode));

    char* pBuffer = NULL;
    pdf_long  lLen    = 0;
    QString text  = QInputDialog::getText(
		    		this, tr("PoDoFoBrowser: From Hex"),
				tr("Please input a hex string:") );

    if( !text.isEmpty() )
    {
        try {
            // This encoding assumption should be safe since we're reading hex strings
            hexfilter->Decode( text.toLatin1(), text.length(), &pBuffer, &lLen );
        } catch( PdfError & e ) {
            podofoError( e );
            return;
        }

	// FIXME XXX TODO big bad encoding assumption
        text = QString::fromLatin1( pBuffer, lLen );
        QMessageBox::information( this, tr("PoDoFoBrowser"), tr("The string converted to ascii:<br>") + text );
    }
}

void PoDoFoBrowser::toolsDisplayCodeForSelection()
{
    QModelIndex idx = GetSelectedItem();
    if (!idx.isValid())
        return;
    PdfObjectModel* model = static_cast<PdfObjectModel*>(listObjects->model());
    const PdfObject* obj = model->GetObjectForIndex(idx);
    std::string s;
    obj->ToString(s);
    QMessageBox::information(this,
		    tr("PDF code for selection"),
            QString::fromLatin1( s.c_str() ) );
}

void PoDoFoBrowser::helpAbout()
{
    QDialog* dlg = new QDialog( this );
    Ui::PodofoAboutDlg dlgUi;
    dlgUi.setupUi(dlg);
    dlg->show();
}

bool PoDoFoBrowser::trySave() 
{
   PdfObjectModel* model = static_cast<PdfObjectModel*>(listObjects->model());
   if( model && model->DocChanged() ) 
   {
       int m = QMessageBox::question( this, tr("File changed"), QString( tr("The file %1 was changed. Do you want to save it?") ).arg( m_filename ), 
                                  QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel );
       
       if( m == QMessageBox::Cancel )
           return false;
       else if( m == QMessageBox::Yes ) 
       {
           if( m_filename.isEmpty() )
               return fileSaveAs();
           else
               fileSave( m_filename );
       }
       else 
           return true;
   }

   return true;
}

void PoDoFoBrowser::GotoObject()
{
    PdfObjectModel * const model = static_cast<PdfObjectModel*>(listObjects->model());

    int index = model->FindObject( m_gotoReference );
    if( index == -1 )
        QMessageBox::warning( this, tr("Object not found"), 
                              tr("The object \"%1 %2\" could not be found!").arg( 
                                  m_gotoReference.ObjectNumber() ).arg( 
                                      m_gotoReference.GenerationNumber() ) );
    else
        listObjects->setCurrentIndex( model->index( index, 0 ) );
}


void PoDoFoBrowser::slotSetStreamEditable(bool e)
{
	if(e)
	{
		textStream->setReadOnly(false);
		textStream->setTextInteractionFlags(Qt::TextEditorInteraction);
		commitButton->setEnabled(true);
	}
	else
	{
		textStream->setReadOnly(true);
		textStream->setTextInteractionFlags(Qt::TextBrowserInteraction);
		commitButton->setEnabled(false);
	}
}

void PoDoFoBrowser::slotCommitStream()
{
	QModelIndex idx = GetSelectedItem();
	if (!idx.isValid())
	{
	    return; // shouldn't happen
	}


	PdfObjectModel * const model = static_cast<PdfObjectModel*>(listObjects->model());

	const PdfObject * obj = model->GetObjectForIndex(idx);
	if (!obj->HasStream())
	{
		return;
	}

	QByteArray cs(m_codecForStream->fromUnicode(textStream->toPlainText()));
	PdfStream * stream = const_cast<PdfObject*>(obj)->GetStream();
	stream->Set(cs.data(), cs.size());
	statusBar()->showMessage( tr("Stream Committed"), 2000 );

}


