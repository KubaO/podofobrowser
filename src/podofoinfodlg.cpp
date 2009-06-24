/***************************************************************************
 *   Copyright (C) 2005 by Pierre Marchand   *
 *   pierremarc@oep-h.com   *
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

#include "podofoinfodlg.h"

#include <QFileInfo>
#include <QLocale>

PodofoInfoDialog::PodofoInfoDialog (const QString& filename, PoDoFo::PdfDocument * document, QWidget * parent)
		:QDialog ( parent )
{
	setupUi ( this );
	if ( document )
	{
		QString itemTemplate ( QString::fromUtf8 ( "<div class=\"item\">\
				<span class=\"name\">%1</span>\
				<span class=\"value\">%2</span>\
				</div>" ) );
		
		QFileInfo fileInfo(filename);
		
		QString filepath ( itemTemplate.arg ( tr ( "file:" ) )
				.arg ( fileInfo.absoluteFilePath() ) );
		double size(fileInfo.size());
		QString unit; 
		if(size < 1024.0)
		{
			unit = QString::fromAscii("%1 B");
		}
		else if(size < (1024.0 * 1024.0))
		{
			size /= 1024.0 ;
			unit = QString::fromAscii("%1 KB");
		}
		else if(size < (1024.0 * 1024.0 * 1024.0))
		{
			size /= 1024.0 *1024.0; 
			unit = QString::fromAscii("%1 MB");
		}
		else 
		{
			size /= 1024.0 * 1024.0 * 1024.0; 
			unit = QString::fromAscii("%1 GB");
		}
		
		QString filesize ( itemTemplate.arg ( tr ( "size:" ) )
				.arg (unit.arg(QLocale::system().toString(size,'f',2))) );
		
		PoDoFo::PdfInfo * info ( document->GetInfo() );

		QString author ( itemTemplate.arg ( tr ( "author:" ) )
		                 .arg ( QString::fromUtf8 ( info->GetAuthor().GetStringUtf8().c_str() ) ) );
		QString creator ( itemTemplate.arg ( tr ( "creator:" ) )
		                  .arg ( QString::fromUtf8 ( info->GetCreator().GetStringUtf8().c_str() ) ) );
		QString keywords ( itemTemplate.arg ( tr ( "keywords:" ) )
		                   .arg ( QString::fromUtf8 ( info->GetKeywords().GetStringUtf8().c_str() ) ) );
		QString subject ( itemTemplate.arg ( tr ( "subject:" ) )
		                  .arg ( QString::fromUtf8 ( info->GetSubject().GetStringUtf8().c_str() ) ) );
		QString title ( itemTemplate.arg ( tr ( "title:" ) )
		                .arg ( QString::fromUtf8 ( info->GetTitle().GetStringUtf8().c_str() ) ) );
		QString producer ( itemTemplate.arg ( tr ( "producer:" ) )
		                   .arg ( QString::fromUtf8 ( info->GetProducer().GetStringUtf8().c_str() ) ) );
		QString trapped ( itemTemplate.arg ( tr ( "trapped:" ) )
		                  .arg ( QString::fromUtf8 ( info->GetTrapped().GetEscapedName().c_str() ) ) );

		QString html ( QString::fromUtf8("<html>\
		               <head>\
		               <style type=\"text/css\">\
		               div.item{margin-bottom:6px}\
		               span.name{font-weight:bold}\
		               span.value{}\
		               </style>\
		               </head>\
		               <body>") );
		html += filepath;
		html += filesize;
		html += title;
		html += author;
		html += subject;
		html += keywords;
		html += creator;
		html += producer;
		html += trapped;

		html +=  QString::fromUtf8("</body</html>");

		infoBrowser->setHtml ( html );
	}

}
