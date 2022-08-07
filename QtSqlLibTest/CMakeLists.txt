begin_project(QtSqlLibTest EXECUTABLE OPTIONAL)

if (QT_USE_VERSION_5)
  set(QT_VERSION "5")
else()
  set(QT_VERSION "6")
endif()

enable_automoc()

require_library(Qt${QT_VERSION} MODULES Core Sql)

require_project(utilsLib PATH utilsLib)

require_library(GTest)
require_library(GMock)

require_project(QtSqlLib)

add_source_directory(tests)
add_source_directory(src)
add_source_directory(include)

add_include_directory(include)
