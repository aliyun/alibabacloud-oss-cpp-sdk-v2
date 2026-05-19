# Scenario: Credential Providers

Demonstrates various ways to configure credentials for the OSS SDK.

## Prerequisites

- Install `alibabacloud-oss-cpp-sdk-v2`
- For `CredentialsCppIntegration`: also install `alibabacloud-credentials-cpp`

## Build

```bash
# Built-in credential samples (no external dependencies)
cmake -B build -DCMAKE_PREFIX_PATH=<oss-sdk-prefix>
cmake --build build

# With alibabacloud-credentials-cpp integration
cmake -B build -DCMAKE_PREFIX_PATH="<oss-sdk-prefix>;<credentials-prefix>"
cmake --build build
```

## Samples

| File | Credential Type | Description |
|------|----------------|-------------|
| StaticCredentials.cpp | AK/SK hardcoded | Simplest setup, for local testing only |
| EnvironmentCredentials.cpp | Environment variables | Recommended default approach |
| StsTokenCredentials.cpp | STS temporary token | AK + SK + SecurityToken from STS AssumeRole |
| CustomProviderFunc.cpp | Custom function | Load from config file, secrets manager, etc. |
| CredentialsCppIntegration.cpp | Alibaba Cloud Credentials | Bridge to credentials-cpp for RAM Role, OIDC, ECS role, etc. |

## Run

```bash
# Static (replace placeholders in source first)
./StaticCredentials --region cn-hangzhou --bucket my-bucket

# Environment (set env vars first)
export OSS_ACCESS_KEY_ID=<your-ak>
export OSS_ACCESS_KEY_SECRET=<your-sk>
./EnvironmentCredentials --region cn-hangzhou --bucket my-bucket

# STS token
./StsTokenCredentials --region cn-hangzhou --bucket my-bucket \
    --ak <ak> --sk <sk> --token <sts-token>

# Custom provider (uses MY_APP_ACCESS_KEY_ID, MY_APP_ACCESS_KEY_SECRET)
export MY_APP_ACCESS_KEY_ID=<your-ak>
export MY_APP_ACCESS_KEY_SECRET=<your-sk>
./CustomProviderFunc --region cn-hangzhou --bucket my-bucket

# Alibaba Cloud Credentials library (auto-detect chain)
./CredentialsCppIntegration --region cn-hangzhou --bucket my-bucket --cred-type default

# Alibaba Cloud Credentials library (ECS RAM role, run on ECS instance)
./CredentialsCppIntegration --region cn-hangzhou --bucket my-bucket --cred-type ecs_ram_role
```
