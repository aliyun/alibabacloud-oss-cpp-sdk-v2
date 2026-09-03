#pragma once

#include "alibabacloud/oss2/agentic/OSSAgenticClient.h"

#include <functional>

namespace alibabacloud {
namespace oss2 {
namespace agentic {

template <typename RequestT>
struct AgenticPaginatorTraits;

template <typename RequestT>
class AgenticPaginator {
  public:
    using OutcomeType = typename OSSAgenticBucketClient::OperationTraits<RequestT>::OutcomeType;

    AgenticPaginator(std::function<OutcomeType(const RequestT&, const OperationOptions*)> callable, RequestT request,
                     OperationOptions options = {})
        : callable_(std::move(callable)),
          request_(std::move(request)),
          options_(std::move(options)),
          first_(true),
          done_(false) {}

    bool hasNext() const {
        return first_ || !done_;
    }

    OutcomeType nextPage() {
        first_ = false;
        auto outcome = callable_(request_, &options_);
        if (outcome.has_value()) {
            if (AgenticPaginatorTraits<RequestT>::isTruncated(outcome.value())) {
                AgenticPaginatorTraits<RequestT>::setNextToken(request_, outcome.value());
            } else {
                done_ = true;
            }
        } else {
            done_ = true;
        }
        return outcome;
    }

  private:
    std::function<OutcomeType(const RequestT&, const OperationOptions*)> callable_;
    RequestT request_;
    OperationOptions options_;
    bool first_;
    bool done_;
};

/**
 * @brief Creates a paginator that iterates through paginated agentic bucket results.
 */
template <typename ClientT, typename RequestT>
auto makeAgenticPaginator(ClientT&& client, RequestT&& request, OperationOptions options = {}) {
    using DecayedRequest = std::decay_t<RequestT>;
    auto fn = [c = std::forward<ClientT>(client)](const DecayedRequest& req, const OperationOptions* opts) mutable {
        return std::invoke(OSSAgenticBucketClient::OperationTraits<DecayedRequest>::method, c, req, opts);
    };
    return AgenticPaginator<DecayedRequest>(std::move(fn), std::forward<RequestT>(request), std::move(options));
}

template <>
struct AgenticPaginatorTraits<models::ListAgenticBucketsRequest> {
    static bool isTruncated(const models::ListAgenticBucketsResult& result) {
        return result.getIsTruncated();
    }
    static void setNextToken(models::ListAgenticBucketsRequest& request,
                             const models::ListAgenticBucketsResult& result) {
        request.setContinuationToken(result.getNextContinuationToken().value_or(""));
    }
};

template <>
struct AgenticPaginatorTraits<models::ListBucketSpacesRequest> {
    static bool isTruncated(const models::ListBucketSpacesResult& result) {
        return result.getIsTruncated();
    }
    static void setNextToken(models::ListBucketSpacesRequest& request, const models::ListBucketSpacesResult& result) {
        request.setContinuationToken(result.getNextContinuationToken().value_or(""));
    }
};

} // namespace agentic
} // namespace oss2
} // namespace alibabacloud
