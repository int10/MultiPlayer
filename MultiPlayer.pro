TEMPLATE = app
CONFIG -= app_bundle
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets opengl

DEFINES += BUILD_QTAV_STATIC BUILD_QTAVWIDGETS_STATIC

INCLUDEPATH += ./ ./QtAV ./widgets ./widgets/QtAVWidgets

#LIBS += -LD:/Play/build-QtAV-Qt_5_4_1MingWStatic-Release/lib_win_x86 -lQt5AVWidgets -lQt5AV
LIBS += -L$$PWD/lib -L$$PWD/lib/ffmpeg \
           -lQt5AVWidgets -lQt5AV -lgdiplus \
           -lswresample -lavdevice -lavfilter -lavcodec -lavformat -lswscale -lavutil -lstrmiids -lvfw32 -lz

include(src.pri)

FORMS +=

HEADERS +=

SOURCES +=

RC_FILE = Player.rc
