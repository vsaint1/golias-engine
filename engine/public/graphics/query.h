#pragma once
#include "graphics/render_types.h"
#include "stdafx.h"

namespace golias {

    class Query {

    public:
        explicit Query(QueryType type);
        ~Query();

        void Begin();
        void End();

        /// @brief  Non-blocking result read. Returns the elapsed nanoseconds in *nanoseconds.
        QueryResult GetResult(uint64_t* nanoseconds) const;

        QueryType GetType() const;

    private:
        Query(const Query&)            = delete;
        Query& operator=(const Query&) = delete;

    private:
        GLuint mQueryId = 0;
        QueryType mType = QueryType::TimeElapsed;
    };

} // namespace golias
