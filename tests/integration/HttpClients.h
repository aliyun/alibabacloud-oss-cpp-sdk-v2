#pragma once

#include <httplib.h>
#include <string>
#include <map>

namespace alibabacloud {
namespace oss2 {
namespace test {

class HttpClient {
  public:
    struct Response {
        long statusCode = 0;
        std::string body;
        std::map<std::string, std::string> headers;
    };

    Response get(const std::string& url) {
        return get(url, {});
    }

    Response get(const std::string& url, const std::map<std::string, std::string>& headers) {
        auto [base, path] = splitUrl(url);
        httplib::Client cli(base);
        cli.set_connection_timeout(connectTimeout_);
        cli.set_read_timeout(readWriteTimeout_);
        cli.set_follow_location(true);
        cli.set_url_encode(false);
        httplib::Headers h(headers.begin(), headers.end());
        auto res = cli.Get(path, h);
        return toResponse(res);
    }

    Response put(const std::string& url, const std::string& data) {
        return put(url, data, {});
    }

    Response put(const std::string& url, const std::string& data,
                 const std::map<std::string, std::string>& headers) {
        auto [base, path] = splitUrl(url);
        httplib::Client cli(base);
        cli.set_connection_timeout(connectTimeout_);
        cli.set_read_timeout(readWriteTimeout_);
        cli.set_follow_location(true);
        cli.set_url_encode(false);
        httplib::Headers h(headers.begin(), headers.end());

        std::string contentType;
        auto ct = h.find("Content-Type");
        if (ct != h.end()) {
            contentType = ct->second;
            h.erase(ct);
        }

        auto res = cli.Put(path, h, data.size(),
                [&data](size_t offset, size_t length, httplib::DataSink& sink) {
                    sink.write(data.data() + offset, std::min(length, data.size() - offset));
                    return true;
                }, contentType);
        return toResponse(res);
    }

    void setConnectTimeout(long seconds) {
        connectTimeout_ = std::chrono::seconds(seconds);
    }

    void setReadWriteTimeout(long seconds) {
        readWriteTimeout_ = std::chrono::seconds(seconds);
    }

  private:
    static std::pair<std::string, std::string> splitUrl(const std::string& url) {
        // scheme://host[:port]/path[?query]
        auto schemeEnd = url.find("://");
        if (schemeEnd == std::string::npos) {
            return {"http://" + url, "/"};
        }
        auto pathStart = url.find('/', schemeEnd + 3);
        if (pathStart == std::string::npos) {
            return {url, "/"};
        }
        return {url.substr(0, pathStart), url.substr(pathStart)};
    }

    static Response toResponse(const httplib::Result& res) {
        Response r;
        if (res) {
            r.statusCode = res->status;
            r.body = res->body;
            for (const auto& h : res->headers) {
                r.headers[h.first] = h.second;
            }
        }
        return r;
    }

    std::chrono::seconds connectTimeout_{10};
    std::chrono::seconds readWriteTimeout_{30};
};

} // namespace test
} // namespace oss2
} // namespace alibabacloud
