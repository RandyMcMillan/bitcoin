# Create the super cache so modules will add themselves to it.
cache(, super)

!QTDIR_build: cache(CONFIG, add, $$list(QTDIR_build))

TEMPLATE = subdirs
SUBDIRS = qtbase qtdeclarative qttools qttranslations

qtdeclarative.depends = qtbase
qttools.depends = qtbase qtdeclarative
qttranslations.depends = qttools

load(qt_configure)
