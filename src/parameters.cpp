/*
 *	parameters.cpp -- parameters dialogue handling.
 *	Copyright (C) 2013 Gregoire Liglet and Florent Chiron, University of Kent.
 *
 *	Modifications (C) 2013-2014 Fred Barnes, University of Kent  <frmb@kent.ac.uk>
 */

#include <iostream>

#include <QDebug>
#include <QFrame>
#include <QGridLayout>
#include <QIcon>
#include <QList>
#include <QSettings>
#include <QStackedWidget>

#include "editorconfiguration.h"
#include "mainwindow.h"
#include "arduinoconfiguration.h"
#include "optionconfig.h"
#include "parameters.h"
#include "ui_parameters.h"


/*{{{  Parameters::Parameters (QWidget *parent) : QDialog (parent), ui (new Ui::Parameters)*/
/*
 *	constructor.
 */
Parameters::Parameters (QWidget *parent) : QDialog (parent), ui (new Ui::Parameters)
{
	ui->setupUi (this);
	_editorConfig = new EditorConfiguration ();
	initializeOptions ();
	buildLayout ();
}
/*}}}*/
/*{{{  Parameters::~Parameters ()*/
/*
 *	destructor.
 */
Parameters::~Parameters ()
{
	delete ui;
}
/*}}}*/

/*{{{  void Parameters::buildLayout (void)*/
/*
 *	connects up things in the parameters layout.
 */
void Parameters::buildLayout (void)
{
	QWidget::setFixedSize (this->size ());

	_optionListWidget = findChild<QListWidget *> ("listWidget");
	_widgetStack = findChild<QStackedWidget *> ("stackedWidget");
	QList<OptionConfig *>::Iterator it = _optionList->begin ();

	int i = 0;
	while (it != _optionList->end ()) {
		_optionListWidget->insertItem (i, (*it)->name ());
		_optionListWidget->item (i)->setIcon (((OptionConfig *) (*it))->icon ());
		_widgetStack->addWidget ((*it));
		++it;
		i++;
	}

	_widgetStack->setCurrentWidget (_arduinoConfig);
	connect (_optionListWidget, SIGNAL (itemClicked (QListWidgetItem *)), SLOT (chgOption (QListWidgetItem *)));
	_okButton = findChild<QPushButton *> ("okButton");
	connect (_okButton, SIGNAL (clicked ()), SLOT (okClicked ()));
	_cancelButton = findChild<QPushButton *> ("cancelButton");
	connect (_cancelButton, SIGNAL (clicked ()), SLOT (cancelClicked ()));
	connect (this, SIGNAL (rejected ()), SLOT (cancelClicked ()));
	_applyButton = findChild<QPushButton *> ("applyButton");
	connect (_applyButton, SIGNAL (clicked ()), SLOT (applyClicked ()));
}
/*}}}*/
/*{{{  void Parameters::chgOption (QListWidgetItem *current)*/
/*
 *	called when something in the list is selected -- change stack.
 */
void Parameters::chgOption (QListWidgetItem *current)
{
	OptionConfig *currentOption = findByName (current->text ());
	_widgetStack->setCurrentWidget (currentOption);
}
/*}}}*/
/*{{{  OptionConfig *Parameters::findByName (QString name)*/
/*
 *	finds a particular option configuration by name.
 */
OptionConfig *Parameters::findByName (QString name)
{
	QList<OptionConfig *>::Iterator it = _optionList->begin ();

	while (it != _optionList->end ()) {
		if ((*it)->name () == name) {
			return (*it);
		}
		++it;
	}
	return NULL;
}
/*}}}*/
/*{{{  void Parameters::initializeOptions (void)*/
/*
 *	initialises option-handling.
 */
void Parameters::initializeOptions (void)
{
	_optionList = new QList<OptionConfig *> ();
	_arduinoConfig = new ArduinoConfiguration ();

	_arduinoConfig->setName ("Arduino configuration");
	_arduinoConfig->setIcon (QIcon (":/images/gears32.png"));
	_optionList->append ((OptionConfig *)_arduinoConfig);

	_editorConfig->setName ("Editor configuration");
	_editorConfig->setIcon (QIcon (":/images/new.png"));
	_optionList->append ((OptionConfig *)_editorConfig);
}
/*}}}*/
/*{{{  void Parameters::readSettings (void)*/
/*
 *	reads saved settings.
 */
void Parameters::readSettings (void)
{
	/* Note: this is done elsewhere */
}
/*}}}*/
/*{{{  void Parameters::writeSettings (void)*/
/*
 *	writes settings.
 */
void Parameters::writeSettings (void)
{
	for (QList<OptionConfig *>::Iterator it = _optionList->begin (); it != _optionList->end (); ++it) {
		(*it)->writeSettings ();
	}
}
/*}}}*/
/*{{{  ArduinoConfiguration *Parameters::arduinoConfig (void)*/
/*
 *	returns a pointer to a structure containing AVR/Arduino configuration information.
 */
ArduinoConfiguration *Parameters::arduinoConfig (void)
{
	return _arduinoConfig;
}
/*}}}*/
/*{{{  EditorConfiguration *Parameters::editorConfig (void)*/
/*
 *	returns a pointer to a structure containing editor configuration information.
 */
EditorConfiguration *Parameters::editorConfig (void)
{
	return _editorConfig;
}
/*}}}*/

/*{{{  void Parameters::okClicked ()*/
/*
 *	called when the 'ok' button is clicked.
 */
void Parameters::okClicked ()
{
	applyClicked ();
	hide ();
}
/*}}}*/
/*{{{  void Parameters::cancelClicked ()*/
/*
 *	called when the 'cancel' button is clicked.
 */
void Parameters::cancelClicked ()
{
	_editorConfig->cancelParams ();
	_arduinoConfig->cancelParams ();
	hide ();
}
/*}}}*/
/*{{{  void Parameters::applyClicked ()*/
/*
 *	called when the 'apply' button is clicked
 */
void Parameters::applyClicked ()
{
	writeSettings ();
	_editorConfig->applyParams ();
	_arduinoConfig->applyParams ();
}
/*}}}*/

