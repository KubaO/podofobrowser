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
    qDebug("Beginning background load of %i objects", doc->GetObjects().size());
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
    const std::vector<PdfObject*> objs = m_pDoc->GetObjects();
    const int objCount = objs.size();

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
        objs[++m_lastObjectIdx]->GetDataType();
        emit progress(m_lastObjectIdx);
    }
}
