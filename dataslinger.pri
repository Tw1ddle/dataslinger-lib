INCLUDEPATH += $$PWD/dataslinger
INCLUDEPATH += $$PWD/lib/concurrentqueue/ # Library dependency

HEADERS += $$files($$PWD/dataslinger/*.h, true)
SOURCES += $$files($$PWD/dataslinger/*.cpp, true)
