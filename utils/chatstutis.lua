-- Wireshark dissector for the Chatstutis protocol
-- UDP default port 45678 : peer discovery
-- TCP default port 56789 : chat packets
-- Install: copy into your Wireshark plugins directory, then Analyze -> Reload Lua Plugins
-- Linux/MacOS : ~/.local/lib/wireshark/plugins

local chatstutis = Proto("Chatstutis", "Chatstutis Protocol")

-- shared fields
local version = ProtoField.uint8("chatstutis.version", "Version", base.DEC)
local msg_type = ProtoField.uint8("chatstutis.message_type", "Message Type", base.DEC)

-- only in discovery packets
local listening_port = ProtoField.uint16("chatstutis.listening_port", "Listening Port", base.DEC)
local username = ProtoField.string("chatstutis.username", "Username")

local message = ProtoField.string("chatstutis.message", "Message") -- only in chat packets

local DISCOVERY_TYPES = { [0] = "Discover", [1] = "Disconnect" }
local CHAT_TYPES = { [0] = "Chat", [1] = "Close Connection" }

local DISCOVERY_LEN = 34
local CHAT_LEN = 1058

chatstutis.fields = { version, msg_type, username, message }


local function parse_common_fields(buffer, subtree)
    subtree:add(version, buffer(0, 1))
    local msg_type_item = subtree:add(msg_type, buffer(1, 1))
    local msg_type_val = buffer(1, 1):uint()
    return msg_type_item, msg_type_val
end

function chatstutis.dissector(buffer, pinfo, tree)
    local length = buffer:len()
    if length == 0 then return end

    pinfo.cols.protocol = "Chatstutis"

    -- determine packet type
    local is_udp = (length == DISCOVERY_LEN)
    local is_tcp = (length == CHAT_LEN)

    if is_udp then
        pinfo.cols.info:set("Peer Discovery")
        local subtree = tree:add(chatstutis, buffer(), "Chatstutis Peer Discovery Packet")
        local type_item, type_val = parse_common_fields(buffer, subtree)
        subtree:add(username, buffer(2, 32))

        local type_name = DISCOVERY_TYPES[type_val] or
            string.format("Unknown (0x%02x)", type_val)
        type_item:append_text(" (" .. type_name .. ")")
        pinfo.cols.info:set("Peer Discovery [" .. type_name .. "]")
    elseif is_tcp then
        pinfo.cols.info:set("Chat")
        local subtree = tree:add(chatstutis, buffer(), "Chatstutis Chat Packet");
        local type_item, type_val = parse_common_fields(buffer, subtree);
        subtree:add(message, buffer(2, 1024));

        local type_name = DISCOVERY_TYPES[type_val] or
            string.format("Unknown (0x%02x)", type_val)
        type_item:append_text(" (" .. type_name .. ")")
        pinfo.cols.info:set("Chat Packet [" .. type_name .. "]")
    else
        -- unknown packet
        local subtree = tree:add(chatstutis, buffer(), "Chatstutis Packet (unrecognized length: " .. length .. ")")
        pinfo.cols.info:set("Chatstutis (unknown)")
    end
end

local udp_table = DissectorTable.get("udp.port")
udp_table:add(45678, chatstutis)

local tcp_table = DissectorTable.get("tcp.port")
tcp_table:add(56789, chatstutis)