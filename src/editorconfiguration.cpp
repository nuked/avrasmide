/*
 *	editorconfigation.cpp -- handlers for editor configuration window.
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

#include <QColorDialog>
#include <QDebug>
#include <QMainWindow>
// #include <QWindow>
#include <QMessageBox>
#include <QSettings>
#include <QSignalMapper>
#include <QVector>

#include "editorconfiguration.h"
#include "ui_editorconfiguration.h"
#include "mainwindow.h"
#include "optionconfig.h"


/*{{{  EditorConfiguration::EditorConfiguration (QWidget *parent) : OptionConfig (parent), ui (new Ui::EditorConfiguration)*/
/*
 *	constructor.
 */
EditorConfiguration::EditorConfiguration (QWidget *parent) : OptionConfig (parent), ui (new Ui::EditorConfiguration)
{
	ui->setupUi (this);
	_changed = false;
	readSettings ();
	initDefaults ();
	buildUi ();
}
/*}}}*/
/*{{{  EditorConfiguration::~EditorConfiguration ()*/
/*
 *	destructor.
 */
EditorConfiguration::~EditorConfiguration ()
{
	delete ui;
}
/*}}}*/

/*{{{  void EditorConfiguration::initDefaults (void)*/
/*
 *	initialises editor settings to their defaults and creates dialog boxes.
 */
void EditorConfiguration::initDefaults (void)
{
	_colorDialog = new QColorDialog ();
	_colorDialog->setModal (true);
	_fontDialog = new QFontDialog ();
	_fontDialog->setModal (true);
	initColors ();
}
/*}}}*/
/*{{{  void EditorConfiguration::initColors (void)*/
/*
 *	initialises colours (from compiled-in defaults).
 */
void EditorConfiguration::initColors (void)
{
	_colors.clear ();
	t_color c1 = { _backgroundColor, ui->textEdit_2, QString (DEFAULT_backgroundColor) };
	t_color c2 = { _keywordColor, ui->textEdit_3, QString (DEFAULT_keywordColor) };
	t_color c3 = { _symbolColor, ui->textEdit_4, QString (DEFAULT_symbolColor) };
	t_color c4 = { _numberColor, ui->textEdit_5, QString (DEFAULT_numberColor) };
	t_color c5 = { _stringColor, ui->textEdit_6, QString (DEFAULT_stringColor) };
	t_color c6 = { _commentColor, ui->textEdit_7, QString (DEFAULT_commentColor) };
	t_color c7 = { _nameColor, ui->textEdit_8, QString (DEFAULT_nameColor) };
	t_color c8 = { _cursorColor, ui->textEdit_9, QString (DEFAULT_cursorColor) };
	t_color c9 = { _lineColor, ui->textEdit_10, QString (DEFAULT_lineColor) };
	_colors.append (c1);
	_colors.append (c2);
	_colors.append (c3);
	_colors.append (c4);
	_colors.append (c5);
	_colors.append (c6);
	_colors.append (c7);
	_colors.append (c8);
	_colors.append (c9);
}
/*}}}*/
/*{{{  void EditorConfiguration::buildUi (void)*/
/*
 *	builds the user-interface signals and slots for editor configuration.
 */
void EditorConfiguration::buildUi (void)
{
	QSignalMapper *signalMapper = new QSignalMapper (this);

	connect (ui->pushButton_10, SIGNAL (clicked ()), SLOT (resetSettings ()));
	connect (ui->pushButton, SIGNAL (clicked ()), SLOT (showFontDialog ()));
	connect (_fontDialog, SIGNAL (currentFontChanged (QFont)), SLOT (changeFont (QFont)));
	connect (_colorDialog, SIGNAL (colorSelected (QColor)), SLOT (changeColor (QColor)));
	connect (ui->pushButton_2, SIGNAL (clicked ()), signalMapper, SLOT (map ()));
	connect (ui->pushButton_3, SIGNAL (clicked ()), signalMapper, SLOT (map ()));
	connect (ui->pushButton_4, SIGNAL (clicked ()), signalMapper, SLOT (map ()));
	connect (ui->pushButton_5, SIGNAL (clicked ()), signalMapper, SLOT (map ()));
	connect (ui->pushButton_6, SIGNAL (clicked ()), signalMapper, SLOT (map ()));
	connect (ui->pushButton_7, SIGNAL (clicked ()), signalMapper, SLOT (map ()));
	connect (ui->pushButton_8, SIGNAL (clicked ()), signalMapper, SLOT (map ()));
	connect (ui->pushButton_11, SIGNAL (clicked ()), signalMapper, SLOT (map ()));
	connect (ui->pushButton_12, SIGNAL (clicked ()), signalMapper, SLOT (map ()));

	signalMapper->setMapping (ui->pushButton_2, 0);
	signalMapper->setMapping (ui->pushButton_3, 1);
	signalMapper->setMapping (ui->pushButton_4, 2);
	signalMapper->setMapping (ui->pushButton_5, 3);
	signalMapper->setMapping (ui->pushButton_6, 4);
	signalMapper->setMapping (ui->pushButton_7, 5);
	signalMapper->setMapping (ui->pushButton_8, 6);
	signalMapper->setMapping (ui->pushButton_11, 7);
	signalMapper->setMapping (ui->pushButton_12, 8);

	connect (signalMapper, SIGNAL (mapped (int)), this, SLOT (showColorDialog (int)));
	connect (ui->pushButton_9, SIGNAL (clicked ()), SLOT (resetDefaults ()));

	applyColors ();
	applyFont ();
}
/*}}}*/

/*{{{  void EditorConfiguration::showFontDialog (void)*/
/*
 *	shows the font selection dialog.
 */
void EditorConfiguration::showFontDialog (void)
{
	_fontDialog->show ();
}
/*}}}*/
/*{{{  void EditorConfiguration::changeFont (QFont newFont)*/
/*
 *	called when the font-change widget is used (called with new font).
 */
void EditorConfiguration::changeFont (QFont newFont)
{
	_fontOld = _font;
	_fontSizeOld = _font.pointSize ();
	_fontWeightOld = _font.weight ();
	_fontItalicOld = _font.italic ();
	_fontFamilyOld = _font.family ();
	_font = newFont;
	ui->textEdit->clear ();
	ui->textEdit->setPlainText (_font.toString ());
	_fontFamilyOld = _fontFamily;
	_fontFamily = _font.family ();
	_fontWeightOld = _fontWeight;
	_fontWeight = _font.weight ();
	_fontSizeOld = _fontSize;
	_fontSize = _font.pointSize ();
	_fontItalicOld = _fontItalic;
	_fontItalic = _font.italic ();
	//    emit updateStyle();
	_changed = true;
}
/*}}}*/
/*{{{  void EditorConfiguration::showColorDialog (int id)*/
/*
 *	shows the colour picker dialog
 */
void EditorConfiguration::showColorDialog (int id)
{
	_currentId = id;
	_colorDialog->show ();
}
/*}}}*/
/*{{{  void EditorConfiguration::changeColor (QColor newColor)*/
/*
 *	called to change colour of _currentId (response to clicking the various change buttons).
 */
void EditorConfiguration::changeColor (QColor newColor)
{
	_backgroundColorOld = new QColor (_backgroundColor->name ());
	_keywordColorOld = new QColor (_keywordColor->name ());
	_symbolColorOld = new QColor (_symbolColor->name ());
	_numberColorOld = new QColor (_numberColor->name ());
	_stringColorOld = new QColor (_stringColor->name ());
	_commentColorOld = new QColor (_commentColor->name ());
	_nameColorOld = new QColor (_nameColor->name ());
	_cursorColorOld = new QColor (_cursorColor->name ());
	_lineColorOld = new QColor (_lineColor->name ());

	if (_currentId <= ID_MAX) {
		_colors[_currentId].color->setRgb (newColor.rgb ());
		_colors[_currentId].text->clear ();
		_colors[_currentId].text->setPlainText (QString (newColor.name ()));
		QPalette p = _colors[_currentId].text->palette ();
		p.setColor (QPalette::Base, _colors[_currentId].color->name ());
		p.setColor (QPalette::Text, Qt::white);
		_colors[_currentId].text->setPalette (p);
		_changed = true;
	}
}
/*}}}*/
/*{{{  void EditorConfiguration::applyColors (void)*/
/*
 *	applies colours.
 */
void EditorConfiguration::applyColors (void)
{
	QVector < t_color >::Iterator it = _colors.begin ();
	while (it != _colors.end ()) {
		(*it).text->clear ();
		(*it).text->setPlainText (QString ((*it).color->name ()));
		QPalette p = (*it).text->palette ();
		p.setColor (QPalette::Base, (*it).color->name ());
		p.setColor (QPalette::Text, Qt::white);
		(*it).text->setPalette (p);
		++it;
	}
}
/*}}}*/
/*{{{  void EditorConfiguration::applyFont (void)*/
/*
 *	applies font.
 */
void EditorConfiguration::applyFont (void)
{
	ui->textEdit->setPlainText (_font.toString ());
}
/*}}}*/
/*{{{  void EditorConfiguration::resetDefaults (void)*/
/*
 *	resets the editor settings to the compiled-in defaults.
 */
void EditorConfiguration::resetDefaults (void)
{
	resetDefaultFont ();
	resetDefaultColors ();
	_cursorSize = DEFAULT_cursorSize;
	ui->spinBox->setValue (_cursorSize);
	//    emit updateStyle();
}
/*}}}*/
/*{{{  void EditorConfiguration::resetDefaultFont (void)*/
/*
 *	resets font settings to compiled-in defaults.
 */
void EditorConfiguration::resetDefaultFont (void)
{
	_fontFamily = QString (DEFAULT_fontFamily);
	_fontSize = DEFAULT_fontSize;
	_fontWeight = DEFAULT_fontWeight;
	_fontItalic = DEFAULT_fontItalic;
	_font = QFont (_fontFamily);
	ui->textEdit->clear ();
	ui->textEdit->setPlainText (_font.family ());
}
/*}}}*/
/*{{{  void EditorConfiguration::resetDefaultColors (void)*/
/*
 *	resets colours to compiled-in defaults.
 */
void EditorConfiguration::resetDefaultColors (void)
{
	QVector < t_color >::Iterator it = _colors.begin ();

	while (it != _colors.end ()) {
		(*it).color->setNamedColor ((*it).defaultColor);
		(*it).text->clear ();
		(*it).text->setPlainText (QString ((*it).color->name ()));

		QPalette p = (*it).text->palette ();

		p.setColor (QPalette::Base, (*it).color->name ());
		p.setColor (QPalette::Text, Qt::white);

		(*it).text->setPalette (p);
		++it;
	}
}
/*}}}*/

/*{{{  void EditorConfiguration::readSettings (void)*/
/*
 *	reads editor settings from the global config.
 */
void EditorConfiguration::readSettings (void)
{
	QSettings settings ("unikent", "avr-asm-ide");

	_font.setFamily (_fontFamily = settings.value ("fontFamily", DEFAULT_fontFamily).toString ());
	_font.setPointSize (_fontSize = settings.value ("fontSize", DEFAULT_fontSize).toInt ());
	_font.setWeight (_fontWeight = settings.value ("fontWeight", DEFAULT_fontWeight).toInt ());
	_font.setItalic (_fontItalic = settings.value ("fontItalic", DEFAULT_fontItalic).toBool ());

	_backgroundColor = new QColor (settings.value ("backgroundColor", DEFAULT_backgroundColor).toString ());
	_keywordColor = new QColor (settings.value ("keywordColor", DEFAULT_keywordColor).toString ());
	_symbolColor = new QColor (settings.value ("symbolColor", DEFAULT_symbolColor).toString ());
	_numberColor = new QColor (settings.value ("numberColor", DEFAULT_numberColor).toString ());
	_stringColor = new QColor (settings.value ("stringColor", DEFAULT_stringColor).toString ());
	_commentColor = new QColor (settings.value ("commentColor", DEFAULT_commentColor).toString ());
	_nameColor = new QColor (settings.value ("nameColor", DEFAULT_nameColor).toString ());
	_cursorColor = new QColor (settings.value ("cursorColor", DEFAULT_cursorColor).toString ());
	_lineColor = new QColor (settings.value ("lineColor", DEFAULT_lineColor).toString ());
	_cursorSize = settings.value ("cursorSize", DEFAULT_cursorSize).toInt ();
}
/*}}}*/
/*{{{  void EditorConfiguration::writeSettings (void)*/
/*
 *	writes editor settings to the global config.
 */
void EditorConfiguration::writeSettings (void)
{
	QSettings settings ("unikent", "avr-asm-ide");

	settings.setValue ("fontFamily", _fontFamily);
	settings.setValue ("fontSize", _fontSize);
	settings.setValue ("fontWeight", _fontWeight);
	settings.setValue ("fontItalic", _fontItalic);

	settings.setValue ("backgroundColor", _backgroundColor->name ());
	settings.setValue ("keywordColor", _keywordColor->name ());
	settings.setValue ("symbolColor", _symbolColor->name ());
	settings.setValue ("numberColor", _numberColor->name ());
	settings.setValue ("stringColor", _stringColor->name ());
	settings.setValue ("commentColor", _commentColor->name ());
	settings.setValue ("nameColor", _nameColor->name ());
	settings.setValue ("cursorColor", _cursorColor->name ());
	settings.setValue ("lineColor", _lineColor->name ());
	settings.setValue ("cursorSize", _cursorSize);
}
/*}}}*/
/*{{{  QColor *EditorConfiguration::backgroundColor (void)*/
/*
 *	returns the configured background colour.
 */
QColor *EditorConfiguration::backgroundColor (void)
{
	return _backgroundColor;
}
/*}}}*/
/*{{{  QColor *EditorConfiguration::keywordColor (void)*/
/*
 *	returns the configured keyword colour.
 */
QColor *EditorConfiguration::keywordColor (void)
{
	return _keywordColor;
}
/*}}}*/
/*{{{  QColor *EditorConfiguration::symbolColor (void)*/
/*
 *	returns the configured symbol colour.
 */
QColor *EditorConfiguration::symbolColor (void)
{
	return _symbolColor;
}
/*}}}*/
/*{{{  QColor *EditorConfiguration::numberColor (void)*/
/*
 *	returns the configured number (constant) colour.
 */
QColor *EditorConfiguration::numberColor (void)
{
	return _numberColor;
}
/*}}}*/
/*{{{  QColor *EditorConfiguration::stringColor (void)*/
/*
 *	returns the configured string (constant) colour.
 */
QColor *EditorConfiguration::stringColor (void)
{
	return _stringColor;
}
/*}}}*/
/*{{{  QColor *EditorConfiguration::commentColor (void)*/
/*
 *	returns the configured comment colour.
 */
QColor *EditorConfiguration::commentColor (void)
{
	return _commentColor;
}
/*}}}*/
/*{{{  QColor *EditorConfiguration::nameColor (void)*/
/*
 *	returns the configured name colour.
 */
QColor *EditorConfiguration::nameColor (void)
{
	return _nameColor;
}
/*}}}*/
/*{{{  QColor *EditorConfiguration::cursorColor (void)*/
/*
 *	returns the configured cursor colour.
 */
QColor *EditorConfiguration::cursorColor (void)
{
	return _cursorColor;
}
/*}}}*/
/*{{{  QColor *EditorConfiguration::lineColor (void)*/
/*
 *	returns the configured line colour.
 */
QColor *EditorConfiguration::lineColor (void)
{
	return _lineColor;
}
/*}}}*/
/*{{{  int EditorConfiguration::cursorSize (void)*/
/*
 *	returns the configured cursor size.
 */
int EditorConfiguration::cursorSize (void)
{
	return _cursorSize;
}
/*}}}*/
/*{{{  void EditorConfiguration::autoSetCursorSize (void)*/
/*
 *	sets the cursor size to the spin-box value.
 */
void EditorConfiguration::autoSetCursorSize (void)
{
	_cursorSize = ui->spinBox->value ();
}
/*}}}*/
/*{{{  QFont EditorConfiguration::font (void)*/
/*
 *	returns the configured font.
 */
QFont EditorConfiguration::font (void)
{
	return _font;
}
/*}}}*/

/*{{{  void EditorConfiguration::resetSettings (void)*/
/*
 *	clears saved settings.
 */
void EditorConfiguration::resetSettings (void)
{
	QSettings settings ("unikent", "avr-asm-ide");

	int reset = QMessageBox::information (this, tr ("Application"),
					      tr ("Do you want to reset all setings ?"),
					      QMessageBox::Yes | QMessageBox::No);
	if (reset == QMessageBox::Yes) {
		if (globMainWindow->maybeSave ()) {
			settings.clear ();
			qApp->exit (EXIT_CODE_REBOOT);
		}
	}
}
/*}}}*/
/*{{{  void EditorConfiguration::applyParams (void)*/
/*
 *	applies settings to the editor.
 */
void EditorConfiguration::applyParams (void)
{
	autoSetCursorSize ();
	emit updateStyle ();
	_changed = false;
}
/*}}}*/
/*{{{  void EditorConfiguration::cancelParams (void)*/
/*
 *	cancels changing of editor settings.
 */
void EditorConfiguration::cancelParams (void)
{
	if (_changed) {
		resetParams ();
		initColors ();
		applyColors ();
	}
	_changed = false;
}
/*}}}*/
/*{{{  void EditorConfiguration::resetParams (void)*/
/*
 *	resets editor settings (to previous/old values when changes cancelled).
 */
void EditorConfiguration::resetParams (void)
{
	_fontSize = _fontSizeOld;
	_fontWeight = _fontWeightOld;
	_cursorSize = _cursorSizeOld;
	_fontItalic = _fontItalicOld;
	_fontFamily = _fontFamilyOld;
	_font = _fontOld;
	_backgroundColor->setNamedColor (_backgroundColorOld->name ());
	_keywordColor->setNamedColor (_keywordColorOld->name ());
	_symbolColor->setNamedColor (_symbolColorOld->name ());
	_numberColor->setNamedColor (_numberColorOld->name ());
	_stringColor->setNamedColor (_stringColorOld->name ());
	_commentColor->setNamedColor (_commentColorOld->name ());
	_nameColor->setNamedColor (_nameColorOld->name ());
	_cursorColor->setNamedColor (_cursorColorOld->name ());
	_lineColor->setNamedColor (_lineColorOld->name ());
}
/*}}}*/

