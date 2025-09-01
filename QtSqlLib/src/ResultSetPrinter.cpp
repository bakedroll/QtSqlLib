#include <QtSqlLib/ResultSetPrinter.h>

#include <QtSqlLib/ColumnStatistics.h>

#include <cmath>

namespace QtSqlLib
{

static constexpr const char* s_blobText = "<BLOB>";
static constexpr int s_blobTextLength = std::char_traits<char>::length(s_blobText);

static constexpr const char* s_nullText = "NULL";
static constexpr int s_nullTextLength = std::char_traits<char>::length(s_nullText);

static std::vector<API::IID::Type> generateAllPossibleIdsForColumn(API::IID::Type columnId)
{
  return {
    columnId,
    ColumnStatistics::min(columnId).id(),
    ColumnStatistics::max(columnId).id(),
    ColumnStatistics::sum(columnId, ColumnStatistics::EMethod::All).id(),
    ColumnStatistics::sum(columnId, ColumnStatistics::EMethod::Distict).id(),
    ColumnStatistics::count(columnId, ColumnStatistics::EMethod::All).id(),
    ColumnStatistics::count(columnId, ColumnStatistics::EMethod::Distict).id(),
    ColumnStatistics::count(ColumnStatistics::EMethod::All).id(),
    ColumnStatistics::count(ColumnStatistics::EMethod::Distict).id(),
    ColumnStatistics::avg(columnId, ColumnStatistics::EMethod::All).id(),
    ColumnStatistics::avg(columnId, ColumnStatistics::EMethod::Distict).id()
  };
}

static int integerLength(qlonglong value)
{
  return value == 0 ? 1 : (static_cast<int>(std::log(std::abs(value)) / std::log(10.0)) + (value < 0 ? 2 : 1));
}

static int realDecimalPlaces(double value)
{
  if (std::abs(value) < std::numeric_limits<double>::epsilon())
  {
    return 0;
  }

	for (int i=0;; ++i)
	{
		const auto decimals = std::fabs((value - std::floor(value)) / value);
		if (decimals <= std::numeric_limits<double>::epsilon())
    {
      const auto hasComma = i > 0;
			return i + (hasComma ? 1 : 0);
    }
    value *= 10;
	}
  return 0;
}

static int realLength(double value)
{
  const auto integerPart = static_cast<int>(value);
  return integerLength(integerPart) + realDecimalPlaces(value);
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

static QString elideString(const QString& str, int maxLen)
{
  if (str.length() > maxLen)
  {
    return str.left(maxLen-3).append("...");
  }
  return str;
}

ResultSetPrinter::ResultSetPrinter(
    API::ISchema& schema,
    ResultSet& resultSet,
    int maxColumnWidth) :
  m_resultSet(resultSet),
  m_headerStage(HeaderStage::COLUMN_NAMES)
{
  if (!m_resultSet.isAtBeginning())
  {
    m_resultSet.resetIteration();
  }

  maxColumnWidth = std::max(maxColumnWidth, 5);
  std::map<RelationshipId, ColumnMetaInfoLocation> columnMetaInfoLocations;
  QueryIdentifiers queryIdentifiers;

  while (m_resultSet.hasNextTuple())
  {
    prepareColumnMetaInfo(
      schema, m_resultSet.nextTuple(), maxColumnWidth,
      queryIdentifiers, m_columnMetaInfoList, columnMetaInfoLocations);

    while (m_resultSet.hasNextJoinedTuple())
    {
      prepareColumnMetaInfo(
        schema, m_resultSet.nextJoinedTuple(), maxColumnWidth,
        queryIdentifiers, m_columnMetaInfoList, columnMetaInfoLocations);
    }
  }

  m_resultSet.resetIteration();
}

ResultSetPrinter::~ResultSetPrinter() = default;

bool ResultSetPrinter::isEndOfTable() const
{
  return !((m_headerStage < HeaderStage::FINISHED) || m_resultSet.hasNextJoinedTuple() || m_resultSet.hasNextTuple());
}

QString ResultSetPrinter::nextPrinterLine()
{
  if (m_headerStage < HeaderStage::FINISHED)
  {
    if (m_headerStage == HeaderStage::COLUMN_NAMES)
    {
      m_headerStage = HeaderStage::SEPARATOR;
      return headerColumnNamesLine();
    }
    else if (m_headerStage == HeaderStage::SEPARATOR)
    {
      m_headerStage = HeaderStage::FINISHED;
      return headerSeparatorLine();
    }
  }

  if (m_resultSet.hasNextJoinedTuple())
  {
    auto tuple = m_resultSet.nextJoinedTuple();
    return "#####\n";
  }
  else if (m_resultSet.hasNextTuple())
  {
    auto tuple = m_resultSet.nextTuple();
    return "#####\n";
  }

  return "";
}

void ResultSetPrinter::prepareColumnMetaInfo(
  API::ISchema& schema,
  const TupleView& tupleView,
  int maxColumnWidth,
  QueryIdentifiers& queryIdentifiers,
  std::vector<ColumnMetaInfo>& columnMetaInfoList,
  std::map<RelationshipId, ColumnMetaInfoLocation>& columnMetaInfoLocations)
{
  const auto relationshipId = tupleView.relationshipId();
  const auto tableId = tupleView.tableId();

  if (columnMetaInfoLocations.count(relationshipId) == 0)
  {
    ColumnMetaInfoLocation metaInfoLocation;
    metaInfoLocation.position = columnMetaInfoList.size();

    queryIdentifiers.addTableIdentifier(relationshipId, tableId);
    const auto& table = schema.getTables().at(tableId);
    for (const auto& column : table.columns)
    {
      const auto columnId = column.first;
      const auto ids = generateAllPossibleIdsForColumn(columnId);
      for (const auto id : ids)
      {
        if (tupleView.hasColumnValue(id))
        {
          ColumnHelper::ColumnData columnData;
          columnData.relationshipId = relationshipId;
          columnData.columnId = id;

          const auto caption = queryIdentifiers.resolveColumnIdentifier(schema, columnData);

          ColumnMetaInfo metaInfo {
            caption,
            column.second.type,
            std::min(maxColumnWidth, caption.length()),
            columnData
          };

          columnMetaInfoList.emplace_back(metaInfo);
        }
      }
    }

    metaInfoLocation.size = columnMetaInfoList.size() - metaInfoLocation.position;
    columnMetaInfoLocations.emplace(std::make_pair(relationshipId, metaInfoLocation));
  }

  auto& metaInfoLocation = columnMetaInfoLocations.at(relationshipId);
  for (auto i=metaInfoLocation.position; i<metaInfoLocation.position + metaInfoLocation.size; ++i)
  {
    auto& columnMetaInfo = columnMetaInfoList.at(i);
    const auto value = tupleView.columnValue(columnMetaInfo.data.columnId);
    columnMetaInfo.width = std::clamp(determineValueTextLength(value, columnMetaInfo.type), columnMetaInfo.width, maxColumnWidth);
  }
}

QString ResultSetPrinter::headerColumnNamesLine() const
{
  QString line;
  for (const auto& columnMetaInfo : m_columnMetaInfoList)
  {
    if (!line.isEmpty())
    {
      line.append('|');
    }
    line.append(QString(" %1 ").arg(
      elideString(columnMetaInfo.caption, columnMetaInfo.width).
      leftJustified(columnMetaInfo.width, ' ')));
  }
  return line;
}

QString ResultSetPrinter::headerSeparatorLine() const
{
  auto sumWidth = 0;
  // TODO: more elegant
  for (const auto& columnMetaInfo : m_columnMetaInfoList)
  {
    sumWidth += columnMetaInfo.width;
  }
  return QString(sumWidth + m_columnMetaInfoList.size() * 3 - 1, '-');
}

}
