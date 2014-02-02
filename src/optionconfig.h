/*
 *	optionconfig.h -- option configuration object.
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

#ifndef OPTIONCONFIG_H
#define OPTIONCONFIG_H

#include <QIcon>
#include <QString>
#include <QWidget>

class OptionConfig : public QWidget
{
private:
	QString _name;
	QIcon _icon;

public:
	explicit OptionConfig (QWidget *parent = 0);
	virtual void setName (QString);
	void setName (const char *);
	QString name ();
	void setIcon (QIcon);
	QIcon icon ();
	virtual void writeSettings ();
};

#endif // OPTIONCONFIG_H
