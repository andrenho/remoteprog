#!/usr/bin/env python3

from http.server import BaseHTTPRequestHandler, HTTPServer
import base64
import json
import tempfile
import subprocess

PORT = 8376

def upload_firmware(cpu, binary):  # returns stdout, stderr
    with tempfile.NamedTemporaryFile(mode="wb", delete=True) as f:
        f.write(binary)
        cmd = ""
        if cpu == "rp2040":
            cmd = ['openocd', '-f', 'interface/raspberrypi-swd.cfg', '-f', 'target/rp2040.cfg', '-c', f'program {f.name} verify reset exit']
        elif cpu == "rp2350":
            cmd = ['openocd', '-f', 'interface/raspberrypi-swd.cfg', '-f', 'target/rp2350.cfg', '-c', 'adapter speed 5000', '-c', 'rp2350.dap.core1 cortex_m reset_config sysresetreq', '-c', f'program {f.name} verify; reset; exit']
        else:
            return False, "", f"CPU {cpu} unsupported."
        try:
            r = subprocess.run(cmd, capture_output=True, text=True, check=True)
            print(r.stderr)
            return True, r.stdout, r.stderr
        except subprocess.CalledProcessError as e:
            print(e.stderr)
            return False, e.stdout, e.stderr

class FileUploadHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        # get user submitted info
        content_length = int(self.headers['Content-Length'])
        recv = self.rfile.read(content_length)
        data = json.loads(recv)
        # upload firmware
        status, stdout, stderr = upload_firmware(data['cpu'], base64.b64decode(data['firmware']))
        # send response
        self.send_response(200 if status else 500)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(stderr).encode('utf-8'))
        print('-------------------------------------------------------------------------------------')

if __name__ == '__main__':
    server_address = ('', PORT)
    httpd = HTTPServer(server_address, FileUploadHandler)
    print(f'Starting server on port {PORT}')
    httpd.serve_forever()
