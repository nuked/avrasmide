/*
 *	avrasmfileparser.cpp
 *	Gregoire Liglet and Florent Chiron, University of Kent, 2013.
 *	Modifications, Fred Barnes, University of Kent <frmb@kent.ac.uk>
 */

#include "avrasmfileparser.h"
#include "language.h"

#include <QRegExp>
#include <QStringRef>
#include <QDebug>

/*{{{  AVRASMFileParser::AVRASMFileParser (QObject *parent) : QObject (parent)*/
/*
 *	constructor.
 */
AVRASMFileParser::AVRASMFileParser (QObject *parent) : QObject (parent)
{
	_cursor = 0;
}
/*}}}*/
/*{{{  AVRASMFileParser::AVRASMFileParser (QFile *file, QObject *parent) : QObject (parent), _file (file)*/
/*
 *	constructor.
 */
AVRASMFileParser::AVRASMFileParser (QFile *file, QObject *parent) : QObject (parent), _file (file)
{
	_cursor = 0;
}
/*}}}*/

/*{{{  bool AVRASMFileParser::matchKeyword (void)*/
/*
 *	see if the word under the cursor is a keyword.
 */
bool AVRASMFileParser::matchKeyword (void)
{
	static const QRegExp keywordRegExp ("^[a-zA-Z][a-zA-Z0-9_]*");

	// Use a copy of the model RegExp 'keywordRegExp' because indexOf() will modify it
	QRegExp tmpRegExp (keywordRegExp);

	if (_input.indexOf (tmpRegExp, _cursor) == _cursor) {
		QStringRef matchedString (&_input, _cursor, tmpRegExp.matchedLength ());

		return AVRASMKeywords.contains (matchedString.toString ());
	}
	return false;
}
/*}}}*/
/*{{{  bool AVRASMFileParser::matchName (void)*/
/*
 *	see if the word under the cursor is a name.
 */
bool AVRASMFileParser::matchName (void)
{
	static const QRegExp nameRegExp ("^[a-zA-Z][a-zA-Z0-9_]*");

	return _input.indexOf (nameRegExp, _cursor) == _cursor;
}
/*}}}*/
/*{{{  bool AVRASMFileParser::matchReal (void)*/
/*
 *	see if the text under the cursor is a real (floating-point) value.
 */
bool AVRASMFileParser::matchReal (void)
{
	static const QRegExp realRegExp ("^[0-9]+\\.[0-9]+");

	/* FIXME: ought to handle 2.3E2 and similar things */
	return _input.indexOf (realRegExp, _cursor) == _cursor;
}
/*}}}*/
/*{{{  bool AVRASMFileParser::matchInteger (void)*/
/*
 *	see if the text under the cursor is an integer.
 */
bool AVRASMFileParser::matchInteger (void)
{
	static const QRegExp hexRegExp ("^0x[0-9a-f]+");
	static const QRegExp binRegExp ("^(0b)?[0-1]+");
	static const QRegExp decRegExp ("^[0-9]+");

	// First test for hexadecimal number
	if (_input.indexOf (hexRegExp, _cursor) == _cursor) {
		return true;
	}
	// Then check for binary numbers
	if (_input.indexOf (binRegExp, _cursor) == _cursor) {
		return true;
	}
	// Then check for decimal numbers
	if (_input.indexOf (decRegExp, _cursor) == _cursor) {
		return true;
	}
	return false;
}
/*}}}*/
/*{{{  bool AVRASMFileParser::matchComment (void)*/
/*
 *	see if the text under the cursor is a comment.
 */
bool AVRASMFileParser::matchComment (void)
{
	static const QRegExp commentRegExp ("^;[^\\n\\r]*[\\n\\r]");

	return _input.indexOf (commentRegExp, _cursor) == _cursor;
}
/*}}}*/
/*{{{  bool AVRASMFileParser::matchString (void)*/
/*
 *	see if the text under the cursor is a (double-quoted) string.
 */
bool AVRASMFileParser::matchString (void)
{
	int tmpCursor = _cursor;

	// Start with a double-quote
	if (_input.at (tmpCursor++) != '\"') {
		return false;
	}
	// Then loop through all characters
	for (; tmpCursor < _input.length () && _input.at (tmpCursor) != '\"'; ++tmpCursor) {
		if (_input.at (tmpCursor) == '\\') {
			// Escape char
			++tmpCursor;
			if (tmpCursor == _input.length ()) {
				qWarning () << __FUNCTION__ << ": Unexpected end of file";
				return false;
			}
			switch (_input.at (tmpCursor).toLatin1 ()) {
			case 'n':
			case 'r':
			case 't':
			case '\'':
			case '\"':
			case '\\':
				break;
			default:
				qWarning () << __FUNCTION__ << ": Unhandled escape: \\" << _input.at (tmpCursor).toLatin1 ();
				return false;
			}
		}
	}
	return tmpCursor < _input.length ();
}
/*}}}*/
/*{{{  bool AVRASMFileParser::matchCharacter (void)*/
/*
 *	see if the text under the cursor is a quoted character.
 */
bool AVRASMFileParser::matchCharacter (void)
{
	static const QSet < QChar > knownEscapeChars = { 'n', 'r', '\'', '\"', 't', '\\' };
	int tmpCursor = _cursor;

	// Start with a single quote
	if (_input.at (tmpCursor++) != '\'') {
		return false;
	}
	if (tmpCursor + 1 >= _input.length ()) {
		qWarning () << __FUNCTION__ << ": Unexpected end of file";
		return false;
	}
	if (_input.at (tmpCursor) == '\\') {
		// Escape character
		++tmpCursor;
		if (!knownEscapeChars.contains (_input.at (tmpCursor))) {
			qWarning () << __FUNCTION__ << ": Unknown escape character \'\\" << _input.at (tmpCursor).toLatin1 () << '\'';
			return false;
		}
		++tmpCursor;
	} else {
		++tmpCursor;
	}
	// Expect closing single quote
	if (_input.at (tmpCursor) != '\'') {
		qWarning () << __FUNCTION__ << ": Malformed character constant";
		return false;
	}
	++tmpCursor;
	return true;
}
/*}}}*/
/*{{{  bool AVRASMFileParser::matchSymbol (void)*/
/*
 *	see if the text under the cursor is an assembly symbol.
 */
bool AVRASMFileParser::matchSymbol (void)
{
      for (const QString & symbol:AVRASMSymbols) {
		if (_input.length () > _cursor + symbol.length ()) {
			if (_input.indexOf (symbol, _cursor) == _cursor) {
				return true;
			}
		}
	}
	return false;
}
/*}}}*/

// Getters/Setters

/*{{{  QFile *AVRASMFileParser::file () const*/
/*
 *	returns the file-handle associated with this instance.
 */
QFile *AVRASMFileParser::file () const
{
	return _file;
}
/*}}}*/
/*{{{  void AVRASMFileParser::setFile (QFile *file)*/
/*
 *	sets the file-handle associated with this instance.
 */
void AVRASMFileParser::setFile (QFile *file)
{
	_file = file;
}
/*}}}*/

