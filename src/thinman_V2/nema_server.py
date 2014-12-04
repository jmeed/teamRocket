import tornado.httpserver
import tornado.websocket
import tornado.ioloop
import tornado.web
import serial
import json
import logging
import time
import threading
import random
logging.basicConfig()
log = logging.getLogger(__name__)
logging.getLogger().setLevel(logging.INFO)

# ser = serial.Serial('COM7', 9600)
last_time = 0

active_ws_connections = set()


class WSHandler(tornado.websocket.WebSocketHandler):
    def check_origin(self, origin):
        return True

    def open(self):
        print 'new connection'
        active_ws_connections.add(self)

    def on_message(self, message):
        pass

    def on_close(self):
        if self in active_ws_connections:
            active_ws_connections.remove(self)
        print 'connection closed'


application = tornado.web.Application([
    (r'/ws', WSHandler),
])


def broadcast_message(msg):
    print msg
    for conn in active_ws_connections:
        try:
            conn.write_message(msg)
        except Exception:
            log.exception("failed to send message, removing offending ws connection")
            active_ws_connections.remove(conn)

def forward_serial():
    while True:
        line = "$GPGGA,123519,4217.%03d,N,08342.%03d,W,1,08,0.9,545.4,M,46.9,M,,*47\n" % (random.randrange(0,999), random.randrange(0,999))
        time.sleep(1.0)
        if not line:
            break

        tornado.ioloop.IOLoop.instance().add_callback(broadcast_message, line)

threading.Thread(target=forward_serial).start()

if __name__ == "__main__":
    http_server = tornado.httpserver.HTTPServer(application)
    http_server.listen(8888)
    tornado.ioloop.IOLoop.instance().start()
