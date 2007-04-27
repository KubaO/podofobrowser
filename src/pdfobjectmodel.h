#ifndef PDFOBJECTMODEL_H
#define PDFOBJECTMODEL_H

#include <utility>

#include <QAbstractTableModel>
#include <QMap>

#include <podofo/podofo.h>

namespace PoDoFo {
    class PdfDocument;
    class PdfObject;
    class PdfName;
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

    /**
     * Enumerator that identifies the columns in the model.
     */
    enum Column
    {
        Column_ParentIdentifier = 0,
        Column_RawValue = 1,
        Column_Type = 2
    };

    PdfObjectModel(PoDoFo::PdfDocument* doc, QObject* parent = 0, bool catalogRooted = true);
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

    // Find the object `ref' and return its index or -1
    int FindObject( const PoDoFo::PdfReference & ref );

    // Convenience wrappers around some really common node/PdfObject functions
    inline bool IndexIsDictionary(const QModelIndex & index) const;
    inline bool IndexIsArray(const QModelIndex & index) const;
    inline bool IndexIsReference(const QModelIndex & index) const;
    int IndexChildCount(const QModelIndex & index) const;

    // Invalidate the children of `index' so that they're re-loaded next time
    // they're needed. You MUST call this before modifying the children of an object.
    void InvalidateChildren(const QModelIndex & index);

    // These perhaps shouldn't really be public as they're only necessary
    // where the tree is modified, and only the model should be doing that.
    void PrepareForSubtreeChange(const QModelIndex& index);
    void SubtreeChanged(const QModelIndex& index);

    // Insert a dictionary key into the PDF document.
    // The key will be named `keyName' and be inserted into the dictionary
    // at `parent'. If false is returned the change was not completed
    // successfully.
    bool insertKey(const PoDoFo::PdfName& keyName, const QModelIndex & parent );

    // Insert an array element into the array at `parent'.
    // The element is inserted before `row' such that the new
    // element ends up with row number `row'. If the change
    // could not be completed false is returned.
    bool insertElement( int row, const QModelIndex & parent );

    // Delete the direct object at `index'. Trying to delete an indirect object
    // will fail and return false - you must instead remove the reference(s) to
    // it.
    bool deleteIndex(const QModelIndex & index);

    // Create a new indirect object and replace the index `sel' with a new
    // indirect reference to it.
    bool createNewObject(const QModelIndex & index);

    /** \return true iff the document has changed */
    bool DocChanged() const throw() { return m_bDocChanged; }

private:
    // have any changes been made to the document tree through the model?
    bool m_bDocChanged;

    void setupModelData_CatalogRooted(PoDoFo::PdfDocument* doc);
    void setupModelData_IndirectRooted(PoDoFo::PdfDocument* doc);

    // PdfObjectModelTree instance for the model
    void * m_pTree;

    // This inherited method explicitly fails
    virtual bool insertRow(int,const QModelIndex&);
};

bool PdfObjectModel::IndexIsDictionary(const QModelIndex & index) const
{
    return GetObjectForIndex(index)->IsDictionary();
}

bool PdfObjectModel::IndexIsArray(const QModelIndex & index) const
{
    return GetObjectForIndex(index)->IsArray();
}

bool PdfObjectModel::IndexIsReference(const QModelIndex & index) const
{
    return GetObjectForIndex(index)->IsReference();
}

#endif
