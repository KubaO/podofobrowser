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
#include "podofoutil.h"
#include "ui_podofoaboutdlg.h"
#include "pdfobjectmodel.h"

#include <qapplication.h>
#include <qcursor.h>
#include <q3filedialog.h>
#include <qinputdialog.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <q3progressdialog.h> 
#include <qsettings.h>
#include <qsplitter.h>
#include <qstatusbar.h>
#include <q3table.h>
#include <q3textedit.h>
//Added by qt3to4:
#include <Q3ValueList>

#include <cassert>

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

PoDoFoBrowser::PoDoFoBrowser()
    : Q3MainWindow(0, "PoDoFoBrowser", Qt::WDestructiveClose ),
      PoDoFoBrowserBase(),
      m_pDocument( NULL )
{
    setupUi(this);

    clear();

    connect( buttonImport, SIGNAL( clicked() ), this, SLOT( slotImportStream() ) );
    connect( buttonExport, SIGNAL( clicked() ), this, SLOT( slotExportStream() ) );
    connect( actionInsert_Before, SIGNAL( activated() ), this, SLOT( editInsertBefore() ) );
    connect( actionInsert_After,  SIGNAL( activated() ), this, SLOT( editInsertAfter() ) );
    connect( actionInsert_Key,    SIGNAL( activated() ), this, SLOT( editInsertKey() ) );
    connect( actionInsert_Child,  SIGNAL( activated() ), this, SLOT( editInsertChildBelow() ) );
    connect( actionRemove_Item,   SIGNAL( activated() ), this, SLOT( editRemoveItem()) );
    connect( actionCreate_Missing_Object, SIGNAL( activated() ), this, SLOT( editCreateMissingObject()) );

    show();
    statusBar()->message( tr("Ready"), 2000 );

    loadConfig();
    parseCmdLineArgs();
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
    delete oldModel; oldModel = 0;
}

PoDoFoBrowser::~PoDoFoBrowser()
{
    saveConfig();

    delete m_pDocument;
}

void PoDoFoBrowser::loadConfig()
{
    int w,h;

    Q3ValueList<int> list;
    QSettings       settings;

    settings.setPath( "podofo.sf.net", "podofobrowser" );

    w = settings.readNumEntry("/geometry/width", width() );
    h = settings.readNumEntry("/geometry/height", height() );

    list << width()/3;
    list << (width()/3 * 2);

    splitter2->setSizes( list );
    splitter3->setSizes( list );

    this->resize( w, h );
}

void PoDoFoBrowser::saveConfig()
{
    QSettings settings;

    settings.setPath( "podofo.sf.net", "podofobrowser" );

    settings.writeEntry("/geometry/width", width() );
    settings.writeEntry("/geometry/height", height() );
}

void PoDoFoBrowser::parseCmdLineArgs()
{
    if( qApp->argc() >= 2 )
    {
        fileOpen( QString( qApp->argv()[1] ) );
    }
}

void PoDoFoBrowser::clear()
{
    m_filename = QString::null;
    setCaption( "PoDoFoBrowser" );

    labelStream->setText("");
    textStream->clear();
    buttonImport->setEnabled( false );
    buttonExport->setEnabled( false );
    textStream->setEnabled(false);
    
    ModelChange(NULL);

    delete m_pDocument; m_pDocument=0;

    m_pCurObject      = NULL;
    m_lastItem        = NULL;
    m_pDocument       = NULL;
}

void PoDoFoBrowser::fileNew()
{
    PdfError         eCode;

    if( !trySave() ) 
        return;

    this->clear();

    m_pDocument = new PdfDocument();

    ModelChange( new PdfObjectModel(m_pDocument, listObjects) );
}

void PoDoFoBrowser::fileOpen( const QString & filename )
{
    QStringList      lst;

    clear();

    QApplication::setOverrideCursor( Qt::WaitCursor );

    // TODO: leaking old document?
    try {
        m_pDocument = new PdfDocument( filename.latin1() );
    } catch( PdfError & e ) {
        QApplication::restoreOverrideCursor();
        podofoError( e );
        return;
    }

    ModelChange( new PdfObjectModel(m_pDocument, listObjects) );
    
    m_filename = filename;
    setCaption( m_filename );

    QApplication::restoreOverrideCursor();
    statusBar()->message(  QString( tr("Loaded file %1 successfully") ).arg( filename ), 2000 );
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
    statusBar()->message(  QString( tr("Wrote file %1 successfully") ).arg( filename ), 2000 );

    m_filename = filename;
    setCaption( m_filename );

    return true;
}

void PoDoFoBrowser::UpdateMenus()
{
    PdfObjectModel * const model = static_cast<PdfObjectModel*>(listObjects->model());
    QModelIndex sel = GetSelectedItem();

    if (!sel.isValid())
    {
    }

    // Can add a child to any array or dictionary
    actionInsert_Child->setEnabled( sel.isValid() && (model->IndexIsDictionary(sel) || model->IndexIsArray(sel)) );
    // Can create missing child only of a dangling reference
    actionCreate_Missing_Object->setEnabled( sel.isValid() && model->IndexIsReference(sel) && !model->IndexChildCount(sel) );
    
    QModelIndex parent = sel.parent();
    bool enableInsertBeforeAfter = parent.isValid() && model->IndexIsArray(parent);
    actionInsert_Before->setEnabled(enableInsertBeforeAfter);
    actionInsert_After->setEnabled(enableInsertBeforeAfter);
    actionInsert_Key->setEnabled(parent.isValid() && model->IndexIsDictionary(parent));
    actionRemove_Item->setEnabled(false); //XXX
}

// Triggered when the selected object in the list view changes
void PoDoFoBrowser::treeSelectionChanged( const QModelIndex & current, const QModelIndex & previous )
{
    UpdateMenus();

    textStream->clear();
    buttonImport->setEnabled( false );
    buttonExport->setEnabled( false );
    textStream->setEnabled(false);

    PdfObjectModel * const model = static_cast<PdfObjectModel*>(listObjects->model());
    if (!model)
    {
        labelStream->setText("No document available");
        return;
    }

    const PdfObject* object = model->GetObjectForIndex(current);
    if (!object)
    {
        labelStream->setText("No object available");
        return;
    }

    const PdfReference & ref (object->Reference());
    if (!ref.IsIndirect())
    {
        labelStream->setText("Object is not indirect so it can not have a stream");
        return;
    }

    try {
        if (!object->HasStream())
        {
            labelStream->setText( QString("Object %1 %2 obj has no stream").arg(ref.ObjectNumber()).arg(ref.GenerationNumber()) );
            return;
        }
    }
    catch ( PdfError & e ) {
        labelStream->setText("Error while testing for object stream");
        podofoError( e );
        return;
    }

    char * pBuf = NULL;
    long lLen = -1;
    model->PrepareForSubtreeChange(current);
    try {
        object->GetStream()->GetFilteredCopy( &pBuf, &lLen );
    } catch( PdfError & e ) {
        model->SubtreeChanged(current);
        labelStream->setText("Unable to filter object stream");
        podofoError( e );
        return;
    }
    model->SubtreeChanged(current);

    assert(pBuf);
    assert(lLen >= 0);

    QByteArray data;
    data.duplicate( pBuf, lLen );
    free( pBuf );
    textStream->setEnabled(true);
    // TODO: sane encoding-safe approach to binary data
    // TODO: hex editor
    textStream->setText( QString( data ) );
    labelStream->setText( QString("Stream associated with %1 %2 obj").arg(ref.ObjectNumber()).arg(ref.GenerationNumber()));
    buttonImport->setEnabled( true );
    buttonExport->setEnabled( true );
}

void PoDoFoBrowser::fileOpen()
{
   if( !trySave() ) 
       return;

    QString filename = Q3FileDialog::getOpenFileName( QString::null, tr("PDF File (*.pdf)"), this );
    if( !filename.isNull() )
        fileOpen( filename );
}

bool PoDoFoBrowser::fileSave()
{
    if( m_filename.isEmpty() )
        return fileSaveAs();
    else
        return fileSave( m_filename );
}

bool PoDoFoBrowser::fileSaveAs()
{
    QString filename = Q3FileDialog::getSaveFileName( QString::null, tr("PDF File (*.pdf)"), this );
    if( !filename.isNull() )
        return fileSave( filename );
    else
        return false;
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
    QString keyName = QInputDialog::getText(this, "Key Name", "Key Name (unescaped, without leading /)",
                                            QLineEdit::Normal, "", &ok);
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
    if (!parent.isValid()) return; // shouldn't happen
    PdfObjectModel * const model = static_cast<PdfObjectModel*>(listObjects->model());
    if (model->IndexIsDictionary(parent))
    {
        bool ok = false;
        QString keyName = QInputDialog::getText(this, "Key Name", "Key Name (unescaped, without leading /)",
                                            QLineEdit::Normal, "", &ok);
	if (ok)
            model->insertKey( PdfName( keyName.toUtf8().data() ), parent);
    }
    else if (model->IndexIsArray(parent))
	model->insertElement( model->IndexChildCount(parent), parent );
    else
	qDebug("editInsertChildBelow() on item that's not a multivalued container");
}

void PoDoFoBrowser::editRemoveItem()
{
}

void PoDoFoBrowser::editCreateMissingObject()
{
}


void PoDoFoBrowser::slotImportStream()
{
    /*
    PdfError eCode;
    char*    pBuf     = NULL;
    long     lLen     = 0;
    QString  filename;
    QFile    file;

    if( !m_pCurObject )
        return; 

    filename = Q3FileDialog::getOpenFileName( QString::null, tr("File (*)"), this );
    if( filename.isNull() )
        return;
    
    file.setName( filename );
    if( !file.open( QIODevice::ReadOnly ) )
    {
        QMessageBox::critical( this, tr("Error"), QString( tr("Cannot open file %1 for reading.") ).arg( filename ) );
        return;
    }

    pBuf = (char*)malloc( file.size() * sizeof(char) );
    if( !pBuf )
    {
        QMessageBox::critical( this, tr("Error"), tr("Not enough memory to import this stream.") );
        file.close();
        return;
    }

    lLen = file.readBlock( pBuf, file.size() );
    file.close();

    try {
        m_pCurObject->GetStream()->Set( pBuf, lLen );
    } catch( PdfError & e ) {
        podofoError( e );
        return;
    }

    m_bObjectChanged = true;;
    streamChanged( m_pCurObject );
    statusBar()->message( QString( tr("Stream imported from %1") ).arg( filename ), 2000 );    
    */
}

void PoDoFoBrowser::slotExportStream()
{
    /*
    PdfError eCode;
    char*    pBuf     = NULL;
    long     lLen     = 0;
    QString  filename;
    QFile    file;

    if( !m_pCurObject )
        return; 

    filename = Q3FileDialog::getSaveFileName( QString::null, tr("File (*)"), this );
    if( filename.isNull() )
        return;

    try {
        m_pCurObject->GetStream()->GetFilteredCopy( &pBuf, &lLen );    
    } catch( PdfError & e ) {
        podofoError( e );
        return;
    }

    file.setName( filename );
    if( !file.open( QIODevice::WriteOnly ) )
    {
        QMessageBox::critical( this, tr("Error"), QString( tr("Cannot open file %1 for writing.") ).arg( filename ) );
        return;
    }
    file.writeBlock( pBuf, lLen );
    file.close();

    statusBar()->message( QString( tr("Stream exported to %1") ).arg( filename ), 2000 );    
    */
}

void PoDoFoBrowser::toolsToHex()
{
    const PdfFilter * const hexfilter = PdfFilterFactory::Create(ePdfFilter_ASCIIHexDecode);

    char* pBuffer = NULL;
    long  lLen    = 0;
    QString text  = QInputDialog::getText( tr("PoDoFoBrowser"), tr("Please input a string:") );

    if( QString::null != text ) 
    {
        try {
            hexfilter->Encode( text.latin1(), text.length(), &pBuffer, &lLen );
        } catch( PdfError & e ) {
            podofoError( e );
            return;
        }

        text.setLatin1( pBuffer, lLen );
        QMessageBox::information( this, tr("PoDoFoBrowser"), tr("The string converted to hex:<br>") + text );
    }
}

void PoDoFoBrowser::toolsFromHex()
{
    const PdfFilter * const hexfilter = PdfFilterFactory::Create(ePdfFilter_ASCIIHexDecode);

    char* pBuffer = NULL;
    long  lLen    = 0;
    QString text  = QInputDialog::getText( tr("PoDoFoBrowser"), tr("Please input a hex string:") );

    if( QString::null != text ) 
    {
        try {
            hexfilter->Decode( text.latin1(), text.length(), &pBuffer, &lLen );
        } catch( PdfError & e ) {
            podofoError( e );
            return;
        }

        text.setLatin1( pBuffer, lLen );
        QMessageBox::information( this, tr("PoDoFoBrowser"), tr("The string converted to ascii:<br>") + text );
    }
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
