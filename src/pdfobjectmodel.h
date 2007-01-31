#ifndef PDFOBJECTMODEL_H
#define PDFOBJECTMODEL_H

#include <utility>

#include <QAbstractTableModel>
#include <QMap>

namespace PoDoFo {
        class PdfDocument;
        class PdfObject;
};

/*
 * A Qt model to represent the PDF's top-level indirect object
 * tree and all child dictionaries of it.
 *
 * Doc: model-view-model.html
 *
 * Each indirect object is a top-level item on a distinct row.
 * Any contained dictionaries are in a child table, again
 * with one row per dictionary.
 *
 * Each table (top level and child tables) has the following columns:
 *
 * 1: Identifier.
 * 	- Object number and generation number for indirect objects
 * 	- Name of key in parent dictionary for children of a dictionary
 * 	- ??? for children of an array
 * 2: The value of the /Type key if object is a dictionary
 * 3: Any other useful identifying information about the object
 *
 * All are Qt::DisplayRole roles with string values.
 */
class PdfObjectModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    PdfObjectModel(PoDoFo::PdfDocument* doc, QObject* parent = 0);
    virtual ~PdfObjectModel();

    virtual QVariant data(const QModelIndex& index, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column,
                              const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

    // Return the object associed with `index'
    const PoDoFo::PdfObject* GetObjectForIndex(const QModelIndex & index) const;

    // Invalidate the children of `index' so that they're re-loaded next time
    // they're needed. You MUST call this before modifying the children of an object.
    void InvalidateChildren(const QModelIndex & index);

private:
    void setupModelData(PoDoFo::PdfDocument* doc);

    // PdfObjectModelTree instance for the model - stored as void* to avoid
    // exposing anonymous namespace types.
    void * m_pTree;
};

#endif
