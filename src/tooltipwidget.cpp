/*
 *	tooltipwidget.cpp -- tool-tip widget for AVR-ASM-IDE.
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

#include "tooltipwidget.h"

#include <QStringList>
#include <QLabel>


/*{{{  TooltipWidget::TooltipWidget (QWidget *parent) : QStackedWidget (parent)*/
/*
 *	constructor
 */
TooltipWidget::TooltipWidget (QWidget *parent) : QStackedWidget (parent)
{
	setAutoFillBackground (true);
}
/*}}}*/
/*{{{  void TooltipWidget::setTooltips (const QStringList &tooltipsContent)*/
/*
 *	sets up tooltip contents.
 */
void TooltipWidget::setTooltips (const QStringList &tooltipsContent)
{
	QWidget *tmpWidget = 0;
	int index = 0;

	// Delete all tooltips currently in the stacked widget
	while ((tmpWidget = currentWidget ())) {
		removeWidget (tmpWidget);
		delete tmpWidget;
	}

	// Create the new tooltips
	for (const QString &content:tooltipsContent) {
		QLabel *labelTooltipContent;

		// Insert the navigation text (page number and previous/next page) if needed
		if (tooltipsContent.count () > 1) {
			labelTooltipContent = new QLabel (content +
					QString ("<br/><br/>%1/%2 <br/>F2:← F3:→").arg (index + 1).arg (tooltipsContent.count ()));
		} else {
			labelTooltipContent = new QLabel (content);
		}
		labelTooltipContent->setStyleSheet ("QLabel { border: 1px solid black; border-radius: 5px; }");
		labelTooltipContent->setAlignment (Qt::AlignTop);
		addWidget (labelTooltipContent);
		++index;
	}
}
/*}}}*/
/*{{{  void TooltipWidget::nextTooltip ()*/
/*
 *	shows the next tool-tip.
 */
void TooltipWidget::nextTooltip ()
{
	if (count ()) {
		setCurrentIndex ((currentIndex () + 1) % count ());
	}
}
/*}}}*/
/*{{{  void TooltipWidget::previousTooltip ()*/
/*
 *	shows the previous tool-tip.
 */
void TooltipWidget::previousTooltip ()
{
	if (count ()) {
		setCurrentIndex (currentIndex () == 0 ? count () - 1 : currentIndex () - 1);
	}
}
/*}}}*/

