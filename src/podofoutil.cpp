#include "podofoutil.h"
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

    QString msg = QString( "PoDoFoBrowser encounter an error.\nError: %1 %2\n%3\n" ).arg( eCode.GetError() ).arg( pszName ? pszName : "" ).arg( pszMsg ? pszMsg : "" );

    if( eCode.GetCallstack().size() )
        msg += "Callstack:\n";

    while( it != eCode.GetCallstack().end() )
    {
        if( !(*it).GetFilename().empty() )
            msg += QString("\t#%1 Error Source: %2:%3\n").arg( i ).arg( (*it).GetFilename().c_str() ).arg( (*it).GetLine() );

        if( !(*it).GetInformation().empty() )
            msg += QString("\t\tInformation: %1\n").arg( (*it).GetInformation().c_str() );
        
        ++i;
        ++it;
    }
    
    QMessageBox::warning( 0, "Error", msg );
}
