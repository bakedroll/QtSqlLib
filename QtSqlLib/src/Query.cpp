#include "QtSqlLib/Query/Query.h"

#include "QtSqlLib/API/IQueryVisitor.h"

namespace QtSqlLib::Query
{

void Query::accept(API::IQueryVisitor& visitor)
{
  visitor.visit(*this);
}

}
