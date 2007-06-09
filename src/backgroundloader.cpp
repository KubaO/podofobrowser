#include "backgroundloader.h"

#include <podofo/podofo.h>
using namespace PoDoFo;

#include <iostream>

BackgroundLoader::BackgroundLoader(PdfDocument* doc)
    : m_timer(),
      m_lastObjectIdx(-1),
      m_pDoc(doc)
{
    connect(&m_timer, SIGNAL(timeout()), SLOT(loadNextObject()));
    qDebug("Beginning background load of %i objects", doc->GetObjects().GetSize());
}

BackgroundLoader::~BackgroundLoader()
{
}

void BackgroundLoader::start()
{
    m_timer.start();
}

void BackgroundLoader::loadNextObject()
{
    // FIXME: We should not have to cast away constness in pdfvecobjects
    // just to index its members.
    PdfVecObjects & objs = const_cast<PdfVecObjects&>(m_pDoc->GetObjects());
    const int objCount = objs.GetSize();

    if (m_lastObjectIdx + 1 >= objCount)
    {
        m_timer.stop();
        emit progress(objCount);
        emit done();
        return;
    }
    else
    {
        // XXX no podofo support for directly forcing delayed load
	// FIXME unsafe if objects are removed while loading happening.
        objs[++m_lastObjectIdx]->GetDataType();
        emit progress(m_lastObjectIdx);
    }
}
