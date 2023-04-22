#include <hex/helpers/http_requests.hpp>

namespace hex {

    std::string HttpRequest::s_proxyUrl;

    void HttpRequest::setDefaultConfig() {
        curl_easy_setopt(this->m_curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
        curl_easy_setopt(this->m_curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
        curl_easy_setopt(this->m_curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(this->m_curl, CURLOPT_USERAGENT, "ImHex/1.0");
        curl_easy_setopt(this->m_curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        curl_easy_setopt(this->m_curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(this->m_curl, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(this->m_curl, CURLOPT_TIMEOUT_MS, 0L);
        curl_easy_setopt(this->m_curl, CURLOPT_CONNECTTIMEOUT_MS, this->m_timeout);
        curl_easy_setopt(this->m_curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(this->m_curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(this->m_curl, CURLOPT_XFERINFODATA, this);
        curl_easy_setopt(this->m_curl, CURLOPT_XFERINFOFUNCTION, progressCallback);
        curl_easy_setopt(this->m_curl, CURLOPT_PROXY, s_proxyUrl.c_str());
    }

    size_t HttpRequest::writeToVector(void *contents, size_t size, size_t nmemb, void *userdata) {
        auto &response = *reinterpret_cast<std::vector<u8>*>(userdata);
        auto startSize = response.size();

        response.resize(startSize + size * nmemb);
        std::memcpy(response.data() + startSize, contents, size * nmemb);

        return size * nmemb;
    }

    size_t HttpRequest::writeToFile(void *contents, size_t size, size_t nmemb, void *userdata) {
        auto &file = *reinterpret_cast<wolv::io::File*>(userdata);

        file.writeBuffer(reinterpret_cast<const u8*>(contents), size * nmemb);

        return size * nmemb;
    }

    int HttpRequest::progressCallback(void *contents, curl_off_t dlTotal, curl_off_t dlNow, curl_off_t ulTotal, curl_off_t ulNow) {
        auto &request = *static_cast<HttpRequest *>(contents);

        if (dlTotal > 0)
            request.m_progress = float(dlNow) / dlTotal;
        else if (ulTotal > 0)
            request.m_progress = float(ulNow) / ulTotal;
        else
            request.m_progress = 0.0F;

        return request.m_canceled ? CURLE_ABORTED_BY_CALLBACK : CURLE_OK;
    }

}