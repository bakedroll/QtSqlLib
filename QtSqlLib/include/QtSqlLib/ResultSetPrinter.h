#pragma once

#include <QtSqlLib/API/ISchema.h>
#include <QtSqlLib/ColumnHelper.h>
#include <QtSqlLib/QueryIdentifiers.h>
#include <QtSqlLib/ResultSet.h>

#include <QString>

#include <map>
#include <optional>
#include <vector>

namespace QtSqlLib
{

class ResultSetPrinter
{
public:
  explicit ResultSetPrinter(
    API::ISchema& schema,
    ResultSet& resultSet,
    int maxColumnWidth);

  virtual ~ResultSetPrinter();

  bool isEndOfTable() const;
  QString nextPrinterLine();

private:
  using RelationshipId = std::optional<API::IID::Type>;

  struct ColumnMetaInfo
  {
    QString caption;
    API::DataType type;
    int width;
    ColumnHelper::ColumnData data;
  };

  struct ColumnMetaInfoLocation
  {
    std::size_t position = 0ULL;
    std::size_t size = 0ULL;
  };

  enum HeaderStage
  {
    COLUMN_NAMES = 0,
    SEPARATOR = 1,
    FINISHED = 2
  };

  ResultSet& m_resultSet;

  std::vector<ColumnMetaInfo> m_columnMetaInfoList;
  HeaderStage m_headerStage;

  static void prepareColumnMetaInfo(
    API::ISchema& schema,
    const TupleView& tupleView,
    int maxColumnWidth,
    QueryIdentifiers& queryIdentifiers,
    std::vector<ColumnMetaInfo>& columnMetaInfoList,
    std::map<RelationshipId, ColumnMetaInfoLocation>& columnMetaInfoLocations);

  QString headerColumnNamesLine() const;
  QString headerSeparatorLine() const;

};

}
