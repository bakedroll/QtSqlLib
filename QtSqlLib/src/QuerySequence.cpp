#include "QtSqlLib/Query/QuerySequence.h"

#include "QtSqlLib/API/IQueryVisitor.h"

namespace QtSqlLib::Query
{

void QuerySequence::accept(API::IQueryVisitor& visitor)
{
  visitor.visit(*this);
}

void QuerySequence::addQuery(std::unique_ptr<API::IQueryElement> query)
{
  m_queryElements.emplace_back(std::move(query));
}

void QuerySequence::prepare(API::ISchema& schema)
{
}

void QuerySequence::traverse(API::IQueryVisitor& visitor)
{
  for (const auto& queryElement : m_queryElements)
  {
    queryElement->accept(visitor);
  }
}

int QuerySequence::getNumQueries() const
{
  return static_cast<int>(m_queryElements.size());
}

API::IQueryElement& QuerySequence::getQuery(int index)
{
  return *m_queryElements.at(index);
}

}
