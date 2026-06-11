
#include "SerdeObjectSelect.h"
#include "SerdeUtils.h"
#include "alibabacloud/oss2/Error.h"
#include "src/internal/SelectFrameDecoder.h"
#include "src/utils/Utils.h"

namespace alibabacloud {
namespace oss2 {
namespace transform {


inline static std::string toXmlText(const models::CSVInputFormat& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");
    if (value.fileHeaderInfo.has_value()) {
        str.append(toXmlText(value.fileHeaderInfo.value(), "FileHeaderInfo"));
    }
    if (value.recordDelimiter.has_value()) {
        str.append(toXmlText(value.recordDelimiter.value(), "RecordDelimiter"));
    }
    if (value.fieldDelimiter.has_value()) {
        str.append(toXmlText(value.fieldDelimiter.value(), "FieldDelimiter"));
    }
    if (value.quoteCharacter.has_value()) {
        str.append(toXmlText(value.quoteCharacter.value(), "QuoteCharacter"));
    }
    if (value.commentCharacter.has_value()) {
        str.append(toXmlText(value.commentCharacter.value(), "CommentCharacter"));
    }
    if (value.allowQuotedRecordDelimiter.has_value()) {
        str.append(toXmlText(value.allowQuotedRecordDelimiter.value(), "AllowQuotedRecordDelimiter"));
    }
    if (value.range.has_value()) {
        str.append(toXmlText(value.range.value(), "Range"));
    }
    str.append("</").append(tag).append(">");
    return str;
}

inline static std::string toXmlText(const models::JSONInputFormat& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");
    if (value.type.has_value()) {
        str.append(toXmlText(value.type.value(), "Type"));
    }
    if (value.parseJsonNumberAsString.has_value()) {
        str.append(toXmlText(value.parseJsonNumberAsString.value(), "ParseJsonNumberAsString"));
    }
    if (value.range.has_value()) {
        str.append(toXmlText(value.range.value(), "Range"));
    }
    str.append("</").append(tag).append(">");
    return str;
}

inline static std::string toXmlText(const models::InputSerialization& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");
    if (value.compressionType.has_value()) {
        str.append(toXmlText(value.compressionType.value(), "CompressionType"));
    }
    if (value.csv.has_value()) {
        str.append(toXmlText(value.csv.value(), "CSV"));
    }
    if (value.json.has_value()) {
        str.append(toXmlText(value.json.value(), "JSON"));
    }
    str.append("</").append(tag).append(">");
    return str;
}

inline static std::string toXmlText(const models::CSVOutputFormat& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");
    if (value.recordDelimiter.has_value()) {
        str.append(toXmlText(value.recordDelimiter.value(), "RecordDelimiter"));
    }
    if (value.fieldDelimiter.has_value()) {
        str.append(toXmlText(value.fieldDelimiter.value(), "FieldDelimiter"));
    }
    str.append("</").append(tag).append(">");
    return str;
}

inline static std::string toXmlText(const models::JSONOutputFormat& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");
    if (value.recordDelimiter.has_value()) {
        str.append(toXmlText(value.recordDelimiter.value(), "RecordDelimiter"));
    }
    str.append("</").append(tag).append(">");
    return str;
}

inline static std::string toXmlText(const models::OutputSerialization& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");
    if (value.csv.has_value()) {
        str.append(toXmlText(value.csv.value(), "CSV"));
    }
    if (value.json.has_value()) {
        str.append(toXmlText(value.json.value(), "JSON"));
    }
    if (value.keepAllColumns.has_value()) {
        str.append(toXmlText(value.keepAllColumns.value(), "KeepAllColumns"));
    }
    if (value.outputRawData.has_value()) {
        str.append(toXmlText(value.outputRawData.value(), "OutputRawData"));
    }
    if (value.outputHeader.has_value()) {
        str.append(toXmlText(value.outputHeader.value(), "OutputHeader"));
    }
    if (value.enablePayloadCrc.has_value()) {
        str.append(toXmlText(value.enablePayloadCrc.value(), "EnablePayloadCrc"));
    }
    str.append("</").append(tag).append(">");
    return str;
}

inline static std::string toXmlText(const models::SelectRequestOptions& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");
    if (value.skipPartialDataRecord.has_value()) {
        str.append(toXmlText(value.skipPartialDataRecord.value(), "SkipPartialDataRecord"));
    }
    if (value.maxSkippedRecordsAllowed.has_value()) {
        str.append(toXmlText(value.maxSkippedRecordsAllowed.value(), "MaxSkippedRecordsAllowed"));
    }
    str.append("</").append(tag).append(">");
    return str;
}

inline static std::string toXmlText(const models::SelectRequest& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");
    if (value.expression.has_value()) {
        str.append(toXmlText(value.expression.value(), "Expression"));
    }
    if (value.inputSerialization.has_value()) {
        str.append(toXmlText(value.inputSerialization.value(), "InputSerialization"));
    }
    if (value.outputSerialization.has_value()) {
        str.append(toXmlText(value.outputSerialization.value(), "OutputSerialization"));
    }
    if (value.options.has_value()) {
        str.append(toXmlText(value.options.value(), "Options"));
    }
    str.append("</").append(tag).append(">");
    return str;
}

inline static std::string toXmlText(const models::CSVMetaRequest& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");
    if (value.inputSerialization.has_value()) {
        str.append(toXmlText(value.inputSerialization.value(), "InputSerialization"));
    }
    if (value.overwriteIfExists.has_value()) {
        str.append(toXmlText(value.overwriteIfExists.value(), "OverwriteIfExists"));
    }
    str.append("</").append(tag).append(">");
    return str;
}

inline static std::string toXmlText(const models::JSONMetaRequest& value, const std::string& tag) {
    std::string str;
    str.append("<").append(tag).append(">");
    if (value.inputSerialization.has_value()) {
        str.append(toXmlText(value.inputSerialization.value(), "InputSerialization"));
    }
    if (value.overwriteIfExists.has_value()) {
        str.append(toXmlText(value.overwriteIfExists.value(), "OverwriteIfExists"));
    }
    str.append("</").append(tag).append(">");
    return str;
}


OperationInput fromSelectObject(const models::SelectObjectRequest& request) {
    auto input = OperationInput{"SelectObject", "POST"};

    input.headers.emplace("Content-Type", "application/xml");

    for (const auto& [k, v] : request.getHeaders()) {
        input.headers.insert_or_assign(k, v);
    }

    for (const auto& [k, v] : request.getParameters()) {
        input.parameters.insert_or_assign(k, v);
    }

    if (!request.getProcess().empty()) {
        input.parameters.insert_or_assign("x-oss-process", request.getProcess());
    }

    std::string md5 = "1B2M2Y8AsgTpgAmY7PhCfg==";
    if (request.hasSelectRequest()) {
        auto str = toXmlText(request.getSelectRequest(), "SelectRequest");
        md5 = utils::CalcContentMD5(str);
        input.body = RequestBody::fromString(std::move(str));
    }
    input.headers.emplace("Content-MD5", std::move(md5));

    input.bucket = request.getBucket();
    input.key = request.getKey();

    return input;
}

Outcome<models::SelectObjectResult, OperationError> toSelectObject(OperationOutput&& output) {
    return models::SelectObjectResult(output.statusCode, std::move(output.headers));
}


OperationInput fromCreateSelectObjectMeta(const models::CreateSelectObjectMetaRequest& request) {
    auto input = OperationInput{"CreateSelectObjectMeta", "POST"};

    input.headers.emplace("Content-Type", "application/xml");

    for (const auto& [k, v] : request.getHeaders()) {
        input.headers.insert_or_assign(k, v);
    }

    for (const auto& [k, v] : request.getParameters()) {
        input.parameters.insert_or_assign(k, v);
    }

    if (!request.getProcess().empty()) {
        input.parameters.insert_or_assign("x-oss-process", request.getProcess());
    }

    std::string md5 = "1B2M2Y8AsgTpgAmY7PhCfg==";
    if (request.hasSelectMetaRequest()) {
        const auto& metaReq = request.getSelectMetaRequest();
        std::string str;
        if (auto* csv = std::get_if<models::CSVMetaRequest>(&metaReq)) {
            str = toXmlText(*csv, "CsvMetaRequest");
        } else if (auto* json = std::get_if<models::JSONMetaRequest>(&metaReq)) {
            str = toXmlText(*json, "JsonMetaRequest");
        }
        md5 = utils::CalcContentMD5(str);
        input.body = RequestBody::fromString(std::move(str));
    }
    input.headers.emplace("Content-MD5", std::move(md5));

    input.bucket = request.getBucket();
    input.key = request.getKey();

    return input;
}

Outcome<models::CreateSelectObjectMetaResult, OperationError> toCreateSelectObjectMeta(OperationOutput&& output) {
    if (output.body != nullptr) {
        std::istreambuf_iterator<char> isb(*output.body), end;
        std::string raw(isb, end);

        internal::SelectMetaFrameParser parser;
        parser.write(reinterpret_cast<const uint8_t*>(raw.data()), raw.size());
        if (parser.fail()) {
            return makeUnexpected(
                OperationError(SerdeErrorCode::DeserializationFailed,
                               {{"Code", "SelectMetaFrameError"}, {"Message", parser.errorMessage()}}));
        }
        models::SelectObjectMeta meta;
        meta.offset = parser.offset();
        meta.totalScanned = parser.totalScanned();
        meta.status = parser.status();
        meta.splitsCount = parser.splitsCount();
        meta.rowsCount = parser.rowsCount();
        meta.columnsCount = parser.columnsCount();
        meta.errorMessage = parser.errorMessage();
        return models::CreateSelectObjectMetaResult(output.statusCode, std::move(output.headers), std::move(meta));
    }
    return models::CreateSelectObjectMetaResult(output.statusCode, std::move(output.headers));
}

} // namespace transform
} // namespace oss2
} // namespace alibabacloud
