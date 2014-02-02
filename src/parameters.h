/*
 *	parameters.h -- parameters dialogue object.
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

#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <QDialog>
#include <QList>
#include <QListView>
#include <QListWidget>
#include <QPushButton>
#include <QStackedWidget>

#include "editorconfiguration.h"
#include "arduinoconfiguration.h"
#include "optionconfig.h"

namespace Ui
{
	class Parameters;
}

class Parameters:public QDialog
{
	Q_OBJECT
private slots:
	void chgOption (QListWidgetItem *);
	void okClicked ();
	void cancelClicked ();
	void applyClicked ();
public:
	static Parameters &getInstance (bool reset = false)
	{
		static Parameters *instance = 0;

		if (reset) {
			delete instance;
			instance = new Parameters ();
		}
		return *instance;
	}

	ArduinoConfiguration *arduinoConfig ();
	EditorConfiguration *editorConfig ();

private:
	explicit Parameters (QWidget *parent = 0);
	~Parameters ();

	void buildLayout ();
	void initializeOptions ();
	void readSettings ();
	void writeSettings ();
	OptionConfig *findByName (QString);
	Ui::Parameters * ui;
	EditorConfiguration *_editorConfig;
	ArduinoConfiguration *_arduinoConfig;
	QListWidget *_optionListWidget;
	QList<OptionConfig *>*_optionList;
	QStackedWidget *_widgetStack;
	QPushButton *_okButton;
	QPushButton *_cancelButton;
	QPushButton *_applyButton;
};


#endif // PARAMETERS_H

