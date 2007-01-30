/***************************************************************************
 *   Copyright (C) 2005 by Dominik Seichter                                *
 *   domseichter@web.de                                                    *
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

#include "pdflistviewitem.h"

#include <podofo/podofo.h>

#include <string>

PdfListViewItem::PdfListViewItem( Q3ListView* parent, PoDoFo::PdfObject* object )
    : Q3ListViewItem( parent, parent->lastItem() ), m_pObject( object ), m_bInitialized( false )
{
    m_sText = QString( "%1 %2 R  " ).arg( m_pObject->Reference().ObjectNumber() ).arg( m_pObject->Reference().GenerationNumber() );
    m_sType = "";
    setText( 0, m_sText + m_sType );
}

PdfListViewItem::PdfListViewItem( Q3ListViewItem* parent, PoDoFo::PdfObject* object, const QString & key )
    : Q3ListViewItem( parent ), m_pObject( object ), m_bInitialized( false )
{
    m_sText = ""; //QString( "%1 %2 R  " ).arg( m_pObject->ObjectNumber() ).arg( m_pObject->GenerationNumber() );
    m_sType = key;
    setText( 0, m_sText + m_sType );
}
 
PdfListViewItem::~PdfListViewItem()
{
}

void PdfListViewItem::init()
{
    PoDoFo::PdfError     eCode;
    PoDoFo::TCIKeyMap     it;
    PoDoFo::PdfVariant   var;
    PdfListViewItem*     child;
    std::string          str;

    if( m_bInitialized ) 
        return;

    if( !(m_pObject && m_pObject->IsDictionary()) ) 
    {
        m_bInitialized = true;
        return;
    }

    if( m_sType.isEmpty() && m_pObject->GetDictionary().HasKey( PoDoFo::PdfName::KeyType ) )
    {
        m_pObject->GetDictionary().GetKey( PoDoFo::PdfName::KeyType )->ToString( str );
        m_sType = str.c_str();
    }

    setText( 0, m_sText + m_sType );

    if( m_pObject->GetDictionary().GetKeys().size() )
    {
        this->setOpen( true );

        it = m_pObject->GetDictionary().GetKeys().begin();
        while( it != m_pObject->GetDictionary().GetKeys().end() )
        {
            if( ((*it).second)->IsDictionary() )
            {
                child = new PdfListViewItem( this, const_cast<PoDoFo::PdfObject*>((*it).second), QString( (*it).first.GetName().c_str() ) );
                child->init();
            }
            ++it;
        }
    }

    m_bInitialized = true;
}

int PdfListViewItem::compare( Q3ListViewItem* i, int col, bool ascending ) const
{
    PdfListViewItem* item = dynamic_cast<PdfListViewItem*>(i);

    if( col || !item )
        return Q3ListViewItem::compare( i, col, ascending );
    else
    {
        if( item->object()->Reference() == this->object()->Reference() )
            return 0;

        if( item->object()->Reference() < this->object()->Reference() )
            return ascending ? 1 : -1;
        else 
            return ascending ? -1 : 1;
    }
}
