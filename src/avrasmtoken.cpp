/*
 *	avrasmtoken.cpp -- infrastructure for token handling
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

#include "avrasmtoken.h"

#include <QMap>
#include <QString>

/*{{{  map for token name lookup*/
QMap<QString, AVRASMToken::TokenType> AVRASMToken::_tokenTypeStrings = {
	{ "NOTOKEN",	NOTOKEN  },
	{ "KEYWORD",	KEYWORD  },
	{ "INTEGER",	INTEGER  },
	{ "REAL",	REAL     },
	{ "STRING",	STRING   },
	{ "NAME",	NAME     },
	{ "SYMBOL",	SYMBOL   },
	{ "COMMENT",	COMMENT  },
	{ "NEWLINE",	NEWLINE  },
	{ "INDENT",	INDENT   },
	{ "OUTDENT",	OUTDENT  },
	{ "INAME",	INAME    },
	{ "LSPECIAL",	LSPECIAL },
	{ "END",	END      }
};
/*}}}*/

/*{{{  QString AVRASMToken::tokenTypeToString (TokenType tokenType)*/
/*
 *	string representation of token.
 */
QString AVRASMToken::tokenTypeToString (TokenType tokenType)
{
	return _tokenTypeStrings.key(tokenType);
}
/*}}}*/
/*{{{  AVRASMToken::TokenType AVRASMToken::stringToTokenType (const QString &tokenString)*/
/*
 *	token representation of string.
 */
AVRASMToken::TokenType AVRASMToken::stringToTokenType (const QString &tokenString)
{
	return _tokenTypeStrings.value(tokenString, (AVRASMToken::TokenType)-1);
}
/*}}}*/

/*{{{  AVRASMToken::AVRASMToken(QObject *parent) : QObject(parent)*/
/*
 *	constructor.
 */
AVRASMToken::AVRASMToken(QObject *parent) : QObject(parent)
{
}
/*}}}*/

/*{{{  int AVRASMToken::length () const*/
/*
 *	returns the token's length (characters).
 */
int AVRASMToken::length () const
{
	return _length;
}
/*}}}*/
/*{{{  void AVRASMToken::setLength (int value)*/
/*
 *	sets the token's length (characters).
 */
void AVRASMToken::setLength (int value)
{
	_length = value;
}
/*}}}*/
/*{{{  int AVRASMToken::column () const*/
/*
 *	returns the column this token lives at (in source code).
 */
int AVRASMToken::column () const
{
	return _column;
}
/*}}}*/
/*{{{  void AVRASMToken::setColumn (int value)*/
/*
 *	sets the column this token lives at.
 */
void AVRASMToken::setColumn (int value)
{
	_column = value;
}
/*}}}*/
/*{{{  int AVRASMToken::line () const*/
/*
 *	gets the line-number of this token.
 */
int AVRASMToken::line () const
{
	return _line;
}
/*}}}*/
/*{{{  void AVRASMToken::setLine (int value)*/
/*
 *	sets the line-number for this token.
 */
void AVRASMToken::setLine (int value)
{
	_line = value;
}
/*}}}*/
/*{{{  AVRASMToken::TokenType AVRASMToken::type () const*/
/*
 *	gets the type of this token.
 */
AVRASMToken::TokenType AVRASMToken::type () const
{
	return _type;
}
/*}}}*/
/*{{{  void AVRASMToken::setType (const TokenType &type)*/
/*
 *	sets the type of this token.
 */
void AVRASMToken::setType (const TokenType &type)
{
	_type = type;
}
/*}}}*/
/*{{{  QString AVRASMToken::origin () const*/
/*
 *	gets the origin of this token.
 */
QString AVRASMToken::origin () const
{
	return _origin;
}
/*}}}*/
/*{{{  void AVRASMToken::setOrigin (const QString &origin)*/
/*
 *	sets the originl of this token.
 */
void AVRASMToken::setOrigin (const QString &origin)
{
	_origin = origin;
}
/*}}}*/
/*{{{  QString AVRASMToken::value () const*/
/*
 *	gets the value of this token.
 */
QString AVRASMToken::value () const
{
	return _value;
}
/*}}}*/
/*{{{  void AVRASMToken::setValue (const QString &value)*/
/*
 *	sets the value of a token.
 */
void AVRASMToken::setValue (const QString &value)
{
	_value = value;
}
/*}}}*/
/*{{{  QDebug operator << (QDebug qdebug, const AVRASMToken &token)*/
/*
 *	for debugging.
 */
QDebug operator << (QDebug qdebug, const AVRASMToken &token)
{
	bool autoInsertSpaces = qdebug.autoInsertSpaces();

	qdebug.setAutoInsertSpaces(false);
	qdebug << "<Token: "
		<< " type="   << AVRASMToken::tokenTypeToString(token.type()) << ""
		<< " origin=" << token.origin() << ""
		<< " line="   << token.line() << ""
		<< " column=" << token.column() << ""
		<< " length=" << token.length() << ""
		<< " value=" << token.value() << ">";
	qdebug.setAutoInsertSpaces(autoInsertSpaces);
	return qdebug;
}
/*}}}*/

