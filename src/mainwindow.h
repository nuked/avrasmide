/*
 *	mainwindow.h -- main-window for AVR-ASM-IDE.
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

// based on some code which had this copyright notice (I think):

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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QProcess>

#include "avrasmlexer.h"
#include "parameters.h"

#define EXIT_CODE_REBOOT 12345
#define DEFAULT_examplePath "./examples/"
#define APP_NAME "AVR-ASM-IDE"

class QAction;
class QMenu;
class QsciScintilla;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	~MainWindow();
	bool maybeSave();

protected:
	void closeEvent(QCloseEvent *);

private slots:
	bool build();
	bool sendToBoard (void);
	void showOption (void);
	void newFile (void);
	void open (void);
	void open (QString);
	bool save (void);
	bool saveAs (void);
	void about (void);
	void documentWasModified (void);
	void readyReadStandardOutput (void);
	void readyReadStandardError (void);
	void buildFinished (int);
	void sendToBoardFinished (int, QProcess::ExitStatus);
	void resetConsole (void);
	void buildAndRun (void);
	void updateNoccPath (void);
	void openExample (int);
	void consoleCursorPosChange (void);

private:
	void logWarning (QString);
	void logError (QString);
	void logInfo (QString);
	void logDebug (QString);
	void initTextEdit (void);
	void createActions (void);
	void createMenus (void);
	void createToolBars (void);
	void createStatusBar (void);
	void createOptionDialog (void);
	void createProcesses (void);
	void readSettings (void);
	void writeSettings (void);
	bool saveBuild (void);
	void loadFile (const QString &);
	bool saveFile (const QString &);
	void setCurrentFile (const QString &);
	QStringList exampleList (void);
	QList<QAction *> buildExampleMenu (void);
	QString strippedName (const QString &);

	QsciScintilla *_textEdit;
	AVRASMLexer *_lexer;

	QMenu *_fileMenu;
	QMenu *_editMenu;
	QMenu *_buildMenu;
	QMenu *_helpMenu;
	QMenu *_exampleMenu;
	QToolBar *_buildToolBar;
	QToolBar *_fileToolBar;
	QToolBar *_editToolBar;
	QAction *_buildAct;
	QAction *_sendToBoardAct;
	QAction *_buildAndSendAct;
	QAction *_newAct;
	QAction *_openAct;
	QAction *_optionAct;
	QAction *_saveAct;
	QAction *_saveAsAct;
	QAction *_exitAct;
	QAction *_cutAct;
	QAction *_copyAct;
	QAction *_pasteAct;
	QAction *_aboutAct;
	QAction *_aboutQtAct;
	QAction *_cleanAct;
	Parameters *_params;
	QProcess _buildProcess;
	QProcess _sendProcess;
	QPlainTextEdit *_console;
	QString _curFile;
	QString _noccLog;
	QString _noccError;
	QString _avrdudeLog;
	QString _avrdudeError;
	QStringList _exampleList;

	bool _isFillingLog;		/* true if currently adding to the log/console */
};

extern MainWindow* globMainWindow;

#endif
