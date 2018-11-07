INCLUDEPATH += $$PWD/dataslinger

# Include library dependencies
INCLUDEPATH += $$PWD/lib/concurrentqueue/

HEADERS += $$files($$PWD/dataslinger/*.h, true)
SOURCES += $$files($$PWD/dataslinger/*.cpp, true)
