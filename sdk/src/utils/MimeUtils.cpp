
#include "Utils.h"
#include <map>
#include <string>

namespace alibabacloud {
namespace oss2 {
namespace utils {


const std::string& LookupMimeType(const std::string& name) {
    const static std::map<std::string, std::string> mimeType = {{"html", "text/html"},
                                                                {"htm", "text/html"},
                                                                {"shtml", "text/html"},
                                                                {"css", "text/css"},
                                                                {"xml", "text/xml"},
                                                                {"gif", "image/gif"},
                                                                {"jpeg", "image/jpeg"},
                                                                {"jpg", "image/jpeg"},
                                                                {"js", "application/x-javascript"},
                                                                {"atom", "application/atom+xml"},
                                                                {"rss", "application/rss+xml"},
                                                                {"mml", "text/mathml"},
                                                                {"txt", "text/plain"},
                                                                {"jad", "text/vnd.sun.j2me.app-descriptor"},
                                                                {"wml", "text/vnd.wap.wml"},
                                                                {"htc", "text/x-component"},
                                                                {"png", "image/png"},
                                                                {"tif", "image/tiff"},
                                                                {"tiff", "image/tiff"},
                                                                {"wbmp", "image/vnd.wap.wbmp"},
                                                                {"ico", "image/x-icon"},
                                                                {"jng", "image/x-jng"},
                                                                {"bmp", "image/x-ms-bmp"},
                                                                {"svg", "image/svg+xml"},
                                                                {"svgz", "image/svg+xml"},
                                                                {"webp", "image/webp"},
                                                                {"jar", "application/java-archive"},
                                                                {"war", "application/java-archive"},
                                                                {"ear", "application/java-archive"},
                                                                {"hqx", "application/mac-binhex40"},
                                                                {"doc ", "application/msword"},
                                                                {"pdf", "application/pdf"},
                                                                {"ps", "application/postscript"},
                                                                {"eps", "application/postscript"},
                                                                {"ai", "application/postscript"},
                                                                {"rtf", "application/rtf"},
                                                                {"xls", "application/vnd.ms-excel"},
                                                                {"ppt", "application/vnd.ms-powerpoint"},
                                                                {"wmlc", "application/vnd.wap.wmlc"},
                                                                {"kml", "application/vnd.google-earth.kml+xml"},
                                                                {"kmz", "application/vnd.google-earth.kmz"},
                                                                {"7z", "application/x-7z-compressed"},
                                                                {"cco", "application/x-cocoa"},
                                                                {"jardiff", "application/x-java-archive-diff"},
                                                                {"jnlp", "application/x-java-jnlp-file"},
                                                                {"run", "application/x-makeself"},
                                                                {"pl", "application/x-perl"},
                                                                {"pm", "application/x-perl"},
                                                                {"prc", "application/x-pilot"},
                                                                {"pdb", "application/x-pilot"},
                                                                {"rar", "application/x-rar-compressed"},
                                                                {"rpm", "application/x-redhat-package-manager"},
                                                                {"sea", "application/x-sea"},
                                                                {"swf", "application/x-shockwave-flash"},
                                                                {"sit", "application/x-stuffit"},
                                                                {"tcl", "application/x-tcl"},
                                                                {"tk", "application/x-tcl"},
                                                                {"der", "application/x-x509-ca-cert"},
                                                                {"pem", "application/x-x509-ca-cert"},
                                                                {"crt", "application/x-x509-ca-cert"},
                                                                {"xpi", "application/x-xpinstall"},
                                                                {"xhtml", "application/xhtml+xml"},
                                                                {"zip", "application/zip"},
                                                                {"wgz", "application/x-nokia-widget"},
                                                                {"bin", "application/octet-stream"},
                                                                {"exe", "application/octet-stream"},
                                                                {"dll", "application/octet-stream"},
                                                                {"deb", "application/octet-stream"},
                                                                {"dmg", "application/octet-stream"},
                                                                {"eot", "application/octet-stream"},
                                                                {"iso", "application/octet-stream"},
                                                                {"img", "application/octet-stream"},
                                                                {"msi", "application/octet-stream"},
                                                                {"msp", "application/octet-stream"},
                                                                {"msm", "application/octet-stream"},
                                                                {"mid", "audio/midi"},
                                                                {"midi", "audio/midi"},
                                                                {"kar", "audio/midi"},
                                                                {"mp3", "audio/mpeg"},
                                                                {"ogg", "audio/ogg"},
                                                                {"m4a", "audio/x-m4a"},
                                                                {"ra", "audio/x-realaudio"},
                                                                {"3gpp", "video/3gpp"},
                                                                {"3gp", "video/3gpp"},
                                                                {"mp4", "video/mp4"},
                                                                {"mpeg", "video/mpeg"},
                                                                {"mpg", "video/mpeg"},
                                                                {"mov", "video/quicktime"},
                                                                {"webm", "video/webm"},
                                                                {"flv", "video/x-flv"},
                                                                {"m4v", "video/x-m4v"},
                                                                {"mng", "video/x-mng"},
                                                                {"asx", "video/x-ms-asf"},
                                                                {"asf", "video/x-ms-asf"},
                                                                {"wmv", "video/x-ms-wmv"},
                                                                {"avi", "video/x-msvideo"},
                                                                {"ts", "video/MP2T"},
                                                                {"m3u8", "application/x-mpegURL"},
                                                                {"apk", "application/vnd.android.package-archive"}};

    const static std::string defaultType("application/octet-stream");
    std::string::size_type last_pos = name.find_last_of('.');
    std::string::size_type first_pos = name.find_first_of('.');
    std::string prefix, ext, ext2;

    if (last_pos == std::string::npos) {
        return defaultType;
    }

    // extract the last extension
    if (last_pos != std::string::npos) {
        ext = name.substr(1 + last_pos, std::string::npos);
    }
    if (last_pos != std::string::npos) {
        if (first_pos != std::string::npos && first_pos < last_pos) {
            prefix = name.substr(0, last_pos);
            // Now get the second to last file extension
            std::string::size_type next_pos = prefix.find_last_of('.');
            if (next_pos != std::string::npos) {
                ext2 = prefix.substr(1 + next_pos, std::string::npos);
            }
        }
    }

    ext = ToLower(ext.c_str());
    auto iter = mimeType.find(ext);
    if (iter != mimeType.end()) {
        return (*iter).second;
    }

    if (first_pos == last_pos) {
        return defaultType;
    }

    ext2 = ToLower(ext2.c_str());
    iter = mimeType.find(ext2);
    if (iter != mimeType.end()) {
        return (*iter).second;
    }

    return defaultType;
}


} // namespace utils
} // namespace oss2
} // namespace alibabacloud