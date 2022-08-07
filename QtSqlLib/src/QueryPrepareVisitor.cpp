#include "QtSqlLib/QueryPrepareVisitor.h"

#include "QtSqlLib/API/IQuerySequence.h"

namespace QtSqlLib
{

QueryPrepareVisitor::QueryPrepareVisitor(API::ISchema& schema)
  : m_schema(schema)
{
}

QueryPrepareVisitor::~QueryPrepareVisitor() = default;

void QueryPrepareVisitor::visit(API::IQuery& query)
{
}

void QueryPrepareVisitor::visit(API::IQuerySequence& query)
{
  query.prepare(m_schema);
  query.traverse(*this);
}

}
