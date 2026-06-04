#!/usr/bin/env python3
import subprocess
import json
import sys
import os

def send_request(proc, method, params, id=1):
    request = {
        "jsonrpc": "2.0",
        "id": id,
        "method": method,
        "params": params
    }
    req_str = json.dumps(request)
    content_length = len(req_str)
    message = f"Content-Length: {content_length}\r\n\r\n{req_str}"
    print(f"Sending: {message[:100]}...", file=sys.stderr)
    proc.stdin.write(message.encode())
    proc.stdin.flush()

    headers = {}
    while True:
        line = proc.stdout.readline().decode('utf-8')
        print(f"Header line: {repr(line)}", file=sys.stderr)
        if line == '\r\n' or line == '\n' or line == '':
            break
        if ':' in line:
            key, value = line.split(':', 1)
            headers[key.strip()] = value.strip()

    if 'Content-Length' in headers:
        body_len = int(headers['Content-Length'])
        body = proc.stdout.read(body_len).decode('utf-8')
        print(f"Body: {body[:200]}...", file=sys.stderr)
        return json.loads(body)
    return None

env = os.environ.copy()
env['DYLD_LIBRARY_PATH'] = '/tmp/tree-sitter'
proc = subprocess.Popen(
    ['./dist/css-lsp'],
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    cwd='/Users/a/Space/Projects/HumanHorizon/lsp-suite/css-lsp',
    env=env
)

resp = send_request(proc, "initialize", {}, id=1)
print("Initialize response:", resp)

params = {
    "textDocument": {
        "uri": "file:///tmp/test.css"
    }
}
resp = send_request(proc, "textDocument/documentSymbol", params, id=2)
print("DocumentSymbol response:", json.dumps(resp, indent=2))

send_request(proc, "shutdown", {}, id=3)

proc.stdin.close()
proc.wait()