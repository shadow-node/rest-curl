{
  "targets": [
    {
      "target_name": "restcurl",
      "sources": [ "restcurl.c", "deps/restclient-cpp/connection.cc", "deps/restclient-cpp/helpers.cc", "deps/restclient-cpp/restclient.cc" ],
      "cflags_cc!": [
        "-std=c++11",
        "-stdlib=libc++",
        "-fexceptions"
      ],
      "include_dirs": [
        "deps"
      ],
      "link_settings": {
        "libraries": [
          "-lcurl",
          "-ldl"
        ]
      },
      'conditions': [
        ['OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
          }
        }]
      ]
    }
  ]
}
