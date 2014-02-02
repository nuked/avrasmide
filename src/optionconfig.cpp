/*
 *	optionconfig.cpp
 *	Gregoire Liglet and Florent Chiron, University of Kent, 2013
 */

#include <iostream>

#include <QIcon>

#include "optionconfig.h"

OptionConfig::OptionConfig (QWidget * parent): QWidget (parent)
{
}

void OptionConfig::setName (QString newName)
{
	_name = newName;
}

void OptionConfig::setName (const char *newName)
{
	_name = newName;
}

QString OptionConfig::name ()
{
	return _name;
}

void OptionConfig::setIcon (QIcon newIcon)
{
	_icon = newIcon;
}

QIcon OptionConfig::icon ()
{
	return _icon;
}

void OptionConfig::writeSettings ()
{
	return;
}

