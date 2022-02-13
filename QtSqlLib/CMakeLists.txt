begin_project(QtSqlLib LIBRARY)

enable_automoc()

if (QT_USE_VERSION_5)
  set(QT_VERSION "5")
else()
  set(QT_VERSION "6")
endif()

require_library(Qt${QT_VERSION} MODULES Core Sql)

require_project(utilsLib PATH utilsLib)

add_source_directory(include/API)
add_source_directory(include/Query)
add_source_directory(include)
add_source_directory(src)

add_include_directory(include)
