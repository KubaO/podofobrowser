/*
Copyright (C) 2006 Evan Teran
                   eteran@alum.rit.edu

Modified by Craig Ringer to use a QIODevice interface rather than a
pre-allocated byte array, and to operate on a view of the target data
rather than owning a copy of the data.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#ifndef QHEXVIEW_20060506_H_
#define QHEXVIEW_20060506_H_

#include <QAbstractScrollArea>
#include <QByteArray>
#include <QMap>
#include <QString>
#include <QVector>

class QMenu;
class ByteStream;
class CommentServerInterface;

class QHexView : public QAbstractScrollArea {
	Q_OBJECT
	
public:
#if QT_POINTER_SIZE == 4
	typedef uint32_t address_t;
#elif QT_POINTER_SIZE == 8
	typedef uint64_t address_t;
#endif // QT_POINTER_SIZE	
public:
	QHexView(QWidget * parent = 0);
	
public:
	void setCommentServer(CommentServerInterface *p);
	CommentServerInterface * commentServer() const;
		
protected:
	virtual void paintEvent(QPaintEvent * event);
	virtual void resizeEvent(QResizeEvent * event);
	virtual void mousePressEvent(QMouseEvent * event);
	virtual void mouseMoveEvent(QMouseEvent * event);
	virtual void mouseReleaseEvent(QMouseEvent * event);
	virtual void keyPressEvent(QKeyEvent * event);
	virtual void mouseDoubleClickEvent(QMouseEvent * event);
	virtual void contextMenuEvent(QContextMenuEvent * event);

public slots:
	void setShowAddress(bool);
	void setShowAsciiDump(bool);
	void setShowHexDump(bool);
	void setShowComments(bool);
	//void setLineColor(QColor);
	//void setAddressColor(QColor);
	void setWordWidth(int);
	void setRowWidth(int);
	void setFont(const QFont &font);
	void setShowAddressSeparator(bool value);
	void repaint();
		
public:
	address_t addressOffset() const;
	bool showHexDump() const;
	bool showAddress() const;
	bool showAsciiDump() const;
	bool showComments() const;
	QColor lineColor() const;
	QColor addressColor() const;
	int wordWidth() const;
	int rowWidth() const;
	
private:
	int m_RowWidth;			// amount of "words" per row
	int m_WordWidth;		// size of a "word" in bytes
	QColor m_AddressColor;	// colour of the address in display
	bool m_ShowHex;			// should we show the hex display?
	bool m_ShowAscii;		// should we show the ascii display?
	bool m_ShowAddress;		// should we show the address display?
	bool m_ShowComments;

	QIODevice * m_io;		// The data stream. We do not own this object
					// and must never destroy it.
					
	size_t m_dataSize;		// Size of data source being operated on.

	mutable QByteArray m_buf;	// The current data segment being operated on.
					// For a random I/O device it's just the visible
					// page; for a sequential I/O device it's everything
					// up to the current point.
	
	mutable size_t m_bufOffset;	// Offset in bytes of the buffer into the data.

public:
	/**
	 * Operate on the I/O device `d', which must remain valid until this
	 * object is destroyed, setData(...) is called with a different
	 * target, or clear() is called.
	 *
	 * The second parameter, s, is the total size in bytes of the available data.
	 * For a random I/O stream you may just pass size() ; for a sequential stream
	 * you'll have to derive it some other way.
	 *
	 * The target device must already be open.
	 *
	 * If the target device is not seekable, the hex editor widget may
	 * use quite a bit of memory or attempt to use a temp file to store
	 * the data.
	 */
	void setData(QIODevice* d, size_t s);

	/**
	 * Release all references to the current data being accessed and reset the
	 * viewer to empty.
	 */
	inline void clear() { setData(0,0); }

	void setAddressOffset(address_t offset);
	void scrollTo(unsigned int offset);
	
	address_t selectedBytesAddress() const;
	unsigned int selectedBytesSize() const;
	QByteArray selectedBytes() const;
	QMenu *createStandardContextMenu();

public slots:
	void selectAll();
	void deselect();
	bool hasSelectedText() const;
	void mnuSetFont();
	void mnuCopy();

private:
	// Attempt to ensure that the working buffer contains data from
	// `offset' up to at least `size'. Prefetching etc may be used
	// internally.
	// This operation does touch object state, but only internal
	// machinery invisible though the interface.
	void fetchData(unsigned int offset, unsigned int size) const;

	void updateScrollbars();
	
	bool isSelected(int index) const;
	bool isInViewableArea(int index) const;
	
	int pixelToWord(int x, int y) const;
	
	unsigned int charsPerWord() const;
	int hexDumpLeft() const;
	int asciiDumpLeft() const;
	int commentLeft() const;
	unsigned int addressLen() const;
	int line1() const;
	int line2() const;
	int line3() const;

	unsigned int bytesPerRow() const;
	
	int dataSize() const;
	
	void drawAsciiDump(QPainter &painter, unsigned int offset, unsigned int row) const;
	void drawHexDump(QPainter &painter, unsigned int offset, unsigned int row, int &wordCount) const;
	void drawComments(QPainter &painter, unsigned int offset, unsigned int row) const;
	
	QString formatAddress(address_t address);
	
private:
	static bool isPrintable(unsigned int ch);
	static QAction *addToggleActionToMenu(QMenu *menu, const QString &caption, bool checked, QObject *reciever, const char *slot);
	
private:
	address_t m_Origin;
	address_t m_AddressOffset;		// this is the offset that our base address is relative to
	int m_SelectionStart;			// index of first selected word (or -1)
	int m_SelectionEnd;				// index of last selected word (or -1)
	int m_FontWidth;				// width of a character in this font
	int m_FontHeight;				// height of a character in this font
	
	enum {
		Highlighting_None,
		Highlighting_Data,
		Highlighting_Ascii
	} m_Highlighting;
	
	QColor m_EvenWord;
	QColor m_NonPrintableText;
	char m_UnprintableChar;

	bool m_ShowLine1;
	bool m_ShowLine2;
	bool m_ShowLine3;
	bool m_ShowAddressSeparator;	// should we show ':' character in address to seperate high/low portions
	char m_AddressFormatString[32];
	
	CommentServerInterface *m_CommentServer;
};

#endif
