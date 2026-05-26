#pragma once

#include "alibabacloud/oss2/Types.h"

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <variant>

namespace alibabacloud {
namespace oss2 {
namespace models {

// Expression type constants for SelectObject SQL expression.
namespace SelectExpressionType {
inline constexpr char SQL[] = "SQL";
}

// CSV header handling mode for InputSerialization.
namespace CSVHeaderInfo {
inline constexpr char None[] = "None";
inline constexpr char Ignore[] = "Ignore";
inline constexpr char Use[] = "Use";
}

// JSON document format type for InputSerialization.
namespace JsonType {
inline constexpr char DOCUMENT[] = "DOCUMENT";
inline constexpr char LINES[] = "LINES";
}

// Compression type for InputSerialization.
namespace CompressionType {
inline constexpr char NONE[] = "NONE";
inline constexpr char GZIP[] = "GZIP";
}


// CSV format parameters for InputSerialization.
// All delimiter values are base64-encoded strings.
struct ALIBABACLOUD_OSS_API CSVInputFormat final {
    // How to handle the first row: "None", "Ignore", or "Use"
    std::optional<std::string> headerInfo;
    // Base64-encoded record delimiter (e.g., "Cg==" for "\n")
    std::optional<std::string> recordDelimiter;
    // Base64-encoded field delimiter (e.g., "LA==" for ",")
    std::optional<std::string> fieldDelimiter;
    // Base64-encoded quote character
    std::optional<std::string> quoteChar;
    // Base64-encoded comment character
    std::optional<std::string> commentChar;
    // Whether to allow record delimiters within quoted fields
    std::optional<bool> allowQuotedRecordDelimiter;
    // Row range ("line-range=start-end") or split range ("split-range=start-end")
    std::optional<std::string> range;

    template <typename ValueT = std::string>
    CSVInputFormat& setHeaderInfo(ValueT&& value) {
        headerInfo = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = std::string>
    CSVInputFormat& setRecordDelimiter(ValueT&& value) {
        recordDelimiter = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = std::string>
    CSVInputFormat& setFieldDelimiter(ValueT&& value) {
        fieldDelimiter = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = std::string>
    CSVInputFormat& setQuoteChar(ValueT&& value) {
        quoteChar = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = std::string>
    CSVInputFormat& setCommentChar(ValueT&& value) {
        commentChar = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = bool>
    CSVInputFormat& setAllowQuotedRecordDelimiter(ValueT&& value) {
        allowQuotedRecordDelimiter = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = std::string>
    CSVInputFormat& setRange(ValueT&& value) {
        range = std::forward<ValueT>(value);
        return *this;
    }
};


// JSON format parameters for InputSerialization.
struct ALIBABACLOUD_OSS_API JSONInputFormat final {
    // JSON document type: "DOCUMENT" or "LINES"
    std::optional<std::string> jsonType;
    // Whether to parse JSON numbers as strings to preserve precision
    std::optional<bool> parseJsonNumberAsString;
    // Row range ("line-range=start-end") or split range ("split-range=start-end")
    std::optional<std::string> range;

    template <typename ValueT = std::string>
    JSONInputFormat& setJsonType(ValueT&& value) {
        jsonType = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = bool>
    JSONInputFormat& setParseJsonNumberAsString(ValueT&& value) {
        parseJsonNumberAsString = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = std::string>
    JSONInputFormat& setRange(ValueT&& value) {
        range = std::forward<ValueT>(value);
        return *this;
    }
};


// Input serialization parameters specifying the format of the source object.
// Set either csv or json, not both.
struct ALIBABACLOUD_OSS_API InputSerialization final {
    // Compression type of the source object: "NONE" or "GZIP"
    std::optional<std::string> compressionType;
    // CSV format parameters (mutually exclusive with json)
    std::optional<CSVInputFormat> csv;
    // JSON format parameters (mutually exclusive with csv)
    std::optional<JSONInputFormat> json;

    template <typename ValueT = std::string>
    InputSerialization& setCompressionType(ValueT&& value) {
        compressionType = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = CSVInputFormat>
    InputSerialization& setCsv(ValueT&& value) {
        csv = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = JSONInputFormat>
    InputSerialization& setJson(ValueT&& value) {
        json = std::forward<ValueT>(value);
        return *this;
    }
};


// CSV format parameters for OutputSerialization.
// All delimiter values are base64-encoded strings.
struct ALIBABACLOUD_OSS_API CSVOutputFormat final {
    std::optional<std::string> recordDelimiter;
    std::optional<std::string> fieldDelimiter;
    std::optional<std::string> quoteChar;

    template <typename ValueT = std::string>
    CSVOutputFormat& setRecordDelimiter(ValueT&& value) {
        recordDelimiter = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = std::string>
    CSVOutputFormat& setFieldDelimiter(ValueT&& value) {
        fieldDelimiter = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = std::string>
    CSVOutputFormat& setQuoteChar(ValueT&& value) {
        quoteChar = std::forward<ValueT>(value);
        return *this;
    }
};


// JSON format parameters for OutputSerialization.
struct ALIBABACLOUD_OSS_API JSONOutputFormat final {
    // Base64-encoded record delimiter for output JSON records
    std::optional<std::string> recordDelimiter;

    template <typename ValueT = std::string>
    JSONOutputFormat& setRecordDelimiter(ValueT&& value) {
        recordDelimiter = std::forward<ValueT>(value);
        return *this;
    }
};


// Output serialization parameters specifying the format of the query result.
// Set either csv or json, not both.
struct ALIBABACLOUD_OSS_API OutputSerialization final {
    // CSV format parameters (mutually exclusive with json)
    std::optional<CSVOutputFormat> csv;
    // JSON format parameters (mutually exclusive with csv)
    std::optional<JSONOutputFormat> json;
    // Whether to include all columns in output even if not selected
    std::optional<bool> keepAllColumns;
    // Whether to output raw data without frame encoding
    std::optional<bool> outputRawData;
    // Whether to enable CRC32 checksum per frame payload
    std::optional<bool> enablePayloadCrc;
    // Whether to include CSV header row in output
    std::optional<bool> outputHeader;

    template <typename ValueT = CSVOutputFormat>
    OutputSerialization& setCsv(ValueT&& value) {
        csv = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = JSONOutputFormat>
    OutputSerialization& setJson(ValueT&& value) {
        json = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = bool>
    OutputSerialization& setKeepAllColumns(ValueT&& value) {
        keepAllColumns = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = bool>
    OutputSerialization& setOutputRawData(ValueT&& value) {
        outputRawData = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = bool>
    OutputSerialization& setEnablePayloadCrc(ValueT&& value) {
        enablePayloadCrc = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = bool>
    OutputSerialization& setOutputHeader(ValueT&& value) {
        outputHeader = std::forward<ValueT>(value);
        return *this;
    }
};


// Options for controlling SelectObject behavior on malformed records.
struct ALIBABACLOUD_OSS_API SelectRequestOptions final {
    // Whether to skip records that have fewer columns than expected
    std::optional<bool> skipPartialDataRecord;
    // Maximum number of skipped records allowed before returning an error
    std::optional<std::int64_t> maxSkippedRecordsAllowed;

    template <typename ValueT = bool>
    SelectRequestOptions& setSkipPartialDataRecord(ValueT&& value) {
        skipPartialDataRecord = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = std::int64_t>
    SelectRequestOptions& setMaxSkippedRecordsAllowed(ValueT&& value) {
        maxSkippedRecordsAllowed = std::forward<ValueT>(value);
        return *this;
    }
};


// The XML body payload for a SelectObject request.
struct ALIBABACLOUD_OSS_API SelectRequest final {
    // Base64-encoded SQL expression (e.g., base64("select * from ossobject"))
    std::optional<std::string> expression;
    // Format description of the source object
    std::optional<InputSerialization> inputSerialization;
    // Format description of the query result
    std::optional<OutputSerialization> outputSerialization;
    // Behavior options for malformed records
    std::optional<SelectRequestOptions> options;

    template <typename ValueT = std::string>
    SelectRequest& setExpression(ValueT&& value) {
        expression = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = InputSerialization>
    SelectRequest& setInputSerialization(ValueT&& value) {
        inputSerialization = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = OutputSerialization>
    SelectRequest& setOutputSerialization(ValueT&& value) {
        outputSerialization = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = SelectRequestOptions>
    SelectRequest& setOptions(ValueT&& value) {
        options = std::forward<ValueT>(value);
        return *this;
    }
};


// Request for the SelectObject API.
// Executes an SQL expression against a CSV or JSON object.
class ALIBABACLOUD_OSS_API SelectObjectRequest final : public RequestModel {
  public:
    SelectObjectRequest() = default;

    inline const std::string& getBucket() const { return bucket_; }
    template <typename ValueT = std::string>
    SelectObjectRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    inline const std::string& getKey() const { return key_; }
    template <typename ValueT = std::string>
    SelectObjectRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // The process type: "csv/select" or "json/select"
    inline const std::string& getProcess() const { return process_; }
    template <typename ValueT = std::string>
    SelectObjectRequest& setProcess(ValueT&& value) {
        process_ = std::forward<ValueT>(value);
        return *this;
    }

    // The XML body containing the SQL expression and serialization settings
    inline const SelectRequest& getSelectRequest() const { return selectRequest_.at(0); }
    inline bool hasSelectRequest() const { return selectRequest_.find(0) != selectRequest_.end(); }
    template <typename ValueT = SelectRequest>
    SelectObjectRequest& setSelectRequest(ValueT&& value) {
        selectRequest_.insert_or_assign(0, std::forward<ValueT>(value));
        return *this;
    }

    // Custom sink factory for writing the decoded response body
    inline const std::optional<SinkFactory>& getSinkFactory() const { return sinkFactory_; }
    SelectObjectRequest& setSinkFactory(SinkFactory value) {
        sinkFactory_ = std::move(value);
        return *this;
    }

  private:
    std::string bucket_;
    std::string key_;
    std::string process_;
    std::map<int, SelectRequest> selectRequest_;
    std::optional<SinkFactory> sinkFactory_;
};


// Result of the SelectObject API.
class ALIBABACLOUD_OSS_API SelectObjectResult final : public ResultModel {
  public:
    SelectObjectResult() = default;
    SelectObjectResult(int statusCode, HeaderCollection headers)
            : ResultModel(statusCode, std::move(headers)) {}

    // The frame-decoded response body stream
    inline const std::shared_ptr<std::iostream>& getBody() const { return body_; }
    void setBody(std::shared_ptr<std::iostream> body) { body_ = std::move(body); }

  private:
    std::shared_ptr<std::iostream> body_;
};


// The XML body payload for CreateSelectObjectMeta with CSV format.
struct ALIBABACLOUD_OSS_API CSVMetaRequest final {
    // Input format description of the CSV object
    std::optional<InputSerialization> inputSerialization;
    // Whether to overwrite existing cached metadata
    std::optional<bool> overwriteIfExists;

    template <typename ValueT = InputSerialization>
    CSVMetaRequest& setInputSerialization(ValueT&& value) {
        inputSerialization = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = bool>
    CSVMetaRequest& setOverwriteIfExists(ValueT&& value) {
        overwriteIfExists = std::forward<ValueT>(value);
        return *this;
    }
};


// The XML body payload for CreateSelectObjectMeta with JSON format.
struct ALIBABACLOUD_OSS_API JSONMetaRequest final {
    // Input format description of the JSON object
    std::optional<InputSerialization> inputSerialization;
    // Whether to overwrite existing cached metadata
    std::optional<bool> overwriteIfExists;

    template <typename ValueT = InputSerialization>
    JSONMetaRequest& setInputSerialization(ValueT&& value) {
        inputSerialization = std::forward<ValueT>(value);
        return *this;
    }
    template <typename ValueT = bool>
    JSONMetaRequest& setOverwriteIfExists(ValueT&& value) {
        overwriteIfExists = std::forward<ValueT>(value);
        return *this;
    }
};


// Union type for the CreateSelectObjectMeta request body.
using SelectMetaRequest = std::variant<CSVMetaRequest, JSONMetaRequest>;


// Request for the CreateSelectObjectMeta API.
// Retrieves metadata (row count, column count, splits) for a CSV or JSON object.
class ALIBABACLOUD_OSS_API CreateSelectObjectMetaRequest final : public RequestModel {
  public:
    CreateSelectObjectMetaRequest() = default;

    inline const std::string& getBucket() const { return bucket_; }
    template <typename ValueT = std::string>
    CreateSelectObjectMetaRequest& setBucket(ValueT&& value) {
        bucket_ = std::forward<ValueT>(value);
        return *this;
    }

    inline const std::string& getKey() const { return key_; }
    template <typename ValueT = std::string>
    CreateSelectObjectMetaRequest& setKey(ValueT&& value) {
        key_ = std::forward<ValueT>(value);
        return *this;
    }

    // The process type: "csv/meta" or "json/meta"
    inline const std::string& getProcess() const { return process_; }
    template <typename ValueT = std::string>
    CreateSelectObjectMetaRequest& setProcess(ValueT&& value) {
        process_ = std::forward<ValueT>(value);
        return *this;
    }

    // The XML body describing input format (CSVMetaRequest or JSONMetaRequest)
    inline const SelectMetaRequest& getSelectMetaRequest() const { return selectMetaRequest_.at(0); }
    inline bool hasSelectMetaRequest() const { return selectMetaRequest_.find(0) != selectMetaRequest_.end(); }
    template <typename ValueT>
    CreateSelectObjectMetaRequest& setSelectMetaRequest(ValueT&& value) {
        selectMetaRequest_.insert_or_assign(0, std::forward<ValueT>(value));
        return *this;
    }

  private:
    std::string bucket_;
    std::string key_;
    std::string process_;
    std::map<int, SelectMetaRequest> selectMetaRequest_;
};


// Parsed metadata from the CreateSelectObjectMeta framed binary response.
struct ALIBABACLOUD_OSS_API SelectObjectMeta final {
    std::int64_t offset{};
    std::int64_t totalScanned{};
    std::int32_t status{};
    std::int32_t splitsCount{};
    std::int64_t rowsCount{};
    std::int32_t columnsCount{};
    std::string errorMessage;
};


// Result of the CreateSelectObjectMeta API.
// Contains metadata parsed from the framed binary response.
class ALIBABACLOUD_OSS_API CreateSelectObjectMetaResult final : public ResultModel {
  public:
    CreateSelectObjectMetaResult() = default;
    CreateSelectObjectMetaResult(int statusCode, HeaderCollection headers)
            : ResultModel(statusCode, std::move(headers)) {}
    CreateSelectObjectMetaResult(int statusCode, HeaderCollection headers, SelectObjectMeta body)
            : ResultModel(statusCode, std::move(headers)), body_(std::move(body)) {}

    // Byte offset of the metadata in the object
    inline std::int64_t getOffset() const { return body_.offset; }

    // Total bytes scanned during metadata creation
    inline std::int64_t getTotalScanned() const { return body_.totalScanned; }

    // HTTP-like status code from the end frame (200 = success)
    inline std::int32_t getStatus() const { return body_.status; }

    // Number of splits in the object
    inline std::int32_t getSplitsCount() const { return body_.splitsCount; }

    // Total number of rows in the object
    inline std::int64_t getRowsCount() const { return body_.rowsCount; }

    // Total number of columns (CSV only, 0 for JSON)
    inline std::int32_t getColumnsCount() const { return body_.columnsCount; }

    // Error message from the end frame (empty on success)
    inline const std::string& getErrorMessage() const { return body_.errorMessage; }

  private:
    SelectObjectMeta body_;
};

} // namespace models
} // namespace oss2
} // namespace alibabacloud
