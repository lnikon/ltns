#pragma once

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#include <curl/curl.h>
#include <curl/easy.h>

namespace Networking::Utilities {

size_t writeData(void *ptr, std::size_t size, std::size_t nmemb, void *stream);

void DownloadFile(
  const std::string &url,
  const std::string &destinationPath,
  const std::string &certificatePath = "");

} // namespace Networking::Utilities
