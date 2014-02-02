#ifndef AVRASMTOKEN_H
#define AVRASMTOKEN_H

#include <QObject>
#include <QDebug>

class AVRASMToken : public QObject
{
    Q_OBJECT
public:

    typedef enum TokenType {
        NOTOKEN  = 0,
        KEYWORD  = 1,
        INTEGER  = 2,
        REAL     = 3,
        STRING   = 4,
        NAME     = 5,
        SYMBOL   = 6,
        COMMENT  = 7,
        NEWLINE  = 8,
        INDENT   = 9,
        OUTDENT  = 10,
        INAME    = 11,
        LSPECIAL = 12,
        END      = 13
    } TokenType;

    explicit AVRASMToken(QObject *parent = 0);

    int       line() const;
    void      setLine(int value);
    int       column() const;
    void      setColumn(int value);
    int       length() const;
    void      setLength(int value);
    TokenType type() const;
    void      setType(const TokenType &type);
    QString   origin() const;
    void      setOrigin(const QString &origin);
    QString   value() const;
    void      setValue(const QString &value);

    static QString   tokenTypeToString(TokenType tokenType);
    static TokenType stringToTokenType(const QString &tokenString);

private:
    int       _line;
    int       _column;
    int       _length;
    TokenType _type;
    QString   _origin;
    QString   _value;

    static QMap<QString, TokenType> _tokenTypeStrings;
};

QDebug operator<<(QDebug qdebug, const AVRASMToken &token);

#endif // AVRASMTOKEN_H
