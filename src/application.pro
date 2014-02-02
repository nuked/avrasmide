CONFIG       += static release c++11

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets \
          serialport
}

HEADERS       = mainwindow.h \
    parameters.h \
    optionconfig.h \
    editorconfiguration.h \
    avrasmtoken.h \
    avrasmlexer.h \
    language.h \
    arduinoconfiguration.h \
    tooltipwidget.h \
    avrasmfileparser.h

SOURCES       = main.cpp \
                mainwindow.cpp \
    parameters.cpp \
    optionconfig.cpp \
    editorconfiguration.cpp \
    avrasmtoken.cpp \
    avrasmlexer.cpp \
    arduinoconfiguration.cpp \
    language.cpp \
    tooltipwidget.cpp \
    avrasmfileparser.cpp

RESOURCES     = application.qrc

LIBS         += -L$$[QT_INSTALL_LIBS] -lqscintilla2

FORMS += \
    parameters.ui \
    editorconfiguration.ui \
    arduinoconfiguration.ui

RC_FILE = avrasmide.rc
