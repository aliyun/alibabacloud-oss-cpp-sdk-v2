
#pragma once

#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/signer/SignerV4.h"
#include "src/internal/ExecuteMiddleware.h"
#include "src/internal/OSSUtils.h"


namespace alibabacloud {
namespace oss2 {
namespace internal {


class SignerExecuteMiddleware final : public ExecuteMiddleware {
  public:
    SignerExecuteMiddleware(std::unique_ptr<ExecuteMiddleware> nextHandler, std::shared_ptr<Signer> signer,
                            std::shared_ptr<CredentialsProvider> provider)
        : nextHandler_(std::move(nextHandler)), signer_(std::move(signer)), provider_(std::move(provider)) {}

    std::unique_ptr<ResponseMessage> Execute(std::unique_ptr<RequestMessage>& request,
                                             ExecuteContext& context) override {
        if (provider_ == nullptr) {
            updateError(context, CredentialsErrorCode::ProviderNull, "IllegalArgument",
                        "Credentials provider is null.");
            return nullptr;
        }

        if (provider_->getAuthType() == CredentialsProvider::AuthType::ANONYMOUS) {
            return nextHandler_->Execute(request, context);
        }

        auto cred = provider_->getCredentials();

        if (!cred.hasKeys()) {
            auto code = cred.isErrorRetryable() ? CredentialsErrorCode::FetchError : CredentialsErrorCode::Empty;
            updateError(context, code, "CredentialsError", cred.getError().value_or("Credentials is null or empty."));
            return nullptr;
        }

        context.signingContext.credentials = std::move(cred);
        context.signingContext.request = request.get();

        if (!signer_->sign(context.signingContext)) {
            updateError(context, SignerErrorCode::SignFailed, "SignatureError",
                        "The signer encountered an error while signing.");
            return nullptr;
        }

        return nextHandler_->Execute(request, context);
    }

  private:
    std::unique_ptr<ExecuteMiddleware> nextHandler_;
    std::shared_ptr<Signer> signer_;
    std::shared_ptr<CredentialsProvider> provider_;
};

} // namespace internal
} // namespace oss2
} // namespace alibabacloud