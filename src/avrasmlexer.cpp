/*
 *	avrasmlexer.cpp -- lexer for AVR assembler, done by calling nocc to dump the relevant tokens.
 *	Copyright (C) 2013 Gregoire Liglet and Florent Chiron, University of Kent.
 *	Copyright (C) 2013-2015 Fred Barnes, University of Kent <frmb@kent.ac.uk>
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

#include "avrasmlexer.h"
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QHelpEvent>
#include <QKeyEvent>
#include <QSet>
#include <QDebug>

// QScintilla
#include "Qsci/qsciapis.h"
#include "Qsci/qsciscintillabase.h"

#ifdef USE_NOCC_LEXER
	#include "avrasmtoken.h"
#endif

#include "language.h"
#include "mainwindow.h"
#include "parameters.h"
#include "tooltipwidget.h"

#ifdef USE_NOCC_LEXER
	// Hold the name of the temporary file which will be created to contain the asm code and
	// given to nocc for lexing
	#define EDITOR_BUFFER_FILENAME ".__tmp_editor_content.asm"
#endif


/*{{{  AVRASMLexer::AVRASMLexer (QObject *parent) : QsciLexerCustom (parent)*/
/*
 *	constructor.
 */
AVRASMLexer::AVRASMLexer (QObject *parent) : QsciLexerCustom (parent)
{
	QsciScintilla *scintillaEditor = dynamic_cast <QsciScintilla*>(parent);

	if (!scintillaEditor) {
		qWarning () << "AVRASMLexer: parent is not an instance of QsciScintilla !";
	}

#ifdef USE_NOCC_LEXER
	_tokensFile = NULL;
	_tokenReader = NULL;
#endif
	_apisReady = false;
	_tooltipWidget = new TooltipWidget (scintillaEditor);
	_tooltipWidget->setAutoFillBackground (true);
	initStyles ();
	if (!loadAPIs ()) {
		qWarning () << "Failed to load APIs";
	}
	// Install an event filter to catch tooltip events in order to display useful information
	scintillaEditor->installEventFilter (this);
	connect (Parameters::getInstance().editorConfig(), SIGNAL (updateStyle()), SLOT (updateStyle()));
}
/*}}}*/
#ifdef USE_NOCC_LEXER
/*{{{  void AVRASMLexer::setNoccPath (const QString &path, const QString &specspath)*/
/*
 *	sets the path to nocc (binary) *and* its specification file.
 */
void AVRASMLexer::setNoccPath (const QString &path, const QString &specspath)
{
	_noccPath = path;
	_noccSpecsPath = specspath;
}
/*}}}*/
/*{{{  void AVRASMLexer::setParameters (Parameters *params)*/
/*
 *	sets the parameters given to nocc (binary)
 */
void AVRASMLexer::setParameters (Parameters *params)
{
	_params = params;
}
/*}}}*/
#endif


// QsciLexer implementation

/*{{{  const char *AVRASMLexer::language (void) const*/
/*
 *	returns the language name
 */
const char *AVRASMLexer::language (void) const
{
	return "AVR Assembly";
}
/*}}}*/
/*{{{  QString AVRASMLexer::description (int style) const*/
/*
 *	returns a string representing particular style constants.
 */
QString AVRASMLexer::description (int style) const
{
	switch (style) {
	case StyleDefault:
		return "Default style for non-specific tokens";
		break;
	case StyleKeyword:
		return "Style for keywords";
		break;
	case StyleNumber:	// StyleSpecial == StyleNumber
		return "Style for numbers";
		break;
	case StyleSymbol:
		return "Style for symbols";
		break;
	case StyleString:
		return "Style for quoted strings";
		break;
	case StyleComment:
		return "Style for comments";
		break;
	case StyleName:
		return "Style for names";
		break;
	default:
		return "Unknown style";
		break;
	};
}
/*}}}*/
/*{{{  const char *AVRASMLexer::wordCharacters (void) const*/
/*
 *	returns a string containg characters that are to be considered parts of words; also sends to scintilla
 */
const char *AVRASMLexer::wordCharacters (void) const
{
	const char *str = "_.abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

	// Only returning the set of characters doesn't seem to change the behaviour of the editor in any way.
	// Sending it directly to Scintilla apparently works.
	editor ()->SendScintilla (QsciScintillaBase::SCI_SETWORDCHARS, str);
	return str;
}
/*}}}*/


// QsciLexerCustom implementation

/*{{{  void AVRASMLexer::styleText (int start, int end)*/
/*
 *	this does the leg-work of text styling between particular 'start' and 'end' points in the buffer.
 *	calls on nocc to do this, dumping tokens into a temporary file and then parsing that for style information.
 */
void AVRASMLexer::styleText (int start, int end)
{
#if 1
	QString editorContent = editor()->text();
	QStringRef editorContentRef (&editorContent, start, end - start);
	QByteArray l1chr = editorContentRef.toLatin1 ();
	int offs = start;
	int l1offs = 0;
	int bol = 1;			/* assume we start scanning from a line start */

	startStyling (offs);

#if 0
fprintf (stderr, "AVRASMLexer::styleText(): start=%d, end=%d\n", start, end);
#endif
	/* Note: call setStyling (N, STYLE) styles 'N' characters from the start/last-styling-end */
	while (offs < end) {
		int i;

		/* skip over whitespace */
		for (i=0; ((offs + i) < end) && ((l1chr[l1offs + i] == ' ') || (l1chr[l1offs + i] == '\t') || (l1chr[l1offs + i] == '\r')); i++);
		if (i > 0) {
			/* style whitespace */
			setStyling (i, 0);
			offs += i;
			l1offs += i;
		}
		if (offs >= end) {
			/* reached end-of-buffer */
			break;			/* while() */
		}

		/* see what we've got here */
		switch (l1chr[l1offs]) {
		case '\n':
			/*{{{  end-of-line*/
			setStyling (1, 0);
			offs++;
			l1offs++;
			bol = 1;
			break;
			/*}}}*/
		case ';':
			/*{{{  comment to end-of-line */
			for (i=1; ((offs + i) < end) && (l1chr[l1offs + i] != '\n'); i++);
			setStyling (i, StyleComment);
			offs += i;
			l1offs += i;
			bol = 0;
			break;
			/*}}}*/
		case '"':
			/*{{{  looking for a string */
			for (i=1; ((offs + i) < end); i++) {
				if ((l1chr[l1offs+i] == '"') && (l1chr[l1offs+i-1] != '\\')) {
					/* end-of-string here */
					i++;
					break;		/* for() */
				} else if (l1chr[l1offs+i] == '\n') {
					/* ran into end-of-line: assume end-of-string */
					break;
				}
			}
			setStyling (i, StyleString);
			offs += i;
			l1offs += i;
			bol = 0;
			break;
			/*}}}*/
		default:
			/* something else */
			if ((l1chr[l1offs] >= '0') && (l1chr[l1offs] <= '9')) {
				/*{{{  probably a number*/
				StyleIdentifier sty = StyleNumber;
				int ishex = 0;
				int isbin = ((l1chr[l1offs] == '0') || (l1chr[l1offs] == '1')) ? 1 : 0;

				i = 1;
				if (((offs + i) < end) && (l1chr[l1offs + i] == 'x')) {
					/* probably a hexadecimal number */
					ishex = 1;
					isbin = 0;
					i++;
				}

				/*{{{  scan through digits (move i along)*/
				while ((offs + i) < end) {
					char ch = l1chr[l1offs+i];

					if ((ch >= '0') && (ch <= '9')) {
						if (ch >= '2') {
							/* not binary */
							isbin = 0;
						}
						i++;
					} else if (ishex && (ch >= 'a') && (ch <= 'f')) {
						i++;
					} else if (ishex && (ch >= 'A') && (ch <= 'F')) {
						i++;
					} else {
						/* anything else, stop and look */
						break;		/* while() */
					}
				}
				/*}}}*/

				/* see if it was a forward/backward label reference (0b, 2f, etc.) */
				if (((offs + i) < end) && !ishex && !isbin && ((l1chr[l1offs+i] == 'b') || (l1chr[l1offs+i] == 'f'))) {
					/* assume it is */
					i++;
					sty = StyleSymbol;
				} else if (((offs + i) < end) && !ishex && isbin && (l1chr[l1offs+i] == 'f')) {
					/* assume it is again */
					i++;
					sty = StyleSymbol;
					isbin = 0;
				} else if (((offs + i) < end) && isbin && (l1chr[l1offs+i] == 'b')) {
					/* assume binary number */
					i++;
				}

				/* anything left that isn't whitespace/etc. is garbage! */
				setStyling (i, sty);
				offs += i;
				l1offs += i;

				for (i=0; ((offs + i) < end) && ((l1chr[l1offs+i] == '_') || ((l1chr[l1offs+i] >= '0') && (l1chr[l1offs+i] <= '9')) ||
						((l1chr[l1offs+i] >= 'a') && (l1chr[l1offs+i] <= 'z')) || ((l1chr[l1offs+i] >= 'A') && (l1chr[l1offs+i] <= 'Z'))); i++);
				if (i > 0) {
					/* some garbage */
					setStyling (i, 0);
					offs += i;
					l1offs += i;
				}
				/*}}}*/
			} else if ((l1chr[l1offs] >= 'a') && (l1chr[l1offs] <= 'z')) {
				/*{{{  probably keyword, name or symbol*/
				/* scoop up characters */
				char kbuf[64];

				for (i=0; ((offs + i) < end) && (i < 63) && ((l1chr[l1offs+i] == '_') || ((l1chr[l1offs+i] >= '0') && (l1chr[l1offs+i] <= '9')) ||
						((l1chr[l1offs+i] >= 'a') && (l1chr[l1offs+i] <= 'z')) || ((l1chr[l1offs+i] >= 'A') && (l1chr[l1offs+i] <= 'Z'))); i++) {
					kbuf[i] = l1chr[l1offs+i];
				}
				kbuf[i] = '\0';

				if (bol && ((offs+i) < end) && (l1chr[l1offs+i] == ':')) {
					/* symbol */
					i++;
					setStyling (i, StyleSymbol);
					offs += i;
					l1offs += i;
				} else {
					QString tmp (kbuf);

					/* see if it's in the keyword stuff */
					if (AVRASMKeywords.contains (tmp)) {
						/* yes :) */
						setStyling (i, StyleKeyword);
						offs += i;
						l1offs += i;
					} else {
						/* assume name */
						setStyling (i, StyleName);
						offs += i;
						l1offs += i;
					}
				}
				/*}}}*/
			} else if (l1chr[l1offs] == '.') {
				/*{{{  probably an assembler directive or local label*/
				/* scoop up characters */
				char kbuf[64];
				int islab = 0;

				kbuf[0] = l1chr[l1offs];
				for (i=1; ((offs + i) < end) && (i < 63) && ((l1chr[l1offs+i] == '_') || ((l1chr[l1offs+i] >= '0') && (l1chr[l1offs+i] <= '9')) ||
						((l1chr[l1offs+i] >= 'a') && (l1chr[l1offs+i] <= 'z')) || ((l1chr[l1offs+i] >= 'A') && (l1chr[l1offs+i] <= 'Z'))); i++) {
					kbuf[i] = l1chr[l1offs+i];
				}
				kbuf[i] = '\0';

				if (bol && (i > 1) && (kbuf[1] == 'L')) {
					/* might be local label */
					int j;

					islab = 1;
					for (j=2; (j<i) && (kbuf[j] >= '0') && (kbuf[j] <= '9'); j++);
					if (j < i) {
						islab = 0;
					} else if (((offs + i) < end) && (l1chr[l1offs + i] == ':')) {
						/* definitely is a local label */
						islab = 1;
					} else {
						/* something else */
						islab = 0;
					}
				}

				if (islab) {
					setStyling (i, StyleSymbol);
					offs += i;
					l1offs += i;
				} else {
					QString tmp (kbuf);

					/* see if it's in the keyword stuff */
					if (DirectivesInfo.contains (tmp)) {
						/* yes :) */
						setStyling (i, StyleSpecial);
						offs += i;
						l1offs += i;
					} else {
						/* assume nothing */
						setStyling (i, StyleDefault);
						offs += i;
						l1offs += i;
					}
				}

				/*}}}*/
			} else if ((l1chr[l1offs] >= 'A') && (l1chr[l1offs] <= 'Z')) {
				/*{{{  probably name or symbol (label)*/
				for (i=1; ((offs + i) < end) && ((l1chr[l1offs+i] == '_') || ((l1chr[l1offs+i] >= '0') && (l1chr[l1offs+i] <= '9')) ||
						((l1chr[l1offs+i] >= 'a') && (l1chr[l1offs+i] <= 'z')) || ((l1chr[l1offs+i] >= 'A') && (l1chr[l1offs+i] <= 'Z'))); i++);
				if (bol && ((offs + i) < end) && (l1chr[l1offs+i] == ':')) {
					i++;
					setStyling (i, StyleSymbol);
				} else {
					setStyling (i, StyleName);
				}
				offs += i;
				l1offs += i;
				/*}}}*/
			} else {
				setStyling (1, 0);
				offs++;
				l1offs++;
			}
			bol = 0;
			break;
		}
	}

	setStyling (end - offs, 0);

#else
	QString tokensFileName = QDir::current().filePath ("tokens_dump");
	QString editorContent = editor()->text();
	QStringRef editorContentRef (&editorContent, start, end - start);
	AVRASMToken *token;
	int startLine;
	int startColumn;

	// Run nocc to tokenize the code and dump the tokens to tokensFileName in XML format
	if (!tokenizeEditorContent (editorContentRef.toUtf8(), tokensFileName)) {
		// Could be only a parse error, but maybe we have tokens anyway
	}
	// Initialize the XML stream reader to read the file containing the dumped tokens
	if (!initTokenReader (tokensFileName)) {
		startStyling (start);
		setStyling (end - start, 0);
		return;
	}
	// Get the line and column corresponding to the given 'start' offset, to later get the token offsets
	editor()->lineIndexFromPosition (start, &startLine, &startColumn);

	// Used to assign a default style (id 0) to the text between each token (usually whitespaces)
	int previousTokenEnd = start;

	// Used to colour the COMMENT tokens. The COMMENT tokens don't have a length, so we colour them
	// when reading the following token.
	bool previousTokenWasComment = false;

	// Used to store the absolute offset of the token in the asm code
	int tokenOffset;

	// Read and colorize each token until all tokens are processed, or until an error occurs
	startStyling (start);
	while ((token = readNextToken())) {
		if (token->origin() != EDITOR_BUFFER_FILENAME) {
			// The token is from another file, most likely a .include'd file, skip it
			delete token;
			continue;
		}
		// Get the token's absolute offset in the asm code
		tokenOffset = getTokenAbsoluteOffset (token, startLine, startColumn);

		// Give a default style to the empty space between this token and the end of the previous one
		setStyling (tokenOffset - previousTokenEnd, (previousTokenWasComment ? 5 : 0));

		// Give the token its corresponding style
		if (token->length ()) {
			setStyling (token->length (), styleForToken (*token));
		}

		previousTokenEnd = tokenOffset + token->length();
		previousTokenWasComment = token->type() == AVRASMToken::COMMENT;

		delete token;
	}
	// Give a default style from the end of the last token until the end of the content
	setStyling (end - previousTokenEnd, 0);

	// Destroy the token reader as we don't need it anymore
	destroyTokenReader ();

	if (_apisReady) {
		editor()->autoCompleteFromAPIs ();
	}
#endif
}
/*}}}*/


// Private functions

/*{{{  void AVRASMLexer::initStyles (void)*/
/*
 *	initialises styles
 */
void AVRASMLexer::initStyles (void)
{
	updateStyle ();
}
/*}}}*/
/*{{{  void AVRASMLexer::updateStyle (void)*/
/*
 *	called to update styles, setting particular style parameters up.
 */
void AVRASMLexer::updateStyle (void)
{
	setColor (QColor ("#ffffff"), -1);
	setColor (QColor (*Parameters::getInstance().editorConfig()->keywordColor ()), StyleKeyword);	// KEYWORD
	setColor (QColor (*Parameters::getInstance().editorConfig()->numberColor ()), StyleNumber);	// INTEGER || REAL
	setColor (QColor (*Parameters::getInstance().editorConfig()->symbolColor ()), StyleSymbol);	// SYMBOL
	setColor (QColor (*Parameters::getInstance().editorConfig()->stringColor ()), StyleString);	// STRING
	setColor (QColor (*Parameters::getInstance().editorConfig()->commentColor ()), StyleComment);	// COMMENT
	setColor (QColor (*Parameters::getInstance().editorConfig()->nameColor ()), StyleName);		// NAME

	// Do not specify the style identifier, apply the font to all styles
	setFont (Parameters::getInstance().editorConfig()->font());

	setPaper (QColor (Parameters::getInstance().editorConfig()->backgroundColor()->name()));
	((QsciScintilla *)parent())->setMarginsBackgroundColor (QColor ("#dddddd"));

	((QsciScintilla *)parent())->setCaretForegroundColor (QColor (Parameters::getInstance ().editorConfig ()->cursorColor ()->name ()));
	((QsciScintilla *)parent())->setCaretLineBackgroundColor (QColor (Parameters::getInstance ().editorConfig ()->lineColor ()->name ()));
	((QsciScintilla *)parent())->setCaretLineVisible (true);
	((QsciScintilla *)parent())->setCaretWidth (Parameters::getInstance ().editorConfig ()->cursorSize ());
}
/*}}}*/

#ifdef USE_NOCC_LEXER
/*{{{  int AVRASMLexer::styleForToken (const AVRASMToken &token) const*/
/*
 *	returns the style associated with a particular source token type
 */
int AVRASMLexer::styleForToken (const AVRASMToken &token) const
{
	static const QMap<AVRASMToken::TokenType,int> tokenTypesStyles {
		{ AVRASMToken::KEYWORD, StyleKeyword },
		{ AVRASMToken::INTEGER, StyleNumber },
		{ AVRASMToken::REAL, StyleNumber },
		{ AVRASMToken::STRING, StyleString },
		{ AVRASMToken::NAME, StyleName },
		{ AVRASMToken::SYMBOL, StyleSymbol },
		{ AVRASMToken::COMMENT, StyleComment },
		{ AVRASMToken::INAME, StyleDefault },
		{ AVRASMToken::LSPECIAL, StyleSpecial }
	};
	return tokenTypesStyles.value (token.type (), 0);
}
/*}}}*/
#endif

/*{{{  bool AVRASMLexer::loadAPIs (void)*/
/*
 *	loads the APIs -- keywords that scintilla uses for auto-completion.
 */
bool AVRASMLexer::loadAPIs (void)
{
	QsciAPIs *apis = new QsciAPIs (this);

	if (apis->loadPrepared (":/resources/keywords.apix")) {
		_apisReady = true;
	} else {
		// This 'else' scope should never be run, but just in case.

		qWarning ("Failed to load prepared API from embedded resources");
		if (apis->loadPrepared ("./keywords.apix")) {
			_apisReady = true;
		} else {
			connect (apis, &QsciAPIs::apiPreparationFinished,[ =] () {
				 _apisReady = true; apis->savePrepared ("./keywords.apix");}
			);
			if (!apis->load (":/resources/keywords.api")) {
				return false;
			}
			apis->prepare ();
		}
	}
	return true;
}
/*}}}*/

#ifdef USE_NOCC_LEXER
/*{{{  bool AVRASMLexer::tokenizeEditorContent (const QByteArray &content, const QString &tokensOutputFileName)*/
/*
 *	puts edit buffer in a temporary file, then lex's it (with lexEditorContent) and removes the temporary file.
 *	should be left with tokens in given output file-name.
 *
 *	returns true on success, false otherwise.
 */
bool AVRASMLexer::tokenizeEditorContent (const QByteArray &content, const QString &tokensOutputFileName)
{
	// Get the editor text and write it to a temporary file
	QString tmpFileName = copyEditorContentToTemporaryFile (content);

	if (tmpFileName == QString ()) {
		return false;
	}
	// Run nocc to lex the content of the editor
	// 0 is the status code for success, != 0 means an error occured
	if (lexEditorContent (tmpFileName, tokensOutputFileName) != 0) {
		return false;
	}
	// Delete the temporary file
    QFile::remove (tmpFileName);

	return true;
}
/*}}}*/
/*{{{  QString AVRASMLexer::copyEditorContentToTemporaryFile (const QByteArray &content)*/
/*
 *	copies the editor contents to a temporary file (prior to lexing for syntax highlighting).
 *	returns the temporary file-name.
 */
QString AVRASMLexer::copyEditorContentToTemporaryFile (const QByteArray &content)
{
	QFile temporaryFile (QDir::temp ().filePath (EDITOR_BUFFER_FILENAME), this);
	QTextStream tempFileStream (&temporaryFile);

	// Open the temporary file
	if (!temporaryFile.open (QIODevice::WriteOnly | QIODevice::Truncate)) {
		qDebug () << "Couldn't open temporary file " << temporaryFile.fileName () << ", error: " << temporaryFile.errorString ();
		return QString ();
	}
	// Write the content to it, flush to be sure everything was written, and close the file
	tempFileStream << content;
	tempFileStream.flush ();
	temporaryFile.close ();

	// Return the path to the temporary file
	return temporaryFile.fileName ();
}
/*}}}*/
/*{{{  int AVRASMLexer::lexEditorContent (const QString &temporaryContentFileName, const QString &tokensOutputFileName)*/
/*
 *	runs nocc on the given temporary file to fill up another file with an XML token-dump.
 */
int AVRASMLexer::lexEditorContent (const QString &temporaryContentFileName, const QString &tokensOutputFileName)
{
	QProcess proc (this);
	QDir dir;
	QString tstr;
	QString noccDir = QFileInfo (dir.relativeFilePath (_noccPath)).path ();

	// Run nocc, tell it to stop at the parsing step and to dump the lexer tokens to the specified file

	proc.setEnvironment (QProcess::systemEnvironment () << "CYGWIN=nodosfilewarning");
	proc.setProgram (_noccPath);
	tstr = temporaryContentFileName;

#ifdef Q_OS_WIN32
	// Change a leading C: into /cygdrive/c or similar.
	if (tstr.at(0).isLetter() && (tstr.at(1) == ':')) {
		QChar dlet = tstr.at(0).toLower ();
		QString tmp2 = "/cygdrive/";

		tstr.remove (0, 2);
		tmp2.append (dlet);
		tstr.prepend (tmp2);
	}
#endif

#if 0
    qDebug () << "trying to run: " << _noccPath << "--specs-file" << \
                 _noccSpecsPath << "--stop-token" << \
                 "--dump-tokens-to" << tokensOutputFileName << \
                 "--target" << "avr-atmel-unknown" << "--unexpected" << \
                 tstr;
    qDebug () << "temporary filename: " << temporaryContentFileName;
#endif

	proc.setArguments (QStringList ()
		<< "--specs-file" << _noccSpecsPath
		<< "--stop-token"
		<< "--dump-tokens-to" << tokensOutputFileName
		<< "--target" << "avr-atmel-unknown"
		<< "--unexpected"
		//               << "-v" // debug
		<< tstr);

	proc.start ();
	proc.waitForFinished ();	// Wait for it to finish

	if (proc.error () != QProcess::UnknownError) {
		noccRuntimeError ();
		return proc.exitCode ();
	}

#if 0
    qDebug () << "running nocc for lex, exit-code was " << proc.exitCode ();
    qDebug () << "Standard error: " << proc.readAllStandardError ();
    qDebug () << "Standard output: " << proc.readAllStandardOutput ();
#endif

	if (proc.exitCode () != 0) {
		qDebug () << "Error executing nocc for lexing: " << proc.errorString ();
		qDebug () << "Standard error output: " << proc.readAllStandardError ();
		qDebug () << "Standard output: " << proc.readAllStandardOutput ();
	}

	return proc.exitCode ();
}
/*}}}*/
/*{{{  void AVRASMLexer::noccRuntimeError (void)*/
/*
 *	called when something goes very wrong when trying to run nocc.
 */
void AVRASMLexer::noccRuntimeError (void)
{
	QMessageBox *msg = new QMessageBox (globMainWindow);
	msg->setIcon (QMessageBox::Critical);
	msg->setWindowTitle (tr ("Error"));
	msg->setText (tr ("An error occured while opening nocc.\nPlease check the path of the binary file in the settings."));
	msg->setModal (true);
	msg->show ();
}
/*}}}*/

/*{{{  bool AVRASMLexer::initTokenReader (const QString &tokensFileName)*/
/*
 *	initialises the token reader (for parsing nocc-lex'd edit-buffer contents)
 */
bool AVRASMLexer::initTokenReader (const QString &tokensFileName)
{
	// Open the tokens dump file
	_tokensFile = new QFile (tokensFileName);

	if (!_tokensFile->open (QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug () << "Couldn't open tokens file " << tokensFileName << " : " << _tokensFile->errorString ();
		return false;
	}
	// Create the XML stream reader used to read the tokens file
	_tokenReader = new QXmlStreamReader (_tokensFile);

	return true;
}
/*}}}*/
/*{{{  AVRASMToken *AVRASMLexer::readNextToken (void)*/
/*
 *	reads the next token from the XML file.
 */
AVRASMToken *AVRASMLexer::readNextToken (void)
{
	while (!_tokenReader->atEnd () && !_tokenReader->hasError ()) {
		// Try reading until next opening tag (e.g. <tokens> or <token>)

		// We're only interested in start elements
		if (_tokenReader->readNext () != QXmlStreamReader::StartElement) {
			continue;
		}
		// Skip the <tokens> tag which starts the list of tokens
		if (_tokenReader->name () == "tokens") {
			continue;
		}
		// Analyze the token and create a corresponding AVRASMToken object
		if (_tokenReader->name () == "token") {
			QString originAttr = _tokenReader->attributes ().value ("origin").toString ();
			QStringRef typeAttr = _tokenReader->attributes ().value ("type");
			QString range = originAttr.section (":", -1);
			QStringRef valueAttr = _tokenReader->attributes ().value ("value");
			AVRASMToken *token = new AVRASMToken (this);

			token->setLine (originAttr.section (':', -2, -2).toInt ());
			if (range.contains ("-")) {
				token->setColumn (range.section ("-", 0, 0).toInt ());
				token->setLength (range.section ("-", 1).toInt () + 1 - token->column ());
			} else {
				token->setColumn (range.toInt ());
				token->setLength (0);
			}
			token->setValue (valueAttr.isNull () == false ? valueAttr.toString () : "");
			if (!valueAttr.isNull ()) {
				token->setValue (valueAttr.toString ());
			}
			token->setOrigin (originAttr.section (':', 0, -3));
			token->setType (AVRASMToken::stringToTokenType (typeAttr.toString ().toUpper ()));
			return token;
		}
	}
	return 0;
}
/*}}}*/
/*{{{  void AVRASMLexer::destroyTokenReader (void)*/
/*
 *	trashes the token-reader (for parsing XML).
 */
void AVRASMLexer::destroyTokenReader (void)
{
	// Delete the XML stream reader
	delete _tokenReader;
	_tokenReader = 0;

	// Delete the tokens file (automatically closing it)
	_tokensFile->remove ();
	delete _tokensFile;

	_tokensFile = 0;
}
/*}}}*/
/*{{{  int AVRASMLexer::whiteSpaceOffsetForLine (int line) const*/
/*
 *	searches for something that isn't whitespace at start of line.
 */
int AVRASMLexer::whiteSpaceOffsetForLine (int line) const
{
	static QRegExp whiteSpaceRegExp ("^[^\\S\\n\\r]+");
	QString lineText = editor ()->text (line);

	// Get the length of the whitespace sequence at the beginning of the given line
	if (lineText.indexOf (whiteSpaceRegExp) < 0) {
		return 0;
	} else {
		return whiteSpaceRegExp.matchedLength ();
	}
}
/*}}}*/
/*{{{  int AVRASMLexer::getTokenAbsoluteOffset (const AVRASMToken *token, int startLine, int startColumn) const*/
/*
 *	gets the absolute token offset in the edit buffer, calculated based on start-of-line
 */
int AVRASMLexer::getTokenAbsoluteOffset (const AVRASMToken *token, int startLine, int startColumn) const
{
	// Get the absolute token offset in the asm code.
	// Use "line - 1" and "column - 1" because tokens start at line 1 and column 1.
	int absoluteOffset = editor ()->positionFromLineIndex (startLine + token->line () - 1,
							       startColumn + token->column () - 1);

	// BUG ? The first token on a line is always at column 1, regardless of how many whitespaces are present before.
	// Take these whitespaces into account to compute the token offset.
	if (token->column () == 1 && token->type () != AVRASMToken::END) {
		absoluteOffset += whiteSpaceOffsetForLine (startLine + token->line () - 1);
	}

	return absoluteOffset;
}
/*}}}*/
#endif

/*{{{  bool AVRASMLexer::eventFilter (QObject *qobj, QEvent *event)*/
/*
 *	watches events in order to hide the tooltip.
 */
bool AVRASMLexer::eventFilter (QObject *qobj, QEvent *event)
{
	// Filter events to decide whether or not to hide the tooltip, or update it, or navigate between the different tooltips

	switch (event->type ()) {
		// Set the tooltip to information related to the context under the mouse cursor
	case QEvent::ToolTip:
		{
			updateTooltip (static_cast <QHelpEvent *>(event)->pos ());
			break;
		}
	case QEvent::KeyRelease:
		{
			QKeyEvent *keyEvent = static_cast <QKeyEvent *>(event);

			// Pressing F2 and F3 allows navigating between several tooltips (e.g. the 'ld' opcode has 7 different tooltips)
			if (_tooltipWidget->isVisible ()) {
				if (keyEvent->key() == Qt::Key_F2) {
					_tooltipWidget->previousTooltip ();
				} else if (keyEvent->key() == Qt::Key_F3) {
					_tooltipWidget->nextTooltip ();
				}
			}
			// No 'break', go to the next case
		}
	case QEvent::KeyPress:
		{
			QKeyEvent *keyEvent = static_cast <QKeyEvent *>(event);
			const int key = keyEvent->key ();
			QSet<int> ignoredKeys = { Qt::Key_Shift, Qt::Key_Control, Qt::Key_Alt, Qt::Key_Meta, Qt::Key_AltGr, Qt::Key_F2, Qt::Key_F3 };

			// If a modifier key was pressed, ignore it.
			// Also ignore F2 and F3, as they are handled above. Otherwise, it would cause the tooltip to disappear,
			// instead of navigating between the different tooltips.
			if (((keyEvent->modifiers () & Qt::KeyboardModifierMask) != 0) || ignoredKeys.contains (key)) {
				break;
			}
			if (_tooltipWidget->isVisible () && ((key == Qt::Key_Tab) || (key == Qt::Key_Return))) {
				_tooltipWidget->hide ();
				break;
			}
#if 0
			qDebug() << "QEvent::KeyPress, got: " << key;
#endif
			/* fall through */
		}
	case QEvent::Leave:
	case QEvent::WindowActivate:
	case QEvent::WindowDeactivate:
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonRelease:
	case QEvent::MouseButtonDblClick:
	case QEvent::FocusIn:
	case QEvent::FocusOut:
	case QEvent::Wheel:
		{
			// For all these events, hide the tooltip
			_tooltipWidget->hide ();
			break;
		}
	case QEvent::MouseMove:
		{
			if (!_tooltipWidget->rect().contains (static_cast <QMouseEvent *>(event)->pos())) {
				_tooltipWidget->hide ();
			}
		}
	default:
		break;
	}

	return false;
}
/*}}}*/
/*{{{  void AVRASMLexer::updateTooltip (const QPoint &tooltipPosition)*/
/*
 *	updates tool-tip based on what the word is, i.e. provides documentation for particular things.
 */
void AVRASMLexer::updateTooltip (const QPoint &tooltipPosition)
{
	static QStringList (AVRASMLexer::*tooltipGenerators[]) (const QString &) const = {
		&AVRASMLexer::tooltipForOpcode,
		&AVRASMLexer::tooltipForDirective
	};

	QString wordUnderCursor = editor()->wordAtPoint (tooltipPosition);
	QStringList tooltipContent;

	for (auto tooltipGenerator:tooltipGenerators) {
		tooltipContent = (this->*tooltipGenerator)(wordUnderCursor);

		if (!tooltipContent.empty ()) {
			break;
		}
	}

	if (tooltipContent.empty ()) {
		_tooltipWidget->hide ();
	} else {
		if (wordUnderCursor != _lastTooltipContext) {
			_tooltipWidget->setTooltips (tooltipContent);
		}
		_tooltipWidget->move (tooltipPosition + QPoint (10, 10));
		_tooltipWidget->resize (_tooltipWidget->sizeHint ());
		_tooltipWidget->show ();
		_lastTooltipContext = wordUnderCursor;
	}
}
/*}}}*/
/*{{{  QStringList AVRASMLexer::tooltipForOpcode (const QString &opcode) const*/
/*
 *	returns a list of strings containing a tool-tip for a particular opcode string.
 */
QStringList AVRASMLexer::tooltipForOpcode (const QString &opcode) const
{
	// A few lambdas to format the tooltip with HTML
	static auto wrapInTag =[](const QString & tag, const QString & text) {
		return (QStringList () << "<" << tag << ">" << text << "</" << tag << ">").join ("");
	};
	static auto table = std::bind (wrapInTag, "table", std::placeholders::_1);
	static auto tr = std::bind (wrapInTag, "tr", std::placeholders::_1);
	static auto td = std::bind (wrapInTag, "td", std::placeholders::_1);
	static auto b = std::bind (wrapInTag, "b", std::placeholders::_1);

	QStringList tooltipContent;
	auto opcodeIterator = OpcodesInfo.constFind (opcode.toUpper ());

	// We couldn't find the given parameter in the opcodes map, it is not an opcode
	if (opcodeIterator == OpcodesInfo.constEnd ()) {
		return QStringList ();
	}

	// Get the structure holding the opcode's information
	while (opcodeIterator != OpcodesInfo.constEnd () && opcodeIterator.key () == opcode.toUpper ()) {
		const OpcodeInfo & opcodeInfo = *opcodeIterator;

		// Generate the tooltip content
		tooltipContent << b (opcodeInfo.name)
			+ (opcodeInfo.description.length () > 0 ? ": " + opcodeInfo.description : "")
			+ table ((QStringList ()
				  << (opcodeInfo.parameters.length () > 0 ? tr (td (b ("Parameters:")) + td (opcodeInfo.parameters)) : "")
				  << (opcodeInfo.operations.length () > 0 ? tr (td (b ("Operations:")) + td (opcodeInfo.operations.join ("<br/>"))) : "")
				  << (opcodeInfo.flags.length () > 0 ? tr (td (b ("Flags:")) + td (opcodeInfo.flags)) : "")
				  << (opcodeInfo.nClocks.length () > 0 ? tr (td (b ("#Clocks:")) + td (opcodeInfo.nClocks)) : "")).join ("\n"));
		++opcodeIterator;
	}

	return tooltipContent;
}
/*}}}*/
/*{{{  QStringList AVRASMLexer::tooltipForDirective (const QString &directive) const*/
/*
 *	returns a string containing a tool-tip for a particular directive string.
 */
QStringList AVRASMLexer::tooltipForDirective (const QString &directive) const
{
	// A few lambdas to format the tooltip with HTML
	static auto wrapInTag =[](const QString & tag, const QString & text) {
		return (QStringList () << "<" << tag << ">" << text << "</" << tag << ">").join ("");
	};
	static auto table = std::bind (wrapInTag, "table", std::placeholders::_1);
	static auto tr = std::bind (wrapInTag, "tr", std::placeholders::_1);
	static auto td = std::bind (wrapInTag, "td", std::placeholders::_1);
	static auto b = std::bind (wrapInTag, "b", std::placeholders::_1);

	auto directiveIterator = DirectivesInfo.constFind (directive.toLower ());

	QString tooltipContent;

	// We couldn't find the given parameter in the directives map, it is not a directive
	if (directiveIterator == DirectivesInfo.constEnd ()) {
		return QStringList ();
	}

	// Get the structure holding the opcode's information
	const DirectiveInfo & directiveInfo = *directiveIterator;

	// Generate the tooltip content
	tooltipContent = b (directiveInfo.name)
		+ (directiveInfo.description.length () > 0 ? ": " + directiveInfo.description : "")
		+ table ((QStringList ()
			  << (directiveInfo.parameters.length () > 0 ? tr (td (b ("Parameters:")) + td (directiveInfo.parameters)) : "")
			  << (directiveInfo.example.length () > 0 ? tr (td (b ("Example:")) + td (directiveInfo.example)) : "")).join ("\n"));
	return QStringList (tooltipContent);
}
/*}}}*/

