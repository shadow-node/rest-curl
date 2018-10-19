#include "restclient-cpp/restclient.h"
#include "restclient-cpp/connection.h"
#include <iostream>
#include <time.h>
#include <uv.h>
#include <list>
#include <thread>
#include <mutex>

#define SERVER_URL "https://qqmusic-mobile-pro.rokid.com/rokid-cas-qqmusic/uniformEntry"

struct RestCurlResponseData
{
  int code;
  std::string body;
  RestClient::HeaderFields headers;
};

static std::mutex mutex;
static uv_async_t async;
static uv_timer_t timer;
static std::list<RestCurlResponseData *> responseCbs;

static RestCurlResponseData *doRequest(char *body, size_t bodySize, void *env, void *ref)
{
  RestCurlResponseData *response = new RestCurlResponseData();
  RestClient::Response r = RestClient::post(SERVER_URL, "application/json", body);
  response->code = r.code;
  response->headers.swap(r.headers);
  response->body.swap(r.body);
  free(body);
  return response;
}

static void doRequestAsync(char *body, size_t bodySize, void *env, void *ref)
{
  RestCurlResponseData *response = doRequest(body, bodySize, env, ref);
  mutex.lock();
  responseCbs.push_back(response);
  mutex.unlock();
  uv_async_send(&async);
}

static void onAsyncRequestFinish(uv_async_t *handle)
{
  std::list<RestCurlResponseData *> cbs;
  mutex.lock();
  cbs.swap(responseCbs);
  mutex.unlock();

  int code;
  std::string *body;
  size_t argc = 2;
  for (auto ite = cbs.begin(); ite != cbs.end(); ++ite)
  {
    code = (*ite)->code;
    body = &(*ite)->body;
    std::cout << "code: " << code << std::endl;
    std::cout << "body: " << *body << std::endl;
    delete (*ite);
  }
}

void onTimer(uv_timer_t *handle)
{
  size_t bodySize = 2;
  char *body = (char *)malloc(bodySize + 1);
  memcpy(body, "{}", bodySize);
  body[bodySize] = '\0';
  std::thread th(doRequestAsync, body, bodySize, nullptr, nullptr);
  th.detach();
}

int syncMain()
{
  while (true)
  {
    size_t bodySize = 2;
    char *body = (char *)malloc(bodySize + 1);
    memcpy(body, "{}", bodySize);
    body[bodySize] = '\0';
    RestCurlResponseData *response = doRequest(body, bodySize, nullptr, nullptr);
    std::cout << "code: " << response->code << std::endl;
    std::cout << "body: " << response->body << std::endl;
    delete response;
  }
  return 0;
}

int asyncMain()
{
  uv_async_init(uv_default_loop(), &async, onAsyncRequestFinish);
  uv_timer_init(uv_default_loop(), &timer);
  uv_timer_start(&timer, onTimer, 0, 200);
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  return 0;
}

int main(int argc, char *argv[])
{
  if (argc == 2 && strcmp(argv[1], "async") == 0)
  {
    std::cout << "request with async" << std::endl;
    return asyncMain();
  }
  else
  {
    std::cout << "request with sync" << std::endl;
    return syncMain();
  }
}