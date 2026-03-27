
#pragma once

#include <string>

namespace alibabacloud {
namespace oss2 {
namespace internal {
class Url {
  public:
    explicit Url(const std::string& url = "");
    ~Url();
    bool operator==(const Url& url) const;
    bool operator!=(const Url& url) const;

    std::string authority() const;
    void clear();
    std::string fragment() const;
    void fromString(const std::string& url);
    bool hasFragment() const;
    bool hasQuery() const;
    std::string host() const;
    bool isEmpty() const;
    bool isValid() const;
    int port() const;
    std::string password() const;
    std::string path() const;
    std::string query() const;
    std::string scheme() const;
    void setAuthority(const std::string& authority);
    void setFragment(const std::string& fragment);
    void setHost(const std::string& host);
    void setPassword(const std::string& password);
    void setPath(const std::string& path);
    void setPort(int port);
    void setQuery(const std::string& query);
    void setScheme(const std::string& scheme);
    void setUserInfo(const std::string& userInfo);
    void setUserName(const std::string& userName);
    std::string toString() const;
    std::string userInfo() const;
    std::string userName() const;

  private:
    std::string scheme_;
    std::string userName_;
    std::string password_;
    std::string host_;
    std::string path_;
    int port_;
    std::string query_;
    std::string fragment_;
};
} // namespace internal
} // namespace oss2
} // namespace alibabacloud