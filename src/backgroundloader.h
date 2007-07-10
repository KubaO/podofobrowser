#ifndef PODOFOBROWSER_BACKGROUNDLOADER_H
#define PODOFOBROWSER_BACKGROUNDLOADER_H

#include <QObject>
#include <QTimer>

namespace PoDoFo {
    class PdfDocument;
    class PdfObject;
}

class BackgroundLoader : public QObject
{
    Q_OBJECT

public:
    BackgroundLoader(PoDoFo::PdfDocument* doc);

    virtual ~BackgroundLoader();


public slots:
    void loadNextObject();
    void start();

signals:
    // Progress loading from 0 to number of objects
    void progress(int);
    void done();

private:
    QTimer m_timer;
    // We use an integer index into the vector because it doesn't actually matter
    // if we miss a few items due to insertions/deletions. On the other hand, using
    // an invalid iterator could be _BAD_.
    int m_lastObjectIdx;
    // Document
    PoDoFo::PdfDocument* m_pDoc;
};

#endif
