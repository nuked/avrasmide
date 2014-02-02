/*
 *	main.cpp -- wrapper to get things started.
 *	Copyright (C) 2013 Gregoir Liglet and Florent Chiron, Univeristy of Kent.
 *	Modifications, Fred Barnes, University of Kent  <frmb@kent.ac.uk>
 */

#include <iostream>

#include <QApplication>

#include "mainwindow.h"


int main(int argc, char *argv[])
{
	int currentExitCode = 0;
	Q_INIT_RESOURCE(application);

	while (true) {
		QApplication *app = new QApplication (argc, argv);
		MainWindow *mainWin = new MainWindow ();

		globMainWindow = mainWin;
		mainWin->show ();
		if (app->exec () != EXIT_CODE_REBOOT) {
			break;
		}
		delete mainWin;
		delete app;
	}

	return currentExitCode;
}


