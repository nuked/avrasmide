/*
 *	avrasmfileparser.h
 *	Gregoire Liglet and Florent Chiron, University of Kent, 2013
 */

#ifndef AVRASMFILEPARSER_H
#define AVRASMFILEPARSER_H

#include <QObject>

class QFile;

class AVRASMFileParser:public QObject
{
Q_OBJECT public:
	explicit AVRASMFileParser (QObject * parent = 0);
	AVRASMFileParser (QFile * file, QObject * parent = 0);

	bool matchKeyword ();
	bool matchName ();
	bool matchReal ();
	bool matchInteger ();
	bool matchComment ();
	bool matchString ();
	bool matchCharacter ();
	bool matchSymbol ();

	// Getters/Setters
	QFile *file () const;
	void setFile (QFile * file);

private:
	QFile * _file;
	QString _input;
	int _cursor;
};

#endif // AVRASMFILEPARSER_H
