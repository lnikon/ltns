#pragma once

#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>

#include <curl/curl.h>
#include <curl/easy.h>

namespace Networking::Utilities {

struct FileDownloadOptions {
	 std::string url;
   std::string destinationPath;
   std::string certificatePath;
};

auto writeData(void *ptr, std::size_t size, std::size_t nmemb, void *stream) -> size_t;

void DownloadFile(const FileDownloadOptions& options);

} // namespace Networking::Utilities
