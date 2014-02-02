/*
 *	arduinoconfiguration.h -- holds nocc and avrdude configuration information
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


#ifndef ARDUINOCONFIGURATION_H
#define ARDUINOCONFIGURATION_H

#include <QDialog>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStringList>
#include <QStringRef>
#include <QWidget>
#include <QTextBrowser>

#include "optionconfig.h"

/* NOCC and avrdude settings */
#ifdef Q_OS_WIN32
#define DEFAULT_avrdudePath "c:\\frmb\\avrdude\\avrdude.exe"
#define DEFAULT_opt_C2 "c:\\frmb\\avrdude\\avrdude.conf"
#define DEFAULT_noccPath "c:\\frmb\\nocc\\nocc.exe"
#define DEFAULT_noccSpecsPath "c:\\frmb\\nocc\\nocc.specs.xml"
#define DEFAULT_noccParams "%filename% -I c:\\frmb\\nocc\\addons\\headers -E c:\\frmb\\nocc\\addons"
#define DEFAULT_opt_P2 "COM4"
#else
#define DEFAULT_avrdudePath "./utils/avrdude/linux/avrdude"
#define DEFAULT_opt_C2 "./utils/avrdude/linux/avrdude.conf"
#define DEFAULT_noccPath "./utils/nocc/linux/nocc"
#define DEFAULT_noccSpecsPath "./utils/nocc/linux/nocc.specs.xml"
#define DEFAULT_noccParams "%filename% -I ./utils/nocc/windows/addons/headers -E ./utils/nocc/windows/addons"
#define DEFAULT_opt_P2 "ttyACM0"
#endif

#define DEFAULT_opt_B2 ""
#define DEFAULT_opt_b "115200"
#define DEFAULT_opt_c "arduino"
#define DEFAULT_opt_D2 false
#define DEFAULT_opt_E2 ""
#define DEFAULT_opt_e false
#define DEFAULT_opt_F2 false
#define DEFAULT_opt_i ""
#define DEFAULT_opt_n false
#define DEFAULT_opt_O false
#define DEFAULT_opt_p "ATMEGA328P"
#define DEFAULT_opt_q false
#define DEFAULT_opt_s false
#define DEFAULT_opt_t false
#define DEFAULT_opt_U2 "flash:w:%filename%"
#define DEFAULT_opt_u false
#define DEFAULT_opt_V2 false
#define DEFAULT_opt_v false
#define DEFAULT_opt_x ""
#define DEFAULT_opt_Y2 ""
#define DEFAULT_opt_y ""


namespace Ui
{
	class ArduinoConfiguration;
}

class ArduinoConfiguration : public OptionConfig
{
Q_OBJECT
public:
	explicit ArduinoConfiguration (QWidget * parent = 0);
	~ArduinoConfiguration ();

	QString noccPath (void);
	QString noccSpecsPath (void);
	QString noccParams (void);
	QStringList *noccProcessedParams (QString);
	QStringList *avrdudeProcessedParams (QString, QString);
	QStringList avrdudeParams (void);
	QString avrdudePath (void);
	void applyParams (void);
	void cancelParams (void);

signals:
	void noccPathChanged (void);
	void noccSpecsPathChanged (void);
	void noccParamsChanged (void);
	void avrdudePathChanged (void);
	void avrdudeParamsChanged (void);

private slots:
	void noccChgPath (void);
	void noccChgSpecsPath (void);
	void noccChgParams (void);

	void setCustomMode (bool);
	void resetDefaults (void);

	void browseAvrdude (void);
	void browseAvrdudeConf (void);

private:
	Ui::ArduinoConfiguration *ui;

	void readSettings (void);
	void writeSettings (void);
	void buildUi (void);
	void initializeForms (void);
	void initComPorts (void);
	void retrieveValues (void);

	bool _changed;
	QPushButton *_noccBrowseBtn;
	QPushButton *_noccSpecsBrowseBtn;
	QString _noccDefaultPath;
	QString _noccPath;
	QString _noccOldPath;
	QString _noccSpecsPath;
	QString _noccOldSpecsPath;
	QLabel *_noccPathLbl;
	QLabel *_noccSpecsPathLbl;
	QString _noccDefaultParams;
	QString _noccParams;
	QString _noccOldParams;
	QStringList _noccParamsList;
	QStringList _noccDefaultParamsList;
	QPlainTextEdit *_noccParamsText;
	bool _customMode;
	QStringList _availablePorts;
	
	QTextBrowser *_noccUsageBrowser;
	//avrdude options

	QString _avrdudePath;
	QString _opt_B2;
	QString _opt_b;
	QString _opt_C2;
	QString _opt_c;
	bool _opt_D2;
	QString _opt_E2;
	bool _opt_e;
	bool _opt_F2;
	QString _opt_i;
	bool _opt_n;
	bool _opt_O;
	QString _opt_P2;
	QString _opt_p;
	bool _opt_q;
	bool _opt_s;
	bool _opt_t;
	QString _opt_U2;
	bool _opt_u;
	bool _opt_V2;
	bool _opt_v;
	QString _opt_x;
	QString _opt_Y2;
	QString _opt_y;
};

#endif	/* !ARDUINOCONFIGURATION_H */

