#include "avrasmtoken.h"
#include <QMap>
#include <QString>

QMap<QString, AVRASMToken::TokenType> AVRASMToken::_tokenTypeStrings = {
    { "NOTOKEN",  NOTOKEN  },
    { "KEYWORD",  KEYWORD  },
    { "INTEGER",  INTEGER  },
    { "REAL",     REAL     },
    { "STRING",   STRING   },
    { "NAME",     NAME     },
    { "SYMBOL",   SYMBOL   },
    { "COMMENT",  COMMENT  },
    { "NEWLINE",  NEWLINE  },
    { "INDENT",   INDENT   },
    { "OUTDENT",  OUTDENT  },
    { "INAME",    INAME    },
    { "LSPECIAL", LSPECIAL },
    { "END",      END      }
};

QString                AVRASMToken::tokenTypeToString(TokenType tokenType) {
    return _tokenTypeStrings.key(tokenType);
}

AVRASMToken::TokenType AVRASMToken::stringToTokenType(const QString &tokenString) {
    return _tokenTypeStrings.value(tokenString, (AVRASMToken::TokenType)-1);
}

AVRASMToken::AVRASMToken(QObject *parent) :
    QObject(parent)
{
}

int AVRASMToken::length() const
{
    return _length;
}

void AVRASMToken::setLength(int value)
{
    _length = value;
}

int AVRASMToken::column() const
{
    return _column;
}

void AVRASMToken::setColumn(int value)
{
    _column = value;
}

int AVRASMToken::line() const
{
    return _line;
}

void AVRASMToken::setLine(int value)
{
    _line = value;
}

AVRASMToken::TokenType AVRASMToken::type() const
{
    return _type;
}

void AVRASMToken::setType(const TokenType &type)
{
    _type = type;
}

QString AVRASMToken::origin() const {
    return _origin;
}

void AVRASMToken::setOrigin(const QString &origin) {
    _origin = origin;
}

QString AVRASMToken::value() const
{
    return _value;
}

void AVRASMToken::setValue(const QString &value)
{
    _value = value;
}

QDebug operator<<(QDebug qdebug, const AVRASMToken &token) {
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
