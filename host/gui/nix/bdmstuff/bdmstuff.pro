#-------------------------------------------------
#
# Project created by QtCreator 2019-08-12T16:43:57
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = bdmstuff
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QMAKE_CFLAGS += -O1
QMAKE_CXXFLAGS +=  -pthread
CONFIG += c++11s

SOURCES += \
        ../../../core/core.c \
        ../../../core/core_debug.c \
        ../../../core/core_requests.c \
        ../../../core/core_requests_CPU32.c \
        ../../../core/core_requests_HCS12.c \
        ../../../core/core_requests_JTAG.c \
        ../../../core/core_requests_MPC.c \
        ../../../core/core_worker.c \
        ../../../core/debug/dbg_hcs12.c \
        ../../../core/targets/BMW_cluster.c \
        ../../../core/targets/Trionic.c \
        ../../../core/targets/checksum/checksum.c \
        ../../../core/targets/checksum/edc16c39/csum_16c39.c \
        ../../../core/targets/checksum/edc16c39/csum_16c39_saab95.c \
        ../../../core/targets/generic.c \
        ../../../core/targets/hcs12.c \
        ../../../core/targets/mpc5566.c \
        ../../../core/targets/mpc562.c \
        ../../../core/targets/68hc08_uart.c \
        ../../../core/utils/utils.c \
        ../../../core/utils/utl_md5.c \
        debugwindow.cpp \
        main.cpp \
        mainwindow.cpp \
        worker.cpp

HEADERS += \
        ../../../core/core.h \
        ../../../core/core_debug.h \
        ../../../core/core_requests.h \
        ../../../core/core_requests_CPU32.h \
        ../../../core/core_requests_HCS12.h \
        ../../../core/core_requests_JTAG.h \
        ../../../core/core_requests_MPC.h \
        ../../../core/core_worker.h \
        ../../../core/debug/dbg_hcs12.h \
        ../../../core/targets/checksum/checksum.h \
        ../../../core/targets/checksum/csum_private.h \
        ../../../core/targets/checksum/edc16c39/csum_16c39_private.h \
        ../../../core/targets/drivers/TxDriver.bin.h \
        ../../../core/targets/drivers/hcs12driver.h \
        ../../../core/targets/drivers/hcs12driver_eeprom.h \
        ../../../core/targets/target_descriptors.h \
        ../../../core/targets/targets.h \
        ../../../core/utils/utils.h \
        debugwindow.h \
        main.h \
        mainwindow.h \
        worker.h

FORMS += \
        debugwindow.ui \
        mainwindow.ui


LIBS += -L../../../libusb/libusb/.libs -lusb-1.0

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
