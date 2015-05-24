temperature = "test_tmp"
humidity = "test_humidity"
MAX_TRY_LOOP = 10

WIFI_STATUS_RESPONSES = {[2] = 'Wrong password',
    [1] = '<meta http-equiv="refresh" content="8" > Wait a second...(the page will refresh)',
    [3] = 'No access point found',
    [4] = 'Connect failed',
    [5] = 'Connected to wifi'}

function set_wifi()
    local wifi_cred = {
        ssid = 'cool wifi, man',
        pwd = 'coolpassword'
    }
    local ip_cfg = {
      ip = "192.168.1.1",
      netmask = "255.255.255.0",
      gateway = "192.168.1.1"
    }

    wifi.setmode(wifi.STATIONAP)

    wifi.ap.config(wifi_cred)

    wifi.ap.setip(ip_cfg)
end

function connect_to_ap(ssid, pass)
    -- print("connecting...")
    wifi.sta.config(ssid, pass)
    -- print("configured")
    wifi.sta.connect()
    -- print("returning connect")
    return wifi.sta.status()
end

function process_request(client, request)
    client:send("<html>")
    -- print("wifi status: "..wifi.sta.status())
    local status = wifi.sta.status()
    if status == 0 then
        local ssid = string.match(request, "ssid=([^&]+)")
        local pass = string.match(request, "pass=([^&]+)")
        if ssid ~= nil and pass ~= nil then
            -- print("ssid: "..ssid)
            -- print("pass: "..pass)
            local result = connect_to_ap(ssid, pass)
            client:send(WIFI_STATUS_RESPONSES[result])
        else
            client:send("<h3>Hello from ESP8266</h3>")
            client:send("<form action='/' method='post'>");
            client:send("<p>SSID</p><input name='ssid' type='text' />")
            client:send("<p>PASS</p><input name='pass' type='password' /><br>")
            client:send("<input type='submit' />")
            client:send("</form>")
        end
    else
        client:send(WIFI_STATUS_RESPONSES[status])
    end
    if temperature ~= nil then
        client:send("<h1>Temperature: "..temperature.."</h1>")
    end
    if humidity ~= nil then
        client:send("<h1>Humidity: "..humidity.."</h1>")
    end

    client:send("</html>")
end

function create_server()
    -- print("Creating server...")
    srv = net.createServer(net.TCP)
    srv:listen(80, function(conn)
        conn:on('receive', function(client, request)
            -- print(request)
            process_request(client, request)
            client:close()
        end)
    end)
end

function close_server()
    if srv then
        srv:close()
    end
end

function send_data()
    local ipaddr = '184.106.153.149'
    local body = "/update?key=CUTRB50CQ2REU78M&field1="..temperature.."&field2="..humidity
    conn=net.createConnection(net.TCP, true)
    conn:on("receive", function(conn, payload)
        print(payload)
    end)
    conn:on("disconnection", function(conn,payload)
        print("disconnect")
        conn:close()
    end)

    conn:on("connection", function(conn, payload)
        print("sending...")
        conn:send("GET "..body)
        conn:send("HEAD / HTTP/1.0\r\n")
        conn:send("Accept: */*\r\n")
        conn:send("User-Agent: Mozilla/4.0 (compatible; ESP8266;)\r\n")
        conn:send("\r\n")

    end)

    conn:connect(80, ipaddr)
end

close_server()
set_wifi()
create_server()
wifi.sta.disconnect()
tmr.alarm(0, 2500, 1, function() print('ASK') end )
tmr.alarm(1, 10000, 1, send_data)
