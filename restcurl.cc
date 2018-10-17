#include <mutex>
#include <thread>
#include <list>
extern "C"
{
  #include <uv.h>
  #include <node_api.h>
  #include "./common.h"
}

#include "restclient-cpp/restclient.h"
#include "restclient-cpp/connection.h"

#define SERVER_URL "https://qqmusic-mobile-pro.rokid.com/rokid-cas-qqmusic/uniformEntry"

struct RestCurlResponseData
{
  napi_env env;
  napi_ref cbRef;
  int code;
  std::string body;
  RestClient::HeaderFields headers;
};

static std::mutex mutex;
static uv_async_t async;
static std::list<RestCurlResponseData *> responseCbs;

static void doRequest(const char *body, size_t bodySize, napi_env env, napi_ref cbRef)
{
  RestCurlResponseData *response = new RestCurlResponseData();
  RestClient::Response r = RestClient::post(SERVER_URL, "application/json", body);
  response->env = env;
  response->cbRef = cbRef;
  response->code = r.code;
  response->headers.swap(r.headers);
  response->body.swap(r.body);
  mutex.lock();
  responseCbs.push_back(response);
  mutex.unlock();
  uv_async_send(&async);
  delete body;
}

static void onRequestFinish(uv_async_t *handle)
{
  std::list<RestCurlResponseData *> cbs;
  mutex.lock();
  cbs.swap(responseCbs);
  mutex.unlock();

  int code;
  std::string *body;
  napi_ref cbRef;
  size_t argc = 2;
  napi_env env;
  napi_value cb;
  napi_value recv;
  napi_value resName;
  napi_async_context ctx;
  napi_handle_scope scope;
  napi_value argv[argc];
  for (auto ite = cbs.begin(); ite != cbs.end(); ++ite)
  {
    code = (*ite)->code;
    body = &(*ite)->body;
    env = (*ite)->env;
    cbRef = (*ite)->cbRef;
    NAPI_CALL_RETURN_VOID(env, napi_open_handle_scope(env, &scope));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, cbRef, &cb));
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "restcurl", NAPI_AUTO_LENGTH, &resName));
    NAPI_CALL_RETURN_VOID(env, napi_async_init(env, cb, resName, &ctx));
    NAPI_CALL_RETURN_VOID(env, napi_get_global(env, &recv));
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, code, &argv[0]));
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, body->c_str(), body->size(), &argv[1]));
    NAPI_CALL_RETURN_VOID(env, napi_make_callback(env, ctx, recv, cb, argc, argv, nullptr));
    NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, cbRef));
    NAPI_CALL_RETURN_VOID(env, napi_async_destroy(env, ctx));
    NAPI_CALL_RETURN_VOID(env, napi_close_handle_scope(env, scope));
    delete (*ite);
  }
}

static napi_value request(napi_env env, napi_callback_info info)
{
  size_t argc = 2;
  napi_value argv[argc];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
  if (argc != 2)
  {
    NAPI_CALL(env, napi_throw_error(env, nullptr, "Wrong arguments number"));
    return nullptr;
  }
  napi_valuetype type;
  napi_value cb = argv[1];
  NAPI_CALL(env, napi_typeof(env, cb, &type));
  if (type != napi_function)
  {
    NAPI_CALL(env, napi_throw_error(env, nullptr, "Argument 2 expected a function"));
    return nullptr;
  }
  size_t bodySize;
  NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], NULL, 0, &bodySize));
  char *body = (char *)malloc(bodySize + 1);
  NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], body, bodySize, &bodySize));
  body[bodySize] = '\0';
  napi_ref cbRef;
  NAPI_CALL(env, napi_create_reference(env, cb, 1, &cbRef));
  std::thread th(doRequest, body, bodySize, env, cbRef);
  th.detach();
  return nullptr;
}

static napi_value Init(napi_env env, napi_value exports)
{
  uv_loop_s* loop;
  NAPI_CALL(env, napi_get_uv_event_loop(env, &loop));
  uv_async_init(loop, &async, onRequestFinish);
  napi_property_descriptor desc[] = {
    DECLARE_NAPI_PROPERTY("request", request),
  };
  size_t property_count = sizeof(desc) / sizeof(*desc);
  NAPI_CALL(env, napi_define_properties(env, exports, property_count, desc));

  return exports;
}

NAPI_MODULE(restcurl, Init)
