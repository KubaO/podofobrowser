#ifndef PODOFOBROWSER_UTIL_H
#define PODOFOBROWSER_UTIL_H

namespace PoDoFo {
    class PdfError;
    class PdfObject;
};

void podofoError( const PoDoFo::PdfError & eCode );
void printObject( const PoDoFo::PdfObject* obj );

#endif
