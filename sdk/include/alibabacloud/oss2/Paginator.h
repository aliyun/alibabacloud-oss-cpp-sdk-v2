#pragma once

#include "alibabacloud/oss2/OSSClient.h"

#include <functional>

namespace alibabacloud {
namespace oss2 {

template <typename RequestT>
struct PaginatorTraits;

template <typename RequestT>
class Paginator {
  public:
    using OutcomeType = typename OSSClient::OperationTraits<RequestT>::OutcomeType;

    Paginator(std::function<OutcomeType(const RequestT&, const OperationOptions*)> callable, RequestT request,
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
            if (PaginatorTraits<RequestT>::isTruncated(outcome.value())) {
                PaginatorTraits<RequestT>::setNextToken(request_, outcome.value());
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
 * @brief Creates a Paginator that automatically iterates through paginated API results.
 *
 * The returned Paginator is single-pass: once all pages have been consumed (hasNext() returns
 * false), it cannot be reset. To iterate from the beginning again, call makePaginator() to
 * create a new instance.
 *
 * @code
 *   auto paginator = makePaginator(client,
 *       models::ListObjectsV2Request()
 *           .setBucket("my-bucket")
 *           .setMaxKeys(100));
 *   while (paginator.hasNext()) {
 *       const auto outcome = paginator.nextPage();
 *       if (!outcome.has_value()) break;
 *       for (const auto& obj : outcome.value().getContents()) {
 *           // process obj
 *       }
 *   }
 * @endcode
 *
 * @param client   An OSSClient instance (lvalue reference or shared_ptr).
 * @param request  The initial request; pagination tokens are managed internally.
 * @param options  Per-request options (timeout, retry, etc.); defaults to empty.
 * @return A Paginator<RequestT> ready to iterate.
 */
template <typename ClientT, typename RequestT>
auto makePaginator(ClientT&& client, RequestT&& request, OperationOptions options = {}) {
    using DecayedRequest = std::decay_t<RequestT>;
    auto fn = [c = std::forward<ClientT>(client)](const DecayedRequest& req, const OperationOptions* opts) mutable {
        return std::invoke(OSSClient::OperationTraits<DecayedRequest>::method, c, req, opts);
    };
    return Paginator<DecayedRequest>(std::move(fn), std::forward<RequestT>(request), std::move(options));
}

// PaginatorTraits specializations

template <>
struct PaginatorTraits<models::ListBucketsRequest> {
    static bool isTruncated(const models::ListBucketsResult& result) {
        return result.getIsTruncated();
    }
    static void setNextToken(models::ListBucketsRequest& request, const models::ListBucketsResult& result) {
        request.setMarker(result.getNextMarker());
    }
};

template <>
struct PaginatorTraits<models::ListObjectsRequest> {
    static bool isTruncated(const models::ListObjectsResult& result) {
        return result.getIsTruncated();
    }
    static void setNextToken(models::ListObjectsRequest& request, const models::ListObjectsResult& result) {
        request.setMarker(result.getNextMarker());
    }
};

template <>
struct PaginatorTraits<models::ListObjectsV2Request> {
    static bool isTruncated(const models::ListObjectsV2Result& result) {
        return result.getIsTruncated();
    }
    static void setNextToken(models::ListObjectsV2Request& request, const models::ListObjectsV2Result& result) {
        request.setContinuationToken(result.getNextContinuationToken());
    }
};

template <>
struct PaginatorTraits<models::ListObjectVersionsRequest> {
    static bool isTruncated(const models::ListObjectVersionsResult& result) {
        return result.getIsTruncated();
    }
    static void setNextToken(models::ListObjectVersionsRequest& request,
                             const models::ListObjectVersionsResult& result) {
        request.setKeyMarker(result.getNextKeyMarker());
        request.setVersionIdMarker(result.getNextVersionIdMarker());
    }
};

template <>
struct PaginatorTraits<models::ListMultipartUploadsRequest> {
    static bool isTruncated(const models::ListMultipartUploadsResult& result) {
        return result.getIsTruncated();
    }
    static void setNextToken(models::ListMultipartUploadsRequest& request,
                             const models::ListMultipartUploadsResult& result) {
        request.setKeyMarker(result.getNextKeyMarker());
        request.setUploadIdMarker(result.getNextUploadIdMarker());
    }
};

template <>
struct PaginatorTraits<models::ListPartsRequest> {
    static bool isTruncated(const models::ListPartsResult& result) {
        return result.getIsTruncated();
    }
    static void setNextToken(models::ListPartsRequest& request, const models::ListPartsResult& result) {
        request.setPartNumberMarker(result.getNextPartNumberMarker());
    }
};

} // namespace oss2
} // namespace alibabacloud
