/*
 *	optionconfig.h
 *	Gregoire Liglet and Florent Chiron, University of Kent, 2013.
 */

#ifndef OPTIONCONFIG_H
#define OPTIONCONFIG_H

#include <QIcon>
#include <QString>
#include <QWidget>

class OptionConfig:public QWidget
{
private:
	QString _name;
	QIcon _icon;

public:
	explicit OptionConfig (QWidget * parent = 0);
	virtual void setName (QString);
	void setName (const char *);
	QString name ();
	void setIcon (QIcon);
	QIcon icon ();
	virtual void writeSettings ();
};

#endif // OPTIONCONFIG_H
