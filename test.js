var restcurl = require('./build/Release/restcurl.node')

var data = '{"locale":"zh-cn","domain":"com.rokid.cas.qqmusic","appVersion":"1.0.0","signMethod":"MD5","appId":"RBA66C902A6347DD86CA8D419B0BB974","masterId":"0504031838000016","deviceId":"0504031838000016","deviceType":"0ABA0AA4F71949C4A3FB0418BF025113","businessParams":{},"requestTimestamp":1539426109,"intent":"play_random","nonce":"0504031838000016153942610925","requestId":1539426109,"sessionId":1539426109,"sign":"DD0D8B21DCDAEE6076D338E59908BFF8"}'
setInterval(() => {
  restcurl.request(data, function () {
    console.log(JSON.stringify(arguments))
  })
}, 1000)
