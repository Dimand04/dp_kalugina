QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    authorizationwindow.cpp \
    edit_role.cpp \
    edit_user.cpp \
    main.cpp \
    mainwidget.cpp \
    qpushlabel.cpp

HEADERS += \
    authorizationwindow.h \
    edit_role.h \
    edit_user.h \
    mainwidget.h \
    qpushlabel.h

FORMS += \
    authorizationwindow.ui \
    edit_role.ui \
    edit_user.ui \
    mainwidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
