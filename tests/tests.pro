QT += core testlib network widgets
TARGET = test_login
CONFIG += console
CONFIG -= app_bundle

SOURCES += /root/DailyReport/tests/test_login.cpp \
           /root/DailyReport/src/utils.cpp \
           /root/DailyReport/src/apimanager.cpp \
           /root/DailyReport/src/cloudsessionmanager.cpp \
           /root/DailyReport/src/datamodel.cpp

INCLUDEPATH += /root/DailyReport/src

HEADERS += /root/DailyReport/src/utils.h \
           /root/DailyReport/src/apimanager.h \
           /root/DailyReport/src/cloudsessionmanager.h \
           /root/DailyReport/src/datamodel.h
