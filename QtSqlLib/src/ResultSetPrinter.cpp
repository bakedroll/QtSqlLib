#include <QtSqlLib/ResultSetPrinter.h>

#include <QtSqlLib/ColumnStatistics.h>
#include <QtSqlLib/DatabaseException.h>

#include <cmath>

namespace QtSqlLib
{

static constexpr const char* s_blobText = "<BLOB>";
static constexpr int s_blobTextLength = std::char_traits<char>::length(s_blobText);

static constexpr const char* s_nullText = "NULL";
static constexpr int s_nullTextLength = std::char_traits<char>::length(s_nullText);

static int integerLength(qlonglong value)
{
  return value == 0 ? 1 : (static_cast<int>(std::log(std::abs(value)) / std::log(10.0)) + (value < 0 ? 2 : 1));
}

static int realLength(double value)
{
  return QVariant(value).toString().length();
}

static int determineValueTextLength(const QVariant& value, API::DataType type)
{
  if (value.isNull())
  {
    return s_nullTextLength;
  }

  switch (type)
  {
  case API::DataType::Blob:
    return s_blobTextLength;
  case API::DataType::Text:
  case API::DataType::Varchar:
    return value.toString().length();
  case API::DataType::Integer:
    return integerLength(value.toLongLong());
  case API::DataType::Real:
    return realLength(value.toDouble());
  default:
    break;
  }
  return 0;
}

static QString columnValueToString(const QVariant& value, API::DataType type)
{
  if (value.isNull())
  {
    return s_nullText;
  }

  switch (type)
  {
  case API::DataType::Blob:
    return s_blobText;
  default:
    break;
  }
  return value.toString();
}

static QString elideString(const QString& str, int maxLen)
{
  if (str.length() > maxLen)
  {
    return str.left(maxLen-3).append("...");
  }
  return str;
}

static API::DataType columnDataType(const API::Table& table, API::IID::Type columnId)
{
  if (ColumnStatistics::isColumnStatistics(columnId))
  {
    const auto statistics = ColumnStatistics::fromId(columnId);
    if (statistics.type() == ColumnStatistics::EType::Avg)
    {
      return API::DataType::Real;
    }
    return table.columns.at(statistics.columnId()).type;
  }
  return table.columns.at(columnId).type;
}

ResultSetPrinter::ResultSetPrinter(
    API::ISchema& schema,
    ResultSet& resultSet,
    int maxColumnWidth) :
  m_resultSet(resultSet),
  m_printerStage(EPrinterStage::HEADER)
{
  if (!m_resultSet.isAtBeginning())
  {
    m_resultSet.resetIteration();
  }

  maxColumnWidth = std::max(maxColumnWidth, 5);

  prepareHeaderColumnMetaInfo(schema, m_resultSet.queryMetaInfo(), maxColumnWidth, m_columnMetaInfoList, m_columnMetaInfoLocations);
  for (const auto& joinQueryMetaInfo : m_resultSet.joinQueryMetaInfos())
  {
    prepareHeaderColumnMetaInfo(schema, joinQueryMetaInfo, maxColumnWidth, m_columnMetaInfoList, m_columnMetaInfoLocations);
  }

  while (m_resultSet.hasNextTuple())
  {
    prepareResultColumnMetaInfo(m_resultSet.nextTuple(), maxColumnWidth, m_columnMetaInfoList, m_columnMetaInfoLocations);

    while (m_resultSet.hasNextJoinedTuple())
    {
      prepareResultColumnMetaInfo(m_resultSet.nextJoinedTuple(), maxColumnWidth, m_columnMetaInfoList, m_columnMetaInfoLocations);
    }
  }

  m_resultSet.resetIteration();
}

ResultSetPrinter::~ResultSetPrinter() = default;

bool ResultSetPrinter::isEndOfTable() const
{
  return m_printerStage == EPrinterStage::FINISHED;
}

QString ResultSetPrinter::nextPrinterLine()
{
  switch (m_printerStage)
  {
  case EPrinterStage::HEADER:
    m_printerStage = EPrinterStage::SEPARATOR;
    return headerColumnNamesLine();
  case EPrinterStage::SEPARATOR:
    m_printerStage = m_resultSet.hasNextTuple() ? EPrinterStage::CONTENT : EPrinterStage::FINISHED;
    return headerSeparatorLine();
  case EPrinterStage::FINISHED:
    return "";
  default:
    break;
  }

  if (m_bufferState.currentPointer == m_columnValuesBuffer.size())
  {
    if (!m_columnValuesBuffer.empty() && m_resultSet.hasNextJoinedTuple())
    {
      fillJoinedColumnValuesBuffer();
    }
    else if (m_resultSet.hasNextTuple())
    {
      m_columnValuesBuffer.resize(1);
      auto& columnValues = m_columnValuesBuffer.at(0);
      if (columnValues.empty())
      {
        columnValues.resize(m_columnMetaInfoList.size());
      }
      else
      {
        std::fill(columnValues.begin(), columnValues.end(), QVariant());
      }

      m_bufferState = BufferState();
      m_receivedRelationships.clear();

      const auto tuple = m_resultSet.nextTuple();
      writeColumnValues(tuple, columnValues);
      fillJoinedColumnValuesBuffer();
    }
    else
    {
      return "";
    }
  }

  const auto line = contentLine();
  m_bufferState.currentPointer++;

  if (m_bufferState.currentPointer == m_columnValuesBuffer.size() && !m_resultSet.hasNextJoinedTuple() && !m_resultSet.hasNextTuple())
  {
    m_printerStage = EPrinterStage::FINISHED;
  }

  return line;
}

void ResultSetPrinter::prepareHeaderColumnMetaInfo(
  API::ISchema& schema,
  const API::QueryMetaInfo& queryMetaInfo,
  int maxColumnWidth,
  std::vector<ColumnMetaInfo>& columnMetaInfoList,
  std::map<RelationshipId, ColumnMetaInfoLocation>& columnMetaInfoLocations)
{
  const auto relationshipId = queryMetaInfo.relationshipId;
  const auto tableId = queryMetaInfo.tableId;

  ColumnMetaInfoLocation metaInfoLocation;
  metaInfoLocation.begin = columnMetaInfoList.size();

  QueryIdentifiers queryIdentifiers;
  queryIdentifiers.addTableIdentifier(relationshipId, tableId);
  const auto& table = schema.getTables().at(tableId);

  for (const auto& column : queryMetaInfo.columns)
  {
    ColumnHelper::ColumnData columnData;
    columnData.relationshipId = relationshipId;
    columnData.columnId = column.columnId;

    const auto caption = queryIdentifiers.resolveColumnIdentifier(schema, columnData);
    ColumnMetaInfo metaInfo {
      caption,
      columnDataType(table, column.columnId),
      std::min(maxColumnWidth, caption.length()),
      column.columnId
    };

    columnMetaInfoList.emplace_back(metaInfo);
  }

  metaInfoLocation.end = columnMetaInfoList.size();
  columnMetaInfoLocations.emplace(std::make_pair(relationshipId, metaInfoLocation));
}

void ResultSetPrinter::prepareResultColumnMetaInfo(
  const TupleView& tupleView,
  int maxColumnWidth,
  std::vector<ColumnMetaInfo>& columnMetaInfoList,
  std::map<RelationshipId, ColumnMetaInfoLocation>& columnMetaInfoLocations)
{
  const auto relationshipId = tupleView.relationshipId();
  if (columnMetaInfoLocations.count(relationshipId) == 0)
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "Unexpected relationshipId");
  }

  auto& metaInfoLocation = columnMetaInfoLocations.at(relationshipId);
  for (auto i=metaInfoLocation.begin; i<metaInfoLocation.end; ++i)
  {
    auto& columnMetaInfo = columnMetaInfoList.at(i);
    const auto value = tupleView.columnValue(columnMetaInfo.columnId);
    columnMetaInfo.width = std::clamp(determineValueTextLength(value, columnMetaInfo.type), columnMetaInfo.width, maxColumnWidth);
  }
}

void ResultSetPrinter::fillJoinedColumnValuesBuffer()
{
  while (m_resultSet.hasNextJoinedTuple())
  {
    const auto joinedTuple = m_resultSet.nextJoinedTuple();
    const auto relationshipId = joinedTuple.relationshipId().value();

    const auto it = std::find(m_receivedRelationships.cbegin(), m_receivedRelationships.cend(), relationshipId);
    if (it == m_receivedRelationships.end())
    {
      m_receivedRelationships.emplace_back(relationshipId);
      writeColumnValues(joinedTuple, m_columnValuesBuffer.at(0));
    }
    else
    {
      const auto index = static_cast<size_t>(it - m_receivedRelationships.cbegin());
      if (index < m_receivedRelationships.size()-1)
      {
        m_receivedRelationships.resize(index+1);
        m_bufferState.copyTo = m_columnValuesBuffer.size();
      }

      const auto startIndex = m_columnValuesBuffer.size();
      const auto numElements = m_bufferState.copyTo;
      m_columnValuesBuffer.resize(m_columnValuesBuffer.size() + numElements);
      std::copy(
        m_columnValuesBuffer.begin(),
        m_columnValuesBuffer.begin() + m_bufferState.copyTo,
        m_columnValuesBuffer.begin() + startIndex);

      for (size_t i = startIndex; i < startIndex + numElements; ++i)
      {
        writeColumnValues(joinedTuple, m_columnValuesBuffer.at(i));
      }

      break;
    }
  }
}

void ResultSetPrinter::writeColumnValues(const TupleView& tupleView, std::vector<QVariant>& values)
{
  const auto relationshipId = tupleView.relationshipId();
  if (m_columnMetaInfoLocations.count(relationshipId) == 0)
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "Received results for unexpected relationshipId");
  }

  const auto& location = m_columnMetaInfoLocations.at(relationshipId);
  for (auto i=location.begin; i<location.end; ++i)
  {
    const auto& metaInfo = m_columnMetaInfoList.at(i);
    values[i] = tupleView.columnValue(metaInfo.columnId);
  }
}

QString ResultSetPrinter::makeRowLine(const std::function<QString(size_t)>& columnFunc) const
{
  QString line;
  for (size_t i=0; i<m_columnMetaInfoList.size(); ++i)
  {
    if (!line.isEmpty())
    {
      line.append('|');
    }

    const auto& columnMetaInfo = m_columnMetaInfoList.at(i);
    line.append(QString(" %1 ").arg(
      elideString(columnFunc(i), columnMetaInfo.width).
      leftJustified(columnMetaInfo.width, ' ')));
  }
  return line;
}

QString ResultSetPrinter::headerColumnNamesLine() const
{
  return makeRowLine([this](size_t i) -> QString {
    const auto& columnMetaInfo = m_columnMetaInfoList.at(i);
    return columnMetaInfo.caption;
  });
}

QString ResultSetPrinter::headerSeparatorLine() const
{
  auto sumWidth = std::accumulate(m_columnMetaInfoList.cbegin(), m_columnMetaInfoList.cend(), 0, [](int a, const ColumnMetaInfo& b) -> int {
    return a + b.width;
  });
  return QString(sumWidth + m_columnMetaInfoList.size() * 3 - 1, '-');
}

QString ResultSetPrinter::contentLine() const
{
  return makeRowLine([this](size_t i) -> QString {
    const auto& columnMetaInfo = m_columnMetaInfoList.at(i);
    const auto & currentColumnValues = m_columnValuesBuffer.at(m_bufferState.currentPointer);
    return columnValueToString(currentColumnValues.at(i), columnMetaInfo.type);
  });
}

}
