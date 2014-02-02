/*
 *	mainwindow.cpp -- AVR-ASM-IDE main window.
 *	Copyright (C) 2013 Gregoire Liglet and Florent Chiron, University of Kent.
 *
 *	File based on QT example code, Copyright (C) 2004-2006 Trolltech ASA.
 *
 *	Modifications (C) 2013 Fred Barnes, University of Kent <frmb@kent.ac.uk>
 */

/****************************************************************************
**
** Copyright (C) 2004-2006 Trolltech ASA. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** Licensees holding a valid Qt License Agreement may use this file in
** accordance with the rights, responsibilities and obligations
** contained therein.  Please consult your licensing agreement or
** contact sales@trolltech.com if any conditions of this licensing
** agreement are not clear to you.
**
** Further information about Qt licensing is available at:
** http://www.trolltech.com/products/qt/licensing.html or by
** contacting info@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#include <iostream>

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QGridLayout>
#include <QIcon>
#include <QIODevice>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPalette>
#include <QPlainTextEdit>
#include <QPoint>
#include <QProgressBar>
#include <QProcess>
#include <QPushButton>
#include <QSet>
#include <QSettings>
#include <QSignalMapper>
#include <QSize>
#include <QStatusBar>
#include <QTextCharFormat>
#include <QTextStream>
#include <QToolBar>
#include <Qsci/qscilexercpp.h>

#include <Qsci/qsciscintilla.h>

#include "mainwindow.h"
#include "parameters.h"
#include "avrasmlexer.h"


MainWindow *globMainWindow = 0;


/*{{{  MainWindow::MainWindow ()*/
/*
 *	constructor
 */
MainWindow::MainWindow ()
{
	QIcon icon (":icon.icon");
	QStringList args;
	int i;
	QString lfname = "";

	_isFillingLog = false;
	this->setWindowIcon (icon);
	createOptionDialog ();
	readSettings ();
	initTextEdit ();
	createActions ();
	createMenus ();
	createToolBars ();
	createStatusBar ();

	connect (_textEdit, SIGNAL (textChanged ()), this, SLOT (documentWasModified ()));
	connect (_console, SIGNAL (cursorPositionChanged ()), this, SLOT (consoleCursorPosChange ()));

	setCurrentFile ("");
	createProcesses ();

	args = qApp->arguments ();
	for (i=1; i<args.count(); i++) {
		QString str = args.at (i);

		if (!str.startsWith ("-")) {
			/* probably a file-name */
			lfname = str;
		}
		// qDebug() << "MainWindow::MainWindow(): args.at(" << i << ") = [" << str << "]";
	}

	if (!lfname.isEmpty ()) {
		loadFile (lfname);
	}
}

/*}}}*/
/*{{{  MainWindow::~MainWindow ()*/
/*
 *	destructor.
 */
MainWindow::~MainWindow ()
{
}

/*}}}*/

/*{{{  void MainWindow::initTextEdit (void)*/
/*
 *	initialises the text-editor part
 */
void MainWindow::initTextEdit (void)
{
	QVBoxLayout *layout = new QVBoxLayout;

	_console = new QPlainTextEdit ();
	_console->setReadOnly (true);

#ifdef Q_OS_UNIX
	QFont newFont ("Courier 10 Pitch", 10, 1, 0);
#else
	QFont newFont ("Lucida console", 10, 1, 0);
#endif

	_console->setFont (newFont);

	QPalette p = _console->palette ();

	p.setColor (QPalette::Active, QPalette::Base, Qt::black);
	p.setColor (QPalette::Inactive, QPalette::Base, Qt::black);
	p.setColor (QPalette::Text, Qt::white);

	_console->setPalette (p);

	setCentralWidget (new QWidget);
	_textEdit = new QsciScintilla;
	_textEdit->setEolMode (QsciScintilla::EolUnix);
	layout->addWidget (_textEdit, 3);
	layout->addWidget (_console, 1);

	centralWidget ()->setLayout (layout);

	_lexer = new AVRASMLexer (_textEdit);
	//    QsciLexerCPP *lexer = new QsciLexerCPP(_textEdit);
	//check nocc here
	_lexer->setNoccPath (_params->arduinoConfig()->noccPath (), _params->arduinoConfig()->noccSpecsPath ());
	_lexer->setParameters (_params);
	_textEdit->setLexer (_lexer);
	_textEdit->setMarginLineNumbers (1, true);
	_textEdit->setMarginWidth (1, "-----");

}

/*}}}*/
/*{{{  void MainWindow::createOptionDialog (void)*/
/*
 *	create the options dialogue.
 */
void MainWindow::createOptionDialog (void)
{
	_params = &(Parameters::getInstance (true));
	connect (_params->arduinoConfig (), SIGNAL (noccPathChanged ()), SLOT (updateNoccPath ()));
}

/*}}}*/
/*{{{  void MainWindow::updateNoccPath (void)*/
/*
 *	updates the lexer's nocc path from the configuration setting.
 */
void MainWindow::updateNoccPath (void)
{
	_lexer->setNoccPath (_params->arduinoConfig()->noccPath (), _params->arduinoConfig()->noccSpecsPath ());
}

/*}}}*/
/*{{{  void MainWindow::closeEvent (QCloseEvent *event)*/
/*
 *	called when a close event happens.
 */
void MainWindow::closeEvent (QCloseEvent *event)
{
	if (maybeSave ()) {
		writeSettings ();
		event->accept ();
	} else {
		event->ignore ();
	}
}
/*}}}*/
/*{{{  void MainWindow::createProcesses (void)*/
/*
 *	sets-up handlers for build and send (compile+upload) processes.
 */
void MainWindow::createProcesses (void)
{
	QStringList env = QProcess::systemEnvironment ();

	env << "CYGWIN=nodosfilewarning";
	_buildProcess.setEnvironment (env);
	connect (&_buildProcess, SIGNAL (finished (int)), SLOT (buildFinished (int)));
	connect (&_buildProcess, SIGNAL (readyReadStandardOutput ()), this, SLOT (readyReadStandardOutput ()));
	connect (&_buildProcess, SIGNAL (readyReadStandardError ()), this, SLOT (readyReadStandardError ()));

	connect (&_sendProcess, SIGNAL (finished (int, QProcess::ExitStatus)), SLOT (sendToBoardFinished (int, QProcess::ExitStatus)));
	connect (&_sendProcess, SIGNAL (readyReadStandardOutput ()), this, SLOT (readyReadStandardOutput ()));
	connect (&_sendProcess, SIGNAL (readyReadStandardError ()), this, SLOT (readyReadStandardError ()));
}
/*}}}*/

/*{{{  bool MainWindow::build (void)*/
/*
 *	called to run nocc to build the application.
 *	returns true on success, false otherwise.
 */
bool MainWindow::build (void)
{
	// qDebug () << "building " << _curFile << " " << *(_params->arduinoConfig ()->noccProcessedParams (_curFile));

	if (_curFile.isEmpty ()) {
		QMessageBox::information (this, tr (APP_NAME), tr ("You must open a script to build it."), QMessageBox::Ok | QMessageBox::Escape);
		return false;
	}

	QStringList *parameters = _params->arduinoConfig()->noccProcessedParams (_curFile);

	if (saveBuild ()) {
		statusBar ()->showMessage (tr ("Building..."));

		/* add specs-file setting to start of parameters */
		parameters->insert (0, "--specs-file");
		parameters->insert (1, _params->arduinoConfig()->noccSpecsPath ());

		QString tmp = _params->arduinoConfig()->noccPath ();
		for (QStringList::Iterator it = parameters->begin (); it != parameters->end (); ++it) {
			tmp.append (" ");
			tmp.append ((*it));
			// std::cout << "param " << (*it).toStdString () << std::endl;
		}
		logDebug (tmp);

		_buildProcess.start (_params->arduinoConfig()->noccPath (), *parameters);
		if (!_buildProcess.waitForFinished (5000)) {			// wait for it to finish completely (5 seconds max).
			/* failed to finish or crashed, so report */
			statusBar ()->showMessage (tr ("Failed to start nocc"));
			qDebug () << "Failed to start nocc for build, code " << _buildProcess.errorString();
		} else if (_buildProcess.error () != QProcess::UnknownError) {
			QMessageBox *msg = new QMessageBox (globMainWindow);
			msg->setIcon (QMessageBox::Critical);
			msg->setWindowTitle (tr ("Error"));
			msg->setText (tr ("An error occured while opening nocc.\nPlease check the path of the binary file\nand its parameters in the settings."));
			msg->setModal (true);
			msg->show ();
		}
	}
	return true;
}
/*}}}*/
/*{{{  void MainWindow::buildFinished (int exitCode)*/
/*
 *	called when the build (compile) is finished
 */
void MainWindow::buildFinished (int exitCode)
{
	if (exitCode == 0) {
		logInfo (_noccLog);
		logInfo (_noccError);
		logInfo ("Build complete with success !");
	} else {
		logInfo (_noccLog);
		logError (_noccError);
		logError ("Build failed.");
		statusBar ()->showMessage (tr ("Build complete"));
	}
	_noccError.clear ();
	_noccLog.clear ();
}
/*}}}*/

/*{{{  bool MainWindow::sendToBoard (void)*/
/*
 *	called to send the compiled program to the board (flash and EEPROM hex files).
 *	returns true on success, false otherwise.
 */
bool MainWindow::sendToBoard (void)
{
	if (_curFile.isEmpty ()) {
		QMessageBox::information (this, tr (APP_NAME), tr ("You must open a script to build it."), QMessageBox::Ok | QMessageBox::Escape);
		return false;
	}

	QDir *dir = new QDir ();
	QString relativeCurfile = dir->relativeFilePath (_curFile);
	QString binFile = relativeCurfile;

	binFile.replace (QString (".asm"), QString (".flash.hex"));

	QString eepromFile = relativeCurfile;

	eepromFile.replace (QString (".asm"), QString (".eeprom.hex"));

	QStringList *parameters = _params->arduinoConfig ()->avrdudeProcessedParams (binFile, eepromFile);

	if (saveBuild ()) {
		QString tmp = _params->arduinoConfig ()->avrdudePath ();
		for (QStringList::Iterator it = parameters->begin (); it != parameters->end (); ++it) {
			tmp.append (" ");
			tmp.append ((*it));
			// std::cout << "param " << (*it).toStdString () << std::endl;
		}
		logDebug (tmp);
		_sendProcess.start (_params->arduinoConfig ()->avrdudePath (), *parameters);
		if (_sendProcess.error () != QProcess::UnknownError) {
			QMessageBox *msg = new QMessageBox (globMainWindow);

			msg->setIcon (QMessageBox::Critical);
			msg->setWindowTitle (tr ("Error"));
			msg->setText (tr ("An error occured while opening avrdude.\nPlease check the path of the binary file in the settings."));
			msg->setModal (true);
			msg->show ();
		}
	}
	return true;
}
/*}}}*/
/*{{{  void MainWindow::sendToBoardFinished (int exitCode, QProcess::ExitStatus exitStatus)*/
/*
 *	called when the code which uploads to the arduino is done.
 */
void MainWindow::sendToBoardFinished (int exitCode, QProcess::ExitStatus exitStatus)
{
	logInfo (_avrdudeLog);
	logInfo (_avrdudeError);
	if (exitCode == 0) {
		logInfo (_avrdudeLog);
		logInfo (_avrdudeError);
		logInfo ("Upload to board succeeded !");
	} else {
		logInfo (_avrdudeLog);
		logError (_avrdudeError);
		logError ("Upload to board failed.");
		statusBar ()->showMessage (tr ("Build complete"));
	}
	_avrdudeLog.clear ();
	_avrdudeError.clear ();
}
/*}}}*/

/*{{{  void MainWindow::buildAndRun (void)*/
/*
 *	builds and runs the program.
 */
void MainWindow::buildAndRun (void)
{
	if (build ()) {
		sendToBoard ();
	}
}
/*}}}*/
/*{{{  void MainWindow::readyReadStandardOutput (void)*/
/*
 *	called when the build or send processes have stdout available to read.
 */
void MainWindow::readyReadStandardOutput (void)
{
	_noccLog.append (_buildProcess.readAllStandardOutput ());
	_avrdudeLog.append (_sendProcess.readAllStandardOutput ());
	//    logInfo(_buildProcess.readAllStandardOutput());
	//    logInfo(_sendProcess.readAllStandardOutput());
}
/*}}}*/
/*{{{  void MainWindow::readyReadStandardError (void)*/
/*
 *	called when the build or send processes have stderr available to read.
 */
void MainWindow::readyReadStandardError (void)
{
	_noccError.append (_buildProcess.readAllStandardError ());
	_avrdudeError.append (_sendProcess.readAllStandardError ());
	//    logError(_buildProcess.readAllStandardError());
	//    logError(_sendProcess.readAllStandardError());

}
/*}}}*/

/*{{{  void MainWindow::showOption (void)*/
/*
 *	called to display the options (parameters) dialogue.
 */
void MainWindow::showOption (void)
{
	if (this->_params) {
		_params->show ();
	} else {
		std::cerr << "MainWindow::showOption(): params window not initialized" << std::endl;
		exit (1);
	}
}
/*}}}*/
/*{{{  void MainWindow::newFile (void)*/
/*
 *	called when the 'new' button is clicked (or select in menu).
 */
void MainWindow::newFile (void)
{
	if (maybeSave ()) {
		_textEdit->clear ();
		setCurrentFile ("");
	}
}
/*}}}*/
/*{{{  void MainWindow::open (void)*/
/*
 *	called when the 'open' button is clicked (or select in menu).
 */
void MainWindow::open (void)
{
	if (maybeSave ()) {
		QString fileName = QFileDialog::getOpenFileName (this);

		if (!fileName.isEmpty ()) {
			loadFile (fileName);
		}
	}
}
/*}}}*/
/*{{{  void MainWindow::open (QString fileName)*/
/*
 *	called to open a specific file;  maybe saves current first.
 */
void MainWindow::open (QString fileName)
{
	if (maybeSave ()) {
		if (!fileName.isEmpty ()) {
			loadFile (fileName);
		}
	}
}
/*}}}*/
/*{{{  bool MainWindow::save (void)*/
/*
 *	called when the 'save' button is clicked (or select in menu).
 *	returns true on success, false otherwise.
 */
bool MainWindow::save (void)
{
	if (_curFile.isEmpty ()) {
		return saveAs ();
	} else {
		return saveFile (_curFile);
	}
}
/*}}}*/
/*{{{  bool MainWindow::saveAs (void)*/
/*
 *	called when the 'save-as' button is clicked (or select in menu).
 *	returns true on success, false otherwise.
 */
bool MainWindow::saveAs (void)
{
	QString fileName = QFileDialog::getSaveFileName (this);
	if (fileName.isEmpty ())
		return false;

	return saveFile (fileName);
}
/*}}}*/
/*{{{  void MainWindow::about (void)*/
/*
 *	called when the 'about' button is clicked (or select in menu).
 */
void MainWindow::about (void)
{
	this->setWindowIcon (QIcon ("icon.ico"));
	QMessageBox::about (this, tr ("About AVR-ASM-IDE"),
			    tr
			    ("The <b>AVR-ASM-IDE</b> is an easy to use assembly programing language editor." \
			     "With the IDE developed using the Qt framework, it provides for the assembling " \
			     "and uploading of AVR code to Arduino boards using the nocc compiler framework and avrdude."));
}
/*}}}*/

/*{{{  void MainWindow::documentWasModified (void)*/
/*
 *	sets the modified-state based on the text-editor's modification state.
 */
void MainWindow::documentWasModified (void)
{
	setWindowModified (_textEdit->isModified ());
}
/*}}}*/

/*{{{  void MainWindow::createActions (void)*/
/*
 *	create the various actions.
 */
void MainWindow::createActions (void)
{
	_buildAct = new QAction (QIcon (":/images/wrench32.png"), tr ("&Build"), this);
	_buildAct->setShortcut (tr ("Ctrl+B"));
	_buildAct->setStatusTip (tr ("Build the current script"));
	connect (_buildAct, SIGNAL (triggered ()), this, SLOT (build ()));

	_buildAndSendAct = new QAction (QIcon (":/images/buildandrun32.png"), tr ("&Build and run"), this);
	_buildAndSendAct->setShortcut (QKeySequence ("Ctrl+Shift+B"));
	_buildAndSendAct->setStatusTip (tr ("Build the current script and upload it to the board"));
	connect (_buildAndSendAct, SIGNAL (triggered ()), this, SLOT (buildAndRun ()));

	_cleanAct = new QAction (QIcon (":/images/clean32.png"), tr ("&Clear console"), this);
	_cleanAct->setStatusTip (tr ("Clear the content of the console"));
	connect (_cleanAct, SIGNAL (triggered ()), this, SLOT (resetConsole ()));

	_sendToBoardAct = new QAction (QIcon (":/images/arrow32.png"), tr ("&Send to board"), this);
	_sendToBoardAct->setShortcut (tr ("Ctrl+R"));
	_sendToBoardAct->setStatusTip (tr ("Upload the current script on the board"));
	connect (_sendToBoardAct, SIGNAL (triggered ()), this, SLOT (sendToBoard ()));

	_optionAct = new QAction (QIcon (":/images/gears32.png"), tr ("&Options"), this);
	_optionAct->setStatusTip (tr ("Shows the options menu"));
	connect (_optionAct, SIGNAL (triggered ()), this, SLOT (showOption ()));

	_newAct = new QAction (QIcon (":/images/new.png"), tr ("&New"), this);
	_newAct->setShortcut (tr ("Ctrl+N"));
	_newAct->setStatusTip (tr ("Create a new file"));
	connect (_newAct, SIGNAL (triggered ()), this, SLOT (newFile ()));

	_openAct = new QAction (QIcon (":/images/open.png"), tr ("&Open..."), this);
	_openAct->setShortcut (tr ("Ctrl+O"));
	_openAct->setStatusTip (tr ("Open an existing file"));
	connect (_openAct, SIGNAL (triggered ()), this, SLOT (open ()));

	_saveAct = new QAction (QIcon (":/images/save.png"), tr ("&Save"), this);
	_saveAct->setShortcut (tr ("Ctrl+S"));
	_saveAct->setStatusTip (tr ("Save the document to disk"));
	connect (_saveAct, SIGNAL (triggered ()), this, SLOT (save ()));

	_saveAsAct = new QAction (tr ("Save &As..."), this);
	_saveAsAct->setStatusTip (tr ("Save the document under a new name"));
	connect (_saveAsAct, SIGNAL (triggered ()), this, SLOT (saveAs ()));

	_exitAct = new QAction (tr ("E&xit"), this);
	_exitAct->setShortcut (tr ("Ctrl+Q"));
	_exitAct->setStatusTip (tr ("Exit the application"));
	connect (_exitAct, SIGNAL (triggered ()), this, SLOT (close ()));

	_cutAct = new QAction (QIcon (":/images/cut.png"), tr ("Cu&t"), this);
	_cutAct->setShortcut (tr ("Ctrl+X"));
	_cutAct->setStatusTip (tr ("Cut the current selection's contents to the " "clipboard"));
	connect (_cutAct, SIGNAL (triggered ()), _textEdit, SLOT (cut ()));

	_copyAct = new QAction (QIcon (":/images/copy.png"), tr ("&Copy"), this);
	_copyAct->setShortcut (tr ("Ctrl+C"));
	_copyAct->setStatusTip (tr ("Copy the current selection's contents to the " "clipboard"));
	connect (_copyAct, SIGNAL (triggered ()), _textEdit, SLOT (copy ()));

	_pasteAct = new QAction (QIcon (":/images/paste.png"), tr ("&Paste"), this);
	_pasteAct->setShortcut (tr ("Ctrl+V"));
	_pasteAct->setStatusTip (tr ("Paste the clipboard's contents into the current " "selection"));
	connect (_pasteAct, SIGNAL (triggered ()), _textEdit, SLOT (paste ()));

	_aboutAct = new QAction (tr ("&About"), this);
	_aboutAct->setStatusTip (tr ("Show the application's About box"));
	connect (_aboutAct, SIGNAL (triggered ()), this, SLOT (about ()));

	_aboutQtAct = new QAction (tr ("About &Qt"), this);
	_aboutQtAct->setStatusTip (tr ("Show the Qt library's About box"));
	connect (_aboutQtAct, SIGNAL (triggered ()), qApp, SLOT (aboutQt ()));

	_cutAct->setEnabled (false);
	_copyAct->setEnabled (false);
	connect (_textEdit, SIGNAL (copyAvailable (bool)), _cutAct, SLOT (setEnabled (bool)));
	connect (_textEdit, SIGNAL (copyAvailable (bool)), _copyAct, SLOT (setEnabled (bool)));
}
/*}}}*/
/*{{{  void MainWindow::createMenus (void)*/
/*
 *	creates the drop-down menus.
 */
void MainWindow::createMenus (void)
{
	_fileMenu = menuBar ()->addMenu (tr ("&File"));
	_fileMenu->addAction (_newAct);
	_fileMenu->addAction (_openAct);
	_fileMenu->addAction (_saveAct);
	_fileMenu->addAction (_saveAsAct);
	_fileMenu->addSeparator ();
	_exampleMenu = _fileMenu->addMenu (tr ("&Examples"));
	_exampleMenu->addActions (buildExampleMenu ());
	_fileMenu->addSeparator ();
	_fileMenu->addAction (_exitAct);

	_editMenu = menuBar ()->addMenu (tr ("&Edit"));
	_editMenu->addAction (_cutAct);
	_editMenu->addAction (_copyAct);
	_editMenu->addAction (_pasteAct);
	_editMenu->addSeparator ();
	_editMenu->addAction (_optionAct);

	_buildMenu = menuBar ()->addMenu (tr ("&Build"));
	_buildMenu->addAction (_buildAct);
	_buildMenu->addAction (_sendToBoardAct);
	_buildMenu->addAction (_buildAndSendAct);

	//    menuBar()->addSeparator();

	_helpMenu = menuBar ()->addMenu (tr ("&Help"));
	_helpMenu->addAction (_aboutAct);
	_helpMenu->addAction (_aboutQtAct);
}
/*}}}*/
/*{{{  void MainWindow::createToolBars (void)*/
/*
 *	creates the tool-bar.
 */
void MainWindow::createToolBars (void)
{
	_fileToolBar = addToolBar (tr ("File"));
	_fileToolBar->addAction (_newAct);
	_fileToolBar->addAction (_openAct);
	_fileToolBar->addAction (_saveAct);

	_editToolBar = addToolBar (tr ("Edit"));
	_editToolBar->addAction (_cutAct);
	_editToolBar->addAction (_copyAct);
	_editToolBar->addAction (_pasteAct);

	_buildToolBar = addToolBar (tr ("Build"));
	_buildToolBar->addAction (_buildAct);
	_buildToolBar->addAction (_sendToBoardAct);
	_buildToolBar->addAction (_buildAndSendAct);
	_buildToolBar->addAction (_cleanAct);
}
/*}}}*/
/*{{{  void MainWindow::createStatusBar (void)*/
/*
 *	creates the status-bar.
 */
void MainWindow::createStatusBar (void)
{
	statusBar ()->showMessage (tr ("Ready"));
}
/*}}}*/
/*{{{  void MainWindow::readSettings (void)*/
/*
 *	reads saved settings and applies.
 */
void MainWindow::readSettings (void)
{
	QSettings settings ("unikent", "avr-asm-ide");

	QPoint pos = settings.value ("pos", QPoint (200, 200)).toPoint ();
	QSize size = settings.value ("size", QSize (400, 400)).toSize ();

	resize (size);
	move (pos);
}
/*}}}*/
/*{{{  void MainWindow::writeSettings (void)*/
/*
 *	writes out settings.
 */
void MainWindow::writeSettings (void)
{
	QSettings settings ("unikent", "avr-asm-ide");

	settings.setValue ("pos", pos ());
	settings.setValue ("size", size ());
}
/*}}}*/
/*{{{  bool MainWindow::maybeSave (void)*/
/*
 *	maybe saves the editor content (prompting user first).
 *	returns true on success (unchanged, saved, or changes discarded) false otherwise.
 */
bool MainWindow::maybeSave (void)
{
	if (_textEdit->isModified ()) {
		int ret = QMessageBox::warning (this, tr (APP_NAME),
						tr ("The document has been modified.\n" "Do you want to save your changes?"),
						QMessageBox::Yes | QMessageBox::Default,
						QMessageBox::No,
						QMessageBox::Cancel | QMessageBox::Escape);
		if (ret == QMessageBox::Yes) {
			return save ();
		} else if (ret == QMessageBox::Cancel) {
			return false;
		}
	}
	return true;
}
/*}}}*/

/*{{{  bool MainWindow::saveBuild (void)*/
/*
 *	prompts for save before build.
 *	returns true if unchanged or saved, false otherwise.
 */
bool MainWindow::saveBuild (void)
{
	if (_textEdit->isModified ()) {
		int ret = QMessageBox::warning (this, tr (APP_NAME),
						tr ("The document has been modified.\n" "Do you want to save your changes?"),
						QMessageBox::Yes | QMessageBox::Default,
						QMessageBox::No,
						QMessageBox::Cancel | QMessageBox::Escape);
		if (ret == QMessageBox::Yes) {
			return save ();
		} else if (ret == QMessageBox::Cancel) {
			return false;
		} else if (ret == QMessageBox::No) {
			return false;
		}
	}
	return true;
}
/*}}}*/
/*{{{  void MainWindow::loadFile (const QString &fileName)*/
/*
 *	called to load a file into the editor.
 */
void MainWindow::loadFile (const QString &fileName)
{
	QFile file (fileName);
	if (!file.open (QFile::ReadOnly)) {
		QMessageBox::warning (this, tr (APP_NAME), tr ("Cannot read file %1:\n%2.")
				      .arg (fileName)
				      .arg (file.errorString ()));
		return;
	}

	QTextStream in (&file);
	QApplication::setOverrideCursor (Qt::WaitCursor);
	_textEdit->setText (in.readAll ());
	QApplication::restoreOverrideCursor ();

	setCurrentFile (fileName);
	statusBar ()->showMessage (tr ("File loaded"), 2000);
}
/*}}}*/
/*{{{  bool MainWindow::saveFile (const QString &fileName)*/
/*
 *	saves the editor content to the specified file-name.
 *	returns true on success, false otherwise.
 */
bool MainWindow::saveFile (const QString &fileName)
{
	QFile file (fileName);
	if (!file.open (QFile::WriteOnly)) {
		QMessageBox::warning (this, tr (APP_NAME), tr ("Cannot write file %1:\n%2.")
				      .arg (fileName)
				      .arg (file.errorString ()));
		return false;
	}

	QTextStream out (&file);
	QApplication::setOverrideCursor (Qt::WaitCursor);
	out << _textEdit->text ();
	QApplication::restoreOverrideCursor ();

	setCurrentFile (fileName);
	statusBar ()->showMessage (tr ("File saved"), 2000);
	return true;
}
/*}}}*/

/*{{{  void MainWindow::setCurrentFile (const QString &fileName)*/
/*
 *	sets the current file name in the local class and window title, etc.
 */
void MainWindow::setCurrentFile (const QString &fileName)
{
	QDir *dir = new QDir ();

	_curFile = dir->relativeFilePath (fileName);
	_textEdit->setModified (false);
	setWindowModified (false);

	QString shownName;
	if (_curFile.isEmpty ()) {
		shownName = "untitled.txt";
	} else {
		shownName = strippedName (_curFile);
	}

	setWindowTitle (tr ("%1[*] - %2").arg (shownName).arg (tr (APP_NAME)));
}
/*}}}*/
/*{{{  QString MainWindow::strippedName (const QString &fullFileName)*/
/*
 *	gets the last part of a file name (stripped).
 */
QString MainWindow::strippedName (const QString &fullFileName)
{
	return QFileInfo (fullFileName).fileName ();
}
/*}}}*/

/*{{{  void MainWindow::resetConsole (void)*/
/*
 *	resets (clears) the console.
 */
void MainWindow::resetConsole (void)
{
	_console->clear ();
}
/*}}}*/
/*{{{  void MainWindow::consoleCursorPosChange (void)*/
/*
 *	signalled when cursor position is changed on the console (selecting it?)
 */
void MainWindow::consoleCursorPosChange (void)
{
	QTextCursor tcur = _console->textCursor ();
	int tpos_start, tpos_end;
	static int protectme = 0;
	QTextDocument *doc = _console->document ();

	if (!protectme && !_isFillingLog && !_curFile.isEmpty()) {
		int i;
		QString makeline ("");
		QString m1 ("nocc: ");

		/* incase movePosition triggers an update when we call setTextCursor */
		protectme++;

		/* rewind to start-of-line */
		tcur.movePosition (QTextCursor::StartOfLine, QTextCursor::MoveAnchor, 1);
		tpos_start = tcur.position ();
		tcur.movePosition (QTextCursor::EndOfLine, QTextCursor::MoveAnchor, 1);
		tpos_end = tcur.position ();

		/* characters between tpos_start and (tpos_end-1) are the line */
		/* looking for something like "nocc: FILENAME:LINE (error|warning|info|notice)" */
		/* XXX: this is *grim*, but I can't see an obviously cleaner way other than using
		 *	something other than a QPlainTextArea (e.g. QStringList)
		 */
		for (i=0; (i<6) && ((i+tpos_start) < tpos_end) && (doc->characterAt (i+tpos_start) == m1.at (i)); i++);
		if (i == 6) {
			/* matched "nocc: " */
			QFileInfo cfileinfo (_curFile);
			QString fname = cfileinfo.fileName ();

			for (i=0; (i<fname.count()) && ((i+tpos_start+6) < tpos_end) && (doc->characterAt (i+tpos_start+6) == fname.at (i)); i++);
			if (i == fname.count ()) {
				QString nval ("");

				tpos_start += (i + 7);			/* adjust tpos_start to be from the "LINE (...) */

				// qDebug() << "fname part is: " << fname;
				for (i=0; (i+tpos_start) < tpos_end; i++) {
					if (doc->characterAt (i+tpos_start).isDigit ()) {
						nval.append (doc->characterAt (i+tpos_start));
					} else {
						break;			/* for() */
					}
				}
				/* skip whitespace */
				for (; ((i+tpos_start) < tpos_end) && (doc->characterAt (i+tpos_start).isSpace ()); i++);

				/* scoop up the rest */
				tpos_start += i;
				for (i=0; (i+tpos_start) < tpos_end; i++) {
					makeline.append (doc->characterAt (i+tpos_start));
				}

				if (!nval.isEmpty ()) {
					bool c_ok;
					int line;

					line = nval.toInt (&c_ok, 10);
					if (c_ok) {
						/* got a sensible looking line number */
						//qDebug() << "found current file line " << line << " in stuff";
						_textEdit->setCursorPosition (line-1, 0);
						_textEdit->setFocus (Qt::OtherFocusReason);
					}
				}
			}
#if 0
			QRegExp re_err (QString ("nocc: ").append (fname).append (":").append);


			for (i=tpos_start; i<tpos_end; i++) {
				makeline.append (doc->characterAt (i));
			}
#endif
		}

		// qDebug() << "Selected line: " << makeline;
		// _console->setTextCursor (tcur);

		protectme--;
	}

	// qDebug () << "consoleCursorPosChange!  position is: " << tpos;
}
/*}}}*/
/*{{{  void MainWindow::logInfo (QString msg)*/
/*
 *	writes a string into the on-screen console (informational).
 */
void MainWindow::logInfo (QString msg)
{
	QTextCharFormat tf;

	if (msg.isEmpty ()) {
		/* nothing! */
		return;
	}
	_isFillingLog = true;
	tf = _console->currentCharFormat ();
	tf.setForeground (QBrush (Qt::green));

	_console->setCurrentCharFormat (tf);
	_console->appendPlainText (QString ("info: ").append (msg));
	_isFillingLog = false;
}
/*}}}*/
/*{{{  void MainWindow::logWarning (QString msg)*/
/*
 *	writes a string into the on-screen console (warning)
 */
void MainWindow::logWarning (QString msg)
{
	QTextCharFormat tf;

	if (msg.isEmpty ()) {
		/* nothing! */
		return;
	}
	_isFillingLog = true;
	tf = _console->currentCharFormat ();
	tf.setForeground (QBrush (Qt::yellow));

	_console->setCurrentCharFormat (tf);
	_console->appendPlainText (QString ("warning: ").append (msg));
	_isFillingLog = false;
	return;
}
/*}}}*/
/*{{{  void MainWindow::logError (QString msg)*/
/*
 *	writes a string into the on-screen console (error)
 */
void MainWindow::logError (QString msg)
{
	QTextCharFormat tf;

	if (msg.isEmpty ()) {
		/* nothing! */
		return;
	}
	_isFillingLog = true;
	tf = _console->currentCharFormat ();
	tf.setForeground (QBrush (Qt::red));

	_console->setCurrentCharFormat (tf);

	/* see if there's a newline at the end */
	if (msg.endsWith ('\n')) {
		/* probably a complete output line, but remove trailing newline */
		msg.truncate (msg.count() - 1);

		_console->appendPlainText (msg);
	} else {
		// QString time = QDateTime::currentDateTime ().toString ("hh:mm:ss.zzz");
		_console->appendPlainText (QString ("error: ").append (msg));
	}
	_isFillingLog = false;
}
/*}}}*/
/*{{{  void MainWindow::logDebug (QString msg)*/
/*
 *	writes a string into the on-screen console (debugging)
 */
void MainWindow::logDebug (QString msg)
{
	QTextCharFormat tf;

	_isFillingLog = true;
	tf = _console->currentCharFormat ();
	tf.setForeground (QBrush ("#50ffff"));

	_console->setCurrentCharFormat (tf);
	_console->appendPlainText (QString ("debug: ").append (msg));
	_isFillingLog = false;
}
/*}}}*/

/*{{{  void MainWindow::openExample (int id)*/
/*
 *	opens one of the example files.
 */
void MainWindow::openExample (int id)
{
	if (id <= _exampleList.length ()) {
		QFileInfo info = QFileInfo (QString (DEFAULT_examplePath).append (_exampleList.at (id)));
		if (info.isFile ()) {
			open (QString (DEFAULT_examplePath).append (_exampleList.at (id)));
		} else {
			logError (QString ("Error opening ").append (_exampleList.at (id)));
		}
	}
}
/*}}}*/

/*{{{  QList<QAction *> MainWindow::buildExampleMenu (void)*/
/*
 *	builds the examples menu based on list contents.
 */
QList<QAction *> MainWindow::buildExampleMenu (void)
{
	QList <QAction *>actions;
	QSignalMapper *signalMapper = new QSignalMapper (this);
	int i = 0;

	_exampleList = exampleList ();
	for (QStringList::Iterator it = _exampleList.begin (); it != _exampleList.end (); ++it) {
		QAction *tmp = new QAction ((*it), this);

		connect (tmp, SIGNAL (triggered ()), signalMapper, SLOT (map ()));
		signalMapper->setMapping (tmp, i);
		actions.append (tmp);

		i++;
	}
	connect (signalMapper, SIGNAL (mapped (int)), this, SLOT (openExample (int)));

	return actions;
}
/*}}}*/
/*{{{  QStringList MainWindow::exampleList (void)*/
/*
 *	searches the default examples directory for .asm source files.
 */
QStringList MainWindow::exampleList (void)
{
	QDir *dir = new QDir (DEFAULT_examplePath);
	QStringList filters;
	QStringList exampleList;

	filters << "*.asm";
	dir->setNameFilters (filters);
	dir->setSorting (QDir::Name);
	exampleList = dir->entryList ();
	return exampleList;
}
/*}}}*/

