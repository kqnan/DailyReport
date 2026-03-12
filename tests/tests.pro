QT += core testlib network widgets
TARGET = test_login
CONFIG += console
CONFIG -= app_bundle

SOURCES += /root/DailyReport/tests/test_login.cpp \
           /root/DailyReport/src/utils.cpp \
           /root/DailyReport/src/apimanager.cpp

INCLUDEPATH += /root/DailyReport/src

HEADERS += /root/DailyReport/src/utils.h \
           /root/DailyReport/src/apimanager.h
