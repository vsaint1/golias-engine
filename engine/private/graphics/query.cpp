#include "graphics/query.h"

#include "graphics/ogl_commons.h"

namespace golias {

    Query::Query(QueryType type) : mType(type) {
        if (GLAD_GL_ARB_timer_query) {
            glGenQueries(1, &mQueryId);
        }
    }

    Query::~Query() {
        if (mQueryId != 0) {
            glDeleteQueries(1, &mQueryId);
        }
    }

    void Query::Begin() {
        if (mQueryId != 0) {
            glBeginQuery(QueryTargetToGl(mType), mQueryId);
        }
    }

    void Query::End() {
        if (mQueryId != 0) {
            glEndQuery(QueryTargetToGl(mType));
        }
    }

    QueryResult Query::GetResult(uint64_t* nanoseconds) const {
        if (mQueryId == 0 || nanoseconds == nullptr) {
            return QueryResult::Pending;
        }

        GLint available = GL_FALSE;
        glGetQueryObjectiv(mQueryId, GL_QUERY_RESULT_AVAILABLE, &available);

        if (available == GL_FALSE) {
            return QueryResult::Pending;
        }

        if (GLAD_GL_EXT_disjoint_timer_query) {
            GLint disjoint = GL_FALSE;
            glGetIntegerv(GL_GPU_DISJOINT_EXT, &disjoint);

            if (disjoint != GL_FALSE) {
                return QueryResult::Disjoint;
            }
        }

        GLuint64 result = 0;
        glGetQueryObjectui64v(mQueryId, GL_QUERY_RESULT, &result);

        *nanoseconds = static_cast<uint64_t>(result);
        return QueryResult::Available;
    }

    QueryType Query::GetType() const {
        return mType;
    }

} // namespace golias
