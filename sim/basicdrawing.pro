HEADERS       = renderarea.h \
                window.h \
    ../../Public/src/pov_rpi/ldprotocol.h \
    ../../Public/src/pov_rpi/ldserver.h \
    ../../Public/src/pov_rpi/povgeometry.h \
    ../../Public/src/pov_rpi/bmp.h
SOURCES       = main.cpp \
                renderarea.cpp \
                window.cpp \
    ../../Public/src/pov_rpi/povgeometry.c \
    ../../Public/src/pov_rpi/ldserver.c \
    ../../Public/src/pov_rpi/bmp.c
RESOURCES     = basicdrawing.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/painting/basicdrawing
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS basicdrawing.pro images
sources.path = $$[QT_INSTALL_EXAMPLES]/painting/basicdrawing
INSTALLS += target sources

symbian {
    TARGET.UID3 = 0xA000A649
    include($$QT_SOURCE_TREE/examples/symbianpkgrules.pri)
}
