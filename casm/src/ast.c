#include <assert.h>
#include "ast.h"

size_t ast_stmt_size(const Stmt stmt) {
    switch (stmt.type) {
        case STATEMENT_DIRECTIVE: {
            if (stmt.drt.type == DIRECTIVE_DB) return stmt.drt.db.count;
            else return 0;
        }

        case STATEMENT_LABEL:
            return 0;

        case STATEMENT_INSTR:
           return 2;

        default:
            assert(0 && "unknown statement type");
    }
}
