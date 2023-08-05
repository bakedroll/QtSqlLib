#include "InsertIntoReferences.h"

#include "QtSqlLib/ID.h"

namespace QtSqlLib::Query
{

InsertIntoReferences::InsertIntoReferences(API::IID::Type tableId)
  : InsertInto(ID(tableId))
{
}

InsertIntoReferences::~InsertIntoReferences() = default;

void InsertIntoReferences::addForeignKeyValue(const QVariant& value)
{
  m_foreignKeyValues.emplace_back(value);
}

void InsertIntoReferences::bindQueryValues(QSqlQuery& query) const
{
  InsertInto::bindQueryValues(query);

  for (const auto& value : m_foreignKeyValues)
  {
    query.addBindValue(value);
  }
}

}
