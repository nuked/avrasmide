/*
 *	optionconfig.cpp -- Option configuration for AVR-ASM-IDE.
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

#include <iostream>
#include <QIcon>

#include "optionconfig.h"

/*{{{  OptionConfig::OptionConfig (QWidget *parent) : QWidget (parent)*/
/*
 *	constructor.
 */
OptionConfig::OptionConfig (QWidget *parent) : QWidget (parent)
{
}
/*}}}*/
/*{{{  void OptionConfig::setName (QString newName)*/
/*
 *	set name.
 */
void OptionConfig::setName (QString newName)
{
	_name = newName;
}
/*}}}*/
/*{{{  void OptionConfig::setName (const char *newName)*/
/*
 *	set name (from string constant).
 */
void OptionConfig::setName (const char *newName)
{
	_name = newName;
}
/*}}}*/
/*{{{  QString OptionConfig::name ()*/
/*
 *	get name.
 */
QString OptionConfig::name ()
{
	return _name;
}
/*}}}*/
/*{{{  void OptionConfig::setIcon (QIcon newIcon)*/
/*
 *	sets icon.
 */
void OptionConfig::setIcon (QIcon newIcon)
{
	_icon = newIcon;
}
/*}}}*/
/*{{{  QIcon OptionConfig::icon ()*/
/*
 *	gets icon.
 */
QIcon OptionConfig::icon ()
{
	return _icon;
}
/*}}}*/
/*{{{  void OptionConfig::writeSettings ()*/
/*
 *	write settings (does nothing).
 */
void OptionConfig::writeSettings ()
{
	return;
}
/*}}}*/

