#include "restclient-cpp/restclient.h"
#include "restclient-cpp/connection.h"
#include <iostream>
#include <time.h>

double jerry_port_get_current_time(void)
{
  struct timeval tv;
  if (gettimeofday(&tv, NULL) != 0)
  {
    return 0.0;
  }
  return ((double)tv.tv_sec) * 1000.0 + ((double)tv.tv_usec) / 1000.0;
}

int main()
{
  while (true)
  {
    double start = jerry_port_get_current_time();
    RestClient::Response r = RestClient::get("https://www.baidu.com");
    double end = jerry_port_get_current_time();
    std::cout << "code:" << r.code << ", time:" << end - start << "ms" << std::endl;
  }
  return 0;
}