/*
 *	avrasmlexer.h -- generic lexer support for AVR assembler source (builds on QScintilla custom lexer).
 *	Gregoire Liglet and Florent Chiron, University of Kent, 2013.
 *
 *	Modifications (C) 2014 Fred Barnes, University of Kent  <frmb@kent.ac.uk>.
 */


#ifndef AVRASMLEXER_H
#define AVRASMLEXER_H

#include <Qsci/qscilexercustom.h>
#include <QProcess>

#include "parameters.h"

class AVRASMToken;
class QXmlStreamReader;
class QFile;
class TooltipWidget;

class AVRASMLexer:public QsciLexerCustom
{
Q_OBJECT
public:
	explicit AVRASMLexer (QObject *parent = 0);

	void setNoccPath (const QString &path, const QString &specspath);
	void setParameters (Parameters *);

	// Inherited from QsciLexer
	const char *language (void) const;
	QString description (int style) const;
	const char *wordCharacters (void) const;

	// Inherited from QsciLexerCustom
	void styleText (int start, int end);

private slots:
	void updateStyle (void);
	void noccRuntimeError (void);

private:
	typedef enum StyleIdentifier {
		StyleDefault = 0,
		StyleKeyword,
		StyleNumber,
		StyleSymbol,
		StyleString,
		StyleComment,
		StyleName,
		StyleSpecial = StyleNumber
	} StyleIdentifier;

	void initStyles (void);
	int styleForToken (const AVRASMToken &token) const;

	bool loadAPIs (void);

	bool tokenizeEditorContent (const QByteArray &content, const QString &tokensOutputFileName);
	QString copyEditorContentToTemporaryFile (const QByteArray &content);
	int lexEditorContent (const QString &temporaryContentFileName, const QString &tokensOutputFileName);
	bool initTokenReader (const QString &tokensFileName);
	AVRASMToken *readNextToken ();
	void destroyTokenReader ();
	int whiteSpaceOffsetForLine (int line) const;
	int getTokenAbsoluteOffset (const AVRASMToken *token, int startLine, int startColumn) const;

	bool eventFilter (QObject *object, QEvent *event);

	void updateTooltip (const QPoint &tooltipPosition);
	QStringList tooltipForOpcode (const QString &opcode) const;
	QStringList tooltipForDirective (const QString &directive) const;

	QString _noccPath;
	QString _noccSpecsPath;
	QFile *_tokensFile;
	QXmlStreamReader *_tokenReader;
	bool _apisReady;
	TooltipWidget *_tooltipWidget;
	QString _lastTooltipContext;
	Parameters *_params;
};

#endif	/* !AVRASMLEXER_H */


