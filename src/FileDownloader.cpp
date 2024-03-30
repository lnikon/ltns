#include "Utilities/FileDownloader.h"

namespace Networking::Utilities {

auto writeData(void *ptr, std::size_t size, std::size_t nmemb, void *stream) -> size_t
{
  std::size_t written{fwrite(ptr, size, nmemb, (FILE *)stream)};
  return written;
}

void DownloadFile(const FileDownloadOptions& options)
{
  curl_global_init(CURL_GLOBAL_ALL);

  CURL *curlHandle{curl_easy_init()};
  curl_easy_setopt(curlHandle, CURLOPT_URL, options.url.data());
  curl_easy_setopt(curlHandle, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(curlHandle, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, writeData);

  FILE *pagefile{fopen(options.destinationPath.data(), "wb")};
  if (pagefile != nullptr) {
    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, pagefile);
    curl_easy_perform(curlHandle);
    fclose(pagefile);
  }

  curl_easy_cleanup(curlHandle);
  curl_global_cleanup();
}

} // namespace Networking::Utilities
