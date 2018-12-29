--SOHU MEDIA SDK LUA SCRIPT--
--Copyright@sohu-inc

local video_cache = require "videocache"
local cjson       = require "cjson"
local crypto      = require "crypto"
local iobuffer    = require "iobuffer"
local http        = require "http"
local ws          = require "lws"
local webrtc      = require "webrtc"
local timer       = require "timer"

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
  o.handle = http.open()
  return o
end

function Http:close()
  if (self.handle ~= nil) then
    http.close(self.handle)
    self.handle = nil
  end
end

function Http:set_resolve_callback(cb)
  if (self.handle ~= nil) then
    http.set_callback(self.handle, eHttpCallback_Resolve, cb)
  end
end

function Http:set_connect_callback(cb)
  if (self.handle ~= nil) then
    http.set_callback(self.handle, eHttpCallback_Connect, cb)
  end
end

function Http:set_request_callback(cb)
  if (self.handle ~= nil) then
    http.set_callback(self.handle, eHttpCallback_Request, cb)
  end
end

function Http:set_header_callback(cb)
  if (self.handle ~= nil) then
    http.set_callback(self.handle, eHttpCallback_Header, cb)
  end
end

function Http:set_redirect_callback(cb)
  if (self.handle ~= nil) then
    http.set_callback(self.handle, eHttpCallback_Redirect, cb)
  end
end

function Http:set_body_callback(cb)
  if (self.handle ~= nil) then
    http.set_callback(self.handle, eHttpCallback_Body, cb)
  end
end

function Http:get(url, option)
  if (self.handle ~= nil) then
    return http.get(self.handle, url, option)
  end
end

function Http:async_get(url, option, cb)
  if (self.handle ~= nil) then
    return http.async_get(self.handle, url, option, cb)
  end
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
    ws.start(protocols)
end

function Websocket:open(protocol, check_json_completed, read_cb, error_cb)
  local o = {}
  setmetatable(o, self)
  o.handle = ws.open(protocol, check_json_completed, read_cb, error_cb)
  return o
end

function Websocket:close()
  if (self.handle ~= nil) then
    ws.close(self.handle)
    self.handle = nil
  end
end

function Websocket:connect(url, timeout, connect_cb)
  if (self.handle ~= nil) then
    return ws.connect(self.handle, url, timeout, connect_cb)
  end
end

function Websocket:write(data)
  if (self.handle ~= nil) then
    LogInfo("Send WSMSG: %s",data)
    return ws.write(self.handle, data)
  end
end

--Webrtc Wrapper
Webrtc = {
  handle = nil
}

Webrtc.__index = Webrtc

function Webrtc:open(p2p_svr, internal_audio_source, channels, sample_rate, sample_size, callbacks)
  local o = {}
  setmetatable(o, self)
  o.handle = webrtc.open(p2p_svr, internal_audio_source, channels, sample_rate, sample_size, callbacks)
  return o
end

function Webrtc:close()
  if (self.handle ~= nil) then
    webrtc.close(self.handle)
    self.handle = nil
  end
end

function Webrtc:set_video_source(internal, width, height, fps, capturer_index)
  if (self.handle ~= nil) then
    return webrtc.set_video_source(self.handle, internal, width, height, fps, capturer_index)
  end
end

function Webrtc:create_offer(push_audio, push_video, callback)
  if (self.handle ~= nil) then
    return webrtc.create_offer(self.handle, push_audio, push_video, callback)
  end
end

function Webrtc:set_remote_desc(jsep)
  if (self.handle ~= nil) then
    return webrtc.set_remote_desc(self.handle, jsep)
  end    
end

function Webrtc:create_answer(offer, callback)
  if (self.handle ~= nil) then
    return webrtc.create_answer(self.handle, offer, callback)
  end
end

function Webrtc:start_video_render(cb, opaque)
  if (self.handle ~= nil) then
    return webrtc.start_video_render(self.handle, cb, opaque)
  end
end

function Webrtc:stop_video_render()
  if (self.handle ~= nil) then
    return webrtc.stop_video_render(self.handle)
  end
end

function Webrtc:get_stats(obsvr)
  if (self.handle ~= nil) then
    return webrtc.get_stats(self.handle, obsvr)
  end
end

function Webrtc:enable_tracing(level)
	webrtc.enable_tracing(level)
end

--Timer Wrapper
Timer = {
  handle = nil
}

Timer.__index = Timer

function Timer:open(interval, timeout_count, cb)
  local o = {}
  setmetatable(o, self)
  o.handle = timer.open(interval, timeout_count, cb)
  return o
end

function Timer:close()
  if (self.handle ~= nil) then
    timer.close(self.handle)
    self.handle = nil
  end
end

--Platform Type
local ePlatformType_IOS = 0
local ePlatformType_ANDROID = 1
local ePlatformType_IPAD = 2
local ePlatformType_APAD = 3 
local ePlatformType_IH5 = 4
local ePlatformType_AH5 = 5
local ePlatformType_PCH5 = 6
local ePlatformType_PC = 7

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
  param_key = "d^#Gj9&$Eai@1u*D"
}

function DrmLog(level,fmt, ...)
  local debug_info = debug.getinfo(3)
  sdk_BeeLog(level, debug_info.source, debug_info.currentline, fmt:format(...))
end

function LogError(fmt, ...)
  DrmLog(0, fmt, ...)
end

function LogWarn(fmt, ...)
  DrmLog(1, fmt, ...)
end

function LogInfo(fmt, ...)
  DrmLog(2, fmt, ...)
end

function LogDebug(fmt, ...)
  DrmLog(3, fmt, ...)
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

function SetSysInfo(pt,nt,app_name,app_ver,bee_ver,sys_info,device_id)
  g_sys_info.platform = pt
  g_sys_info.net_type = nt
  g_sys_info.app_name = app_name
  g_sys_info.app_ver = app_ver
  g_sys_info.bee_ver = bee_ver
  g_sys_info.sys_info = sys_info
  g_sys_info.device_id = device_id
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

local eVideoConfNotify_Local_Join = 0
local eVideoConfNotify_Remote_Join = 1
local eVideoConfNotify_Local_Leaved = 2
local eVideoConfNotify_Remote_Leaved = 3
local eVideoConfNotify_Closed = 4
local eVideoConfNotify_Local_Slow_Link = 5
local eVideoConfNotify_Remote_Slow_Link = 6
local eVideoConfNotify_Remote_Members = 7

--Video Room Party
local ePartyType_Local = 0
local ePartyType_Remote = 1

Party = {
  video_room = nil,
  type = ePartyType_Local,
  party_index=-1,
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
  p2p_svr="stun:stun.p2p.hd.sohu.com:4478",
  protocol="janus-protocol",
  webrtc_callbacks = nil,
  stream_name=nil,
  janus_keepalive_timer=nil,
  leave_timer=nil,
  ws_timeout=2000,
  push_video=true,
  internal_video_source=true,
  width=640,
  height=480,
  fps = 30,
  push_audio=true,
  internal_audio_source=true,
  channels=2,
  sample_rate=48000,
  sample_size=2,
  extra_data=nil,
  pull_audio=true,
  pull_video=true,
  max_bitrate=512000,
  min_bitrate=256000,
  stats_timer=nil,
  stats_timeout=1000
}

Party.__index = Party

function Party:new(video_room, type, party_index)
  local o = {}
  setmetatable(o, self)
  o.video_room = video_room
  o.type = type
  o.party_index = party_index
  o.masterd_svr = video_room.masterd_svr
  o.room_name = video_room.room_name
  o.janus_room_name = video_room.janus_room_name
  o.callbacks = {
    on_ice_candidate=handler(o, o.on_ice_candidate),
    on_ice_gathering_change=handler(o, o.on_ice_gathering_change),
    on_media_ready=handler(o, o.on_media_ready)
  }
  LogInfo("[index:%d] New party type %d.", party_index, type)
  return o
end

function Party:push(named_svr, push_video, internal_video_source, width, height, fps, push_audio, internal_audio_source, channels, sample_rate, sample_size, extra_data)
  if (self.type == ePartyType_Local) then
    self.push_video = push_video
    self.internal_video_source = internal_video_source
    self.width = width
    self.height = height
    self.fps = fps
    self.push_audio = push_audio
    self.internal_audio_source = internal_audio_source
    self.channels = channels
    self.sample_rate = sample_rate
    self.sample_size = sample_size
    self.extra_data = extra_data
    return self:request_named_svr(named_svr)
  else
    LogError("[index:%d] Party is not push party.", self.party_index)
    return false
  end
end

function Party:pull(stream_name, pull_audio, pull_video)
  if (self.type == ePartyType_Remote) then
    LogInfo("Pulling stream %s", stream_name)
    self.pull_audio = pull_audio
    self.pull_video = pull_video
    self:request_masterd_svr(stream_name)
  else
    LogError("[index:%d] Party is not pull party.", self.party_index)
  end
end

function Party:request_named_svr(named_svr)
  local ret = false
  if (named_svr) then
    self.named_svr = named_svr
    self.named_conn = Http:open()
    if (self.named_conn) then  
      local push_key_encoded = sdk_UrlEncode(self.video_room.named_push_key)
      local url = string.format("%s/%s?uid=%d", self.named_svr, push_key_encoded, g_sys_info.device_id)
      LogInfo("[index:%d] Requesting named svr %s.", self.party_index, url)
      self.named_conn:async_get(url, {oneoff=0}, handler(self, self.on_request_named_svr))
      ret = true
    else
      LogError("[index:%d] Named svr http conn open fail.", self.party_index)  
    end
  else
    LogError("[index:%d] Invalid named svr url.", self.party_index)
  end
  return ret    
end

function Party:on_request_named_svr(ec1, ec2, msg, rsp)
  LogInfo("[index:%d] Named svr return ec1:%d ec2:%d msg:%s.", self.party_index, ec1, ec2, msg)
  if (ec1 == 0) then
    local json_data = cjson.decode(rsp["http_body"])
    self.stream_name = json_data.stream_name
    local scheduled_svr = json_data.sche_url
    LogInfo("[index:%d][stream:%s] Named svr return status code %d scheduled svr %s.", self.party_index, self.stream_name, rsp.status_code, scheduled_svr)
    self:request_scheduled_svr(scheduled_svr)
  else
    if (rsp) then
      LogError("[index:%d][stream:%s] Named svr return status code %d.", self.party_index, self.stream_name, rsp.status_code)
    end
    self.video_room:on_party_create(false, self)
  end
end

function Party:request_masterd_svr(stream_name)
  local ret = false
  if (stream_name) then
    self.stream_name = stream_name
    self.masterd_conn = Http:open()
    if (self.masterd_conn) then      
      local url = self:create_masterd_url(stream_name, "rtc")
      LogInfo("[index:%d][stream:%s] Requesting masterd svr %s.", self.party_index, self.stream_name, url)
      self.masterd_conn:async_get(url, {}, handler(self, self.on_request_masterd_svr))
      ret = true
    else
      LogError("[index:%d][stream:%s] Masterd svr http conn open fail.", self.party_index, self.stream_name)  
    end
  else
    LogError("[index:%d] Invalid stream name.", self.party_index)
  end
  if (ret == false) then
    self.video_room:on_party_create(false, self)   
  end
end

function Party:on_request_masterd_svr(ec1, ec2, msg, rsp)
  LogInfo("[index:%d][stream:%s] Masterd svr return ec1:%d ec2:%d msg:%s.", self.party_index, self.stream_name, ec1, ec2, msg)
  if (ec1 == 0) then
    local dec_body = DecodeParams(rsp["http_body"], g_sys_info.param_key)
    local json_data = cjson.decode(dec_body)
    local scheduled_svr = json_data.sche_url
    LogInfo("[index:%d][stream:%s] Masterd svr return status code %d scheduled svr %s.", self.party_index, self.stream_name, rsp.status_code, scheduled_svr)
    self:request_scheduled_svr(scheduled_svr)
  else
    if (rsp) then
      LogError("[index:%d][stream:%s] Masterd svr return status code %d.", self.party_index, self.stream_name, rsp.status_code)
    end
    self.video_room:on_party_create(false, self)
  end
end

function Party:request_scheduled_svr(scheduled_svr)
  local ret = false
  if (scheduled_svr) then
    self.scheduled_svr = scheduled_svr
    self.scheduled_conn = Http:open()
    if (self.scheduled_conn) then
      LogInfo("[index:%d][stream:%s] Requesting scheduled svr %s.", self.party_index, self.stream_name, self.scheduled_svr)
      self.scheduled_conn:async_get(scheduled_svr, {}, handler(self, self.on_request_scheduled_svr))
      ret = true
    else
      LogError("[index:%d][stream:%s] Scheduled svr http conn open fail.", self.party_index, self.stream_name)
    end
  else
    LogError("[index:%d][stream:%s] Invalid Scheduled svr url.", self.party_index, self.stream_name)  
  end
  if (ret == false) then
    self.video_room:on_party_create(false, self)
  end
end

function Party:on_request_scheduled_svr(ec1, ec2, msg, rsp)
  LogInfo("[index:%d][stream:%s] Scheduled svr %s return ec1:%d ec2:%d msg:%s.", self.party_index, self.stream_name, self.scheduled_svr, ec1, ec2, msg)
  if (ec1 == 0) then
    local json_data = cjson.decode(rsp["http_body"])  
    local code = json_data.code
    if (code == 1) then
      local janus_svr = json_data.info.webrtc_url
      LogInfo("[index:%d][stream:%s] Scheduled svr return status code %d janus svr %s.", self.party_index, self.stream_name, rsp.status_code, janus_svr)      
      self:connect_janus_svr(janus_svr)      
    else
      if (rsp) then
        LogError("[index:%d][stream:%s] Scheduled svr return status code %d error code %d.", self.party_index, self.stream_name, rsp.status_code, code)   
      end
      self.video_room:on_party_create(false, self)
    end
  else
    if (rsp) then
      LogError("[index:%d][stream:%s] Scheduled svr return status code %d.", self.party_index, self.stream_name, rsp.status_code)   
    end
    self.video_room:on_party_create(false, self)
  end
end

function Party:connect_janus_svr(janus_svr)
  local ret = false    
  if (janus_svr) then
    self.janus_svr = janus_svr
    self.janus_ws = Websocket:open(self.protocol, true, handler(self.video_room, self.video_room.handle_janus_data), handler(self.video_room, self.video_room.handle_janus_error))
    if (self.janus_ws) then
      LogInfo("[index:%d][stream:%s] Requesting janus svr %s.", self.party_index, self.stream_name, self.janus_svr)
      self.janus_ws:connect(self.janus_svr, self.ws_timeout, handler(self, self.on_janus_connect_result))
      ret = true
    else
      LogError("[index:%d][stream:%s] Party janus ws create fail.", self.party_index, self.stream_name)
    end
  else
    LogError("[index:%d][stream:%s] Invalid janus svr url.", self.party_index, self.stream_name)
  end
  if (ret == false) then
    self.video_room:on_party_create(false, self)
  end
end

function Party:on_janus_connect_result(res, data)
  if (res) then
    LogInfo("[index:%d][stream:%s] Party connected to janus svr %s.", self.party_index, self.stream_name, self.janus_svr)
    self:create_session()
  else
    LogError("[index:%d][stream:%s] Party connect to janus svr %s failed %s.", self.party_index, self.stream_name, self.janus_svr, data)
    self.video_room:on_party_create(false, self)
  end
end

function Party:create_session()
  if (self.janus_ws) then
    LogInfo("[index:%d][stream:%s] Creating session", self.party_index, self.stream_name)
    local transaction = self.video_room:create_transaction(true, handler(self, self.on_create_session))
    local msg = string.format("{\"janus\":\"create\",\"transaction\":\"%s\"}",transaction.id)
    self.janus_ws:write(msg)
  else
    LogError("[index:%d][stream:%s] Create session fail, ws not created.", self.party_index, self.stream_name)
    self.video_room:on_party_create(false, self)
  end
end

function Party:on_create_session(success, json_data)
  if (success == false) then
    LogError("[index:%d][stream:%s] Create session fail.", self.party_index, self.stream_name)
    self.video_room:on_party_create(false, self)
  else
    self.session_id = json_data.data.id
    local log = string.format("[index:%d][stream:%s] Create session success %d", self.party_index, self.stream_name, self.session_id)
    LogInfo("%s", log)
    self:create_handle()
  end
end

function Party:create_handle()
  if (self.janus_ws) then
    LogInfo("[index:%d][stream:%s] Creating handle", self.party_index, self.stream_name)
    local transaction = self.video_room:create_transaction(true, handler(self, self.on_create_handle))
    local opaque_id = string.format("videoroomtest-%s", random_string(12))
    local msg = string.format("{\"janus\":\"attach\",\"session_id\":%d,\"plugin\":\"janus.plugin.videoroom\",\"transaction\":\"%s\",\"opaque_id\":\"%s\", \"force-bundle\":true, \"force-rtcp-mux\":true}", self.session_id, transaction.id, opaque_id)
    self.janus_ws:write(msg)
  else
    LogError("[index:%d][stream:%s] Create handle fail, ws not created.", self.party_index, self.stream_name)
    self.video_room:on_party_create(false, self)
  end    
end

function Party:on_create_handle(success, json_data)
  if (success == false) then
    LogError("[index:%d][stream:%s] Create handle fail.", self.party_index, self.stream_name)
    self.video_room:on_party_create(false, self)
  else
    self.handle = json_data.data.id
    self.video_room:on_party_handle_created(self.handle, self)
    
    local log = string.format("[index:%d][stream:%s] Create handle success %d.", self.party_index, self.stream_name, self.handle)
    LogInfo("%s", log)
    if (self.type == ePartyType_Local) then
      self:join()
    else
      self:listen()
    end
  end
end

function Party:join()
  local transaction = self.video_room:create_transaction(true, handler(self, self.on_joined))
  local msg = string.format("{\"janus\":\"message\",\"session_id\":%d,\"handle_id\":%d,\"transaction\":\"%s\",\"body\":{\"request\":\"join\",\"room_name\":\"%s\",\"ptype\":\"publisher\",\"stream_name\":\"%s\"}}", self.session_id, self.handle, transaction.id, self.janus_room_name, self.stream_name)
  self.janus_ws:write(msg)
end

function Party:on_joined(dummy, json_data)
  if (json_data.plugindata.data.videoroom == "joined") then
    LogInfo("[index:%d][stream:%s] Join success.", self.party_index, self.stream_name)
    self.janus_keepalive_timer = Timer:open(5000, -1, handler(self, self.handle_janus_timeout))
    self:publish_local_feed()
  else
    LogError("[index:%d][stream:%s] Join failed.", self.party_index, self.stream_name)
    self.video_room:on_party_create(false, self)
  end
end

function Party:publish_local_feed()
  self.rtc_conn = Webrtc:open(self.p2p_svr, self.internal_audio_source, self.channels, self.sample_rate, self.sample_size, self.callbacks)
  if (self.rtc_conn ~= nil) then
    self.rtc_conn:set_video_source(self.internal_video_source, self.width, self.height, self.fps, 0)     
    self.rtc_conn:create_offer(self.push_audio, self.push_video, handler(self, self.on_create_offer))
  else
    LogError("[index:%d][stream:%s] Create offer failed, webrtc peerconnection open fail.", self.party_index, self.stream_name)
    self.video_room:on_party_create(false, self)
  end
end

function Party:leave()
  --Stop media first.
  if (self.rtc_conn ~= nil) then
    self.rtc_conn:close()
    self.rtc_conn = nil
  end
  
  --Stop signal, should wait for result to ensure websocket msg send/rcv completed.
  local transaction = self.video_room:create_transaction(true, nil)
  local msg = string.format("{\"janus\":\"detach\",\"session_id\":%d,\"handle_id\":%d,\"transaction\":\"%s\"}", self.session_id, self.handle, transaction.id)
  self.janus_ws:write(msg)
  self.leave_timer = Timer:open(1000, 1, handler(self, self.handle_leave_timeout))
end

function Party:handle_leave_timeout()
  LogInfo("[index:%d][stream:%s] Party leave timeout, closing", self.party_index, self.stream_name)
  self:close()
end

function Party:listen()
  local transaction = self.video_room:create_transaction(true, handler(self, self.on_listened))
  local body = string.format("{\"request\":\"join\",\"room_name\":\"%s\",\"ptype\":\"listener\",\"feed_name\":\"%s\", \"audio\":%s, \"video\":%s}", self.janus_room_name, self.stream_name, tostring(self.pull_audio), tostring(self.pull_video))
  local msg = string.format("{\"janus\":\"message\",\"session_id\":%d,\"handle_id\":%d,\"transaction\":\"%s\",\"body\":%s}", self.session_id, self.handle, transaction.id, body)
  self.janus_ws:write(msg)
end

function Party:on_listened(dummy, json_data)
  local success = false
  if (json_data.plugindata.data.videoroom == "attached") then    
    local jsep = json_data.jsep
    if (jsep ~= nil) then
      local sdp = cjson.encode(jsep)
      self.rtc_conn = Webrtc:open(self.p2p_svr, false, 0, 0, 0, self.callbacks)
      self.rtc_conn:create_answer(sdp, handler(self, self.on_create_answer))
      self.janus_keepalive_timer = Timer:open(5000, -1, handler(self, self.handle_janus_timeout))
      success = true
    end
  end
  if (success == false) then
    self.video_room:on_party_create(false, self)
  end
end

function Party:handle_janus_timeout()
  local transaction = self.video_room:create_transaction(false, nil)
  local msg = string.format("{\"janus\":\"keepalive\",\"session_id\":%d, \"transaction\":\"%s\"}", self.session_id, transaction.id)
  self.janus_ws:write(msg)
end

function Party:create_masterd_url(stream_name, stream_format)
  local args = string.format("stream_name=%s&format=%s&p=%d&uid=%s", stream_name, stream_format, g_sys_info.platform, g_sys_info.device_id)
  local enc_args = EncodeParams(args, g_sys_info.param_key)
  local url = string.format("%s/%s", self.masterd_svr, enc_args) 
  return url
end

function Party:on_ice_candidate(candidate)
  self:send_candidate(candidate)
end

function Party:on_ice_gathering_change(new_state)
  if (new_state == eIceGathering_Completed) then
    local completed_msg = string.format("{\"completed\": true}")
    self:send_candidate(completed_msg)
  end
end

function Party:send_local_offer(jsep, cb)
  if (self.janus_ws ~= nil) then
    local transaction = self.video_room:create_transaction(true, cb)
    local body = string.format("{ \"request\":\"configure\",\"audio\":%s,\"video\":%s,\"max_bitrate\":%d,\"min_bitrate\":%d}", tostring(self.push_audio), tostring(self.push_video), self.max_bitrate, self.min_bitrate)
    local msg = string.format("{\"janus\":\"message\",\"session_id\":%d,\"handle_id\":%d,\"transaction\":\"%s\",\"body\":%s,\"jsep\":%s }", self.session_id, self.handle, transaction.id, body, jsep)
    self.janus_ws:write(msg)
  end
end

function Party:send_remote_answer(jsep, cb)
  if (self.janus_ws ~= nil) then
    local transaction = self.video_room:create_transaction(true, cb)
    local body = string.format("{ \"request\": \"start\", \"room_name\" : \"%s\" }", self.janus_room_name)
    local msg = string.format("{\"janus\":\"message\",\"session_id\":%d,\"handle_id\":%d,\"transaction\":\"%s\",\"body\":%s,\"jsep\":%s }", self.session_id, self.handle, transaction.id, body, jsep)
    self.janus_ws:write(msg)
  end 
end

function Party:on_offer_answered(dummy, json_data)
  local jsep = json_data.jsep    
  if (jsep ~= nil and jsep ~= "") then
    local sdp = cjson.encode(jsep)
    self.rtc_conn:set_remote_desc(sdp)
  else
    LogError("[index:%d][stream:%s] Remote answer offer empty.", self.party_index, self.stream_name)
    self.video_room:on_party_create(false, self) 
  end
end

function Party:send_candidate(candidate)
  local transaction = self.video_room:create_transaction(false, nil)    
  local msg = string.format("{\"janus\":\"trickle\",\"session_id\":%d,\"handle_id\":%d,\"transaction\":\"%s\",\"candidate\":%s }", self.session_id, self.handle, transaction.id, candidate)
  self.janus_ws:write(msg)
end

function Party:on_create_offer(local_jsep)
  if (local_jsep ~= nil and local_jsep ~= "") then
    self:send_local_offer(local_jsep, handler(self, self.on_offer_answered))
  else
    LogError("[index:%d][stream:%s] Create local offer fail.", self.party_index, self.stream_name)
    self.video_room:on_party_create(false, self)
  end
end

function Party:on_answer_acked(dummy, json_data)
  if (json_data ~= nil and json_data.plugindata ~= nil and json_data.plugindata.data ~= nil and json_data.plugindata.data.started == "ok") then
    LogInfo("[index:%d][stream:%s] Party listen success.", self.party_index, self.stream_name)
  else
    LogError("[index:%d][stream:%s] Party listen fail.", self.party_index, self.stream_name)
    self.video_room:on_party_create(false, self)
  end  
end

function Party:on_create_answer(jsep)
  if (jsep ~= nil and jsep ~= "") then
    self:send_remote_answer(jsep, handler(self, self.on_answer_acked))
  else
    LogError("[index:%d][stream:%s] Create answer fail.", self.party_index, self.stream_name)
  end
end

function Party:on_webrtcup()
  LogInfo("[index:%d][stream:%s] Party signal ready.", self.party_index, self.stream_name)
  self.signal_ready = true
  self:check_channel_ready()
end

function Party:on_media_ready()
  LogInfo("[index:%d][stream:%s] Party media ready.", self.party_index, self.stream_name)
  self.media_ready = true
  self:check_channel_ready()
end

function Party:on_hangup()
  LogInfo("[index:%d][stream:%s] Party hangup, closing.", self.party_index, self.stream_name)
  self:close()
end

function Party:check_channel_ready()
  if (self.signal_ready and self.media_ready) then
    local data = nil
    if (self.type == ePartyType_Local) then
      data = string.format("{\"type\":%d, \"ec\":0, \"room_name\":\"%s\", \"stream_name\":\"%s\"}", eVideoConfNotify_Local_Join, self.room_name, self.stream_name)
      self.stats_timer = Timer:open(self.stats_timeout, -1, handler(self, self.handle_stats_timeout))
      self.video_room:on_party_create(true, self)
    else
      data = string.format("{\"type\":%d, \"ec\":0, \"room_name\":\"%s\", \"stream_name\":\"%s\"}", eVideoConfNotify_Remote_Join, self.room_name, self.stream_name)
    end
    LogInfo("[index:%d][stream:%s] Party type %d all ready.", self.party_index, self.stream_name, self.type)
    sdk_VideoConfNotify(data)
  end
end

function Party:handle_stats_timeout()
  self:get_stats(0)
end

function Party:process_event(json_data)

end

function Party:start_video_render(cb, opaque)
  LogInfo("[index:%d][stream:%s] start_video_render.", self.party_index, self.stream_name)
  if (self.rtc_conn ~= nil) then    
    self.rtc_conn:start_video_render(cb, opaque)
  else
    LogError("[index:%d][stream:%s] rtc_conn is nil.", self.party_index, self.stream_name)
  end
end

function Party:stop_video_render()
  LogInfo("[index:%d][stream:%s] stop_video_render.", self.party_index, self.stream_name)    
  if (self.rtc_conn ~= nil) then
    self.rtc_conn:stop_video_render()
  end
end

function Party:get_stats(obsvr)
  LogInfo("[index:%d][stream:%s] get_stats.", self.party_index, self.stream_name)    
  if (self.rtc_conn ~= nil) then
    self.rtc_conn:get_stats(obsvr)
  end
end

function Party:report_close()
  local type = nil
  if (self.type == ePartyType_Local) then
    type = eVideoConfNotify_Local_Leaved
  else
    type = eVideoConfNotify_Remote_Leaved
  end
  local data = string.format("{\"type\":%d, \"ec\":0, \"room_name\":\"%s\", \"stream_name\":\"%s\"}", type, self.room_name, self.stream_name)
  sdk_VideoConfNotify(data)
end

function Party:close()
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
    self.video_room:on_party_closed(self.handle, self.stream_name)
  end
  self:report_close()
  LogInfo("Party %s closed", self.stream_name)
end

--Video Room

local ws_protocols = {"janus-protocol"}
VideoRoom = {
  --server="wss://10.2.173.67:8989/janus",
  named_svr="https://name.hd.sohu.com/bee_named",
  --named_svr="https://testlive.hd.sohu.com/bee_named",
  masterd_svr="https://switch.hd.sohu.com/bee_switch",
  --masterd_svr="https://testlive.hd.sohu.com/bee_switch",
  roomed_svr="wss://name.hd.sohu.com/roomd",
  --roomed_svr="wss://10.10.88.148/roomd",
  --roomed_svr="wss://name.hd.sohu.com/roomd_test",
  named_push_key="nnnneeee",
  stream_name=nil,
  protocol="janus-protocol",
  roomed_ws=nil,
  room_name=nil,
  janus_room_name="h264",
  local_party=nil,
  transaction_table={},
  handle_party_table={},
  push_stream_party_table={},
  pull_stream_party_table={},
  push_stream_count=0,
  pull_stream_count=0,
  index_parties={},
  party_index=0,
  switch_message={},
  roomed_keepalive_timer = nil,
  stream_format="rtc",
  leaving=false,
  ws_timeout=2000,
  named_conn=nil,
  mastered_conn=nil,
  push_scheduled_conn=nil,
  pull_scheduled_conn=nil,
  loopback=true,
  push_video=true,
  internal_video_source=true,
  width=640,
  height=480,
  fps=30,
  push_audio=true,
  internal_audio_source=true,
  channels=2,
  sample_rate=48000,
  sample_size=2,
  extra_data=nil,
  auto_pull=false
}

VideoRoom.__index = VideoRoom

function VideoRoom:new(room_name, push_video, internal_video_source, width, height, fps, push_audio, internal_audio_source, channels, sample_rate, sample_size, extra_data, auto_pull)
  local o = {}
  setmetatable(o, self)  
  Webrtc:enable_tracing(5)
  Websocket:start(ws_protocols)
  o.roomed_ws = Websocket:open("", true, handler(o, o.handle_roomed_data), handler(o, o.handle_roomed_error))
  o.room_name = room_name
  o.push_video = push_video
  o.internal_video_source = internal_video_source  
  o.width = width
  o.height = height
  o.fps = fps
  o.push_audio = push_audio
  o.internal_audio_source = internal_audio_source
  o.channels = channels
  o.sample_rate = sample_rate
  o.sample_size = sample_size
  o.extra_data = extra_data
  o.auto_pull = auto_pull
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

function VideoRoom:open()
  if (self.push_video or self.push_audio) then   
    local ret = self:push()
    if (ret) then
      sdk_OnVideoConfJoin(0)
    else
      LogError("push fail")
      sdk_OnVideoConfJoin(-1)
    end
  else
    self.stream_name = "RecvOnly"
    self:request_roomed_svr()
    sdk_OnVideoConfJoin(0)
  end
end

function VideoRoom:close()
  self.leaving = true
  if (self.roomed_ws ~= nil) then
    self.roomed_ws:close()
    self.roomed_ws = nil
  end
  if (self.roomed_keepalive_timer ~= nil) then
    self.roomed_keepalive_timer:close()
    self.roomed_keepalive_timer = nil
  end
  for handle, party in pairs(self.handle_party_table) do
    party:leave()
  end
end

function VideoRoom:request_roomed_svr()
  LogInfo("Requesting room svr %s", self.roomed_svr)
  self.roomed_ws:connect(self.roomed_svr, self.ws_timeout, handler(self, self.handle_roomed_connect))  
end

function VideoRoom:push()
  self.party_index = self.party_index + 1
  local party = Party:new(self, ePartyType_Local, self.party_index)  
  self.index_parties[self.party_index] = party
  return party:push(self.named_svr, self.push_video, self.internal_video_source, self.width, self.height, self.fps, self.push_audio, self.internal_audio_source, self.channels, self.sample_rate, self.sample_size, self.extra_data)
end

function VideoRoom:pull(stream_list)
  for i = 1, #stream_list do
    local stream_name = stream_list[i]
    if ((self.loopback or stream_name ~= self.stream_name) and stream_name ~= "RecvOnly") then
      local exist_party = self.pull_stream_party_table[stream_name]
      if (exist_party == nil) then
        self:pull_stream(stream_name, self.pull_video, self.pull_audio)
      end
    end
  end
end

function VideoRoom:pull_stream(stream_name, pull_video, pull_audio)
  self.party_index = self.party_index + 1
  local party = Party:new(self, ePartyType_Remote, self.party_index)  
  self.index_parties[self.party_index] = party
  self.pull_stream_party_table[stream_name] = party
  self.pull_stream_count = self.pull_stream_count + 1
  party:pull(stream_name, pull_audio, pull_video)
end

function VideoRoom:report_members(stream_list)
  local data = string.format("{\"type\":%d, \"room_name\":\"%s\", \"members\":[", eVideoConfNotify_Remote_Members, self.room_name)
  local count = 0
  for i = 1, #stream_list do
    local stream_name = stream_list[i]
    if ((self.loopback or stream_name ~= self.stream_name) and stream_name ~= "RecvOnly") then
      local exist_party = self.pull_stream_party_table[stream_name]
      if (exist_party == nil) then
        if (count > 0) then
          data = data .. ","
        end
        local member = string.format("{\"stream_name\":\"%s\", \"video\":true, \"audio\":true}", stream_name)
        data = data .. member
        count = count + 1
      end
    end
  end
  data = data .. "]}"
  sdk_VideoConfNotify(data)
end

function VideoRoom:on_party_create(res, party)
  if (party and party.type == ePartyType_Local) then
    if (res) then
      self.stream_name = party.stream_name
      self.push_stream_party_table[party.stream_name] = party
      self.push_stream_count = self.push_stream_count + 1
      self:request_roomed_svr()
    else
      self.index_parties[party.party_index] = nil
      party:close()
    end
  end

  if (party and party.type == ePartyType_Remote) then
    if (res == false) then
      self.pull_stream_party_table[stream_name] = nil
      self.pull_stream_count = self.pull_stream_count - 1
      self.index_parties[party.party_index] = nil
      party:close()
    end     
  end
end

function VideoRoom:on_party_handle_created(handle, party)
  self.handle_party_table[handle] = party
end

function VideoRoom:on_party_closed(handle, stream_name)
  local party = self.handle_party_table[handle]
  if (party ~= nil) then
    self.handle_party_table[handle] = nil
    self.index_parties[party.party_index] = nil
    if (party.type == ePartyType_Local) then
      self.push_stream_party_table[stream_name] = nil
      self.push_stream_count = self.push_stream_count - 1
    else
      self.pull_stream_party_table[stream_name] = nil
      self.pull_stream_count = self.pull_stream_count - 1  
    end
    if (self.leaving and self.push_stream_count == 0 and self.pull_stream_count == 0) then
      sdk_OnVideoConfLeave(0)
    end
  end
end

function VideoRoom:create_transaction(has_rsp, cb)
  local id = random_string(12)
  local transaction = Transaction:new(id, has_rsp, cb)
  self.transaction_table[id] = transaction
  return transaction
end

function VideoRoom:create_session(janus_ws, cb)
  local transaction = self:create_transaction(true, cb)
  local msg = string.format("{\"janus\":\"create\",\"transaction\":\"%s\"}",transaction.id)
  janus_ws:write(msg)
end

function VideoRoom:create_plugin_handle(janus_ws, session_id, cb)
  local transaction = self:create_transaction(true, cb)
  local opaque_id = string.format("videoroomtest-%s", random_string(12))
  local msg = string.format("{\"janus\":\"attach\",\"session_id\":%d,\"plugin\":\"janus.plugin.videoroom\",\"transaction\":\"%s\",\"opaque_id\":\"%s\", \"force-bundle\":true, \"force-rtcp-mux\":true}", session_id, transaction.id, opaque_id)
  janus_ws:write(msg)
end

function VideoRoom:process_transaction_result(success, json_data)
  local transaction = self.transaction_table[json_data.transaction]
  if (transaction == nil) then
    return
  end
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
    self.transaction_table[id] = nil
  else
    if (json_data.plugindata ~= nil and json_data.plugindata.data ~= nil and json_data.plugindata.data.leaving ~= nil) then
      local leaving_stream = json_data.plugindata.data.leaving
      local leaving_push_party = self.push_stream_party_table[leaving_stream]
      if (leaving_push_party ~= nil) then
        leaving_push_party:close()
      end
      local leaving_pull_party = self.pull_stream_party_table[leaving_stream]
      if (leaving_pull_party ~= nil) then
        leaving_pull_party:close()
      end
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
    local data = string.format("{\"type\":%d, \"room_name\":\"%s\", \"stream_name\":\"%s\", \"info\":{\"nack\":%d}}", type, party.room_name, party.stream_name, json_data.nacks)
    sdk_VideoConfNotify(data)
  else
    local log = string.format("No sender %s", json_data)
    LogError("%s", log)
  end
end

function VideoRoom:process_hangup_notify(json_data)
  local info = string.format("Received hangup notify from session %d, handle %d, reason %s.", json_data.session_id, json_data.sender, json_data.reason)
  LogInfo("%s", info)
  local party = self.handle_party_table[json_data.sender]
  if (party ~= nil) then
    party:on_hangup() 
  end 
end

function VideoRoom:process_detached_notify(json_data)
  local info = string.format("Received detach notify from session %d, handle %d.", json_data.session_id, json_data.sender)
  LogInfo("%s", info)
  local party = self.handle_party_table[json_data.sender]
  if (party ~= nil) then
    party:on_hangup() 
  end 
end

function VideoRoom:handle_roomed_timeout()
  local msg = string.format("{\"request\":\"heart\",\"room_name\":\"%s\", \"stream_name\":\"%s\"}", self.room_name, self.stream_name)
  self.roomed_ws:write(msg)
end

function VideoRoom:handle_janus_data(data)
  LogInfo("Rcv WSMSG: %s",data)
  local json_data = cjson.decode(data)
  local janus = json_data.janus
  local f = self.switch_message[janus]
  if (f) then
    f(json_data)
  end
end

function VideoRoom:handle_janus_error(data)
  LogInfo("Rcv err: %s",data)
end

function VideoRoom:handle_roomed_connect(res, data)
  if (res) then
    LogInfo("Connected to roomed svr %s", self.roomed_svr)
    local msg = string.format("{\"request\":\"join\",\"room_name\":\"%s\",\"stream_name\":\"%s\"}", self.room_name, self.stream_name)
    self.roomed_ws:write(msg)    
    self.roomed_keepalive_timer = Timer:open(1000, -1, handler(self, self.handle_roomed_timeout))
  else
    LogError("Failed to connect roomed svr %s %s", self.roomed_svr, data)
  end
end

function VideoRoom:handle_roomed_data(data)
  LogInfo("Rcv ROOMMSG: %s",data)
  local json_data = cjson.decode(data)
  if (json_data.videoroom == "joined" or json_data.videoroom == "event") then
    local list = json_data.list
    if (list ~= nil) then
      stream_list = list:split(":")
      if (stream_list ~= nil) then
        if (self.auto_pull) then
          self:pull(stream_list)
        else
          self:report_members(stream_list)                   
        end
      end
    end
  end
end

function VideoRoom:handle_roomed_error(data)
  LogInfo("Rcv ROOMERR: %s",data)
end

function VideoRoom:start_party_video_render(stream_name, type, cb, opaque)
  LogInfo("Start rendering stream %s type %d", stream_name, type)
  local party = nil
  if (type == ePartyType_Local) then
    party = self.push_stream_party_table[stream_name]
  else
    party = self.pull_stream_party_table[stream_name]
  end
  if (party ~= nil) then
    party:start_video_render(cb, opaque)
  end
end

function VideoRoom:stop_party_video_render(stream_name, type)
  local party = nil
  if (type == ePartyType_Local) then
    party = self.push_stream_party_table[stream_name]
  else
    party = self.pull_stream_party_table[stream_name]
  end
  if (party ~= nil) then
    party:stop_video_render()
  end
end

function VideoRoom:get_stats(stream_name, type, obsvr)
  local party = nil
  if (type == ePartyType_Local) then
    party = self.push_stream_party_table[stream_name]
  else
    party = self.pull_stream_party_table[stream_name]
  end
  if (party ~= nil) then
    party:get_stats(obsvr)
  end
end

function VideoRoom:disconnect_party(stream_name, type)
  local party = nil
  if (type == ePartyType_Local) then
    party = self.push_stream_party_table[stream_name]
    if (party ~= nil) then
      party:leave()
      self.push_stream_party_table[stream_name] = nil
    end
  else
    party = self.pull_stream_party_table[stream_name]
    if (party ~= nil) then
      party:leave()
      self.pull_stream_party_table[stream_name] = nil
    end
  end

end

local video_room = nil

function handle_redirect(code, loc)
  LogInfo("redirect %d to %s", code, loc)
end

function JoinConf(args)
  local json_data = cjson.decode(args)
  local room_name = json_data.room_name

  --Default param.
  local push_audio = true
  local push_video = true
  local pull_audio = true
  local pull_video = true
  local extra_data = nil
  local auto_pull  = false
  local internal_audio_source = true
  local internal_video_source = true
  local width = 640
  local height = 480
  local fps = 30
  local channels = 2
  local sample_rate = 48000
  local sample_size = 2

  --Push properties.
  local push = json_data.push
  if (push ~= nil) then
    if (push.audio ~= nil) then
      if (push.audio.present ~= nil) then
        push_audio = push.audio.present
      end
      if (push.audio.internal ~= nil) then
        internal_audio_source = push.audio.internal
      end
      if (push.audio.channels ~= nil) then
        channels = push.audio.channels
      end
      if (push.audio.sample_rate ~= nil) then
        sample_rate = push.audio.sample_rate
      end
      if (push.audio.sample_size ~= nil) then
        sample_size = push.audio.sample_size
      end    
    end
    if (push.video ~= nil) then
      if (push.video.present ~= nil) then
        push_videoo = push.video.present
      end
      if (push.video.internal ~= nil) then
        internal_video_source = push.video.internal
      end
      if (push.video.width ~= nil) then
        width = push.video.width
      end
      if (push.video.height ~= nil) then
        height = push.video.height
      end
      if (push.video.fps ~= nil) then
        fps = push.video.fps
      end
    end
  end

  --Check for extra_data
  if (json_data.extra ~= nil) then
    extra_data = json_data.extra
  end
  
  --Check for auto_pull  
  if (json_data.auto_pull ~= nil) then
    auto_pull = json_data.auto_pull
  end

  video_room = VideoRoom:new(room_name, push_video, internal_video_source, width, height, fps, push_audio, internal_audio_source, channels, sample_rate, sample_size, extra_data, auto_pull)
  if (video_room ~= nil) then
    LogInfo("Lua create video room success!")
    video_room:open()
  else
    LogError("Lua create video room failed!")
  end
end

function LeaveConf(args)
  local json_data = cjson.decode(args)
  local room_name = json_data.room_name
  if (video_room ~= nil) then
    video_room:close()
    video_room = nil
    LogInfo("LeaveConf success!")
  end
end

function DisconnectConfParty(args)
  local json_data = cjson.decode(args)
  local room_name = json_data.room_name
  local stream_name = json_data.stream_name
  local type = json_data.type
  if (video_room ~= nil) then
    video_room:disconnect_party(stream_name, type)
  else
    LogError("Video room nil when DisconnectConfParty")
  end
end

function StartVideoRender(args)
  LogInfo("StartVideoRender")
  local json_data = cjson.decode(args)
  local room_name = json_data.room_name
  local stream_name = json_data.stream_name
  local type = json_data.type
  local cb = json_data.cb
  local opaque = json_data.opaque
  if (video_room ~= nil) then
    video_room:start_party_video_render(stream_name, type, cb, opaque)
  else
    LogError("Video room nil when StartVideoRender")
  end
end

function StopVideoRender(args)
  local json_data = cjson.decode(args)
  local room_name = json_data.room_name
  local stream_name = json_data.stream_name
  local type = json_data.type
  if (video_room == nil) then
    LogError("Video room nil when StopVideoRender")
  else
    video_room:stop_party_video_render(stream_name, type)
  end
end

function GetStats(args)
  local json_data = cjson.decode(args)
  local room_name = json_data.room_name
  local stream_name = json_data.stream_name
  local type = json_data.type
  local obsvr = json_data.obsvr
  if (video_room == nil) then
    LogError("Video room nil when GetStats")
  else
    video_room:get_stats(stream_name, type, obsvr)
  end
end

function PullStream(args)
  local json_data = cjson.decode(args)
  local room_name = json_data.room_name
  local stream_name = json_data.stream_name
  local pull_video = true
  local pull_audio = true
  local pull = json_data.pull
  if (pull ~= nil) then
    if (pull.video ~= nil) then
      pull_video = pull.video
    end
    if (pull.audio ~= nil) then
      pull_audio = pull.audio   
    end
  end

  if (video_room == nil) then
    LogError("Video room nil when PullStream")
  else
    video_room:pull_stream(stream_name, pull_video, pull_audio)
  end
end

function OnLuaSessionStart()
  math.randomseed(sdk_GetTickGount32())
  math.random(1,100000)
end

OnLuaSessionStart()
