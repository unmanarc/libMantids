#include "atomicexpression.h"

#include <string>

#include <boost/algorithm/string/predicate.hpp>

using namespace std;
using namespace Mantids::Scripts::Expressions;

AtomicExpression::AtomicExpression(std::vector<string> *staticTexts)
    : left(staticTexts)
    , right(staticTexts)
{
    evalOperator = EVAL_OPERATOR_UNDEFINED;
    ignoreCase = false;
    negativeExpression = false;
    setStaticTexts(staticTexts);
}

bool AtomicExpression::compile(std::string expr)
{
    if (boost::starts_with(expr, "!"))
    {
        negativeExpression = true;
        expr = expr.substr(1);
    }
    if (boost::starts_with(expr, "i"))
    {
        ignoreCase = true;
        expr = expr.substr(1);
    }
    this->expr = expr;

    if (substractExpressions("^IS_EQUAL\\(([^,]+),([^\\)]+)\\)$", EVAL_OPERATOR_ISEQUAL))
    {
    }
    else if (substractExpressions("^REGEX_MATCH\\(([^,]+),([^\\)]+)\\)$", EVAL_OPERATOR_REGEXMATCH))
    {
    }
    else if (substractExpressions("^CONTAINS\\(([^,]+),([^\\)]+)\\)$", EVAL_OPERATOR_CONTAINS))
    {
    }
    else if (substractExpressions("^STARTS_WITH\\(([^,]+),([^\\)]+)\\)$", EVAL_OPERATOR_STARTSWITH))
    {
    }
    else if (substractExpressions("^ENDS_WITH\\(([^,]+),([^\\)]+)\\)$", EVAL_OPERATOR_ENDSWITH))
    {
    }
    else if (substractExpressions("^IS_NULL\\(([^\\)]+)\\)$", EVAL_OPERATOR_ISNULL))
    {
    }
    else
    {
        evalOperator = EVAL_OPERATOR_UNDEFINED;
        negativeExpression = false;
        return false;
    }

    return true;
}

bool AtomicExpression::evaluate(const json &values)
{
    std::set<std::string> lvalues = left.resolve(values, evalOperator == EVAL_OPERATOR_REGEXMATCH, ignoreCase);
    std::set<std::string> rvalues = right.resolve(values, evalOperator == EVAL_OPERATOR_REGEXMATCH, ignoreCase);

    switch (evalOperator)
    {
    case EVAL_OPERATOR_UNDEFINED:
        return calcNegative(false);
    case EVAL_OPERATOR_ENDSWITH:
        for (const std::string &lvalue : lvalues)
        {
            for (const std::string &rvalue : rvalues)
            {
                if (ignoreCase && boost::iends_with(lvalue, rvalue))
                {
                    return calcNegative(true);
                }
                else if (!ignoreCase && boost::ends_with(lvalue, rvalue))
                {
                    return calcNegative(true);
                }
            }
        }
        return calcNegative(false);
    case EVAL_OPERATOR_STARTSWITH:
        for (const std::string &lvalue : lvalues)
        {
            for (const std::string &rvalue : rvalues)
            {
                if (ignoreCase && boost::istarts_with(lvalue, rvalue))
                {
                    return calcNegative(true);
                }
                else if (!ignoreCase && boost::starts_with(lvalue, rvalue))
                {
                    return calcNegative(true);
                }
            }
        }
        return calcNegative(false);
    case EVAL_OPERATOR_ISEQUAL:
        for (const std::string &rvalue : rvalues)
        {
            if (!ignoreCase && lvalues.find(rvalue) != lvalues.end())
                return calcNegative(true);
            else if (ignoreCase)
            {
                for (const std::string &lvalue : lvalues)
                {
                    if (boost::iequals(rvalue, lvalue))
                        return calcNegative(true);
                }
            }
        }
        return calcNegative(false);
    case EVAL_OPERATOR_ISNULL:
        return calcNegative(lvalues.empty());
    case EVAL_OPERATOR_CONTAINS:
        for (const std::string &lvalue : lvalues)
        {
            for (const std::string &rvalue : rvalues)
            {
                if (!ignoreCase && boost::contains(lvalue, rvalue))
                {
                    return calcNegative(true);
                }
                else if (ignoreCase && boost::icontains(lvalue, rvalue))
                {
                    return calcNegative(true);
                }
            }
        }
        return calcNegative(false);
    case EVAL_OPERATOR_REGEXMATCH:
#ifdef USE_STD_REGEX
        std::smatch what;  // Para std::regex
#else
        boost::cmatch what;  // Para boost::regex
#endif
        // Regex, any of.
        for (const std::string &lvalue : lvalues)
        {
#ifdef USE_STD_REGEX
            if (right.getRegexp() && std::regex_match(lvalue, what, *right.getRegexp()))  // std::regex_match
#else
            if (right.getRegexp() && boost::regex_match(lvalue.c_str(), what, *right.getRegexp()))  // boost::regex_match
#endif
            {
                return calcNegative(true);  // Si la coincidencia es exitosa
            }
        }
        return calcNegative(false);  // Si ninguna coincidencia es exitosa
    }
    return calcNegative(false);
}

bool AtomicExpression::calcNegative(bool r)
{
    if (negativeExpression)
        return !r;
    return r;
}

bool AtomicExpression::substractExpressions(const std::string &regex, const eEvalOperator &op)
{
#ifdef USE_STD_REGEX
    std::regex exOperatorEqual(regex); // Usamos std::regex
    std::smatch whatDataDecomposed;    // Usamos std::smatch para los resultados
#else
    boost::regex exOperatorEqual(regex);                                  // Usamos boost::regex
    boost::match_results<std::string::const_iterator> whatDataDecomposed; // Resultados con boost
#endif

// Flags de búsqueda (boost::match_default o std::regex_constants::match_default)
#ifdef USE_STD_REGEX
    std::regex_constants::match_flag_type flags = std::regex_constants::match_default;
#else
    boost::match_flag_type flags = boost::match_default;
#endif

    // Iteradores de inicio y fin de la expresión
    std::string::const_iterator start = expr.begin();
    std::string::const_iterator end = expr.end();

// Bucle de búsqueda con regex_search
#ifdef USE_STD_REGEX
    if (std::regex_search(start, end, whatDataDecomposed, exOperatorEqual, flags))
#else
    if (boost::regex_search(start, end, whatDataDecomposed, exOperatorEqual, flags))
#endif
    {
        // Asignación de la expresión izquierda
        left.setExpr(std::string(whatDataDecomposed[1].first, whatDataDecomposed[1].second));

        // Si no es un operador ISNULL, asignar la expresión derecha
        if (op != EVAL_OPERATOR_ISNULL)
        {
            right.setExpr(std::string(whatDataDecomposed[2].first, whatDataDecomposed[2].second));
        }
        else
        {
            right.setExpr("");
        }

        // Evaluar las expresiones izquierda y derecha
        if (!left.calcMode())
            return false;
        if (!right.calcMode())
            return false;

        // Asignar el operador de evaluación
        evalOperator = op;

        // Finalizamos el bucle ya que la expresión se procesó
        return true;
    }

    return false;
}

void AtomicExpression::setStaticTexts(std::vector<std::string> *value)
{
    staticTexts = value;
}
