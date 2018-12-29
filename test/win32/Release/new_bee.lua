--SOHU MEDIA SDK LUA SCRIPT--
--Copyright@sohu-inc

local cjson       = require "cjson"
local http        = require "http"
local ws          = require "lws"
local webrtc      = require "webrtc"
local timer       = require "timer"

--Encrypt/Decrypt
function BeeEncrypt(input,key)
  local xe_args = sdk_xxteaEncrypt(key, input)
  local be_args = sdk_Base64Encode(xe_args) 
  return be_args
end

function BeeDecrypt(input, key)
  local bd_args = sdk_Base64Decode(input)
  local xd_args = sdk_xxteaDecrypt(key, bd_args)
  return xd_args
end

--Class object callback wrapper
function handler(obj, method)
  return function(...)
    return method(obj,...)
  end
end

--Split string
function string:split(sep)  
  local sep, fields = sep or "\t", {}  
  local pattern = string.format("([^%s]+)", sep)  
  self:gsub(pattern, function(c) fields[#fields+1] = c end)
  return fields  
end  

--Http Wrapper

--Http callback type
local eHttpCallback_Resolve  = 0
local eHttpCallback_Connect  = 1
local eHttpCallback_Request  = 2
local eHttpCallback_Header   = 3
local eHttpCallback_Redirect = 4
local eHttpCallback_Body     = 5

Http = {
  handle = nil
}

Http.__index = Http
    
function Http:open()
  local o = {}
  setmetatable(o, self)
  local r1, r2 = pcall(http.open)
  if (r1 == false) then
    LogError("Http open fail, error \"%s\".", r2)
    return nil
  else
    o.handle = r2
    return o
  end
end

function Http:close()
  if (self.handle ~= nil) then
    local r1, r2 = pcall(http.close, self.handle)
    if (r1 == false) then
      LogError("Http close fail, error \"%s\".", r2)
    end
    self.handle = nil
  end
end

function Http:set_resolve_callback(cb)
  if (self.handle ~= nil) then
    local r1, r2 = pcall(http.set_callback, self.handle, eHttpCallback_Resolve, cb)
    if (r1 == false) then
      LogError("Http set_resolve_callback fail, error \"%s\".", r2)
    end
  end
end

function Http:set_connect_callback(cb)
  if (self.handle ~= nil) then
    local r1, r2 = pcall(http.set_callback, self.handle, eHttpCallback_Connect, cb)
    if (r1 == false) then
      LogError("Http set_connect_callback fail, error \"%s\".", r2)
    end
  end
end

function Http:set_request_callback(cb)
  if (self.handle ~= nil) then
    local r1, r2 = pcall(http.set_callback, self.handle, eHttpCallback_Request, cb)
    if (r1 == false) then
      LogError("Http set_request_callback fail, error \"%s\".", r2)
    end
  end
end

function Http:set_header_callback(cb)
  if (self.handle ~= nil) then
    local r1, r2 = pcall(http.set_callback, self.handle, eHttpCallback_Header, cb)
    if (r1 == false) then
      LogError("Http set_header_callback fail, error \"%s\".", r2)
    end
  end
end

function Http:set_redirect_callback(cb)
  if (self.handle ~= nil) then
    local r1, r2 = pcall(http.set_callback, self.handle, eHttpCallback_Redirect, cb)
    if (r1 == false) then
      LogError("Http set_redirect_callback fail, error \"%s\".", r2)
    end
  end
end

function Http:set_body_callback(cb)
  if (self.handle ~= nil) then
    local r1, r2 = pcall(http.set_callback, self.handle, eHttpCallback_Body, cb)
    if (r1 == false) then
      LogError("Http set_body_callback fail, error \"%s\".", r2)
    end
  end
end

function Http:get(url, option)
  if (self.handle ~= nil) then
    local r1, r2, r3, r4, r5 = pcall(http.get, self.handle, url, option)
    if (r1 == false) then
      LogError("Http get fail, error \"%s\".", r2)
    else
      return r2, r3, r4, r5
    end
  end
end

function Http:async_get(url, option, cb)
  local ret = false
  if (self.handle ~= nil) then
    local r1, r2 = pcall(http.async_get, self.handle, url, option, cb)
    if (r1 == false) then
      LogError("Http async_get fail, error \"%s\".", r2)
    end
    ret = r1
  end
  return ret
end

function Http:scanf(format)
  if (self.handle ~= nil) then
    return http.scanf(self.handle, format)
  end
end

function Http:read(buffer, size, condition)
  if (self.handle ~= nil) then
    return http.read(self.handle, buffer, size, condition)
  end
end 

--Websocket Wrapper
--Websocket callback type
local eWsCallback_Error = 0
local eWsCallback_Read = 1
Websocket = {
  handle = nil
}

Websocket.__index = Websocket

function Websocket:start(protocols)
  local r1, r2 = pcall(ws.start, protocols)
  if (r1 == false) then
    LogError("Websocket service start fail, error \"%s\".", r2)
  end
  return r1
end

function Websocket:open(protocol, check_json_completed, read_cb, error_cb)
  local o = {}
  setmetatable(o, self)
  local r1, r2 = pcall(ws.open, protocol, check_json_completed, read_cb, error_cb)
  if (r1 == false) then
    LogError("Websocket open fail, error \"%s\".", r2)
    return nil
  else
    o.handle = r2
    return o
  end
end

function Websocket:close()
  if (self.handle ~= nil) then
    local r1, r2 = pcall(ws.close, self.handle)
    if (r1 == false) then
      LogError("Websocket close fail, error \"%s\".", r2)   
    end
    self.handle = nil
  end
end

function Websocket:connect(url, timeout, connect_cb)
  local ret = false
  if (self.handle ~= nil) then
    local r1, r2 = pcall(ws.connect, self.handle, url, timeout, connect_cb)
    if (r1 == false) then
      LogError("Websocket connect fail, error \"%s\".", r2)
    end
    ret = r1
  end
  return ret
end

function Websocket:write(data)
  local ret = false
  if (self.handle ~= nil) then
    LogInfo("Send WSMSG: %s",data)
    local r1, r2 = pcall(ws.write, self.handle, data)
    if (r1 == false) then
      LogError("Websocket write fail, error \"%s\".", r2)
    end
    ret = r1
  end
  return ret
end

--Webrtc Wrapper
Webrtc = {
  handle = nil
}

Webrtc.__index = Webrtc

function Webrtc:open(p2p_svr, callbacks)
  local o = {}
  setmetatable(o, self)
  local r1, r2 = pcall(webrtc.open, p2p_svr, callbacks)
  if (r1 == false) then
    LogError("Webrtc open fail, error \"%s\".", r2)
    return nil
  else
    o.handle = r2
    return o
  end
end

function Webrtc:close()
  if (self.handle ~= nil) then
    local r1, r2 = pcall(webrtc.close, self.handle)
    if (r1 == false) then
      LogError("Webrtc close fail, error \"%s\".", r2)   
    end
    self.handle = nil
  end
end

function Webrtc:set_video_source(internal, width, height, fps, capturer_index, is_screencast)
  local ret = false
  if (self.handle ~= nil) then
    local r1, r2 = pcall(webrtc.set_video_source, self.handle, internal, width, height, fps, capturer_index, is_screencast)
    if (r1 == false) then
      LogError("Webrtc set_video_source fail, error \"%s\".", r2)
    end
    ret = r1
  end
  return ret
end

function Webrtc:set_audio_source(no_audio_processing, enable_level_control)
  local ret = false
  if (self.handle ~= nil) then
    local r1, r2 = pcall(webrtc.set_audio_source, self.handle, no_audio_processing, enable_level_control)
    if (r1 == false) then
      LogError("Webrtc set_audio_source fail, error \"%s\".", r2)
    end
    ret = r1
  end
  return ret
end

function Webrtc:create_offer(audio_source, video_source, callback)
  local ret = false
  if (self.handle ~= nil) then
    local r1, r2 = pcall(webrtc.create_offer, self.handle, audio_source, video_source, callback)
    if (r1 == false) then
      LogError("Webrtc create_offer fail, error \"%s\".", r2)
    end
    ret = r1
  end
  return ret
end

function Webrtc:set_remote_desc(jsep)
  local ret = false
  if (self.handle ~= nil) then
    local r1, r2 = pcall(webrtc.set_remote_desc, self.handle, jsep)
    if (r1 == false) then
      LogError("Webrtc set_remote_desc fail, error \"%s\".", r2)
    end
    ret = r1
  end
  return ret   
end

function Webrtc:create_answer(offer, callback)
  local ret = false
  if (self.handle ~= nil) then
    local r1, r2 = pcall(webrtc.create_answer, self.handle, offer, callback)
    if (r1 == false) then
      LogError("Webrtc create_answer fail, error \"%s\".", r2)
    end
    ret = r1
  end
  return ret
end

function Webrtc:start_video_renderer(renderer)
  local ret = false
  if (self.handle ~= nil) then
    local r1, r2 = pcall(webrtc.start_video_renderer, self.handle, renderer)
    if (r1 == false) then
      LogError("Webrtc start_video_renderer fail, error \"%s\".", r2)
    end
    ret = r1
  end
  return ret
end

function Webrtc:stop_video_renderer()
  local ret = false
  if (self.handle ~= nil) then
    local r1, r2 = pcall(webrtc.stop_video_renderer, self.handle)
    if (r1 == false) then
      LogError("Webrtc stop_video_renderer fail, error \"%s\".", r2)
    end
    ret = r1
  end
  return ret
end

function Webrtc:get_stats(obsvr)
  local ret = false
  if (self.handle ~= nil) then
    local r1, r2 = pcall(webrtc.get_stats, self.handle, obsvr)
    if (r1 == false) then
      LogError("Webrtc get_stats fail, error \"%s\".", r2)
    end
    ret = r1
  end
  return ret
end

function Webrtc:enable_tracing(level)
  local r1, r2 = pcall(webrtc.enable_tracing, level)
  if (r1 == false) then
    LogError("Webrtc enable_tracing fail, error \"%s\".", r2)
  end
end

function Webrtc:get_audio_input_level()
  local audio_level = -1
  if (self.handle ~= nil) then
    local r1, r2 = pcall(webrtc.get_audio_input_level, self.handle)
    if (r1 == false) then
      LogError("Webrtc get_audio_input_level fail, error \"%s\".", r2)
    end
    audio_level = r2
  end
  return audio_level
end

function Webrtc:get_audio_output_level()
  local audio_level = -1
  if (self.handle ~= nil) then
    local r1, r2 = pcall(webrtc.get_audio_output_level, self.handle)
    if (r1 == false) then
      LogError("Webrtc get_audio_output_level fail, error \"%s\".", r2)
    end
    audio_level = r2
  end
  return audio_level
end

--Timer Wrapper
Timer = {
  handle = nil
}

Timer.__index = Timer

function Timer:open(interval, timeout_count, cb)
  local o = {}
  setmetatable(o, self)
  local r1, r2 = pcall(timer.open, interval, timeout_count, cb)
  if (r1 == false) then
    LogError("Timer open fail, error \"%s\".", r2)
    return nil
  else
    o.handle = r2
    return o
  end
end

function Timer:close()
  if (self.handle ~= nil) then
    local r1, r2 = pcall(timer.close, self.handle)
    if (r1 == false) then
      LogError("Timer close fail, error \"%s\".", r2)
    end
    self.handle = nil
  end
end

--Platform Type
local ePlatformType_None = 0
local ePlatformType_PC = 1
local ePlatformType_Mac = 2
local ePlatformType_IPhone = 3
local ePlatformType_IPad = 4
local ePlatformType_Android_Phone = 5
local ePlatformType_Android_Pad = 6
local ePlatformType_Android_TV = 7
local ePlatformType_Android_Router = 8
local ePlatformType_Android_Box = 9

local g_platform_str = {"PC", "Mac", "IPhone", "IPad", "Android_Phone", "Android_Pad", "Android_TV", "Android_Router", "Android_Box"}

--Net type
local eNetType_WireLine = 1
local eNetType_Wifi = 2
local eNetType_2G = 3
local eNetType_3G = 4
local eNetType_4G = 5

--Encrypt Algorithm
local eALGO_AES = 0
local eALGO_RC4 = 1
local eALGO_XXTEA = 2

--Live Mode
local eLiveMode_Delay = 0
local eLiveMode_Realtime_Fp = 1
local eLiveMode_Realtime = 2
local eLiveMode_Realtime_Cp = 3

--Read Mode
local eReadSome = 0
local eReadExactly = 1
local eReadEnough = 2

--Serialize Typd Id
local _int8_t = 0
local _uint8_t = 1
local _int16_t = 2
local _uint16_t = 3
local _int32_t = 4
local _uint32_t = 5
local _int64_t = 6
local _uint64_t = 7
local _double = 8
local _byte_array = 9
local _string = 10

--Media Type
local eMediaType_None = 0
local eMediaType_Audio = 1
local eMediaType_Video = 2
local eMediaType_Audio_Video = 3

--Protocol Filed Desc
Field = {
  typeid = _int8_t,
  length = 1,
  hton = false
}

Field.__index = Field

function Field:new(typeid, length, hton)
  local o = {}
  setmetatable(o, self)
  o.typeid = typeid
  o.length = length
  o.hton = hton
  return o
end

--Global sys info
local g_sys_info = {
  platform = ePlatformType_IOS,
  net_type = eNetType_WireLine,
  app_name = "test",
  app_ver = "1.0",
  bee_ver = "1.0",
  sys_info = "Win10 64bit",
  device_id = "E839354CAFED",
  param_key = "d^#Gj9&$Eai@1u*D",
  session_id = ""
}

--Log level
local kLogLevel_Fatal = 0
local kLogLevel_Error = 1
local kLogLevel_Warn = 2
local kLogLevel_Trace = 3
local kLogLevel_Info = 4
local kLogLevel_Debug = 5

function BeeLog(level,fmt, ...)
  local debug_info = debug.getinfo(3)
  sdk_BeeLog(level, debug_info.source, debug_info.currentline, fmt:format(...))
end

function LogFatal(fmt, ...)
  BeeLog(kLogLevel_Fatal, fmt, ...)
end

function LogError(fmt, ...)
  BeeLog(kLogLevel_Error, fmt, ...)
end

function LogWarn(fmt, ...)
  BeeLog(kLogLevel_Warn, fmt, ...)
end

function LogTrace(fmt, ...)
  BeeLog(kLogLevel_Trace, fmt, ...)
end

function LogInfo(fmt, ...)
  BeeLog(kLogLevel_Info, fmt, ...)
end

function LogDebug(fmt, ...)
  BeeLog(kLogLevel_Debug, fmt, ...)
end

function EncodeParams(input,key)
  local xe_args = sdk_xxteaEncrypt(key, input)
  local be_args = sdk_Base64Encode(xe_args) 
  return be_args
end

function DecodeParams(input, key)
  local bd_args = sdk_Base64Decode(input)
  local xd_args = sdk_xxteaDecrypt(key, bd_args)
  return xd_args
end

function SetSysInfo(pt,nt,app_name,app_ver,bee_ver,sys_info,device_id,session_id)
  g_sys_info.platform = pt
  g_sys_info.net_type = nt
  g_sys_info.app_name = app_name
  g_sys_info.app_ver = app_ver
  g_sys_info.bee_ver = bee_ver
  g_sys_info.sys_info = sys_info
  g_sys_info.device_id = device_id
  g_sys_info.session_id = session_id
  LogInfo("System info pt:%d, nt:%d, app_name:%s app_ver:%s bee_ver:%s sys_info:%s device_id:%s session_id:%s", pt, nt, app_name, app_ver, bee_ver, sys_info, device_id, session_id)
end

function random_string(len)
  local char_set = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
  local str = ""
  local count = 0
  while (count < len) do
    local i = math.random(1,62)
    local c = string.sub(char_set, i, i)
    str=str..c
    count=count+1
  end
  return str
end

--Transaction
Transaction = {
  id = "",
  acked = false,
  has_rsp = true,
  cb = nil
}

Transaction.__index = Transaction

function Transaction:new(id, has_rsp, cb)
  local o = {}
  setmetatable(o, self)
  o.id = id
  o.has_rsp = has_rsp
  o.cb = cb
  return o
end

function Transaction:process(success, json_data)
  if (self.cb ~= nil) then
    self.cb(success, json_data)
  end
end

local eIceGathering_New = 0
local eIceGathering_Gathering = 1
local eIceGathering_Completed = 2

local kIceConnectionNew = 0
local kIceConnectionChecking = 1
local kIceConnectionConnected = 2
local kIceConnectionCompleted = 3
local kIceConnectionFailed = 4
local kIceConnectionDisconnected = 5
local kIceConnectionClosed = 6

local eVideoConfNotify_Local_Join = 0
local eVideoConfNotify_Remote_Join = 1
local eVideoConfNotify_Local_Leaved = 2
local eVideoConfNotify_Remote_Leaved = 3
local eVideoConfNotify_Closed = 4
local eVideoConfNotify_Local_Slow_Link = 5
local eVideoConfNotify_Remote_Slow_Link = 6
local eVideoConfNotify_Remote_Members = 7
local eVideoConfNotify_Message = 8
local eVideoConfMsgType_Audio_Input_Level = 9
local eVideoConfMsgType_Audio_Output_Level = 10

--Video Room Party
local ePartyType_Local = 0
local ePartyType_Remote = 1

--Svc type
eBeeSvcType_None = 0
eBeeSvcType_VideoRoom = 1
eBeeSvcType_Board = 2
eBeeSvcType_Chat = 3
eBeeSvcType_Doc = 4

--Board type
eBoardType_None = 0
eBoardType_Teacher = 1
eBoardType_Student = 2

--Video room role
eVideoRoomRole_None = 0
eVideoRoomRole_Manager = 1
eVideoRoomRole_Party = 2

--Test global variables for video room
local is_loopback = false

local test_janus = nil
local test_push_stream_name = nil
local test_push_stream_raw_name = nil
local test_pull_stream_name = nil
local test_pull_stream_raw_name = nil

--local test_janus = "wss://10.18.18.57/bee_rtc"
--local test_push_stream_name = "274ab484a4b0daf54329110eb072c6befbffb3865a719fb9df065ebd"
--local test_push_stream_raw_name = "WIN"
--local test_pull_stream_name = "ad84b4846ab0daf53d29110eb072ffbefbffb3865a719fb9d2062e67"
--local test_pull_stream_raw_name = "3cda4b5a13e14790"

Party = {
  video_room = nil,
  type = ePartyType_Local,
  named_svr = nil,
  masterd_svr = nil,
  scheduled_svr = nil,
  janus_svr = nil,
  named_conn = nil,
  masterd_conn = nil,
  scheduled_conn = nil,
  janus_ws = nil,
  rtc_conn = nil,
  session_id = nil,
  handle = nil,
  room_name = nil,
  janus_room_name = nil,
  signal_ready = false,
  media_ready = false,
  all_ready = false,
  p2p_svr="stun:stun.p2p.hd.sohu.com:4478",
  protocol="janus-protocol",
  webrtc_callbacks = nil,
  stream_name_display="",
  stream_name_internal="",
  token=nil,
  janus_keepalive_timer=nil,
  leave_timer=nil,
  ws_timeout=20000,
  push_audio=false,
  audio_source=nil,
  push_video=false, 
  video_source=nil,
  pull_audio=true,
  pull_video=true,
  video_renderer=nil,
  video_renderer_started=false,
  max_bitrate=768000,
  min_bitrate=256000,
  stats_timer=nil,
  stats_timeout=1000,
  uid=nil,
  nick_name=nil,
  token=nil,  
  no_audio_processing=nil,
  enable_level_control=nil,
  send_local_offer_pending=false,
  local_offer_created=false,
  local_offer_sdp=nil,
  candidate_list={},
  closed=nil
}

Party.__index = Party

function Party:new(video_room, type, uid, nick_name, token, stream_name_display, video_renderer)
  local o = {}
  setmetatable(o, self)
  o.video_room = video_room
  o.type = type
  o.masterd_svr = video_room.masterd_svr
  o.room_name = video_room.room_name
  o.janus_room_name = video_room.janus_room_name
  o.uid = uid
  o.nick_name = nick_name
  o.token = token
  o.stream_name_display = stream_name_display
  o.video_renderer = video_renderer
  o.callbacks = {
    on_ice_candidate=handler(o, o.on_ice_candidate),
    on_ice_gathering_change=handler(o, o.on_ice_gathering_change),
    on_ice_connection_change=handler(o, o.on_ice_connection_change),
    on_media_ready=handler(o, o.on_media_ready)
  }
  self:log_info("New party type %d.", type)
  return o
end

function Party:push(named_svr, audio_source, video_source)
  assert(self.type == ePartyType_Local, "Push party must be local")
 
  self.audio_source = audio_source  
  if (self.audio_source ~= 0) then
    self.push_audio = true
  end
  
  self.video_source = video_source
  if (self.video_source ~= 0) then
    self.push_video = true
  end
  
  local ret = self:create_local_offer()
  if (ret) then
    if (test_janus ~= nil and test_push_stream_raw_name ~= nil and test_push_stream_name ~= nil) then
      self:push_test_stream(test_janus, test_push_stream_raw_name, test_push_stream_name)
    else
      self:request_named_svr(named_svr)
    end
  end
  
  assert(ret, "WebRTC peerconnection create failed.")
end

function Party:pull(stream_name, pull_audio, pull_video)
  assert(self.type == ePartyType_Remote, "Pull party must be remote")

  LogInfo("Pulling stream %s", stream_name)
  self.pull_audio = pull_audio
  self.pull_video = pull_video
  if (test_janus ~= nil and stream_name == test_pull_stream_raw_name and test_pull_stream_name ~= nil) then
    self:pull_test_stream(test_janus, test_pull_stream_raw_name, test_pull_stream_name)
  else
    self:request_masterd_svr(stream_name)
  end
end

function Party:push_test_stream(janus_svr, stream_name_display, stream_name_internal)
  self.stream_name_display = stream_name_display
  self.stream_name_internal = stream_name_internal
  self:log_info("Pushing to test janus svr %s.", janus_svr)
  self:connect_janus_svr(janus_svr)
end

function Party:pull_test_stream(janus_svr, stream_name_display, stream_name_internal)
  self.stream_name_display = stream_name_display
  self.stream_name_internal = stream_name_internal
  self:log_info("Pulling from test janus svr %s.", janus_svr)
  self:connect_janus_svr(janus_svr)
end

function Party:request_named_svr(named_svr)  
  self.named_svr = named_svr
  self.named_conn = Http:open()
  
  assert(self.named_svr, "Invalid named svr url.")
  assert(self.named_conn, "Named svr http conn open fail.")
  
  local raw_str = string.format("stream_name=%s&uid=%s&sid=%s&tok=%s", self.stream_name_display, g_sys_info.device_id, g_sys_info.session_id, self.token)
  local encrypted_str = BeeEncrypt(raw_str, g_sys_info.param_key)
  local url = string.format("%s/%s", self.named_svr, encrypted_str)
  self:log_info("Requesting named svr %s.", url)
  assert(self.named_conn:async_get(url, {oneoff=0}, handler(self, self.on_request_named_svr)), "Named svr request fail")
end

function Party:on_request_named_svr(ec1, ec2, msg, rsp)
  self:log_info("Named svr return ec1:%d ec2:%d msg:%s rsp %s.", ec1, ec2, msg, rsp["http_body"])
  if (ec1 == 0) then
    local decrypted_data = BeeDecrypt(rsp["http_body"], g_sys_info.param_key)
    local json_data = cjson.decode(decrypted_data)
    self.stream_name_internal = json_data.stream_name
    local scheduled_svr = json_data.sche_url
    self:log_info("Named svr return status code %d scheduled svr %s.", rsp.status_code, scheduled_svr)
    self:config_net_log(json_data.log_level, json_data.log_base_time)
    self:request_scheduled_svr(scheduled_svr)
  else
    if (rsp) then
      self:log_error("Named svr return status code %d.", rsp.status_code)
    end
    self.video_room:on_party_create(ec1, self)
  end
end

function Party:request_masterd_svr(stream_name)
  assert(stream_name, "stream_name nil")

  self.stream_name_internal = stream_name
  self.masterd_conn = Http:open()
  assert(self.masterd_conn, "Masterd svr http conn open fail.")
   
  local url = self:create_masterd_url(stream_name, "rtc")
  self:log_info("Requesting masterd svr %s.", url)
  
  local ret = self.masterd_conn:async_get(url, {}, handler(self, self.on_request_masterd_svr))
  if (ret == false) then
    self.video_room:on_party_create(23, self)   
  end
end

function Party:on_request_masterd_svr(ec1, ec2, msg, rsp)
  self:log_info("Masterd svr return ec1:%d ec2:%d msg:%s.", ec1, ec2, msg)
  if (ec1 == 0) then
    local dec_body = DecodeParams(rsp["http_body"], g_sys_info.param_key)
    local json_data = cjson.decode(dec_body)
    local scheduled_svr = json_data.sche_url
    self:log_info("Masterd svr return status code %d scheduled svr %s.", rsp.status_code, scheduled_svr)
    self:config_net_log(json_data.log_level, json_data.log_base_time)
    self:request_scheduled_svr(scheduled_svr)
  else
    if (rsp) then
      self:log_error("Masterd svr return status code %d.", rsp.status_code)
    end
    self.video_room:on_party_create(ec1, self)
  end
end

function Party:on_hd(ec1, ec2, msg, rsp)
  LogInfo("@@@ ec1 %d ec2 %d msg %s", ec1, ec2, msg)
  for key, value in pairs(rsp["http_header"]) do
    LogInfo("@@@ %s: %s", key, value)
  end
end

function Party:request_scheduled_svr(scheduled_svr)
  local ret = false
  if (scheduled_svr) then
    self.scheduled_svr = scheduled_svr
    self.scheduled_conn = Http:open()
    self.scheduled_conn:set_header_callback(handler(self, self.on_hd))
    if (self.scheduled_conn) then
      self:log_info("Requesting scheduled svr %s.", self.scheduled_svr)
      self.scheduled_conn:async_get(scheduled_svr, {}, handler(self, self.on_request_scheduled_svr))
      ret = true
    else
      self:log_error("Scheduled svr http conn open fail.")
    end
  else
    self:log_error("Invalid Scheduled svr url.")  
  end
  if (ret == false) then
    self.video_room:on_party_create(23, self)
  end
end

function Party:on_request_scheduled_svr(ec1, ec2, msg, rsp)
  self:log_info("Scheduled svr %s return ec1:%d ec2:%d msg:%s.", self.scheduled_svr, ec1, ec2, msg)
  if (ec1 == 0) then
    local json_data = cjson.decode(rsp["http_body"])  
    local code = json_data.code
    if (code == 1) then
      local janus_svr = json_data.info.webrtc_url
      self.stream_name_internal = json_data.info.stream_name
      self:log_info("Scheduled svr return status code %d janus svr %s.", rsp.status_code, janus_svr)      
      self:connect_janus_svr(janus_svr)
    else
      if (rsp) then
        self:log_error("Scheduled svr return status code %d error code %d.", rsp.status_code, code)   
      end
      self.video_room:on_party_create(35, self)
    end
  else
    if (rsp) then
      self:log_error("Scheduled svr return status code %d.", rsp.status_code)   
    end
    self.video_room:on_party_create(ec1, self)
  end
end

function Party:connect_janus_svr(janus_svr)
  local ret = false    
  if (janus_svr) then
    self.janus_svr = janus_svr
    self.janus_ws = Websocket:open(self.protocol, true, handler(self.video_room, self.video_room.handle_janus_data), handler(self.video_room, self.video_room.handle_janus_error))
    if (self.janus_ws) then
      self:log_info("Requesting janus svr %s.", self.janus_svr)
      ret = self.janus_ws:connect(self.janus_svr, self.ws_timeout, handler(self, self.on_janus_connect_result))
    else
      self:log_error("Party janus ws create fail.")
    end
  else
    self:log_error("Invalid janus svr url.")
  end
  if (ret == false) then
    self.video_room:on_party_create(23, self)
  end
  return ret
end

function Party:on_janus_connect_result(res, data)
  if (res) then
    self:log_info("Party connected to janus svr %s.", self.janus_svr)
    self:create_handle_ex()
  else
    self:log_error("Party connect to janus svr %s failed %s.", self.janus_svr, data)
    self.video_room:on_party_create(23, self)
  end
end

function Party:create_handle_ex()
  local ret = false
  if (self.janus_ws) then
    self:log_info("Creating handle ex")
    local transaction = self.video_room:create_transaction(true, handler(self, self.on_create_handle_ex))
    local opaque_id = string.format("beevideoroom-%s", random_string(12))
    local client_desc = string.format("%s+%s+%s+%s", g_platform_str[g_sys_info.platform], "x", g_sys_info.sys_info, g_sys_info.bee_ver) 
    local msg = string.format("{\"bee\":\"c&a\",\"transaction\":\"%s\",\"plugin\":\"bee.videoroom\",\"opaque_id\":\"%s\",\"clt_des\":\"%s\",\"stream_name\":\"%s\"}",transaction.id, opaque_id, client_desc, self.stream_name_internal)
    ret = self.janus_ws:write(msg)
  end
  if (ret == false) then
    self:log_error("create_handle_ex fail, ws not created.")
    self.video_room:on_party_create(38, self)
  end    
end

function Party:on_create_handle_ex(success, json_data)
  if (success == false) then
    self:log_error("Create handle ex fail.")
    self.video_room:on_party_create(35, self)
  else
    self.session_id = json_data.data.sid
    self.handle = json_data.data.hid
    self.video_room:on_party_handle_created(self.handle, self)
    
    self:log_info("Create handle ex success sid:%d, hid:%d.", self.session_id, self.handle)
    local ret = true
    if (self.type == ePartyType_Local) then
      self:publish_local_offer()
      self:publish_candidates()
    else
      ret = self:listen()
    end
    if (ret == false) then
      self:log_error("Create handle fail.")
      self.video_room:on_party_create(38, self)
    end
  end
end

function Party:create_local_offer()
  local ret = true
  self.rtc_conn = Webrtc:open(self.p2p_svr, self.callbacks)
  if (self.rtc_conn ~= nil) then
    ret = self.rtc_conn:create_offer(self.audio_source, self.video_source, handler(self, self.on_create_offer))
    if (ret == false) then
      self:log_error("WebRTC create offer fail.")
      self.video_room:on_party_create(41, self)
    end
  else
    self:log_error("WebRTC peerconnection open fail.")
    self.video_room:on_party_create(39, self)
    ret = false
  end
  return ret
end

function Party:on_create_offer(local_jsep)
  self.local_offer_sdp = local_jsep
  self.local_offer_created = true
  if (self.local_offer_sdp == nil or self.local_offer_sdp == "") then
    self:log_error("Create local offer fail.")
    self.video_room:on_party_create(41, self)
  elseif (self.send_local_offer_pending) then
    self:publish_local_offer()
  end
  self:log_error("WebRTC local offer created.")
end

function Party:publish_local_offer()
  if (self.local_offer_created) then
    if (self.local_offer_sdp ~= nil and self.local_offer_sdp ~= "") then      
      if (false == self:send_local_offer(self.local_offer_sdp, handler(self, self.on_offer_answered))) then
        self.video_room:on_party_create(38, self)
      end
    end
  else
    self.send_local_offer_pending = true
  end
end

function Party:publish_candidates()
  if (#self.candidate_list > 0 and self.janus_ws and self.session_id and self.handle) then
    for i=1,#self.candidate_list do
      self:send_candidate(self.candidate_list[i])
    end
    self.candidate_list = {}
  end
end

function Party:leave(reason)
  --Stop media first.
  if (self.rtc_conn ~= nil) then
    self.rtc_conn:close()
    self.rtc_conn = nil
  end

  --Stop signal, should wait for result to ensure websocket msg send/rcv completed.
  if (false) then --if (self.session_id and self.janus_ws and reason ~= 6666) then
    local transaction = self.video_room:create_transaction(true, handler(self, self.process_destroy_notify))
    local msg = string.format("{\"bee\":\"destroy\",\"session_id\":%d,\"transaction\":\"%s\"}", self.session_id, transaction.id)
    self.janus_ws:write(msg)
    self.leave_timer = Timer:open(1000, 1, handler(self, self.handle_leave_timeout))
  else
    self:close(reason)
  end
end

function Party:process_destroy_notify(success, json_data)
  local info = string.format("Received destroy notify from session %d.", json_data.session_id)
  LogInfo("%s", info)
  self:close(0)
end

function Party:handle_leave_timeout()
  self:log_info("Party leave timeout, closing")
  self:close(13)
end

function Party:listen()
  local transaction = self.video_room:create_transaction(true, handler(self, self.on_listened))
  local body = string.format("{\"request\":\"join\",\"room_name\":\"%s\",\"ptype\":\"subscriber\",\"feed_name\":\"%s\",\"audio\":%s,\"video\":%s,\"soon\":true}", self.janus_room_name, self.stream_name_internal, tostring(self.pull_audio), tostring(self.pull_video))
  local msg = string.format("{\"bee\":\"message\",\"session_id\":%d,\"handle_id\":%d,\"transaction\":\"%s\",\"body\":%s}", self.session_id, self.handle, transaction.id, body)
  return self.janus_ws:write(msg)
end

function Party:on_listened(dummy, json_data)
  local success = false
  if (json_data.plugindata.data.videoroom == "attached") then    
    local jsep = json_data.jsep
    if (jsep ~= nil) then
      local sdp = cjson.encode(jsep)
      self.rtc_conn = Webrtc:open(self.p2p_svr, self.callbacks)
      self.rtc_conn:create_answer(sdp, handler(self, self.on_create_answer))
      self.janus_keepalive_timer = Timer:open(30000, -1, handler(self, self.handle_janus_timeout))
      success = true
    end
  end
  if (success == false) then
    self.video_room:on_party_create(35, self)
  end
end

function Party:handle_janus_timeout()
  local transaction = self.video_room:create_transaction(false, nil)
  local msg = string.format("{\"bee\":\"keepalive\",\"session_id\":%d, \"transaction\":\"%s\"}", self.session_id, transaction.id)
  if (self.janus_ws) then
    self.janus_ws:write(msg)
  end
end

function Party:create_masterd_url(stream_name, stream_format)
  local args = string.format("stream_name=%s&format=%s&p=%d&uid=%s&tok=%s", stream_name, stream_format, g_sys_info.platform, g_sys_info.device_id, self.token)
  local enc_args = EncodeParams(args, g_sys_info.param_key)
  local url = string.format("%s/%s", self.masterd_svr, enc_args) 
  return url
end

function Party:on_ice_candidate(candidate)
  if (self.janus_ws and self.session_id and self.handle) then
    self:send_candidate(candidate)
  else
    table.insert(self.candidate_list, candidate)
  end
end

function Party:on_ice_gathering_change(new_state)
  if (new_state == eIceGathering_Completed) then
    local completed_msg = string.format("{\"completed\": true}")
    if (self.janus_ws and self.session_id and self.handle) then
      self:send_candidate(completed_msg)
    else
      table.insert(self.candidate_list, completed_msg)
    end
  end
end

function Party:on_ice_connection_change(new_state)
  self:log_info("Ice new state %d.", new_state)
  if (self.closed == false and (new_state == kIceConnectionFailed or new_state == kIceConnectionDisconnected or new_state == kIceConnectionClosed)) then
    self.video_room:on_party_create(51, self)
  end
end

function Party:send_local_offer(jsep, cb)
  local ret = false
  if (self.janus_ws ~= nil) then
    local transaction = self.video_room:create_transaction(true, cb)
    local body = string.format("{ \"request\":\"j&c\",\"room_name\":\"%s\",\"ptype\":\"publisher\",\"stream_name\":\"%s\",\"audio\":%s,\"video\":%s,\"max_bitrate\":%d,\"min_bitrate\":%d}", self.janus_room_name, self.stream_name_internal, tostring(self.push_audio), tostring(self.push_video), self.max_bitrate, self.min_bitrate)
    local msg = string.format("{\"bee\":\"message\",\"session_id\":%d,\"handle_id\":%d,\"transaction\":\"%s\",\"body\":%s,\"jsep\":%s }", self.session_id, self.handle, transaction.id, body, jsep)
    ret = self.janus_ws:write(msg)
  end
  return ret
end

function Party:send_remote_answer(jsep, cb)
  if (self.janus_ws ~= nil) then
    local transaction = self.video_room:create_transaction(true, cb)
    local body = string.format("{ \"request\": \"start\", \"room_name\" : \"%s\" }", self.janus_room_name)
    local msg = string.format("{\"bee\":\"message\",\"session_id\":%d,\"handle_id\":%d,\"transaction\":\"%s\",\"body\":%s,\"jsep\":%s }", self.session_id, self.handle, transaction.id, body, jsep)
    self.janus_ws:write(msg)
  end 
end

function Party:on_offer_answered(dummy, json_data)
  local jsep = json_data.jsep
  if (jsep ~= nil and jsep ~= "") then
    self:log_error("Offer answered.")
    self.janus_keepalive_timer = Timer:open(30000, -1, handler(self, self.handle_janus_timeout))
    local sdp = cjson.encode(jsep)
    self.rtc_conn:set_remote_desc(sdp)
  else
    self:log_error("Remote answer offer empty.")
    self.video_room:on_party_create(43, self) 
  end
end

function Party:send_candidate(candidate)
  local transaction = self.video_room:create_transaction(false, nil)
  local msg = string.format("{\"bee\":\"trickle\",\"session_id\":%d,\"handle_id\":%d,\"transaction\":\"%s\",\"candidate\":%s }", self.session_id, self.handle, transaction.id, candidate)
  self.janus_ws:write(msg)
end

function Party:on_answer_acked(dummy, json_data)
  if (json_data ~= nil and json_data.plugindata ~= nil and json_data.plugindata.data ~= nil and json_data.plugindata.data.started == "ok") then
    self:log_info("Party listen success.")
  else
    self:log_error("Party listen fail.")
    self.video_room:on_party_create(35, self)
  end  
end

function Party:on_create_answer(jsep)
  if (jsep ~= nil and jsep ~= "") then
    self:send_remote_answer(jsep, handler(self, self.on_answer_acked))
  else
    self:log_error("Create answer fail.")
  end
end

function Party:on_webrtcup()
  self:log_info("Party signal ready.")
  self.signal_ready = true
  self:check_channel_ready()
end

function Party:on_media_ready()
  self:log_info("Party media ready.")
  if (self.closed == nil) then
    self:log_info("closed == false")
    self.closed = false
  end
  self.media_ready = true
  if (self.video_renderer ~= nil) then
    self:start_video_renderer()
  end
  self:check_channel_ready()
end

function Party:on_hangup()
  self:log_info("Party hangup, closing.")
  self:close(38)
end

function Party:check_channel_ready()
  if (self.signal_ready and self.media_ready) then
    if (self.type == ePartyType_Local) then
      --self.stats_timer = Timer:open(self.stats_timeout, -1, handler(self, self.handle_stats_timeout))
      self.video_room:on_party_create(0, self)

      if (test_janus ~= nil and test_pull_stream_name ~= nil) then
        self.video_room:handle_media_svc_joinback(nil)        
        self.video_room:pull_test_stream(test_pull_stream_raw_name)
      end
    else
      --self.stats_timer = Timer:open(self.stats_timeout, -1, handler(self, self.handle_stats_timeout))
      local data = string.format("{\"type\":%d, \"ec\":0, \"room_name\":\"%s\", \"stream_name\":\"%s\", \"uid\":\"%s\"}", eVideoConfNotify_Remote_Join, self.room_name, self.stream_name_display, self.uid)
      self:report_data(data)
    end    
    self.all_ready = true
    self:log_info("Party type %d all ready.", self.type)
  end
end

function Party:handle_stats_timeout()
  self:get_stats(0)
end

function Party:process_event(json_data)

end

function Party:start_video_renderer()
  self:log_info("start_video_renderer.")
  if (self.rtc_conn == nil) then
  	self:log_error("rtc_conn is nil.")
  	return
  end  
  if (self.video_renderer == nil or self.video_renderer == 0) then
    self:log_error("Invalid video renderer.")
  	return
  end
  self.video_renderer_started = self.rtc_conn:start_video_renderer(self.video_renderer)
end

function Party:stop_video_renderer()
  self:log_info("stop_video_renderer.")    
  if (self.rtc_conn ~= nil) then
    self.rtc_conn:stop_video_renderer()
  end
end

function Party:get_stats(obsvr)
  --self:log_info("get_stats.")
  if (self.rtc_conn ~= nil) then
    return self.rtc_conn:get_stats(obsvr)
  else
    return false
  end
end

function Party:config_net_log(log_level, log_base_time)
  local level = 0
  local base_time = 0
  if (log_level ~= nil) then
    level = log_level
  end
  if (log_base_time ~= nil) then
    base_time = log_base_time
  end
  sdk_ConfigureNetLog(level, base_time)
end

function Party:report_close(reason)
  local type = nil
  if (self.all_ready) then
    if (self.type == ePartyType_Local) then
      type = eVideoConfNotify_Local_Leaved
    else
      type = eVideoConfNotify_Remote_Leaved
    end
  else
    if (self.type == ePartyType_Local) then
      type = eVideoConfNotify_Local_Join
    else
      type = eVideoConfNotify_Remote_Join
    end   
  end

  local data = string.format("{\"type\":%d, \"ec\":%d, \"room_name\":\"%s\", \"stream_name\":\"%s\", \"uid\":\"%s\"}", type, reason, self.room_name, self.stream_name_display, self.uid)
  self:report_data(data)
end

function Party:close(reason)
  if (self.video_renderer_started) then
    self:stop_video_renderer()
  end
  if (self.named_conn ~= nil) then
    self.named_conn:close()
    self.named_conn = nil
  end
  if (self.masterd_conn ~= nil) then
    self.masterd_conn:close()
    self.masterd_conn = nil
  end
  if (self.scheduled_conn ~= nil) then
    self.scheduled_conn:close()
    self.scheduled_conn = nil
  end    
  if (self.rtc_conn ~= nil) then
    self.rtc_conn:close()
    self.rtc_conn = nil
  end
  if (self.janus_ws ~= nil) then
    self.janus_ws:close()
    self.janus_ws = nil
  end
  if (self.janus_keepalive_timer ~= nil) then
    self.janus_keepalive_timer:close()
    self.janus_keepalive_timer = nil
  end
  if (self.stats_timer ~= nil) then
    self.stats_timer:close()
    self.stats_timer = nil  
  end
  if (self.leave_timer ~= nil) then
    self.leave_timer:close()
    self.leave_timer = nil
  end
  if (self.video_room ~= nil) then
    self.video_room:on_party_closed(self)
  end
  self:report_close(reason)
  self.closed=true
  LogInfo("Party %s closed, reason %d", self.stream_name_display, reason)
end

function Party:get_audio_input_level()
  if (self.rtc_conn ~= nil) then
    return self.rtc_conn:get_audio_input_level()
  else
    return -1
  end
end

function Party:get_audio_output_level(stream_name)
  if (self.rtc_conn ~= nil) then
    return self.rtc_conn:get_audio_output_level()
  else
    return -1
  end
end

function Party:log(level,fmt, ...)
  local debug_info = debug.getinfo(3)
  fmt = "[%s] "..fmt
  sdk_BeeLog(level, debug_info.source, debug_info.currentline, fmt:format(self.stream_name_display, ...))
end

function Party:log_fatal(fmt, ...)
  self:log(kLogLevel_Fatal, fmt, ...)
end

function Party:log_error(fmt, ...)
  self:log(kLogLevel_Error, fmt, ...)
end

function Party:log_warn(fmt, ...)
  self:log(kLogLevel_Warn, fmt, ...)
end

function Party:log_trace(fmt, ...)
  self:log(kLogLevel_Trace, fmt, ...)
end

function Party:log_info(fmt, ...)
  self:log(kLogLevel_Info, fmt, ...)
end

function Party:log_debug(fmt, ...)
  self:log(kLogLevel_Debug, fmt, ...)
end

function Party:report_data(data)
  if (self.video_room ~= nil) then
    self.video_room:report_data(data)
  end
end

--Video Room
local ws_protocols = {"janus-protocol"}
VideoRoom = {
  desc = "Video Room Service",
  --Online services.
  named_svr="https://name.hd.sohu.com/bee_named",
  masterd_svr="https://switch.hd.sohu.com/bee_switch",
  msg_svc_url="wss://name.hd.sohu.com/bee_msg_media",

  --Test services.
  --named_svr="https://testlive.hd.sohu.com/bee_named",
  --masterd_svr="https://testlive.hd.sohu.com/bee_switch",
  --msg_svc_url="wss://name.hd.sohu.com/chengmo_media",
  
  room_name_display=nil,
  room_name_internal=nil,
  room_name_compat=nil,
  stream_name_display=nil,
  stream_name_internal=nil,
  push_video=true,
  video_source=nil,
  push_audio=true,
  audio_source=nil,
  local_video_renderer=nil,
  create=nil,
  uid=nil,
  nick_name=nil,
  token=nil,
  role=nil,
  role_alias="none",  
  protocol="janus-protocol",
  msg_svc_ws=nil,
  ws_timeout=20000,
  janus_room_name="h264",
  stream_format="rtc",
  local_party=nil,
  transaction_table={},
  handle_party_table={},
  push_stream_party_table={},
  pull_stream_party_table={},
  push_stream_count=0,
  pull_stream_count=0,
  switch_message={},  
  leaving=false,
  pushing=false,
  msg_svc_pending=false,
  named_conn=nil,
  mastered_conn=nil,
  push_scheduled_conn=nil,
  pull_scheduled_conn=nil,    
  msg_svc_keepalive_timer=nil,
  msg_svc_list_timer=nil
}

VideoRoom.__index = VideoRoom

function VideoRoom:new()
  local o = {}
  setmetatable(o, self)  
  o.switch_message = {
    ["success"] = function(json_data)
      o:process_transaction_success(json_data)
    end,
    ["error"] = function(json_data)
      o:process_transaction_error(json_data)
    end,
    ["ack"] = function(json_data)
      o:process_ack(json_data)
    end,
    ["event"] = function(json_data)
      o:process_event(json_data)
    end,
    ["webrtcup"] = function(json_data)
      o:process_webrtcup_notify(json_data)
    end,
    ["media"] = function(json_data)
      o:process_media_notify(json_data)
    end,
    ["slowlink"] = function(json_data)
      o:process_slowlink_notify(json_data)
    end,
    ["hangup"] = function(json_data)
      o:process_hangup_notify(json_data)
    end,
    ["detached"] = function(json_data)
      o:process_detached_notify(json_data)
    end
  }
  return o
end

function VideoRoom:join(json_data)    
  assert(json_data.room_name, "room_name nil")
  assert(json_data.create ~= nil, "create nil")  
  assert(json_data.uid, "uid nil")
  assert(json_data.nick_name, "nick_name nil")
  assert(json_data.token, "token nil")
  assert(json_data.role, "role nil")
  
  self.room_name_display = json_data.room_name
  self.room_name_compat = "media_"..json_data.room_name
  self.create = json_data.create
  self.uid = json_data.uid
  self.nick_name = json_data.nick_name
  self.token = json_data.token
  self.role = json_data.role
  self.push_audio = false
  self.push_video = false

  if (self.role == eVideoRoomRole_Party) then
    self.role_alias = "stu"
  elseif (self.role == eVideoRoomRole_Manager) then
    self.role_alias = "tech"
  else
    self.role_alias = "none"
  end
  
  local push = json_data.push
  if (push ~= nil) then
    assert(push.stream_name, "stream_name nil")
    self.stream_name_display = push.stream_name
    if (push.audio ~= nil) then
      if (push.audio.present ~= nil) then
        self.push_audio = push.audio.present        
      end
      self.audio_source = push.audio.source
    end
    if (push.video ~= nil) then
      if (push.video.present ~= nil) then
        self.push_video = push.video.present
      end
      self.video_source = push.video.source
      self.local_video_renderer = push.video.renderer
    end
  end
  
  if (self.push_video) then
    assert(self.video_source ~= nil and self.video_source ~= 0, "Video source nil")
  end
    
  self:open()
end

function VideoRoom:leave(json_data)
  assert(json_data.room_name, "room_name nil")
  assert(json_data.reason, "reason nil")
  self:close(json_data.reason)
end

function VideoRoom:connect_stream(json_data)
  assert(json_data.room_name, "room_name nil")
  assert(json_data.stream_name, "stream_name nil")
  assert(json_data.uid, "uid nil")

  local room_name = json_data.room_name
  local stream_name = json_data.stream_name
  local uid = json_data.uid
  local video_renderers = json_data.renderer
  
  local pull_video = true
  local pull_audio = true
  if (json_data.pull_video ~= nil) then
    pull_video = json_data.pull_video 
  end
  if (json_data.pull_audio ~= nil) then
    pull_audio = json_data.pull_audio 
  end
  self:do_connect_stream(stream_name, uid, nick_name, pull_video, pull_audio, video_renderers)
end

function VideoRoom:disconnect_stream(json_data)
  assert(json_data.room_name, "room_name nil")
  assert(json_data.stream_name, "stream_name nil")
  assert(json_data.uid, "uid nil")
  assert(json_data.type, "type nil")
  assert(json_data.reason, "reason nil")
  
  local room_name = json_data.room_name
  local stream_name = json_data.stream_name
  local uid = json_data.uid
  local type = json_data.type
  self:do_disconnect_stream(uid, stream_name, type, reason)
end

function VideoRoom:get_stats(args)
  local json_data = cjson.decode(args)
  local room_name = json_data.room_name
  local stream_name = json_data.stream_name
  local type = json_data.type
  local obsvr = json_data.obsvr
  if (video_room == nil) then
    LogError("Video room nil when GetStats")
    return false
  else
    return video_room:get_stats(stream_name, type, obsvr)
  end
end

function VideoRoom:get_audio_input_level(args)
  local json_data = cjson.decode(args)
  if (video_room == nil) then
    LogError("Video room nil when GetAudioInputVolume")
  else
    local stream_name = json_data.stream_name
    return video_room:get_audio_input_level(stream_name)
  end
end

function VideoRoom:get_audio_output_level(args)
  local json_data = cjson.decode(args)
  if (video_room == nil) then
    LogError("Video room nil when GetAudioInputVolume")
  else
    local uid = json_data.uid
    local stream_name = json_data.stream_name
    return video_room:get_audio_output_level(uid, stream_name)
  end
end

function VideoRoom:open()
  Webrtc:enable_tracing(5)
  if (Websocket:start(ws_protocols) == false) then
    error("Websocket service start failed", 0)
  end
  self.msg_svc_ws = Websocket:open("", true, handler(self, self.handle_media_svc_data), handler(self, self.handle_media_svc_error))
  assert(self.msg_svc_ws, "Websocket open failed")
  
  if (test_janus == nil) then
    self:request_named_svr_for_svc()
  end
  
  if (self.push_video or self.push_audio) then
    self:push()
  else
    self.stream_name_internal = random_string(32)
    self.msg_svc_pending = true
  end
end

function VideoRoom:close(reason)
  self.leaving = true
  if (self.msg_svc_ws ~= nil) then
    self.msg_svc_ws:close()
    self.msg_svc_ws = nil
  end
  if (self.msg_svc_keepalive_timer ~= nil) then
    self.msg_svc_keepalive_timer:close()
    self.msg_svc_keepalive_timer = nil
  end
  if (self.msg_svc_list_timer ~= nil) then
    self.msg_svc_list_timer:close()
    self.msg_svc_list_timer = nil
  end
 
  local empty = true 
  for stream, party in pairs(self.push_stream_party_table) do
    party:leave(reason)
    empty = false
  end

  for stream, party in pairs(self.pull_stream_party_table) do
    party:leave(reason)
    empty = false
  end

  if (empty) then
    LogInfo("Close empty video room")
  end
end

function VideoRoom:push()
  local party = Party:new(self, ePartyType_Local, self.uid, self.nick_name, self.token, self.stream_name_display, self.local_video_renderer)
  self.push_stream_party_table[party.stream_name_display] = party
  party.no_audio_processing = self.no_audio_processing
  party.enable_level_control = self.enable_level_control
  party:push(self.named_svr, self.audio_source, self.video_source)
  self.pushing = true
end

function VideoRoom:pull_test_stream(test_stream)
  LogInfo("Pulling test stream %s", test_stream)
  if (test_stream == nil) then
    LogError("test stream name nil")
    return
  end
  local media_item = {streamname=test_stream,type="video"}
  local media_list = {media_item}
  local stream_item = {media=media_list,uid=self.uid,type="stu"}
  local stream_list = {stream_item}
  local data = {list=stream_list}
  self:handle_media_svc_listback(data)
end

function VideoRoom:do_connect_stream(stream_name, uid, nick_name, pull_video, pull_audio, video_renderer)
  local party = Party:new(self, ePartyType_Remote, uid, nick_name, self.token, stream_name, video_renderer)
  LogInfo("Add pull stream %s", stream_name)
  self.pull_stream_party_table[stream_name] = party
  self.pull_stream_count = self.pull_stream_count + 1
  party:pull(stream_name, pull_audio, pull_video)
end

function VideoRoom:get_audio_input_level(stream_name)
  local party = self.push_stream_party_table[stream_name]
  local level = -1
  if (party ~= nil) then
    level = party:get_audio_input_level()
  else
    LogInfo("get_audio_input_level party null")
  end
  local data = string.format("{\"type\":%d, \"uid\":\"%s\", \"room_name\":\"%s\", \"stream_name\":\"%s\", \"level\":%d}", eVideoConfMsgType_Audio_Input_Level, self.uid, self.room_name, stream_name, level)
  self:report_data(data)
end

function VideoRoom:get_audio_output_level(uid, stream_name)
  local party = self.pull_stream_party_table[stream_name]
  local level = -1
  if (party ~= nil) then
    level = party:get_audio_output_level()
  end
  local data = string.format("{\"type\":%d, \"uid\":\"%s\", \"room_name\":\"%s\", \"stream_name\":\"%s\", \"level\":%d}", eVideoConfMsgType_Audio_Output_Level, uid, self.room_name, stream_name, level)
  self:report_data(data)
end

function VideoRoom:report_members(stream_list)
  local data = string.format("{\"type\":%d, \"room_name\":\"%s\", \"members\":[", eVideoConfNotify_Remote_Members, self.room_name)
  local count = 0
  for i = 1, #stream_list do
    local stream = stream_list[i]
    local stream_name = stream.stream_name
    local can_pull_myself = false
    if (self.pushing and is_loopback) then
      can_pull_myself = true
    end
    if (can_pull_myself or stream_name ~= self.stream_name_display)  then
      local exist_party = self.pull_stream_party_table[stream_name]
      if (exist_party == nil) then
        if (count > 0) then
          data = data .. ","
        end
        local nick_name = "anonymous"
        if (stream.nick_name ~= nil) then
          nick_name = stream.nick_name
        end
        local media_type = eMediaType_None
        if (stream.media_type == "audio") then
          media_type = eMediaType_Audio
        elseif (stream.media_type == "video") then
          media_type = eMediaType_Audio_Video
        end
        local member = string.format("{\"stream_name\":\"%s\",\"uid\":\"%s\",\"nick_name\":\"%s\",\"media_type\":%d,\"role\":%d}", stream_name, stream.uid, nick_name, media_type, stream.role)
        data = data .. member
        count = count + 1
      end
    end
  end
  data = data .. "]}"
  if (count > 0) then
    self:report_data(data)
  end
end

function VideoRoom:request_named_svr_for_svc()
  self:request_named_svr_for_media_svc()
end

function VideoRoom:on_party_create(res, party)
  if (party and party.type == ePartyType_Local) then
    if (res == 0) then
      self.stream_name_internal = party.stream_name_internal      
      LogInfo("push_stream_party_table %s created", party.stream_name_display)
      self.push_stream_count = self.push_stream_count + 1
      if (self.room_name_internal ~= nil) then
        self:request_media_svc()
      else
        self.msg_svc_pending = true
      end
    else
      party:close(res)
    end
  end

  if (party and party.type == ePartyType_Remote) then
    if (res ~= 0) then
      self.pull_stream_party_table[party.stream_name_display] = nil
      self.pull_stream_count = self.pull_stream_count - 1
      party:close(res)
    end     
  end
end

function VideoRoom:on_party_handle_created(handle, party)
  self.handle_party_table[handle] = party
end

function VideoRoom:on_party_closed(party)
  LogInfo("Party stream %s closed", party.stream_name_display)
  if (party ~= nil) then
    if (party.handle ~= nil) then
      self.handle_party_table[party.handle] = nil
    end
    if (party.type == ePartyType_Local) then
      self.push_stream_party_table[party.stream_name_display] = nil
      self.push_stream_count = self.push_stream_count - 1
    else
      self.pull_stream_party_table[party.stream_name_display] = nil
      self.pull_stream_count = self.pull_stream_count - 1  
    end
    LogInfo("on_party_closed left %d push stream, %d pull stream.", self.push_stream_count, self.pull_stream_count)
    if (self.leaving and self.push_stream_count == 0 and self.pull_stream_count == 0) then
      LogInfo("All party leaved.")
    end
  end
end

function VideoRoom:create_transaction(has_rsp, cb)
  local id = random_string(12)
  local trans_id = string.format("%s+%s", id, g_sys_info.device_id)
  local transaction = Transaction:new(trans_id, has_rsp, cb)
  self.transaction_table[trans_id] = transaction
  return transaction
end

function VideoRoom:process_transaction_result(success, json_data)
  local transaction = self.transaction_table[json_data.transaction]
  if (transaction == nil) then
    return
  end
  local id = json_data.transaction
  transaction:process(success, json_data)
  self.transaction_table[id] = nil
end

function VideoRoom:process_transaction_success(json_data)
  self:process_transaction_result(true, json_data)
end

function VideoRoom:process_transaction_error(json_data)
  self:process_transaction_result(false, json_data)
end

function VideoRoom:process_ack(json_data)
  local id = json_data.transaction
  local transaction = self.transaction_table[id]
  if (transaction ~= nil) then
    transaction.acked = true
    if (transaction.has_rsp == false) then
      self.transaction_table[id] = nil
    end
  end
end

function VideoRoom:process_event(json_data)
  local sender = json_data.sender
  local party = self.handle_party_table[sender]
  if (party == nil) then
    LogError("Invalid event sender %s", sender)
    return
  end
  local transaction = self.transaction_table[json_data.transaction]
  if (transaction ~= nil) then
    transaction:process(true, json_data)    
    local id = json_data.transaction
    self.transaction_table[id] = nil
  else
    if (json_data.plugindata ~= nil and json_data.plugindata.data ~= nil and json_data.plugindata.data.leaving ~= nil) then
      --Leaving message won't be available any more.
    else
      party:process_event(json_data)
    end
  end
end

function VideoRoom:process_webrtcup_notify(json_data)
  local info = string.format("Received webrtcup notify from session %d, handle %d, connection established.", json_data.session_id, json_data.sender)
  LogInfo("%s", info)
  local party = self.handle_party_table[json_data.sender]
  if (party ~= nil) then
    party:on_webrtcup() 
  end
end

function VideoRoom:process_media_notify(json_data)
  local info = string.format("Receiving media notify from session %d, handle %d, type %s, receiving %s.", json_data.session_id, json_data.sender, json_data.type, json_data.receiving)
  LogInfo("%s", info)
end

function VideoRoom:process_slowlink_notify(json_data)
  local info = string.format("Received slowlink notify from session %d, handle %d, uplink %s, nack %d.", json_data.session_id, json_data.sender, json_data.uplink, json_data.nacks)
  LogInfo("%s", info)
  local party = self.handle_party_table[json_data.sender]
  if (party ~= nil) then
    local type = nil
    if (party.type == ePartyType_Local) then
      type = eVideoConfNotify_Local_Slow_Link
    else
      type = eVideoConfNotify_Remote_Slow_Link
    end
    local data = string.format("{\"type\":%d, \"room_name\":\"%s\", \"stream_name\":\"%s\", \"info\":{\"nack\":%d}}", type, party.room_name, party.stream_name_display, json_data.nacks)
    self:report_data(data)
  else
    local log = string.format("No sender %s", json_data)
    LogError("%s", log)
  end
end

function VideoRoom:process_hangup_notify(json_data)
  --Do not handle hangup message now.
  local info = string.format("Received hangup notify from session %d, handle %d, reason %s.", json_data.session_id, json_data.sender, json_data.reason)
  LogInfo("%s", info)
end

function VideoRoom:process_detached_notify(json_data)
  local info = string.format("Received detach notify from session %d, handle %d.", json_data.session_id, json_data.sender)
  LogInfo("%s", info)
  local party = self.handle_party_table[json_data.sender]
  if (party ~= nil) then
    party:close(0) 
  end 
end

function VideoRoom:handle_janus_data(data)
  LogInfo("Rcv WSMSG: %s",data)
  local json_data = cjson.decode(data)
  local msg = json_data.bee
  local f = self.switch_message[msg]
  if (f) then
    f(json_data)
  end
end

function VideoRoom:handle_janus_error(data)
  LogInfo("Rcv err: %s",data)
end

function VideoRoom:request_named_svr_for_media_svc()
  local ret = false
  self.named_conn = Http:open()
  if (self.named_conn) then
    local raw_str = string.format("stream_name=%s&uid=%s&sid=%s&tok=%s", self.room_name_compat, g_sys_info.device_id, g_sys_info.session_id, self.token)
    local encrypted_str = BeeEncrypt(raw_str, g_sys_info.param_key)
    local url = string.format("%s/%s", self.named_svr, encrypted_str)
    LogInfo("Requesting named svr %s for media room id.", url)
    ret = self.named_conn:async_get(url, {oneoff=0}, handler(self, self.on_request_named_svr_for_media_svc))      
  else
    LogError("Named svr http conn open fail.")  
  end
  return ret
end

function VideoRoom:on_request_named_svr_for_media_svc(ec1, ec2, msg, rsp)
  LogInfo("Named svr return ec1:%d ec2:%d msg:%s rsp %s for media svc.", ec1, ec2, msg, rsp["http_body"])
  if (ec1 == 0) then
    local decrypted_data = BeeDecrypt(rsp["http_body"], g_sys_info.param_key)
    local json_data = cjson.decode(decrypted_data)
    self.room_name_internal = json_data.stream_name
    if (self.msg_svc_pending) then
      self:request_media_svc()
    end
  end
end

function VideoRoom:request_media_svc()
  if (test_janus ~= nil and (test_push_stream_name ~= nil or test_pull_stream_name ~= nil)) then
    return
  end
  LogInfo("Requesting media svc %s", self.msg_svc_url)
  self.msg_svc_ws:connect(self.msg_svc_url, self.ws_timeout, handler(self, self.handle_media_svc_connect))  
end

function VideoRoom:handle_media_svc_connect(res, data)
  if (res) then
    LogInfo("Connected to media svc %s", self.msg_svc_url)
    local msg = nil
    if (self.create) then
      msg = string.format("{\"cmd\":\"create\",\"roomid\":\"%s\",\"uid\":\"%s\"}", self.room_name_internal, self.uid)
    else
      if (self.push_video) then
        msg = string.format("{\"cmd\":\"join\",\"roomid\":\"%s\",\"cast\":1,\"uid\":\"%s\",\"profile\":{\"uid\":\"%s\",\"type\":\"%s\",\"media\":[{\"type\":\"video\",\"streamname\":\"%s\"}],\"outinfo\":{\"name\":\"%s\"}}}", self.room_name_internal, self.uid, self.uid, self.role_alias, self.stream_name_display, self.nick_name)
      elseif (self.push_audio) then
        msg = string.format("{\"cmd\":\"join\",\"roomid\":\"%s\",\"cast\":1,\"uid\":\"%s\",\"profile\":{\"uid\":\"%s\",\"type\":\"%s\",\"media\":[{\"type\":\"audio\",\"streamname\":\"%s\"}],\"outinfo\":{\"name\":\"%s\"}}}", self.room_name_internal, self.uid, self.uid, self.role_alias, self.stream_name_display, self.nick_name)
      else
        msg = string.format("{\"cmd\":\"join\",\"roomid\":\"%s\",\"cast\":1,\"uid\":\"%s\",\"profile\":{\"uid\":\"%s\",\"type\":\"%s\",\"outinfo\":{\"name\":\"%s\"}}}", self.room_name_internal, self.uid, self.uid, self.role_alias, self.nick_name)
      end
    end
    self.msg_svc_ws:write(msg)
  else
    LogError("Failed to connect media svc %s %s", self.msg_svc_url, data)    
    self:handle_media_svc_error(data)
  end
end

function VideoRoom:handle_media_svc_createback(data)
  local msg = nil
  if (self.push_video) then
    msg = string.format("{\"cmd\":\"join\",\"roomid\":\"%s\",\"cast\":1,\"uid\":\"%s\",\"profile\":{\"uid\":\"%s\",\"type\":\"%s\",\"media\":[{\"type\":\"video\",\"streamname\":\"%s\"}],\"outinfo\":{\"name\":\"%s\"}}}", self.room_name_internal, self.uid, self.uid, self.role_alias, self.stream_name_display, self.nick_name)
  elseif (self.push_audio) then
    msg = string.format("{\"cmd\":\"join\",\"roomid\":\"%s\",\"cast\":1,\"uid\":\"%s\",\"profile\":{\"uid\":\"%s\",\"type\":\"%s\",\"media\":[{\"type\":\"audio\",\"streamname\":\"%s\"}],\"outinfo\":{\"name\":\"%s\"}}}", self.room_name_internal, self.uid, self.uid, self.role_alias, self.stream_name_display, self.nick_name)
  else
    msg = string.format("{\"cmd\":\"join\",\"roomid\":\"%s\",\"cast\":1,\"uid\":\"%s\",\"profile\":{\"uid\":\"%s\",\"type\":\"%s\",\"outinfo\":{\"name\":\"%s\"}}}", self.room_name_internal, self.uid, self.uid, self.role_alias, self.nick_name)
  end
  self.msg_svc_ws:write(msg)
end

function VideoRoom:handle_media_svc_joinback(cls_data)
  local data = string.format("{\"type\":%d, \"ec\":0, \"room_name\":\"%s\", \"stream_name\":\"%s\"}", eVideoConfNotify_Local_Join, self.room_name, self.stream_name_display)
  self:report_data(data)
  if (cls_data) then
    self.msg_svc_keepalive_timer = Timer:open(30000, -1, handler(self, self.handle_media_svc_timeout))
    --self.msg_svc_list_timer = Timer:open(5000, -1, handler(self, self.handle_media_svc_list_timeout))
  end
end

function VideoRoom:handle_media_svc_listback(data)
  local new_streams = self:check_parties(data)
  if (new_streams ~= nil and #new_streams > 0) then
    self:report_members(new_streams)
  end
end

function VideoRoom:handle_media_svc_receiveback(from, msg)
  local data = string.format("{\"type\":%d,\"ec\":0,\"room_name\":\"%s\",\"from\":\"%s\",\"message\":%s,\"svc\":%d}", eVideoConfNotify_Message, self.room_name, from, msg, eBeeSvcType_VideoRoom)
  self:report_data(data)
end

function VideoRoom:handle_media_svc_data(data)
  LogInfo("Rcv media svc data: %s",data)
  local json_data = cjson.decode(data)

  if (json_data.result ~= nil and json_data.result ~= 1 and json_data.reason ~= nil) then
    local error_msg = string.format("media svc %s error %s", json_data.cmd, json_data.reason)
    self:handle_media_svc_error(error_msg)
    return
  end
  
  if (json_data.cmd == "createback") then
    self:handle_media_svc_createback(json_data)
  elseif (json_data.cmd == "joinback") then
    self:handle_media_svc_joinback(json_data)
  elseif (json_data.cmd == "listback") then
    self:handle_media_svc_listback(json_data)
  elseif (json_data.cmd == "receiveback") then
    local from = json_data.senduid
    self:handle_media_svc_receiveback(from, data)
  end
end

function VideoRoom:handle_media_svc_error(data)
  LogInfo("Media svc error: %s",data)
  local data = string.format("{\"type\":%d,\"ec\":38,\"room_name\":\"%s\",\"stream_name\":\"%s\",\"msg\":\"%s\",\"svc\":%d}", eVideoConfNotify_Local_Join, self.room_name, self.stream_name_display, data, eBeeSvcType_VideoRoom)
  self:report_data(data)
end

function VideoRoom:handle_media_svc_timeout()
  local msg = string.format("{\"cmd\":\"heart\",\"roomid\":\"%s\", \"uid\":\"%s\"}", self.room_name_internal, self.uid)
  self.msg_svc_ws:write(msg)
end

function VideoRoom:handle_media_svc_list_timeout()
  if (self.msg_svc_ws ~= nil) then
    local msg = string.format("{\"cmd\":\"list\",\"roomid\":\"%s\", \"uid\":\"%s\"}", self.room_name_internal, self.uid)
    self.msg_svc_ws:write(msg)
  end
end

function VideoRoom:check_parties(data)
  local report_streams = {}
  local current_streams = {}
  local list = data.list
  if (list == nil and #list == 0) then
    return
  end
  
  for i = 1, #list do
    if (list[i] ~= nil) then
      local stream = {}
      stream.uid = list[i].uid
      if (list[i].outinfo ~= nil and list[i].outinfo.name ~= nil) then
        stream.nick_name = list[i].outinfo.name
      end
      if (list[i].type ~= nil) then
        stream.role_alias = list[i].type
        if (list[i].type == "tech") then
          stream.role = eVideoRoomRole_Manager
        elseif (list[i].type == "stu") then
          stream.role = eVideoRoomRole_Party
        else
          stream.role = eVideoRoomRole_None
        end
      end
      if (list[i].media ~= nil and list[i].media[1] ~= nil and list[i].media[1].streamname ~= nil) then
        local stream_name = list[i].media[1].streamname
        current_streams[stream_name] = stream_name
        if (self.pull_stream_party_table[stream_name] == nil) then
          stream.stream_name = stream_name
          stream.media_type = list[i].media[1].type
          table.insert(report_streams, stream)
        end
      end     
    end
  end
  
  for key, party in pairs(self.pull_stream_party_table) do
    if (current_streams[key] == nil) then
      LogInfo("Party uid %s stream %s leaved, will close now", party.uid, key)
      if (party.all_ready) then
        party:leave(0)
      else
        party:leave(20)
      end
    end
  end
  
  return report_streams
end

function VideoRoom:get_stats(stream_name, type, obsvr)
  local party = nil
  if (type == ePartyType_Local) then
    party = self.push_stream_party_table[stream_name]
  else
    party = self.pull_stream_party_table[stream_name]
  end
  if (party ~= nil) then
    return party:get_stats(obsvr)
  else
    return false
  end
end

function VideoRoom:do_disconnect_stream(uid, stream_name, type, reason)
  local party = nil
  if (type == ePartyType_Local) then
    party = self.push_stream_party_table[stream_name]
    if (party ~= nil) then
      party:leave(reason)
    end
  else
    party = self.pull_stream_party_table[stream_name]
    if (party ~= nil) then
      party:leave(reason)
    end
  end
end

function VideoRoom:report_data(data)
  if (data ~= nil) then
    sdk_SvcNotify(eBeeSvcType_VideoRoom, data)
  end
end

--White Board
local eWhiteBoardNotify_Local_Join = 0
local eWhiteBoardNotify_Remote_Join = 1
local eWhiteBoardNotify_Local_Leave = 2
local eWhiteBoardNotify_Remote_Leave = 3
local eWhiteBoardNotify_Closed = 4
local eWhiteBoardNotify_Message = 5

local eBeeWhiteBoardRole_None = 0
local eBeeWhiteBoardRole_Teacher = 1
local eBeeWhiteBoardRole_Student = 2
    
WhiteBoard = {
  desc = "White Board Service",
  
  --Online services.
  named_svr="https://name.hd.sohu.com/bee_named",
  msg_svc_url="wss://name.hd.sohu.com/bee_msg_bd",

  --Test services.
  --named_svr="https://testlive.hd.sohu.com/bee_named",
  --msg_svc_url="wss://name.hd.sohu.com/chengmo_bd",
  
  room_name_display=nil,
  room_name_internal=nil,
  room_name_compat=nil,
  create=nil,
  uid=nil,
  nick_name=nil,
  token=nil,
  role=nil,
  role_alias="none",
  msg_svc_ws=nil,
  ws_timeout=20000,
  named_conn=nil, 
  msg_svc_keepalive_timer=nil,
  msg_svc_list_timer=nil,
  joined=false,
  member_table={},
}

WhiteBoard.__index = WhiteBoard

function WhiteBoard:new()
  local o = {}
  setmetatable(o, self)  
  return o
end

function WhiteBoard:join(json_data)    
  assert(json_data.room_name, "room_name nil")
  assert(json_data.create ~= nil, "create nil")  
  assert(json_data.uid, "uid nil")
  assert(json_data.nick_name, "nick_name nil")
  assert(json_data.token, "token nil")
  assert(json_data.role, "role nil")
  
  self.room_name_display = json_data.room_name
  self.room_name_compat = "board_"..json_data.room_name
  self.create = json_data.create
  self.uid = json_data.uid
  self.nick_name = json_data.nick_name
  self.token = json_data.token
  self.role = json_data.role
  
  if (self.role == eBeeWhiteBoardRole_Student) then
    self.role_alias = "stu"
  elseif (self.role == eBeeWhiteBoardRole_Teacher) then
    self.role_alias = "tech"
  else
    self.role_alias = "none"
  end

  self:open()
end

function WhiteBoard:leave(json_data)
  assert(json_data.room_name, "room_name nil")
  self:close()
end

function WhiteBoard:send_message(json_data)
  assert(json_data.cmd, "SendMessage cmd nil")
  assert(json_data.uidlist, "SendMessage uidlist nil")
  assert(json_data.uid, "SendMessage uid nil")
  assert(json_data.msg, "SendMessage msg nil")
  
  if (self.msg_svc_ws ~= nil) then
    json_data.roomid = self.room_name_internal
    local msg = cjson.encode(json_data)
    self.msg_svc_ws:write(msg)
  end
end

function WhiteBoard:open()
  if (Websocket:start(ws_protocols) == false) then
    error("Websocket service start failed for white board", 0)
  end
  self.msg_svc_ws = Websocket:open("", true, handler(self, self.handle_msg_svc_data), handler(self, self.handle_msg_svc_error))
  assert(self.msg_svc_ws, "Websocket open failed")
  
  assert(self:request_named_svr_for_msg_svc(), "Request named svr failed")
end

function WhiteBoard:close()
  self.leaving = true
  if (self.msg_svc_ws ~= nil) then
    self.msg_svc_ws:close()
    self.msg_svc_ws = nil
  end
  if (self.msg_svc_keepalive_timer ~= nil) then
    self.msg_svc_keepalive_timer:close()
    self.msg_svc_keepalive_timer = nil
  end
  local data = string.format("{\"type\":%d, \"ec\":0}", eWhiteBoardNotify_Local_Leave, self.room_name)
  self:report_data(data)
end

function WhiteBoard:request_named_svr_for_msg_svc()
  local ret = false
  self.named_conn = Http:open()
  if (self.named_conn) then
    local raw_str = string.format("stream_name=%s&uid=%s&sid=%s&tok=%s", self.room_name_compat, g_sys_info.device_id, g_sys_info.session_id, self.token)
    local encrypted_str = BeeEncrypt(raw_str, g_sys_info.param_key)
    local url = string.format("%s/%s", self.named_svr, encrypted_str)
    LogInfo("Requesting named svr %s for white board room id.", url)
    ret = self.named_conn:async_get(url, {oneoff=0}, handler(self, self.on_request_named_svr_for_msg_svc))      
  else
    LogError("Named svr http conn open fail.")  
  end
  return ret
end

function WhiteBoard:on_request_named_svr_for_msg_svc(ec1, ec2, msg, rsp)
  LogInfo("Named svr return ec1:%d ec2:%d msg:%s rsp %s for board svc.", ec1, ec2, msg, rsp["http_body"])
  assert(ec1 == 0, "on_request_named_svr_for_msg_svc fail "..ec1)
  if (ec1 == 0) then
    local decrypted_data = BeeDecrypt(rsp["http_body"], g_sys_info.param_key)
    local json_data = cjson.decode(decrypted_data)
    self.room_name_internal = json_data.stream_name
    self:request_msg_svc() 
  end
end

function WhiteBoard:request_msg_svc()
  LogInfo("Requesting msg svc %s", self.msg_svc_url)
  self.msg_svc_ws:connect(self.msg_svc_url, self.ws_timeout, handler(self, self.handle_msg_svc_connect))  
end

function WhiteBoard:handle_msg_svc_connect(res, data)
  if (res) then
    LogInfo("Connected to msg svc %s", self.msg_svc_url)
    local msg = nil
    if (self.create) then
      msg = string.format("{\"cmd\":\"create\",\"roomid\":\"%s\",\"uid\":\"%s\"}", self.room_name_internal, self.uid)
    else
      msg = string.format("{\"cmd\":\"join\",\"roomid\":\"%s\",\"cast\":1,\"uid\":\"%s\",\"profile\":{\"uid\":\"%s\",\"type\":\"%s\",\"outinfo\":{\"name\":\"%s\"}}}", self.room_name_internal, self.uid, self.uid, self.role_alias, self.nick_name)
    end
    self.msg_svc_ws:write(msg)
  else
    LogError("Failed to connect msg svc %s %s", self.msg_svc_url, data)    
    self:handle_msg_svc_error(data)
  end
end

function WhiteBoard:handle_msg_svc_createback(data)
  local msg = string.format("{\"cmd\":\"join\",\"roomid\":\"%s\",\"cast\":1,\"uid\":\"%s\",\"profile\":{\"uid\":\"%s\",\"type\":\"%s\",\"outinfo\":{\"name\":\"%s\"}}}", self.room_name_internal, self.uid, self.uid, self.role_alias, self.nick_name)
  self.msg_svc_ws:write(msg)
end

function WhiteBoard:handle_msg_svc_joinback(cls_data)
  local data = string.format("{\"type\":%d, \"ec\":0, \"room_name\":\"%s\"}", eWhiteBoardNotify_Local_Join, self.room_name)
  self:report_data(data)
  if (cls_data) then
    self.msg_svc_keepalive_timer = Timer:open(30000, -1, handler(self, self.handle_msg_svc_timeout))
    self.joined = true
  end
end

function WhiteBoard:handle_msg_svc_listback(data)
  local list = data.list
  if (list == nil and #list == 0) then
    return
  end
  
  local current_members = {}
  
  for i = 1, #list do
    if (list[i] and list[i].uid and list[i].uid ~= self.uid) then
      local member = {}
      member.uid = list[i].uid
      if (list[i].outinfo ~= nil and list[i].outinfo.name ~= nil) then
        member.nick_name = list[i].outinfo.name
      end
      if (list[i].type ~= nil) then
        member.role_alias = list[i].type
        if (list[i].type == "tech") then
          member.role = eBeeWhiteBoardRole_Teacher
        elseif (list[i].type == "stu") then
          member.role = eBeeWhiteBoardRole_Student
        else
          member.role = eBeeWhiteBoardRole_None
        end
      end

      current_members[member.uid] = member.uid
      if (self.member_table[member.uid] == nil) then
        LogInfo("White board member uid %s role %d joined", member.uid, member.role)
        local data = string.format("{\"type\":%d,\"uid\":\"%s\",\"nick_name\":\"%s\",\"role\":%d}", eWhiteBoardNotify_Remote_Join, member.uid, member.nick_name, member.role)
        self:report_data(data)
        table.insert(self.member_table, member)        
      end   
    end
  end
  
  local i = 1
  while i <= #self.member_table do
    local member = self.member_table[i]
    if (current_members[member.uid] == nil) then
      LogInfo("White board member uid %s leaved", member.uid)      
      local data = string.format("{\"type\":%d,\"uid\":\"%s\"}", eWhiteBoardNotify_Remote_Leave, member.uid)
      self:report_data(data)      
      table.remove(self.member_table, i)
    else
      i = i + 1
    end
  end
end

function WhiteBoard:handle_msg_svc_receiveback(from, msg)
  local data = string.format("{\"type\":%d,\"ec\":0,\"room_name\":\"%s\",\"from\":\"%s\",\"message\":%s}", eWhiteBoardNotify_Message, self.room_name, from, msg)
  self:report_data(data)
end

function WhiteBoard:handle_msg_svc_data(data)
  LogInfo("Rcv white board svc data: %s",data)
  local json_data = cjson.decode(data)
  local exist_str = string.format("Room:%s exist", self.room_name)

  if (json_data.result ~= nil and json_data.result ~= 1 and json_data.reason ~= nil and string.find(json_data.reason, exist_str) == nil) then
    local error_msg = string.format("white board svc %s error %s", json_data.cmd, json_data.reason)
    self:handle_msg_svc_error(error_msg)
    return
  end
  
  if (json_data.cmd == "createback") then
    self:handle_msg_svc_createback(json_data)
  elseif (json_data.cmd == "joinback") then
    self:handle_msg_svc_joinback(json_data)
  elseif (json_data.cmd == "listback") then
    self:handle_msg_svc_listback(json_data)
  elseif (json_data.cmd == "receiveback") then
    self:handle_msg_svc_receiveback(json_data.senduid, data)
  end
end

function WhiteBoard:handle_msg_svc_error(data)
  LogInfo("Board svc error: %s",data)
  if (self.joined == false) then
    local data = string.format("{\"type\":%d,\"ec\":38,\"room_name\":\"%s\",\"msg\":\"%s\"}", eWhiteBoardNotify_Local_Join, self.room_name, data)
    self:report_data(data)
  else
    local data = string.format("{\"type\":%d,\"ec\":16,\"room_name\":\"%s\",\"msg\":\"%s\"}", eWhiteBoardNotify_Local_Leave, self.room_name, data)
    self:report_data(data)
    self.joined = false
  end
end

function WhiteBoard:handle_msg_svc_timeout()
  local msg = string.format("{\"cmd\":\"heart\",\"roomid\":\"%s\", \"uid\":\"%s\"}", self.room_name_internal, self.uid)
  self.msg_svc_ws:write(msg)
end

function WhiteBoard:handle_msg_svc_list_timeout()
  if (self.msg_svc_ws ~= nil) then
    local msg = string.format("{\"cmd\":\"list\",\"roomid\":\"%s\", \"uid\":\"%s\"}", self.room_name_internal, self.uid)
    self.msg_svc_ws:write(msg)
  end
end

function WhiteBoard:report_data(data)
  if (data ~= nil) then
    sdk_SvcNotify(eBeeSvcType_Board, data)
  end
end

--Export interface.
local active_svc = {
    [eBeeSvcType_VideoRoom] = VideoRoom:new(),
    [eBeeSvcType_Board] = WhiteBoard:new(),
    [eBeeSvcType_Chat] = nil
}

function GetCapability()
  local capability = {}
  for svc_code, svc in pairs(active_svc) do
    if (svc ~= nil) then
      local svc_info = {}
      svc_info.svc = svc_code
      svc_info.desc = svc.desc
      table.insert(capability, svc_info)
    end
  end
  return capability
end

function GetJsonData(args)
  assert(args, "args nil")
  local json_data = cjson.decode(args)
  assert(json_data, "json_data nil")
  return json_data
end

function Join(svc, args)
  LogInfo("Join(%d, %s)", svc, args)  
  local svc = active_svc[svc]
  assert(svc, "Svc not available")
  svc:join(GetJsonData(args))
end

function Leave(svc, args)
  LogInfo("Leave(%d, %s)", svc, args)  
  local svc = active_svc[svc]
  assert(svc, "Svc not available")
  svc:leave(GetJsonData(args))
end

function ConnectStream(svc, args)
  LogInfo("ConnectStream(%d, %s)", svc, args)  
  local svc = active_svc[svc]
  assert(svc, "Svc not available")
  svc:connect_stream(GetJsonData(args))
end

function DisconnectStream(svc, args)
  LogInfo("DisconnectStream(%d, %s)", svc, args)  
  local svc = active_svc[svc]
  assert(svc, "Svc not available")
  svc:disconnect_stream(GetJsonData(args))
end

function SendMessage(svc, args)
  LogInfo("SendMessage(%d, %s)", svc, args)  
  local svc = active_svc[svc]
  assert(svc, "Svc not available")
  svc:send_message(GetJsonData(args))
end

function OnLuaSessionStart()
  math.randomseed(os.clock()*100000000000)
  math.random(1,62)
  math.random(1,62)
  math.random(1,62)
  math.random(1,62)
  math.random(1,62)
end

OnLuaSessionStart()
