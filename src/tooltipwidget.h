/*
 *	tooltipwidget.h -- tool-tip object.
 *	Copyright (C) 2013 Gregoire Liglet and Florent Chiron, University of Kent.
 *	Copyright (C) 2013-2014 Fred Barnes, University of Kent <frmb@kent.ac.uk>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef TOOLTIPWIDGET_H
#define TOOLTIPWIDGET_H

#include <QStackedWidget>

class QStringList;

class TooltipWidget : public QStackedWidget
{
	Q_OBJECT
public:
	explicit TooltipWidget (QWidget *parent = 0);

	void setTooltips (const QStringList &tooltipsContent);
	void nextTooltip ();
	void previousTooltip ();
};

#endif // TOOLTIPWIDGET_H

