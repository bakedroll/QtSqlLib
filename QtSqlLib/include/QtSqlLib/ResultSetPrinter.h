#pragma once

#include <QtSqlLib/API/ISchema.h>
#include <QtSqlLib/ColumnHelper.h>
#include <QtSqlLib/QueryIdentifiers.h>
#include <QtSqlLib/ResultSet.h>

#include <QString>
#include <QVariant>

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
  using ColumnValuesList = std::vector<QVariant>;

  enum EPrinterStage
  {
    HEADER = 0,
    SEPARATOR = 1,
    CONTENT = 2,
    FINISHED = 3
  };

  struct ColumnMetaInfo
  {
    QString caption;
    API::DataType type;
    int width;
    API::IID::Type columnId;
  };

  struct ColumnMetaInfoLocation
  {
    std::size_t begin = 0ULL;
    std::size_t end = 0ULL;
  };

  struct BufferState
  {
    size_t currentPointer = 0ULL;
    size_t copyTo = 1ULL;
  };

  ResultSet& m_resultSet;
  EPrinterStage m_printerStage;

  std::vector<ColumnMetaInfo> m_columnMetaInfoList;
  std::map<RelationshipId, ColumnMetaInfoLocation> m_columnMetaInfoLocations;

  std::vector<ColumnValuesList> m_columnValuesBuffer;
  BufferState m_bufferState;

  std::vector<API::IID::Type> m_receivedRelationships;

  static void prepareHeaderColumnMetaInfo(
    API::ISchema& schema,
    const API::QueryMetaInfo& queryMetaInfo,
    int maxColumnWidth,
    std::vector<ColumnMetaInfo>& columnMetaInfoList,
    std::map<RelationshipId, ColumnMetaInfoLocation>& columnMetaInfoLocations);

  static void prepareResultColumnMetaInfo(
    const TupleView& tupleView,
    int maxColumnWidth,
    std::vector<ColumnMetaInfo>& columnMetaInfoList,
    std::map<RelationshipId, ColumnMetaInfoLocation>& columnMetaInfoLocations);

  void fillJoinedColumnValuesBuffer();
  void writeColumnValues(const TupleView& tupleView, std::vector<QVariant>& values);

  QString makeRowLine(const std::function<QString(size_t)>& columnFunc) const;

  QString headerColumnNamesLine() const;
  QString headerSeparatorLine() const;
  QString contentLine() const;

};

}
