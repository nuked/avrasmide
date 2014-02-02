/*
 *	editorconfiguration.h -- editor configuration object.
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

#ifndef EDITORCONFIGURATION_H
#define EDITORCONFIGURATION_H

#include <QColorDialog>
#include <QFontDialog>
#include <QTextEdit>
#include <QVector>
#include <QWidget>
#include <QPair>

#include "optionconfig.h"

#define DEFAULT_backgroundColor "black"
#define DEFAULT_keywordColor "yellow"
#define DEFAULT_symbolColor "grey"
#define DEFAULT_numberColor "pink"
#define DEFAULT_stringColor "pink"
#define DEFAULT_commentColor "blue"
#define DEFAULT_nameColor "red"
#define DEFAULT_cursorColor "#00ff00"
#define DEFAULT_lineColor "#505050"
#define DEFAULT_cursorSize 1
#define ID_MAX 8
#define DEFAULT_fontFamily "Lucida console"
#define DEFAULT_fontSize 10
#define DEFAULT_fontWeight 6
#define DEFAULT_fontItalic false

typedef struct s_color
{
	QColor *color;
	QTextEdit *text;
	QString defaultColor;
} t_color;

namespace Ui
{
	class EditorConfiguration;
}

class EditorConfiguration:public OptionConfig
{
	Q_OBJECT
public:
	explicit EditorConfiguration (QWidget *parent = 0);
	~EditorConfiguration ();
	QColor *backgroundColor ();
	QColor *keywordColor ();
	QColor *symbolColor ();
	QColor *numberColor ();
	QColor *stringColor ();
	QColor *commentColor ();
	QColor *nameColor ();
	QColor *cursorColor ();
	QColor *lineColor ();
	QFont font ();
	int cursorSize ();
	void autoSetCursorSize ();
	void applyParams ();
	void cancelParams ();

private slots:
	void showFontDialog ();
	void showColorDialog (int);
	void changeColor (QColor);
	void changeFont (QFont);
	void resetDefaults ();
	void resetSettings ();

signals:
	void updateStyle ();

private:
	Ui::EditorConfiguration *ui;
	void readSettings ();
	void writeSettings ();
	void buildUi ();
	void initDefaults ();
	void initColors ();
	void resetDefaultColors ();
	void resetDefaultFont ();
	void applyColors ();
	void applyFont ();
	void resetParams ();

	bool _changed;
	int _currentId;
	int _fontSize;
	int _fontSizeOld;
	int _fontWeight;
	int _fontWeightOld;
	int _cursorSize;
	int _cursorSizeOld;
	bool _fontItalic;
	bool _fontItalicOld;
	QString _fontFamily;
	QString _fontFamilyOld;
	QFont _font;
	QFont _fontOld;
	QFontDialog *_fontDialog;
	QColor *_backgroundColor;
	QColor *_backgroundColorOld;
	QColor *_keywordColor;
	QColor *_keywordColorOld;
	QColor *_symbolColor;
	QColor *_symbolColorOld;
	QColor *_numberColor;
	QColor *_numberColorOld;
	QColor *_stringColor;
	QColor *_stringColorOld;
	QColor *_commentColor;
	QColor *_commentColorOld;
	QColor *_nameColor;
	QColor *_nameColorOld;
	QColor *_cursorColor;
	QColor *_cursorColorOld;
	QColor *_lineColor;
	QColor *_lineColorOld;
	QColorDialog *_colorDialog;
	QVector<t_color> _colors;
	QVector<t_color> _colorsOld;
};

#endif // EDITORCONFIGURATION_H
