/*
 *	arduinoconfiguration.cpp -- configuration settings for the Arduino
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

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QListWidget>
#include <QMainWindow>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QtWidgets>

#include "arduinoconfiguration.h"
#include "ui_arduinoconfiguration.h"


/*{{{  ArduinoConfiguration::ArduinoConfiguration (QWidget *parent) : OptionConfig (parent), ui (new Ui::ArduinoConfiguration)*/
/*
 *	constructor.
 */
ArduinoConfiguration::ArduinoConfiguration (QWidget *parent) : OptionConfig (parent), ui (new Ui::ArduinoConfiguration)
{
	ui->setupUi (this);
	_changed = false;
	readSettings ();
	buildUi ();
	initializeForms ();
	initComPorts ();
}
/*}}}*/
/*{{{  ArduinoConfiguration::~ArduinoConfiguration ()*/
/*
 *	destructor.
 */
ArduinoConfiguration::~ArduinoConfiguration ()
{
	delete ui;
}
/*}}}*/

/*{{{  void ArduinoConfiguration::buildUi (void)*/
/*
 *	builds the user-interface for the arduino configuration.
 */
void ArduinoConfiguration::buildUi (void)
{
	_noccBrowseBtn = findChild <QPushButton *>("noccBrowseBtn");
	connect (_noccBrowseBtn, SIGNAL (pressed ()), SLOT (noccChgPath ()));

	_noccPathLbl = findChild <QLabel *>("noccPathLbl");
	_noccPathLbl->setWordWrap (true);
	_noccPathLbl->setText (_noccPath);

	_noccSpecsBrowseBtn = findChild <QPushButton *>("noccSpecsBrowseBtn");
	connect (_noccSpecsBrowseBtn, SIGNAL (pressed ()), SLOT (noccChgSpecsPath ()));

	_noccSpecsPathLbl = findChild <QLabel *>("noccSpecsPathLbl");
	_noccSpecsPathLbl->setWordWrap (true);
	_noccSpecsPathLbl->setText (_noccSpecsPath);

	_noccParamsText = findChild <QPlainTextEdit *>("noccParamsText");
	connect (_noccParamsText, SIGNAL (cursorPositionChanged ()), SLOT (noccChgParams ()));

	_noccParamsText->setPlainText (_noccParams);

	_noccUsageBrowser = findChild <QTextBrowser *>("noccUsageBrowser");

	_customMode = false;
	connect (ui->radioButton, SIGNAL (toggled (bool)), SLOT (setCustomMode (bool)));
	ui->radioButton_2->setChecked (true);
	ui->radioButton->setChecked (false);
	ui->plainTextEdit_12->setEnabled (false);

	connect (ui->pushButton, SIGNAL (clicked ()), SLOT (resetDefaults ()));

	connect (ui->toolButton, SIGNAL (clicked ()), SLOT (browseAvrdude ()));
	connect (ui->toolButton_2, SIGNAL (clicked ()), SLOT (browseAvrdudeConf ()));
}
/*}}}*/
/*{{{  void ArduinoConfiguration::browseAvrdude (void)*/
/*
 *	called to browse for avrdude binary.
 */
void ArduinoConfiguration::browseAvrdude (void)
{
	QString fileName = QFileDialog::getOpenFileName (this);
	if (fileName.isEmpty ()) {
		_avrdudePath = DEFAULT_avrdudePath;
	} else
		_avrdudePath = fileName;
	ui->plainTextEdit_3->clear ();
	ui->plainTextEdit_3->appendPlainText (_avrdudePath);
	_changed = true;
	emit avrdudePathChanged ();
}
/*}}}*/
/*{{{  void ArduinoConfiguration::browseAvrdudeConf (void)*/
/*
 *	called to browse for avrdude configuration file.
 */
void ArduinoConfiguration::browseAvrdudeConf (void)
{
	QString fileName = QFileDialog::getOpenFileName (this);
	if (fileName.isEmpty ()) {
		_opt_C2 = DEFAULT_opt_C2;
	} else
		_opt_C2 = fileName;
	ui->plainTextEdit_2->clear ();
	ui->plainTextEdit_2->appendPlainText (_opt_C2);
	_changed = true;
}
/*}}}*/

/*{{{  void ArduinoConfiguration::initializeForms (void)*/
/*
 *	called to initialise the form (fill with options).
 */
void ArduinoConfiguration::initializeForms (void)
{
	ui->plainTextEdit_3->clear ();
	ui->plainTextEdit_3->insertPlainText (_avrdudePath);
	ui->plainTextEdit_16->clear ();
	ui->plainTextEdit_16->insertPlainText (_opt_B2);
	ui->plainTextEdit_15->clear ();
	ui->plainTextEdit_15->insertPlainText (_opt_b);
	ui->plainTextEdit_2->clear ();
	ui->plainTextEdit_2->insertPlainText (_opt_C2);
	ui->plainTextEdit_5->clear ();
	ui->plainTextEdit_5->insertPlainText (_opt_c);
	ui->plainTextEdit_8->clear ();
	ui->plainTextEdit_8->insertPlainText (_opt_E2);
	ui->plainTextEdit_6->clear ();
	ui->plainTextEdit_6->insertPlainText (_opt_i);
	//    ui->plainTextEdit_13->clear();
	//    ui->plainTextEdit_13->insertPlainText(_opt_P2);
	ui->plainTextEdit_14->clear ();
	ui->plainTextEdit_14->insertPlainText (_opt_p);
	ui->plainTextEdit_7->clear ();
	ui->plainTextEdit_7->insertPlainText (_opt_U2);
	ui->plainTextEdit_9->clear ();
	ui->plainTextEdit_9->insertPlainText (_opt_x);
	ui->plainTextEdit_11->clear ();
	ui->plainTextEdit_11->insertPlainText (_opt_Y2);
	ui->plainTextEdit_10->clear ();
	ui->plainTextEdit_10->insertPlainText (_opt_y);

	ui->checkBox_2->setChecked (_opt_F2);
	ui->checkBox_3->setChecked (_opt_e);
	ui->checkBox_11->setChecked (_opt_q);
	ui->checkBox_8->setChecked (_opt_s);
	ui->checkBox_4->setChecked (_opt_O);
	ui->checkBox->setChecked (_opt_D2);
	ui->checkBox_5->setChecked (_opt_n);
	ui->checkBox_6->setChecked (_opt_V2);
	ui->checkBox_9->setChecked (_opt_t);
	ui->checkBox_7->setChecked (_opt_u);
	ui->checkBox_10->setChecked (_opt_v);
}
/*}}}*/
/*{{{  void ArduinoConfiguration::initComPorts (void)*/
/*
 *	uses the QSerialPort stuff to search for serial-ports.
 */
void ArduinoConfiguration::initComPorts (void)
{
	QString arduinoDetected = "";
	int port = 0;

	foreach (const QSerialPortInfo & info, QSerialPortInfo::availablePorts ()) {
		if (info.description ().indexOf ("Arduino") != -1) {
			arduinoDetected.append (info.portName ());
		}
		_availablePorts.append (info.portName ());
	}
	ui->comboBox->addItems (_availablePorts);

	if (arduinoDetected.isEmpty ()) {
		port = ui->comboBox->findText (DEFAULT_opt_P2);
	} else {
		port = ui->comboBox->findText (arduinoDetected);
	}

	if (port != -1) {
		ui->comboBox->setCurrentIndex (port);
	}
}
/*}}}*/

/*{{{  void ArduinoConfiguration::noccChgPath (void)*/
/*
 *	called to change the nocc executable path.
 */
void ArduinoConfiguration::noccChgPath (void)
{
	QString fileName = QFileDialog::getOpenFileName (this);

	if (!fileName.isEmpty ()) {
		QDir *dir = new QDir ();
		_noccOldPath = _noccPath;
		// _noccPath = dir->relativeFilePath (fileName);
		_noccPath = fileName;
		_noccPathLbl->setText (_noccPath);
		_changed = true;
	}
}
/*}}}*/
/*{{{  void ArduinoConfiguration::noccChgSpecsPath (void)*/
/*
 *	called to change the nocc specification-file path.
 */
void ArduinoConfiguration::noccChgSpecsPath (void)
{
	QString fileName = QFileDialog::getOpenFileName (this);

	if (!fileName.isEmpty ()) {
		QDir *dir = new QDir ();

		_noccOldSpecsPath = _noccSpecsPath;
		// _noccSpecsPath = dir->relativeFilePath (fileName);
		_noccSpecsPath = fileName;
		_noccSpecsPathLbl->setText (_noccSpecsPath);
		_changed = true;
	}
}
/*}}}*/
/*{{{  void ArduinoConfiguration::noccChgParams (void)*/
/*
 *	called to change nocc parameters.
 */
void ArduinoConfiguration::noccChgParams (void)
{
	_noccOldParams = _noccParams;
	_noccParams = _noccParamsText->toPlainText ();
	_noccParamsList = _noccParams.split (" ", QString::SkipEmptyParts);
	_changed = true;
}
/*}}}*/

/*{{{  void ArduinoConfiguration::readSettings (void)*/
/*
 *	reads configuration settings.
 */
void ArduinoConfiguration::readSettings (void)
{
	QSettings settings ("unikent", "avr-asm-ide");

	_noccPath = settings.value ("noccPath", DEFAULT_noccPath).toString ();
	_noccSpecsPath = settings.value ("noccSpecsPath", DEFAULT_noccSpecsPath).toString ();
	_noccParams = settings.value ("noccParams", DEFAULT_noccParams).toString ();
	_avrdudePath = settings.value ("avrdudePath", DEFAULT_avrdudePath).toString ();
	_opt_B2 = settings.value ("opt_B2", DEFAULT_opt_B2).toString ();
	_opt_b = settings.value ("opt_b", DEFAULT_opt_b).toString ();
	_opt_C2 = settings.value ("opt_C2", DEFAULT_opt_C2).toString ();
	_opt_c = settings.value ("opt_c", DEFAULT_opt_c).toString ();
	_opt_D2 = settings.value ("opt_D2", DEFAULT_opt_D2).toBool ();
	_opt_E2 = settings.value ("opt_E2", DEFAULT_opt_E2).toString ();
	_opt_e = settings.value ("opt_e", DEFAULT_opt_e).toBool ();
	_opt_F2 = settings.value ("opt_F2", DEFAULT_opt_F2).toBool ();
	_opt_i = settings.value ("opt_i", DEFAULT_opt_i).toString ();
	_opt_n = settings.value ("opt_n", DEFAULT_opt_n).toBool ();
	_opt_O = settings.value ("opt_O", DEFAULT_opt_O).toBool ();
	_opt_P2 = settings.value ("opt_P2", DEFAULT_opt_P2).toString ();
	_opt_p = settings.value ("opt_p", DEFAULT_opt_p).toString ();
	_opt_q = settings.value ("opt_q", DEFAULT_opt_q).toBool ();
	_opt_s = settings.value ("opt_s", DEFAULT_opt_s).toBool ();
	_opt_t = settings.value ("opt_t", DEFAULT_opt_t).toBool ();
	_opt_U2 = settings.value ("opt_U2", DEFAULT_opt_U2).toString ();
	_opt_u = settings.value ("opt_u", DEFAULT_opt_u).toBool ();
	_opt_V2 = settings.value ("opt_V2", DEFAULT_opt_V2).toBool ();
	_opt_v = settings.value ("opt_v", DEFAULT_opt_v).toBool ();
	_opt_x = settings.value ("opt_x", DEFAULT_opt_x).toString ();
	_opt_Y2 = settings.value ("opt_Y2", DEFAULT_opt_Y2).toString ();
	_opt_y = settings.value ("opt_y", DEFAULT_opt_y).toString ();
}
/*}}}*/
/*{{{  void ArduinoConfiguration::writeSettings (void)*/
/*
 *	called to write settings.
 */
void ArduinoConfiguration::writeSettings (void)
{
	retrieveValues ();
	QSettings settings ("unikent", "avr-asm-ide");

	retrieveValues ();
	settings.setValue ("noccPath", _noccPath);
	settings.setValue ("noccSpecsPath", _noccSpecsPath);
	settings.setValue ("noccParams", _noccParams);
	settings.setValue ("avrdudePath", _avrdudePath);
	settings.setValue ("opt_B2", _opt_B2);
	settings.setValue ("opt_b", _opt_b);
	settings.setValue ("opt_C2", _opt_C2);
	settings.setValue ("opt_c", _opt_c);
	settings.setValue ("opt_D2", _opt_D2);
	settings.setValue ("opt_E2", _opt_E2);
	settings.setValue ("opt_e", _opt_e);
	settings.setValue ("opt_F2", _opt_F2);
	settings.setValue ("opt_i", _opt_i);
	settings.setValue ("opt_n", _opt_n);
	settings.setValue ("opt_O", _opt_O);
	settings.setValue ("opt_P2", _opt_P2);
	settings.setValue ("opt_p", _opt_p);
	settings.setValue ("opt_q", _opt_q);
	settings.setValue ("opt_s", _opt_s);
	settings.setValue ("opt_t", _opt_t);
	settings.setValue ("opt_U2", _opt_U2);
	settings.setValue ("opt_u", _opt_u);
	settings.setValue ("opt_V2", _opt_V2);
	settings.setValue ("opt_v", _opt_v);
	settings.setValue ("opt_x", _opt_x);
	settings.setValue ("opt_Y2", _opt_Y2);
	settings.setValue ("opt_y", _opt_y);
}
/*}}}*/
/*{{{  void ArduinoConfiguration::retrieveValues (void)*/
/*
 *	populates local variables with contents of the configuration window.
 */
void ArduinoConfiguration::retrieveValues (void)
{
	_avrdudePath = ui->plainTextEdit_3->toPlainText ();
	_opt_B2 = ui->plainTextEdit_16->toPlainText ();
	_opt_b = ui->plainTextEdit_15->toPlainText ();
	_opt_C2 = ui->plainTextEdit_2->toPlainText ();
	_opt_c = ui->plainTextEdit_5->toPlainText ();
	_opt_E2 = ui->plainTextEdit_8->toPlainText ();
	_opt_i = ui->plainTextEdit_6->toPlainText ();
	//    _opt_P2 = ui->plainTextEdit_13->toPlainText();
	_opt_p = ui->plainTextEdit_14->toPlainText ();
	_opt_U2 = ui->plainTextEdit_7->toPlainText ();
	_opt_x = ui->plainTextEdit_9->toPlainText ();
	_opt_Y2 = ui->plainTextEdit_11->toPlainText ();
	_opt_y = ui->plainTextEdit_10->toPlainText ();

	_opt_F2 = ui->checkBox_2->checkState ();
	_opt_e = ui->checkBox_3->checkState ();
	_opt_q = ui->checkBox_11->checkState ();
	_opt_s = ui->checkBox_8->checkState ();
	_opt_O = ui->checkBox_4->checkState ();
	_opt_n = ui->checkBox_5->checkState ();
	_opt_V2 = ui->checkBox_6->checkState ();
	_opt_t = ui->checkBox_9->checkState ();
	_opt_u = ui->checkBox_7->checkState ();
	_opt_v = ui->checkBox_10->checkState ();
	_opt_D2 = ui->checkBox->checkState ();
}
/*}}}*/
/*{{{  QString ArduinoConfiguration::noccPath (void)*/
/*
 *	returns the configured nocc executable path.
 */
QString ArduinoConfiguration::noccPath (void)
{
	return _noccPath;
}
/*}}}*/
/*{{{  QString ArduinoConfiguration::noccSpecsPath (void)*/
/*
 *	returns the configured nocc specs-file path.
 */
QString ArduinoConfiguration::noccSpecsPath (void)
{
	return _noccSpecsPath;
}
/*}}}*/
/*{{{  QString ArduinoConfiguration::noccParams (void)*/
/*
 *	returns the configured nocc parameter list.
 */
QString ArduinoConfiguration::noccParams (void)
{
	return _noccParams;
}
/*}}}*/

/*{{{  QStringList *ArduinoConfiguration::noccProcessedParams (QString filename)*/
/*
 *	does substitution for file-name in nocc's parameter-list.
 */
QStringList *ArduinoConfiguration::noccProcessedParams (QString filename)
{
	QStringList *tmp = new QStringList (_noccParamsList);

	// qDebug () << "tmp = " << tmp;
	return &(tmp->replaceInStrings ("%filename%", filename));
}
/*}}}*/
/*{{{  void ArduinoConfiguration::setCustomMode (bool val)*/
/*
 *	sets the 'custom-mode' checkbox in the configuration.
 */
void ArduinoConfiguration::setCustomMode (bool val)
{
	_customMode = val;
	if (val == true) {
		ui->tab_2->setEnabled (false);
		ui->plainTextEdit_12->setEnabled (true);
	} else {
		ui->tab_2->setEnabled (true);
		ui->plainTextEdit_12->setEnabled (false);
	}
}
/*}}}*/
/*{{{  void ArduinoConfiguration::resetDefaults (void)*/
/*
 *	resets the configuration to the compiled-in defaults.
 */
void ArduinoConfiguration::resetDefaults (void)
{
	ui->plainTextEdit_3->clear ();
	ui->plainTextEdit_3->insertPlainText (DEFAULT_avrdudePath);
	ui->plainTextEdit_16->clear ();
	ui->plainTextEdit_16->insertPlainText (DEFAULT_opt_B2);
	ui->plainTextEdit_15->clear ();
	ui->plainTextEdit_15->insertPlainText (DEFAULT_opt_b);
	ui->plainTextEdit_2->clear ();
	ui->plainTextEdit_2->insertPlainText (DEFAULT_opt_C2);
	ui->plainTextEdit_5->clear ();
	ui->plainTextEdit_5->insertPlainText (DEFAULT_opt_c);
	ui->plainTextEdit_8->clear ();
	ui->plainTextEdit_8->insertPlainText (DEFAULT_opt_E2);
	ui->plainTextEdit_6->clear ();
	ui->plainTextEdit_6->insertPlainText (DEFAULT_opt_i);
	//    ui->plainTextEdit_13->clear();
	//    ui->plainTextEdit_13->insertPlainText(DEFAULT_opt_P2);
	ui->plainTextEdit_14->clear ();
	ui->plainTextEdit_14->insertPlainText (DEFAULT_opt_p);
	ui->plainTextEdit_7->clear ();
	ui->plainTextEdit_7->insertPlainText (DEFAULT_opt_U2);
	ui->plainTextEdit_9->clear ();
	ui->plainTextEdit_9->insertPlainText (DEFAULT_opt_x);
	ui->plainTextEdit_11->clear ();
	ui->plainTextEdit_11->insertPlainText (DEFAULT_opt_Y2);
	ui->plainTextEdit_10->clear ();
	ui->plainTextEdit_10->insertPlainText (DEFAULT_opt_y);

	ui->checkBox_2->setChecked (DEFAULT_opt_F2);
	ui->checkBox_3->setChecked (DEFAULT_opt_e);
	ui->checkBox_11->setChecked (DEFAULT_opt_q);
	ui->checkBox_8->setChecked (DEFAULT_opt_s);
	ui->checkBox_4->setChecked (DEFAULT_opt_O);
	ui->checkBox->setChecked (DEFAULT_opt_D2);
	ui->checkBox_5->setChecked (DEFAULT_opt_n);
	ui->checkBox_6->setChecked (DEFAULT_opt_V2);
	ui->checkBox_9->setChecked (DEFAULT_opt_t);
	ui->checkBox_7->setChecked (DEFAULT_opt_u);
	ui->checkBox_10->setChecked (DEFAULT_opt_v);
}
/*}}}*/
/*{{{  QStringList ArduinoConfiguration::avrdudeParams (void)*/
/*
 *	returns a string containing avrdude parameters suitable formatted (if custom mode, uses given string).
 */
QStringList ArduinoConfiguration::avrdudeParams (void)
{
	if (_customMode) {
		return ui->plainTextEdit_12->toPlainText ().split (" ", QString::SkipEmptyParts);
	} else {
		QString ret = "";

		if (!_opt_B2.isEmpty ()) {
			ret.append (" -B ").append (_opt_B2);
		}
		if (!_opt_b.isEmpty ()) {
			ret.append (" -b ").append (_opt_b);
		}
		if (!_opt_C2.isEmpty ()) {
			ret.append (" -C ").append (_opt_C2);
		}
		if (!_opt_c.isEmpty ()) {
			ret.append (" -c ").append (_opt_c);
		}
		if (!_opt_E2.isEmpty ()) {
			ret.append (" -E ").append (_opt_E2);
		}
		if (!_opt_i.isEmpty ()) {
			ret.append (" -i ").append (_opt_i);
		}

		/* on UNIX, will need "/dev/" putting up-front */
#ifdef Q_OS_UNIX
		ret.append (" -P ").append ("/dev/").append (ui->comboBox->currentText ());
#else
		ret.append (" -P ").append (ui->comboBox->currentText ());
#endif

		if (!_opt_p.isEmpty ()) {
			ret.append (" -p ").append (_opt_p);
		}
		if (!_opt_U2.isEmpty ()) {
			ret.append (" -U ").append (_opt_U2);
		}
		if (!_opt_x.isEmpty ()) {
			ret.append (" -x ").append (_opt_x);
		}
		if (!_opt_Y2.isEmpty ()) {
			ret.append (" -Y ").append (_opt_Y2);
		}
		if (!_opt_y.isEmpty ()) {
			ret.append (" -y ").append (_opt_y);
		}
		if (_opt_D2) {
			ret.append ("-D");
		}
		if (_opt_e) {
			ret.append ("-e");
		}
		if (_opt_F2) {
			ret.append ("-F");
		}
		if (_opt_n) {
			ret.append ("-n");
		}
		if (_opt_O) {
			ret.append ("-O");
		}
		if (_opt_q) {
			ret.append ("-q");
		}
		if (_opt_s) {
			ret.append ("-s");
		}
		if (_opt_t) {
			ret.append ("-t");
		}
		if (_opt_u) {
			ret.append ("-u");
		}
		if (_opt_V2) {
			ret.append ("-V");
		}
		if (_opt_v) {
			ret.append ("-v");
		}

		QStringList list = ret.split (" ", QString::SkipEmptyParts);
		return list;
	}
}
/*}}}*/
/*{{{  QStringList *ArduinoConfiguration::avrdudeProcessedParams (QString flashName, QString eepromName)*/
/*
 *	returns the avrdude parameters, with substitutions for filenames.
 */
QStringList *ArduinoConfiguration::avrdudeProcessedParams (QString flashName, QString eepromName)
{
	QStringList *tmp = new QStringList (avrdudeParams ());

	for (QStringList::Iterator it = tmp->begin (); it != tmp->end (); ++it) {
		if ((*it).contains ("eeprom:w:%filename%")) {
			(*it) = QString ("eeprom:w:").append (eepromName);
		} else if ((*it).contains ("flash:w:%filename%")) {
			(*it) = QString ("flash:w:").append (flashName);
		}
	}
	return tmp;
}
/*}}}*/
/*{{{  QString ArduinoConfiguration::avrdudePath (void)*/
/*
 *	called to get the avrdude executable path.
 */
QString ArduinoConfiguration::avrdudePath (void)
{
	return _avrdudePath;
}
/*}}}*/
/*{{{  void ArduinoConfiguration::applyParams (void)*/
/*
 *	called when the 'apply' button is clicked (via Parameters::applyClicked()).
 */
void ArduinoConfiguration::applyParams (void)
{
	emit noccSpecsPathChanged ();
	emit noccPathChanged ();
	emit noccParamsChanged ();
	_changed = false;

	/* at this point, try and run the compiler with some help options to populate the in-editor thing */
	QProcess proc (this);

#ifdef Q_OS_WIN32
	/* running on Windows/cygwin, so make sure we set this for nocc */
	proc.setEnvironment (QProcess::systemEnvironment() << "CYGWIN=nodosfilewarning");
#endif

	proc.setProgram (_noccPath);
	proc.setArguments (QStringList() << "--specs-file" << _noccSpecsPath << "--help-ful");

	proc.start ();
	_noccUsageBrowser->clear ();
	if (!proc.waitForFinished (3000)) {		/* wait for 3 seconds or give up */
		QString msg = "Failed to run nocc, error was \"";
		
		msg.append (proc.errorString ());
		msg.append ("\"");

		_noccUsageBrowser->setPlainText (msg);
	} else {
		QString msg = proc.readAllStandardError ();
		
		msg.append (proc.readAllStandardOutput ());

		_noccUsageBrowser->setPlainText (msg);
	}

	return;
}
/*}}}*/
/*{{{  void ArduinoConfiguration::cancelParams ()*/
/*
 *	called when the 'cancel' button is clicked.
 */
void ArduinoConfiguration::cancelParams ()
{
	if (_changed) {
		_noccPath = _noccOldPath;
		_noccPathLbl->setText (_noccOldPath);

		_noccSpecsPath = _noccOldSpecsPath;
		_noccSpecsPathLbl->setText (_noccOldSpecsPath);

		_noccParams = _noccOldParams;
		_noccParamsList = _noccParams.split (" ", QString::SkipEmptyParts);
	}
	_changed = false;
}
/*}}}*/

