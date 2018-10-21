#include <mutex>
#include <thread>
#include <list>
extern "C"
{
#include <node_api.h>
#include "./common.h"
}

#include "restclient-cpp/restclient.h"
#include "restclient-cpp/connection.h"

#define SERVER_URL "https://qqmusic-mobile-pro.rokid.com/rokid-cas-qqmusic/uniformEntry"

struct RestcurlData
{
  napi_ref cbRef;
  napi_async_work work;

  int timeout;

  RestClient::HeaderFields reqHeaders;
  std::string reqBody;
  size_t reqSize;

  int code;
  RestClient::HeaderFields resHeaders;
  std::string resBody;
};

static void doRequest(napi_env env, void *workData)
{
  auto data = (RestcurlData *)workData;
  RestClient::Connection conn("");
  conn.AppendHeader("Content-Type", "application/json");
  RestClient::Response r = conn.post(SERVER_URL, data->reqBody);
  data->code = r.code;
  data->resHeaders.swap(r.headers);
  data->resBody.swap(r.body);
}

static void onRequestFinish(napi_env env, napi_status status, void *workData)
{
  auto data = (RestcurlData *)workData;
  int code = data->code;
  std::string &body = data->resBody;
  napi_ref cbRef = data->cbRef;
  napi_handle_scope scope;
  NAPI_CALL_RETURN_VOID(env, napi_open_handle_scope(env, &scope));
  napi_value cb;
  NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, cbRef, &cb));
  napi_value resName;
  NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "restcurl", NAPI_AUTO_LENGTH, &resName));
  napi_async_context ctx;
  NAPI_CALL_RETURN_VOID(env, napi_async_init(env, cb, resName, &ctx));
  napi_value recv;
  NAPI_CALL_RETURN_VOID(env, napi_get_global(env, &recv));
  size_t argc = 2;
  napi_value argv[argc];
  NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, code, &argv[0]));
  NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, body.c_str(), body.size(), &argv[1]));
  NAPI_CALL_RETURN_VOID(env, napi_make_callback(env, ctx, recv, cb, argc, argv, nullptr));
  NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, cbRef));
  NAPI_CALL_RETURN_VOID(env, napi_async_destroy(env, ctx));
  NAPI_CALL_RETURN_VOID(env, napi_close_handle_scope(env, scope));
  NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, data->work));
  delete data;
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
  auto data = new RestcurlData;
  NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], NULL, 0, &data->reqSize));
  char *reqBody = (char *)malloc(data->reqSize + 1);
  NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], reqBody, data->reqSize + 1, &data->reqSize));
  // assign const char * to std::string
  data->reqBody.assign(reqBody, data->reqSize);
  free(reqBody);
  NAPI_CALL(env, napi_create_reference(env, cb, 1, &data->cbRef));
  napi_value resName;
  NAPI_CALL(env, napi_create_string_utf8(env, "restcurl", NAPI_AUTO_LENGTH, &resName));
  NAPI_CALL(env, napi_create_async_work(env, cb, resName, doRequest, onRequestFinish, data, &data->work));
  NAPI_CALL(env, napi_queue_async_work(env, data->work));
  return nullptr;
}

static napi_value Init(napi_env env, napi_value exports)
{
  napi_property_descriptor desc[] = {
      DECLARE_NAPI_PROPERTY("request", request),
  };
  size_t property_count = sizeof(desc) / sizeof(*desc);
  NAPI_CALL(env, napi_define_properties(env, exports, property_count, desc));

  return exports;
}

NAPI_MODULE(restcurl, Init)
