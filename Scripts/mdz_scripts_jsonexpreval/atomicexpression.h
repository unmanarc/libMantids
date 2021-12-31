#ifndef ATOMICEXPRESSION_H
#define ATOMICEXPRESSION_H

#include <string>
#include <vector>

#include <mdz_hlp_functions/json.h>

#include "atomicexpressionside.h"

namespace Mantids { namespace Scripts { namespace Expressions {

enum eEvalOperator {
    EVAL_OPERATOR_CONTAINS, // operator for multi items..
    EVAL_OPERATOR_REGEXMATCH,
    EVAL_OPERATOR_ISEQUAL,
    EVAL_OPERATOR_STARTSWITH,
    EVAL_OPERATOR_ENDSWITH,
    EVAL_OPERATOR_ISNULL,
    EVAL_OPERATOR_UNDEFINED
};

class AtomicExpression
{
public:
    AtomicExpression(std::vector<std::string> *staticTexts );

    bool compile( std::string expr );
    bool evaluate(const json & values);

    void setStaticTexts(std::vector<std::string> *value);

private:
    bool calcNegative(bool r);
    bool substractExpressions(const std::string &regex, const eEvalOperator & op);

    std::vector<std::string> *staticTexts;
    std::string expr;
    AtomicExpressionSide left,right;
    eEvalOperator evalOperator;
    bool negativeExpression, ignoreCase;

};

}}}
#endif // ATOMICEXPRESSION_H
