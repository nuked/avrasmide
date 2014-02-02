/*
 *	avrasmlexer.cpp -- lexer for AVR assembler, done by calling nocc to dump the relevant tokens.
 *	Copyright (C) 2013 Gregoire Liglet and Florent Chiron, University of Kent.
 *
 *	Modifications (C) 2013 Fred Barnes, University of Kent <frmb@kent.ac.uk>.
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

#include "avrasmtoken.h"
#include "language.h"
#include "mainwindow.h"
#include "parameters.h"
#include "tooltipwidget.h"

// Hold the name of the temporary file which will be created to contain the asm code and
// given to nocc for lexing
#define EDITOR_BUFFER_FILENAME ".__tmp_editor_content.asm"


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

	_tokensFile = NULL;
	_tokenReader = NULL;
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
