// qtractorClipForm.h
//
// ui.h extension file, included from the uic-generated form implementation.
/****************************************************************************
   Copyright (C) 2005-2006, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#ifndef __qtractorClipForm_h
#define __qtractorClipForm_h

#include "ui_qtractorClipForm.h"

#include "qtractorClip.h"


//----------------------------------------------------------------------------
// qtractorClipForm -- UI wrapper form.

class qtractorClipForm : public QDialog
{
	Q_OBJECT

public:

	// Constructor.
	qtractorClipForm(QWidget *pParent = 0, Qt::WFlags wflags = 0);
	// Destructor.
	~qtractorClipForm();

	void setClip(qtractorClip *pClip);
	qtractorClip *clip() const;

public slots:

	void accept();
	void reject();
	void changed();
	void stabilizeForm();

protected:

	qtractorClip::FadeType fadeTypeFromIndex(int iIndex) const;
	int indexFromFadeType(qtractorClip::FadeType fadeType) const;

private:

	// The Qt-designer UI struct...
	Ui::qtractorClipForm m_ui;

	// Instance variables...
	qtractorClip *m_pClip;
	int m_iDirtyCount;
};


#endif	// __qtractorClipForm_h


// end of qtractorClipForm.h
