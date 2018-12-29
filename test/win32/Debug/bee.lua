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
local eHttpCallback_Resolve = 0
local eHttpCallback_Connect = 1
local eHttpCallback_Request = 2
local eHttpCallback_Header  = 3
local eHttpCallback_Body    = 4

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

function Websocket:connect(url)
  if (self.handle ~= nil) then
    return ws.connect(self.handle, url)
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

function Webrtc:open(uri, callbacks)
  local o = {}
  setmetatable(o, self)
  o.handle = webrtc.open(uri, callbacks)
  return o
end

function Webrtc:close()
  if (self.handle ~= nil) then
    webrtc.close(self.handle)
    self.handle = nil
  end
end

function Webrtc:create_offer(callback)
  if (self.handle ~= nil) then
    return webrtc.create_offer(self.handle, callback)
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
    return webrtc.stop_video_render()
  end
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

--Global stream info
local g_stream_info = {
  stream_name = "346262",
  --masterd_ip = "61.135.150.31",
  masterd_ip = "192.168.42.15",
  maseterd_port = 80,
  format = "mp4",
  original = 0,
  encrypt = 0,
  encrypt_algorithm = eALGO_AES,
  edged_url = "",
  edged_key="0123456789ABCDEF",
  gslb_key = "",
  gslb_ts = 0,
  cur_frag_size = 0,
  cur_frag_encrypt = false,
  cdn_conn = nil,
  dec_input = nil,
  dec_output = nil
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

function get_live_mode()
  if (g_stream_info.mode == "delay") then
    return eLiveMode_Delay
  else
    return eLiveMode_Realtime
  end
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

function CreateMasterdUrl()
  local args = string.format("stream_name=%s&format=%s&p=%d&uid=%s",g_stream_info.stream_name,g_stream_info.format,g_sys_info.platform,g_sys_info.device_id)
  local enc_args = EncodeParams(args,g_sys_info.param_key)
  local url = string.format("%s:%d/bee_switch/%s", g_stream_info.masterd_ip, g_stream_info.maseterd_port,enc_args) 
  return url
end

function CreateScheUrl()
  local url = string.format("%s?uid=%s", g_stream_info.sche_url, g_sys_info.device_id)
  if (g_stream_info.encrypt) then
    url = string.format("%s&drmtt=%d",url, g_stream_info.gslb_ts)
  end
  return url
end

function CreateEdgedUrl() 
  local url = string.format("%s?uid=%s&ori=%d&play_sequence=%s&play_time=%s&f=1",g_stream_info.edged_url,g_sys_info.device_id,g_stream_info.original,g_stream_info.play_seq,g_stream_info.play_time)  
  if (g_stream_info.encrypt == 1) then
    local enc_param = string.format("&et=%d&ek=%s", g_stream_info.encrypt_algorithm, g_stream_info.edged_key)
    url = url..enc_param
  end

  LogInfo("edged url:%s", url)
  return url
end

function PrintMasterdRsp(rsp,body)
  LogInfo("HTTP Time:%dms",rsp["http_time"])
  LogInfo("HTTP RspCode:%d",rsp["status_code"])
  for key, value in pairs(rsp["http_header"]) do  
    LogInfo("\t%s : %s", key, value)
  end
  LogInfo("HTTP Body:%s",body)
end

function ProcessMasterdRsp(rsp)
  local dec_body = DecodeParams(rsp["http_body"],g_sys_info.param_key)
  LogInfo("HTTP Body:%s",dec_body)
  PrintMasterdRsp(rsp, dec_body)
  local json_data = cjson.decode(dec_body)
  g_stream_info.sche_url = json_data.sche_url
  g_stream_info.client_ip = json_data.client_ip
  g_stream_info.live_codecs = json_data.live_codecs
  g_stream_info.live_rate = json_data.live_rate
  g_stream_info.mode = json_data.live_mode
  if (json_data.encrypt == "on") then
    g_stream_info.encrypt = true
  else
    g_stream_info.encrypt = false
  end
end

function ProcessSchedRsp(rsp)
  local rsp_body = rsp["http_body"]
  LogInfo("Sche rsp:%s",rsp_body)
  local json_data = cjson.decode(rsp_body)
  g_stream_info.sche_code = json_data.code
  g_stream_info.sche_desc = json_data.desc
  g_stream_info.cdn_id = json_data.info.cdn_id
  g_stream_info.cdn_ip = json_data.info.pop_ip
  g_stream_info.play_seq = json_data.info.start_sequence
  g_stream_info.play_time = json_data.info.play_time
  g_stream_info.edged_url = json_data.info.cdn_url
  g_stream_info.edged_key = json_data.info.drm_key
end

function handle_masterd_resolve(ec1, ec2, msg, hosts)
  LogInfo("Resolve masterd result %d %d %s", ec1, ec2, msg)
  for i = 1, #hosts do
    LogInfo("Resolve masterd host %d:%s", i, hosts[i]);
  end
  return true
end

function handle_masterd_connect(ec1, ec2, msg, host)
  LogInfo("Connect masterd %s result %d %d %s", host, ec1, ec2, msg)
  return true
end

function handle_masterd_request(ec1, ec2, msg, bytes)
  LogInfo("Request masterd with %d bytes result %d %d %s", bytes, ec1, ec2, msg)
  return true
end

function handle_masterd_header(ec1, ec2, msg, rsp)
  LogInfo("Get masterd header result %d %d %s", ec1, ec2, msg)
  LogInfo("Elapsed time:%dms",rsp["http_time"])
  LogInfo("Status Code:%d",rsp["status_code"])
  LogInfo("Response Headers:")
  for key, value in pairs(rsp["http_header"]) do  
    LogInfo("\t%s : %s", key, value)
  end
  return true
end

function handle_masterd_body(ec1, ec2, msg, rsp)
  LogInfo("Get masterd body result %d %d %s", ec1, ec2 ,msg)
  LogInfo("%s", rsp["http_body"])
  return true
end

function request_masterd()
  local url = CreateMasterdUrl()
  local conn = Http:open()
  --conn:set_resolve_callback(handle_masterd_resolve)
  --conn:set_connect_callback(handle_masterd_connect)
  --conn:set_request_callback(handle_masterd_request)
  --conn:set_header_callback(handle_masterd_header)
  LogInfo("request masterd %s", url)
  local ec1,ec2,msg,rsp = conn:get(url, {})
  if (ec1 == 0 and rsp ~= '') then
    ProcessMasterdRsp(rsp)
  end
  conn:close()
  return ec1,ec2,msg
end

function request_sched()
  local url = CreateScheUrl()
  local conn = Http:open()
  LogInfo("request sched %s", url)
  local ec1,ec2,msg,rsp = conn:get(url, {})
  if (ec1 == 0 and rsp ~= '') then
    ProcessSchedRsp(rsp)
  end
  conn:close()
  return ec1,ec2,msg
end

function request_edged()
  local url = CreateEdgedUrl()
  local conn = Http:open()
  local option = {manual = 1}    
  LogInfo("request edged %s", url)
  local ec1,ec2,msg,rsp = conn:get(url, option)
  if (ec1 == 0 and ec2 == 0 and rsp.status_code == 200) then
    g_stream_info.cdn_conn = conn
  else
    conn:close()
  end 
  return ec1, ec2, msg
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

function handle_ws_error(ec, data)
end

function handle_ws_read_data(data)
  LogInfo(data)
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

function get_janus_info()
    local transaction_id = random_string(12)
    local info = string.format("{\"janus\":\"info\",\"transaction\":\"%s\"}",transaction_id)
    return info
end

function on_ice_candidate(candidate)
  LogInfo("ice candidate: %s",candidate)
end

function on_ice_gathering_change(state)
  LogInfo("ice gathering state change: %d",state)
end

local WebrtcCallbacks = {
  on_ice_candidate=on_ice_candidate,
  on_ice_gathering_change=on_ice_gathering_change
}

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

--Video Room Party
local ePartyType_Local = 0
local ePartyType_Remote = 1

local eIceGathering_New = 0
local eIceGathering_Gathering = 1
local eIceGathering_Completed = 2

local eVideoConfNotify_Local_Join = 0
local eVideoConfNotify_Remote_Join = 1
local eVideoConfNotify_Leaved = 2
local eVideoConfNotify_Closed = 3

Party = {
  video_room = nil,
  type = ePartyType_Local,
  rtc_conn = nil,
  session_id = nil,
  handle = nil,
  room_name = nil,
  janus_room_name = nil,
  signal_ready = false,
  media_ready = false,
  p2p_svr="stun:stun.p2p.hd.sohu.com:4478",
  webrtc_callbacks = nil,
  switch_event={},
  stream_name=nil,
  janus_ws = nil,
  janus_keepalive_timer=nil
}

Party.__index = Party

function Party:new(video_room, type, session_id, handle, stream_name, janus_ws)
  local o = {}
  setmetatable(o, self)
  o.video_room = video_room
  o.type = type
  o.session_id = session_id
  o.handle = handle
  o.stream_name = stream_name
  o.janus_ws = janus_ws
  o.room_name = video_room.room_name
  o.janus_room_name = video_room.janus_room_name
  o.callbacks = {
    on_ice_candidate=handler(o, o.on_ice_candidate),
    on_ice_gathering_change=handler(o, o.on_ice_gathering_change),
    on_media_ready=handler(o, o.on_media_ready)
  }
  return o
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
    local body = "{ \"request\": \"configure\", \"audio\" : true, \"video\" : true }"
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

function Party:on_offer_answered(jsep)
  if (jsep ~= nil and jsep ~= "") then
    local sdp = cjson.encode(jsep)
    self.rtc_conn:set_remote_desc(sdp)
  else
    LogError("Remote answer offer empty")     
  end
end

function Party:send_candidate(candidate)
  local transaction = self.video_room:create_transaction(false, nil)    
  local msg = string.format("{\"janus\":\"trickle\",\"session_id\":%d,\"handle_id\":%d,\"transaction\":\"%s\",\"candidate\":%s }", self.session_id, self.handle, transaction.id, candidate)
  self.janus_ws:write(msg)
end

function Party:on_create_offer(local_jsep)
  if (local_jsep ~= nil and local_jsep ~= "") then
    self:send_local_offer(
      local_jsep,
      function(dummy, json_data)
        local remote_jsep = json_data.jsep
        if (remote_jsep ~= nil) then
          self:on_offer_answered(remote_jsep)
        end  
      end)
  else
    LogError("Create local offer fail")
  end
end

function Party:on_create_answer(jsep)
  if (jsep ~= nil and jsep ~= "") then
    self:send_remote_answer(
      jsep,
      function(dummy, json_data)
        if (json_data ~= nil and json_data.plugindata ~= nil and json_data.plugindata.started == "ok") then
          LogInfo("Listen to party %s success", self.stream_name)
        else
          LogError("Listen to party %s fail", self.stream_name)
        end  
      end)
  else
    LogError("Create answer to party %s fail", self.stream_name)
  end
end

function Party:on_webrtcup()
  self.signal_ready = true
  self:check_channel_ready()
end

function Party:on_media_ready()
  self.media_ready = true
  self:check_channel_ready()
end

function Party:on_hangup()
  if (self.janus_ws ~= nil) then
    self.janus_ws:close()
    self.janus_ws = nil    
  end
  if (self.rtc_conn ~= nil) then
    self.rtc_conn:close()
    self.rtc_conn = nil     
  end
  self.webrtc_callbacks = nil
  self.switch_event = nil
end

function Party:check_channel_ready()
  if (self.signal_ready and self.media_ready) then
    local data = nil
    if (self.type == ePartyType_Local) then
      data = string.format("{\"type\":%d, \"ec\":0, \"room_name\":\"%s\", \"stream_name\":\"%s\"}", eVideoConfNotify_Local_Join, self.room_name, self.stream_name)
    else
      data = string.format("{\"type\":%d, \"ec\":0, \"room_name\":\"%s\", \"stream_name\":\"%s\"}", eVideoConfNotify_Remote_Join, self.room_name, self.stream_name)
    end
    sdk_VideoConfNotify(data)
  end
end

function Party:active(cb)
  if (self.type == ePartyType_Local) then
    self:join(cb)
  else
    self:listen(cb)
  end
end

function Party:join(cb)
  local transaction = self.video_room:create_transaction(
    true,
    function(dummy, json_data)
      if (json_data.plugindata.data.videoroom == "joined") then
        self.janus_keepalive_timer = Timer:open(30000, -1, handler(self, self.handle_janus_timeout))
        self:publish_local_feed()
        cb(true)
      else
        LogError("Join error")
        cb(false)
      end
    end)
  local msg = string.format("{\"janus\":\"message\",\"session_id\":%d,\"handle_id\":%d,\"transaction\":\"%s\",\"body\":{\"request\":\"join\",\"room_name\":\"%s\",\"ptype\":\"publisher\",\"stream_name\":\"%s\"}}", self.session_id, self.handle, transaction.id, self.janus_room_name, self.stream_name)
  self.janus_ws:write(msg)
end

function Party:leave()
  local transaction = self.video_room:create_transaction(true, nil)
  local body = string.format("{\"request\":\"leave\",\"room_name\":\"%s\",\"ptype\":\"listener\",\"feed_name\":\"%s\"}", self.janus_room_name, self.stream_name)
  local msg = string.format("{\"janus\":\"message\",\"session_id\":%d,\"handle_id\":%d,\"transaction\":\"%s\",\"body\":%s}", self.session_id, self.handle, transaction.id, body)
  self.janus_ws:write(msg)
  if (self.rtc_conn ~= nil) then
    self.rtc_conn:close()
    self.rtc_conn = nil
  end
end

function Party:listen(cb)
  local transaction = self.video_room:create_transaction(
    true, 
    function(dummy, json_data)
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
      cb(success)
    end)
  local body = string.format("{\"request\":\"join\",\"room_name\":\"%s\",\"ptype\":\"listener\",\"feed_name\":\"%s\"}", self.janus_room_name, self.stream_name)
  local msg = string.format("{\"janus\":\"message\",\"session_id\":%d,\"handle_id\":%d,\"transaction\":\"%s\",\"body\":%s}", self.session_id, self.handle, transaction.id, body)
  self.janus_ws:write(msg)
end

function Party:publish_local_feed()  
  self.rtc_conn = Webrtc:open(self.p2p_svr, self.callbacks)
  if (self.rtc_conn ~= nil) then
    self.rtc_conn:create_offer(handler(self, self.on_create_offer))    
  end
end

function Party:process_leave_event(json_data)
  LogInfo("Party %s left", self.stream_name)
  if (self.video_room ~= nil) then
    self.video_room:on_party_leaved(self.handle, self.stream_name)    
  end
end

function Party:process_send_answer_event(json_data)
  if (json_data ~= nil and json_data.plugindata ~= nil and json_data.plugindata.started == "ok") then
    LogInfo("Listen to party %s success", self.display)
  else
    LogError("Listen to party %s fail", self.display)
  end  
end

function Party:process_event(json_data)

end

function Party:start_video_render(cb, opaque)
  if (self.rtc_conn ~= nil) then
    self.rtc_conn:start_video_render(cb, opaque)
  end
end

function Party:stop_video_render()
  if (self.rtc_conn ~= nil) then
    self.rtc_conn:stop_video_render()
  end
end

function Party:handle_janus_timeout()
  local transaction = self.video_room:create_transaction(false, nil)
  local msg = string.format("{\"janus\":\"keepalive\",\"session_id\":%d, \"transaction\":\"%s\"}", self.session_id, transaction.id)
  self.janus_ws:write(msg)
end

--Video Room
local ws_protocols = {"janus-protocol"}
VideoRoom = {
  --server="wss://10.2.173.67:8989/janus",
  named_svr="https://name.hd.sohu.com/bee_named",
  masterd_svr="https://switch.hd.sohu.com/bee_switch",
  roomed_svr="wss://name.hd.sohu.com/roomed",
  named_push_key="news###nnnneeee",
  stream_name=nil,
  protocol="janus-protocol",
  roomed_ws=nil,
  room_name=nil,
  janus_room_name="h264",
  local_party=nil,
  transaction_table={},
  handle_party_table={},
  stream_party_table={},
  switch_message={},
  roomed_keepalive_timer = nil,
  stream_format="rtc"
}

VideoRoom.__index = VideoRoom

function VideoRoom:new(room_name)
  local o = {}
  setmetatable(o, self)
  Websocket:start(ws_protocols)  
  o.roomed_ws = Websocket:open("", true, handler(o, o.handle_roomed_data), handler(o, o.handle_roomed_error))
  o.room_name = room_name
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
    end
  }
  o:create_video_room()
  return o
end

function VideoRoom:request_named_svr()
  local stream_name, scheduled_svr = nil
  local push_key_encoded = sdk_UrlEncode(self.named_push_key)
  local url = string.format("%s/%s?uid=%d", self.named_svr, push_key_encoded, g_sys_info.device_id)
  local conn = Http:open()
  local ec1,ec2,msg,rsp = conn:get(url, {})
  if (ec1 == 0) then
    local json_data = cjson.decode(rsp["http_body"])
    stream_name = json_data.stream_name
    scheduled_svr = json_data.sche_url
    self.stream_name = stream_name
    ret = true
  end
  conn:close()
  return stream_name, scheduled_svr
end

function VideoRoom:request_scheduled_svr(scheduled_svr)
  local janus_svr = nil
  local conn = Http:open()
  local ec1,ec2,msg,rsp = conn:get(scheduled_svr, {})
  if (ec1 == 0) then
    local json_data = cjson.decode(rsp["http_body"])
    local code = json_data.code
    if (code == 1) then
      janus_svr = json_data.info.webrtc_url
      ret = true
    end
  end
  conn:close()
  return janus_svr
end

function VideoRoom:request_janus_svr(janus_svr, stream_name, type)
  local ret = false
  local janus_ws = Websocket:open(self.protocol, true, handler(self, self.handle_janus_data), handler(self, self.handle_janus_error))
  local res, data = janus_ws:connect(janus_svr)
  if (res) then
    self:create_session(
      janus_ws,
      function(success, json_data)
        if (success == false) then
          LogError("create_session error")
        else
          local session_id = json_data.data.id
          self:create_plugin_handle(
            janus_ws,
            session_id,
            function(success, json_data)
              if (success == false) then
                LogError("create_plugin_handle fail")
              else
                local plugin_handle = json_data.data.id
                self:create_party(
                  type,
                  session_id,
                  plugin_handle,
                  stream_name,
                  janus_ws,
                  function(success)
                    if (success == false) then
                      LogError("create_local_party fail")
                    else
                      LogError("create_local_party success")
                    end
                  end)
              end
            end)
        end
      end)
    ret = true
  end
  return ret
end

function VideoRoom:request_roomed_svr()
  local ret = false
  local res, data = self.roomed_ws:connect(self.roomed_svr)
  if (res) then
    ret = true
    local msg = string.format("{\"request\":\"join\",\"room_name\":\"%s\",\"stream_name\":\"%s\"}", self.room_name, self.stream_name)
    self.roomed_ws:write(msg)    
    self.roomed_keepalive_timer = Timer:open(5000, -1, handler(self, self.handle_roomed_timeout))
  end
  return ret
end

function VideoRoom:create_masterd_url(stream_name)
  local args = string.format("stream_name=%s&format=%s&p=%d&uid=%s", stream_name, self.stream_format, g_sys_info.platform, g_sys_info.device_id)
  local enc_args = EncodeParams(args,g_sys_info.param_key)
  local url = string.format("%s/%s", self.masterd_svr, enc_args) 
  return url
end

function VideoRoom:request_masterd_svr(stream_name)
  local scheduled_svr = nil
  local url = self:create_masterd_url(stream_name)
  local conn = Http:open()
  local ec1,ec2,msg,rsp = conn:get(url, {})
  if (ec1 == 0) then
    local dec_body = DecodeParams(rsp["http_body"], g_sys_info.param_key)
    local json_data = cjson.decode(dec_body)
    scheduled_svr = json_data.sche_url
  end
  conn:close()
  return scheduled_svr
end

function VideoRoom:push()
  while true do
    local stream_name, scheduled_svr = self:request_named_svr()
    if (stream_name == nil or scheduled_svr == nil) then
      LogError("request_named_svr fail")
      break
    end
    ----[[
    local janus_svr = self:request_scheduled_svr(scheduled_svr)
    if (janus_svr == nil) then
      LogError("request_scheduled_svr push fail")
      break
    end
    ret = self:request_janus_svr(janus_svr, stream_name, ePartyType_Local)
    if (ret == false) then
      LogError("request_push_janus_svr fail")
      break
    end
    ----]]
    ret = self:request_roomed_svr()
    if (ret == false) then
      LogError("request_roomed_svr fail")
      break
    end
    
    LogInfo("Push done.")
    break
  end  
end

function VideoRoom:pull(stream_list)  
  for i = 1, #stream_list do
    local stream_name = stream_list[i]
    if (stream_name ~= self.stream_name) then
      local exist_party = self.stream_party_table[stream_name]
      if (exist_party == nil) then
        self:pull_stream(stream_name)
      end
    end
  end
end

function VideoRoom:pull_stream(stream_name)
  while true do
    local scheduled_svr = self:request_masterd_svr(stream_name)
    if (scheduled_svr == nil) then
      LogError("request_masterd fail")
      break
    end
    local janus_svr = self:request_scheduled_svr(scheduled_svr)
    if (janus_svr == nil) then
      LogError("request_scheduled_svr pull fail")
      break
    end
    local ret = self:request_janus_svr(janus_svr, stream_name, ePartyType_Remote)
    if (ret == false) then
      LogError("request_pull_janus_svr fail")
      break
    end
    break
  end
end

function VideoRoom:create_video_room()
  self:push()
end

function VideoRoom:create_party(type, session_id, handle, stream_name, janus_ws, cb)
  local party  = Party:new(self, type, session_id, handle, stream_name, janus_ws)
  self.handle_party_table[handle] = party
  self.stream_party_table[stream_name] = party
  party:active(cb)
end

function VideoRoom:on_party_leaved(handle, stream_name)
  local party = self.handle_party_table[handle]
  if (party ~= nil) then
    self.handle_party_table[handle] = nil
    self.stream_party_table[stream_name] = nil
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
    party:process_event(json_data)
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
  local info = string.format("Received slowlink notify from session %d, handle %d, uplink %s, nack %d.", json_data.session_id, json_data.sender, json_data.uplink, json_data.nack)
  LogInfo("%s", info)
end

function VideoRoom:process_hangup_notify(json_data)
  local info = string.format("Received hangup notify from session %d, handle %d, reason %s.", json_data.session_id, json_data.sender, json_data.reason)
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

function VideoRoom:handle_roomed_data(data)
  LogInfo("Rcv ROOMMSG: %s",data)
  local json_data = cjson.decode(data)
  --if (0 == 1) then
  if (json_data.videoroom == "joined" or json_data.videoroom == "event") then
    local list = json_data.list
    if (list ~= nil) then
      stream_list = list:split(":")
      if (stream_list ~= nil) then
        self:pull(stream_list)
      end
    end
  end
end

function VideoRoom:handle_roomed_error(data)
  LogInfo("Rcv ROOMERR: %s",data)
end

function VideoRoom:start_party_video_render(stream_name, cb, opaque)
  local party = self.stream_party_table[stream_name]
  if (party ~= nil) then
    party:start_video_render(cb, opaque)
  end
end

function VideoRoom:stop_party_video_render(stream_name)
  local party = self.stream_party_table[stream_name]
  if (party ~= nil) then
    party:stop_video_render()
  end
end

function VideoRoom:disconnect_party(stream_name)
  local party = self.stream_party_table[stream_name]
  if (party ~= nil) then
    party:leave()
  end
end

local video_room = nil

function JoinConf(args)
  local json_data = cjson.decode(args)
  local room_name = json_data.room_name
  video_room = VideoRoom:new(room_name)
end

function StartVideoRender(args)
  local json_data = cjson.decode(args)
  local room_name = json_data.room_name
  local stream_name = json_data.stream_name
  local cb = json_data.cb
  local opaque = json_data.opaque
  video_room:start_party_video_render(stream_name, cb, opaque)
end

function StopVideoRender(args)
  local json_data = cjson.decode(args)
  local room_name = json_data.room_name
  local stream_name = json_data.stream_name
  video_room:stop_party_video_render(stream_name)
end

function DisconnectConfParty(args)
  local json_data = cjson.decode(args)
  local room_name = json_data.room_name
  local stream_name = json_data.stream_name
  video_room:disconnect_party(stream_name)
end

function test_https()
  --local url = "https://10.16.14.183:8081"
  local url = "https://name.hd.sohu.com/bee_named/news%23%23%23nnnneeee?uid=test"
  local conn = Http:open()
  LogInfo("request https %s", url)
  local ec1,ec2,msg,rsp = conn:get(url, {})
  if (ec1 == 0 and rsp ~= '') then
    ProcessMasterdRsp(rsp)
  end
  --conn:close()
  return ec1,ec2,msg
end

local unique_cache = false
function StartPlay(args, gslb_key, gslb_ts)
  test_https() 
  --[[
  local json_data = cjson.decode(args)
  g_stream_info.stream_name = json_data.stream_name
  g_stream_info.gslb_key = gslb_key
  g_stream_info.gslb_ts = gslb_ts  
  
  local option = {}
  if (unique_cache) then
    local cache = video_cache.GetCache()    
    sdk_AttachCache(cache)
    local masterd_url = CreateMasterdUrl()
    LogInfo("Requset masterd %s", masterd_url)
    local maseterd_rsp = sdk_HTTPGet(masterd_url, option)
    ProcessMasterdRsp(maseterd_rsp)
    
    --Request Sche
    local sche_url = CreateScheUrl()
    LogInfo("Requset sche %s", sche_url)
    local sche_rsp = sdk_HTTPGet(sche_url, option)
    ProcessSchedRsp(sche_rsp)
    
    --Request Edged
    option["videocache"] = cache
    local edged_url = CreateEdgedUrl()
    sdk_OnPlayed(0,0,'')
    LogInfo("Requset edged %s", edged_url)
    local edged_rsp = sdk_HTTPGet(edged_url, option)
  else
    local ec1 = nil
    local ec2 = nil
    local msg = nil
    while (true) do
      ec1,ec2,msg = request_masterd()
      if (ec1 ~= 0) then
        break
      end
      ec1,ec2,msg = request_sched()
      if (ec1 ~= 0) then
        break
      end
      ec1,ec2,msg = request_edged()
      if (ec1 ~= 0) then
        break
      end
      break
    end
    --sdk_OnPlayed(ec1,ec2,msg) 
  end 
  --]]
end

local LiveHeaderFormat = {
  Field:new(_uint32_t, 1, false), --pkg_len
  Field:new(_uint32_t, 1, false), --pkg_mark
  Field:new(_uint16_t, 1, false), --pkg_code
  Field:new(_uint64_t, 1, false), --sequence
  Field:new(_uint8_t,  1, false), --type
  Field:new(_double,   1, false), --pts
  Field:new(_double,   1, false), --duration
  Field:new(_uint64_t, 1, false), --create_time
  Field:new(_uint32_t, 1, false), --frag_len
}

local LiveHeaderLength = 47
local LiveMark = 844125523

function is_encrypt(type)
  if ((type & 128) > 0) then
    return true   
  else
    return false   
  end
end

function ReadOriginalStream1(buf, len)
  local l = video_cache.SyncReadData(buf, len, eReadSome);
  return l,0
end

function ReadPrivateStream1(buf, len)
  local read_len = 0
  local eof = 0
    
  if (g_stream_info.cur_frag_size == 0) then
    local pkg_len,pkg_mark,pkg_code,seq,type,pts,duration,create_time,frag_len = video_cache.Scanf(LiveHeaderFormat)
    if (pkg_mark ~= LiveMark or (frag_len + LiveHeaderLength) ~= pkg_len) then
      return 0,0 
    end
        
    if (is_encrypt(type)) then
      video_cache.WaitUntil(frag_len)
      g_stream_info.cur_frag_size = crypto.decrypt_cache(g_stream_info.encrypt_algorithm, video_cache.GetCache(), frag_len, g_stream_info.gslb_key)
      g_stream_info.cur_frag_encrypt = true
    else
      g_stream_info.cur_frag_size = frag_len
      g_stream_info.cur_frag_encrypt = false
    end
  end
    
  if (g_stream_info.cur_frag_encrypt) then
    read_len = crypto.read_dec_cache(buf, len)
  else
    local expect_len = math.min(len, g_stream_info.cur_frag_size)
    read_len = video_cache.SyncReadData(buf, expect_len, eReadSome)  
  end
    
  g_stream_info.cur_frag_size = g_stream_info.cur_frag_size - read_len
  return read_len,eof
end

function AsyncRead1(buf, len, seq)
  local read_len = 0
  local eof = 0
  if (g_stream_info.original == 1) then
    read_len,eof = ReadOriginalStream1(buf, len)
  else
    read_len,eof = ReadPrivateStream1(buf, len)
  end
  sdk_OnAsyncRead(read_len, eof)
end

function allocate_dec_buffer(len)
  if (g_stream_info.dec_input == nil) then
    g_stream_info.dec_input = iobuffer.new(len)
  else
    iobuffer.reset(g_stream_info.dec_input, len)
  end
  
  if (g_stream_info.dec_output == nil) then
    g_stream_info.dec_output = iobuffer.new(len)
  else
    iobuffer.reset(g_stream_info.dec_output, len)
  end
end

function ReadOriginalStream2(conn, buf, len)
  local l = conn:read(buf, len, eReadSome)
  return l,0
end

function ReadPrivateStream2(conn, buf, len)
  local read_len = 0
    
  if (g_stream_info.cur_frag_size == 0) then
    local pkg_len,pkg_mark,pkg_code,seq,type,pts,duration,create_time,frag_len = conn:scanf(LiveHeaderFormat)
    if (pkg_mark ~= LiveMark or (frag_len + LiveHeaderLength) ~= pkg_len) then
      return 0,0 
    end
        
    if (is_encrypt(type)) then
      allocate_dec_buffer(frag_len)
      local dec_input_buf,dec_input_len = iobuffer.get(g_stream_info.dec_input)
      local dec_output_buf,dec_output_len = iobuffer.get(g_stream_info.dec_output)
      local http_len = conn:read(dec_input_buf, dec_input_len, eReadExactly)
      g_stream_info.cur_frag_size = crypto.decrypt(g_stream_info.encrypt_algorithm, dec_input_buf, dec_input_len, g_stream_info.gslb_key, dec_output_buf, dec_output_len)
      g_stream_info.cur_frag_encrypt = true
    else
      g_stream_info.cur_frag_size = frag_len
      g_stream_info.cur_frag_encrypt = false
    end
  end
      
  local expect_len = math.min(len, g_stream_info.cur_frag_size)
  if (g_stream_info.cur_frag_encrypt) then
    read_len = iobuffer.read(g_stream_info.dec_output, buf, expect_len)
  else
    read_len = conn:read(buf, expect_len, eReadSome)  
  end    
  g_stream_info.cur_frag_size = g_stream_info.cur_frag_size - read_len
  return read_len
end

function Read(buf, len, seq)
  local read_len = 0
  if (g_stream_info.cdn_conn ~= nil) then
    if (g_stream_info.original == 1) then
      read_len = ReadOriginalStream2(g_stream_info.cdn_conn, buf, len)
    else
      read_len = ReadPrivateStream2(g_stream_info.cdn_conn, buf, len)
    end    
  end
  return read_len
end

function AsyncRead2(buf, len, seq)
  local eof = 1
  local read_len = Read(buf, len, seq)
  sdk_OnAsyncRead(read_len, eof, seq)
end

function AsyncRead3(buf, len, seq)
  local read_buf = buf
  local buff_len = len
  local min_len  = 1024
  local report_end = true
  while (buff_len > 0) do
    local read_len = Read(read_buf, buff_len, seq)
    if (read_len > 0) then
      buff_len = buff_len - read_len    
      read_buf = sdk_ForwardPointer(read_buf, read_len)
      local eof = 0
      if (buff_len < min_len) then
        eof = 1
        report_end  = false
	  end
      local continue = sdk_OnAsyncRead(read_len, eof, seq)
      if (continue == 0 or eof == 1) then
        report_end = false
        break
      end 
	else
      break
    end
  end
  if (report_end) then
    sdk_OnAsyncRead(0, 1, seq)
  end
end

function AsyncRead(buf, len, seq, sustained)
  if (unique_cache) then
    AsyncRead1(buf, len, seq)
  else
    if (sustained == false) then
        AsyncRead2(buf, len, seq)
      else
        AsyncRead3(buf, len, seq)
    end
  end
end

function OnLuaSessionStart()
    math.randomseed(sdk_GetTickGount32())
    math.random(1,100000)
end

OnLuaSessionStart()