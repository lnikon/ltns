#include "Utilities/FileDownloader.h"

namespace Networking::Utilities {

size_t writeData(void *ptr, std::size_t size, std::size_t nmemb, void *stream)
{
  std::size_t written{fwrite(ptr, size, nmemb, (FILE *)stream)};
  return written;
}

void DownloadFile(
  const std::string &url,
  const std::string &destinationPath,
  const std::string &certificatePath)
{

  curl_global_init(CURL_GLOBAL_ALL);

  CURL *curlHandle{curl_easy_init()};
  curl_easy_setopt(curlHandle, CURLOPT_URL, url.data());
  curl_easy_setopt(curlHandle, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(curlHandle, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, writeData);

  FILE *pagefile{fopen(destinationPath.data(), "wb")};
  if (pagefile) {
    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, pagefile);
    curl_easy_perform(curlHandle);
    fclose(pagefile);
  }

  curl_easy_cleanup(curlHandle);
  curl_global_cleanup();
}

} // namespace Networking::Utilities
