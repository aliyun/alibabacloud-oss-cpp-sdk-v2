
#pragma once

#include "AsyncExecuteMiddleware.h"
#include "src/internal/OSSUtils.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/signer/Signer.h"

namespace alibabacloud {
namespace oss2 {
namespace internal {

class SignerAsyncMiddleware final : public AsyncExecuteMiddleware {
  public:
    SignerAsyncMiddleware(std::unique_ptr<AsyncExecuteMiddleware> next,
                          std::shared_ptr<Signer> signer,
                          std::shared_ptr<CredentialsProvider> provider)
            : next_(std::move(next)),
              signer_(std::move(signer)),
              provider_(std::move(provider)) {}

    void handleRequest(const std::shared_ptr<AsyncExecuteState>& state) override {
        if (provider_ == nullptr) {
            updateError(state->context, SdkErrorCode::CREDENTIALS_PROVIDER_NULL,
                        "IllegalArgument", "Credentials provider is null.");
            prev_->handleResponse(state);
            return;
        }

        if (provider_->getAuthType() == CredentialsProvider::AuthType::ANONYMOUS) {
            next_->handleRequest(state);
            return;
        }

        auto cred = provider_->getCredentials();
        if (!cred.hasKeys()) {
            updateError(state->context, SdkErrorCode::CREDENTIALS_EMPTYNULL,
                        "CredentialsError", "Credentials is null or empty.");
            prev_->handleResponse(state);
            return;
        }

        state->context.signingContext.credentials = std::move(cred);
        state->context.signingContext.request = state->request.get();

        if (!signer_->sign(state->context.signingContext)) {
            updateError(state->context, SdkErrorCode::SIGN_ERROR,
                        "SignatureError", "The signer encountered an error while signing.");
            prev_->handleResponse(state);
            return;
        }

        next_->handleRequest(state);
    }

    void handleResponse(const std::shared_ptr<AsyncExecuteState>& state) override {
        prev_->handleResponse(state);
    }

  private:
    std::unique_ptr<AsyncExecuteMiddleware> next_;
    std::shared_ptr<Signer> signer_;
    std::shared_ptr<CredentialsProvider> provider_;
};

} // namespace internal
} // namespace oss2
} // namespace alibabacloud
