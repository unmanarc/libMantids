#include "query_pgsql.h"
#include <stdexcept>
#include "sqlconnector_pgsql.h"
#include <string.h>
#include <mdz_mem_vars/a_allvars.h>

#include <stdexcept>

using namespace Mantids::Database;

Query_PostgreSQL::Query_PostgreSQL()
{
    result = nullptr;
    paramCount = 0;
    currentRow = 0;

    paramValues=nullptr;
    paramLengths=nullptr;
    paramFormats=nullptr;



    dbCnt = nullptr;
    result = nullptr;
}

Query_PostgreSQL::~Query_PostgreSQL()
{
    if (result) PQclear(result);
    result = nullptr;

    free(paramValues);
    free(paramLengths);
    free(paramFormats);
}

bool Query_PostgreSQL::step0()
{
    //  :)
    if (!result) return false;
    if (execStatus != PGRES_TUPLES_OK) return false;

    int i = currentRow;

    if (currentRow >= PQntuples(result)) return false;

    int columnpos = 0;
    for ( const auto &outputVar : resultVars)
    {
        isNull.push_back(PQgetisnull(result,i,columnpos));

        switch (outputVar->getVarType())
        {
        case Memory::Abstract::Var::TYPE_BOOL:
            ABSTRACT_PTR_AS(BOOL,outputVar)->fromString(PQgetvalue(result,i,columnpos));
            break;
        case Memory::Abstract::Var::TYPE_INT8:
            ABSTRACT_PTR_AS(INT8,outputVar)->fromString(PQgetvalue(result,i,columnpos));
            break;
        case Memory::Abstract::Var::TYPE_INT16:
            ABSTRACT_PTR_AS(INT16,outputVar)->fromString(PQgetvalue(result,i,columnpos));
            break;
        case Memory::Abstract::Var::TYPE_INT32:
            ABSTRACT_PTR_AS(INT32,outputVar)->fromString(PQgetvalue(result,i,columnpos));
            break;
        case Memory::Abstract::Var::TYPE_INT64:
            ABSTRACT_PTR_AS(INT64,outputVar)->fromString(PQgetvalue(result,i,columnpos));
            break;
        case Memory::Abstract::Var::TYPE_UINT8:
            ABSTRACT_PTR_AS(UINT8,outputVar)->fromString(PQgetvalue(result,i,columnpos));
            break;
        case Memory::Abstract::Var::TYPE_UINT16:
            ABSTRACT_PTR_AS(UINT16,outputVar)->fromString(PQgetvalue(result,i,columnpos));
            break;
        case Memory::Abstract::Var::TYPE_UINT32:
            ABSTRACT_PTR_AS(UINT32,outputVar)->fromString(PQgetvalue(result,i,columnpos));
            break;
        case Memory::Abstract::Var::TYPE_UINT64:
            ABSTRACT_PTR_AS(UINT64,outputVar)->fromString(PQgetvalue(result,i,columnpos));
            break;
        case Memory::Abstract::Var::TYPE_DOUBLE:
            ABSTRACT_PTR_AS(DOUBLE,outputVar)->fromString(PQgetvalue(result,i,columnpos));
            break;
        case Memory::Abstract::Var::TYPE_BIN:
        {
            Memory::Abstract::BINARY::sBinContainer binContainer;
            binContainer.ptr = (char *)PQgetvalue(result,i,columnpos);
            // TODO: should bytes need to be 64-bit for blob64?
            binContainer.dataSize = PQgetlength(result,i,columnpos);
            ABSTRACT_PTR_AS(BINARY,outputVar)->setValue( &binContainer );
            binContainer.ptr = nullptr; // don't destroy the data.
        } break;
        case Memory::Abstract::Var::TYPE_VARCHAR:
        {
            // This will copy the memory.
            ABSTRACT_PTR_AS(VARCHAR,outputVar)->setValue( PQgetvalue(result,i,columnpos) );
        } break;
        case Memory::Abstract::Var::TYPE_STRING:
        {
            ABSTRACT_PTR_AS(STRING,outputVar)->setValue( PQgetvalue(result,i,columnpos) );
        }break;
        case Memory::Abstract::Var::TYPE_STRINGLIST:
        {
            ABSTRACT_PTR_AS(STRINGLIST,outputVar)->fromString( PQgetvalue(result,i,columnpos) );
        }break;
        case Memory::Abstract::Var::TYPE_DATETIME:
        {
            ABSTRACT_PTR_AS(DATETIME,outputVar)->fromString( PQgetvalue(result,i,columnpos) );
        }break;
        case Memory::Abstract::Var::TYPE_IPV4:
        {
            ABSTRACT_PTR_AS(IPV4,outputVar)->fromString( PQgetvalue(result,i,columnpos) );
        }break;
        case Memory::Abstract::Var::TYPE_MACADDR:
        {
            ABSTRACT_PTR_AS(MACADDR,outputVar)->fromString( PQgetvalue(result,i,columnpos) );
        }break;
        case Memory::Abstract::Var::TYPE_IPV6:
        {
            ABSTRACT_PTR_AS(IPV6,outputVar)->fromString( PQgetvalue(result,i,columnpos) );
        }break;
        case Memory::Abstract::Var::TYPE_PTR:
        {
            // This will reference the memory, but will disappear on the next step
            ABSTRACT_PTR_AS(PTR,outputVar)->setValue( PQgetvalue(result,i,columnpos) );
        } break;
        case Memory::Abstract::Var::TYPE_NULL:
            // Don't copy the value (not needed).
            break;
        }

        columnpos++;
    }

    currentRow++;

    return false;
}

void Query_PostgreSQL::psqlSetDatabaseConnector(PGconn *conn)
{
    this->dbCnt = conn;
}

ExecStatusType Query_PostgreSQL::psqlGetExecStatus() const
{
    return execStatus;
}

bool Query_PostgreSQL::postBindInputVars()
{
    paramCount = 0;

    std::list<std::string> keysIn;
    for (auto & i : InputVars) keysIn.push_back(i.first);

    // Replace the named keys for $0, $1, etc...:
    while (replaceFirstKey(query,keysIn,keysByPos, std::string("$") + std::to_string(paramCount)))
    {
        paramCount++;
    }

    if (paramCount!=keysByPos.size())
    {
        throw std::runtime_error("Param count is not the same size of keys by pos (please report).");
    }

    if (paramValues)
    {
        throw std::runtime_error("Can't bind input variables twice (please report).");
    }

    paramValues = static_cast<char **>(malloc( paramCount * sizeof(char *) ));
    paramLengths = static_cast<int *>(malloc( paramCount * sizeof(int) ));
    paramFormats = static_cast<int *>(malloc( paramCount * sizeof(int) ));

    for (size_t pos=0; pos<keysByPos.size(); pos++)
    {
        std::string * str = nullptr;
        paramFormats[pos] = 0;
        std::string key = keysByPos[pos];

        /*
        Bind params here.
        */
        switch (InputVars[ key ]->getVarType())
        {
        case Memory::Abstract::Var::TYPE_BOOL:
        {
            str = createDestroyableStringForInput(ABSTRACT_PTR_AS(BOOL,InputVars[key])->toString());
        } break;
        case Memory::Abstract::Var::TYPE_INT8:
        {
            str = createDestroyableStringForInput(ABSTRACT_PTR_AS(INT8,InputVars[key])->toString());
        } break;
        case Memory::Abstract::Var::TYPE_INT16:
        {
            str = createDestroyableStringForInput(ABSTRACT_PTR_AS(INT16,InputVars[key])->toString());
        } break;
        case Memory::Abstract::Var::TYPE_INT32:
        {
            str = createDestroyableStringForInput(ABSTRACT_PTR_AS(INT32,InputVars[key])->toString());
        } break;
        case Memory::Abstract::Var::TYPE_INT64:
        {
            str = createDestroyableStringForInput(ABSTRACT_PTR_AS(INT64,InputVars[key])->toString());
        } break;
        case Memory::Abstract::Var::TYPE_UINT8:
        {
            str = createDestroyableStringForInput(ABSTRACT_PTR_AS(UINT8,InputVars[key])->toString());
        } break;
        case Memory::Abstract::Var::TYPE_UINT16:
        {
            str = createDestroyableStringForInput(ABSTRACT_PTR_AS(UINT16,InputVars[key])->toString());
        } break;
        case Memory::Abstract::Var::TYPE_UINT32:
        {
            str = createDestroyableStringForInput(ABSTRACT_PTR_AS(UINT32,InputVars[key])->toString());
        } break;
        case Memory::Abstract::Var::TYPE_UINT64:
        {
            str = createDestroyableStringForInput(ABSTRACT_PTR_AS(UINT64,InputVars[key])->toString());
        } break;
        case Memory::Abstract::Var::TYPE_DATETIME:
        {
            str = createDestroyableStringForInput(ABSTRACT_PTR_AS(DATETIME,InputVars[key])->toString());
        } break;
        case Memory::Abstract::Var::TYPE_DOUBLE:
        {
            str = createDestroyableStringForInput(ABSTRACT_PTR_AS(DOUBLE,InputVars[key])->toString());
        } break;
        case Memory::Abstract::Var::TYPE_BIN:
        {
            auto * i =ABSTRACT_PTR_AS(BINARY,InputVars[key])->getValue();
            paramValues[pos] = i->ptr;
            paramLengths[pos] = i->dataSize;
            paramFormats[pos] = 1;
        } break;
        case Memory::Abstract::Var::TYPE_VARCHAR:
        {
            paramValues[pos] = ABSTRACT_PTR_AS(VARCHAR,InputVars[key])->getValue();
            paramLengths[pos] = strnlen(ABSTRACT_PTR_AS(VARCHAR,InputVars[key])->getValue(),ABSTRACT_PTR_AS(VARCHAR,InputVars[key])->getVarSize());
        } break;
        case Memory::Abstract::Var::TYPE_PTR:
        {
            void * ptr = ABSTRACT_PTR_AS(PTR,InputVars[key])->getValue();
            // Threat PTR as char * (be careful, we should receive strlen compatible string, without null termination will result in an undefined behaviour)
            paramLengths[pos] = strnlen((char *)ptr,(0xFFFFFFFF/2)-1);
            paramValues[pos] = (char *) ptr;
        } break;
        case Memory::Abstract::Var::TYPE_STRING:
        {
            str = (std::string *)ABSTRACT_PTR_AS(STRING,InputVars[key])->getDirectMemory();
        } break;
        case Memory::Abstract::Var::TYPE_STRINGLIST:
            str = createDestroyableStringForInput(ABSTRACT_PTR_AS(STRINGLIST,InputVars[key])->toString());
            break;
        case Memory::Abstract::Var::TYPE_IPV4:
            str = createDestroyableStringForInput(ABSTRACT_PTR_AS(IPV4,InputVars[key])->toString());
            break;
        case Memory::Abstract::Var::TYPE_MACADDR:
            str = createDestroyableStringForInput(ABSTRACT_PTR_AS(MACADDR,InputVars[key])->toString());
            break;
        case Memory::Abstract::Var::TYPE_IPV6:
            str = createDestroyableStringForInput(ABSTRACT_PTR_AS(IPV6,InputVars[key])->toString());
            break;
        case Memory::Abstract::Var::TYPE_NULL:
            paramValues[pos] = nullptr;
            paramLengths[pos] = 0;
            break;
        }

        if (str)
        {
            paramValues[pos] = (char *)str->c_str();
            paramLengths[pos] = str->size();
        }
    }
    return true;
}

bool Query_PostgreSQL::exec0(const ExecType &execType, bool recursion)
{
    if (result)
    {
        throw std::runtime_error("Re-using queries is not supported.");
    }

    // Prepare the query (will lock the db while using ppDb):
    static_cast<SQLConnector_PostgreSQL*>(sqlConnector)->getDatabaseConnector(this);

    if (!dbCnt)
        return false;

    // Prepare the query:
    result = PQexecPrepared(dbCnt,
                            query.c_str(),
                            paramCount,
                            paramValues,
                            paramLengths,
                            paramFormats,
                            0);


    // Maybe is not connected or something failed very hard here.
    if (!result)
    {
        // While not connected...
        while (PQstatus(dbCnt) != CONNECTION_OK && !recursion)
        {
            // Reconnect...
            if ( ((SQLConnector_PostgreSQL*)sqlConnector)->reconnect(0xFFFFABCD) )
            {
                // If reconnected... execute the query again
                bool result2 = exec0(execType,true);

                // If there is any result, return the result...
                if (result)
                    return result2;
                // ...
                if (result2 == true)
                    throw std::runtime_error("how this can be true?.");

                // If no result (eg. disconnected again), then, keep the loop reconnecting
            }
            else
            {
                // The connection reached the max limit
                lastSQLError = "reconnection failed.";
                return false;
            }
        }
        lastSQLError = "connection failed.";
        return false;
    }

    execStatus = PQresultStatus(result);

    numRows=0;
    affectedRows=0;

    if (    execStatus==PGRES_BAD_RESPONSE
            ||
            execStatus==PGRES_FATAL_ERROR
            )
    {
        PQclear(result);
        result = nullptr;
        return false;
    }

    if (execType==EXEC_TYPE_SELECT)
    {
        numRows = PQntuples(result);
        return execStatus == PGRES_TUPLES_OK;
    }
    else
    {
        affectedRows = strtoull(PQcmdTuples(result),nullptr,10);
        if (bFetchLastInsertRowID)
             lastInsertRowID = PQoidValue(result);
        return execStatus == PGRES_COMMAND_OK;
    }
}
