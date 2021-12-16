# Builds a static version of the dataslinger library

TEMPLATE = lib
CONFIG += staticlib c++17

win32:APPVEYOR {
    INCLUDEPATH += $$(DATASLINGER_BOOST_PATH)
    LIBS += "-L$$(DATASLINGER_BOOST_PATH)/lib64-msvc-14.2"
} else {
    INCLUDEPATH += $$(DATASLINGER_BOOST_PATH)
    LIBS += "-L$$(DATASLINGER_BOOST_PATH)/stage/x64/lib/"
}

include($$PWD/dataslinger.pri)
