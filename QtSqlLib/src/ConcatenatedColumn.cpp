#include <QtSqlLib/ConcatenatedColumn.h>

#include <QtSqlLib/DatabaseException.h>

namespace QtSqlLib
{

ConcatenatedColumn::ConcatenatedColumn() = default;

QString ConcatenatedColumn::buildString(const API::IQueryIdentifiers& queryIdentifiers, API::ISchema& schema) const
{
  QString elementsStr;

  for (const auto& element : m_elements)
  {
    if (!elementsStr.isEmpty())
    {
      elementsStr.append(", ");
    }

    if (element.canConvert<QString>())
    {
      elementsStr.append(QString("\"%1\"").arg(element.toString()));
    }
    else if (element.canConvert<ColumnHelper::ColumnData>())
    {
      elementsStr.append(queryIdentifiers.resolveColumnIdentifier(schema, element.value<ColumnHelper::ColumnData>()));
    }
    else
    {
      throw DatabaseException(DatabaseException::Type::UnexpectedError, "Unexpected element type in CONCAT");
    }
  }

  return QString("CONCAT(%1)").arg(elementsStr);
}

void ConcatenatedColumn::makeElements(size_t currentIndex, const QString& str)
{
  m_elements[currentIndex] = str;
}

void ConcatenatedColumn::makeElements(size_t currentIndex, const char* str)
{
  m_elements[currentIndex] = QString(str);
}

}
