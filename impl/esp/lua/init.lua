uart.setup(0, 19200, 8, 0, 1, 0)
temperature = 1000
humidity = 1000

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

    conn:on("connection", function(conn,payload)
        print("sending...")
        conn:send("GET "..body)
        conn:send("HEAD / HTTP/1.0\r\n")
        conn:send("Accept: */*\r\n")
        conn:send("User-Agent: Mozilla/4.0 (compatible; ESP8266;)\r\n")
        conn:send("\r\n")

    end)

    conn:connect(80, ipaddr)

end