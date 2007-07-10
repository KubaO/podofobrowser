#include "podofoutil.h"
#include <string>
#include <iostream>
#include <podofo/podofo.h>
#include <QtCore>
#include <QtGui>
using namespace PoDoFo;

void podofoError( const PdfError & eCode ) 
{
    TCIDequeErrorInfo it = eCode.GetCallstack().begin();
    const char* pszMsg   = PdfError::ErrorMessage( eCode.GetError() );
    const char* pszName  = PdfError::ErrorName( eCode.GetError() );

    int         i        = 0;

    QString msg = QObject::tr("PoDoFoBrowser encounter an error.\n"
		              "Error: %1 %2\n%3\n" )
	    .arg( eCode.GetError() )
	    .arg( QString::fromLocal8Bit(pszName ? pszName : "") )
	    .arg( QString::fromLocal8Bit(pszMsg ? pszMsg : "") );

    if( eCode.GetCallstack().size() )
        msg += QObject::tr("Callstack:\n");

    while( it != eCode.GetCallstack().end() )
    {
        if( !(*it).GetFilename().empty() )
            msg += QObject::tr("\t#%1 Error Source: %2:%3\n")
		    .arg( i )
		    .arg( QString::fromLocal8Bit((*it).GetFilename().c_str()) )
		    .arg( (*it).GetLine() );

        if( !(*it).GetInformation().empty() )
            msg += QObject::tr("\t\tInformation: %1\n")
		    .arg( QString::fromLocal8Bit((*it).GetInformation().c_str()) );
        
        ++i;
        ++it;
    }
    
    QMessageBox::warning( 0, QObject::tr("Error"), msg );
}

void printObject( const PdfObject* obj )
{
    std::cerr << "Printing object: 0x" << obj << std::endl;
    std::string s;
    obj->ToString(s);
    std::cerr << s << std::endl;
}
