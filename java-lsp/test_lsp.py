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

    # Read response headers
    headers = {}
    while True:
        line = proc.stdout.readline().decode('utf-8')
        print(f"Header line: {repr(line)}", file=sys.stderr)
        if line == '\r\n' or line == '\n' or line == '':
            break
        if ':' in line:
            key, value = line.split(':', 1)
            headers[key.strip()] = value.strip()

    # Read body
    if 'Content-Length' in headers:
        body_len = int(headers['Content-Length'])
        body = proc.stdout.read(body_len).decode('utf-8')
        print(f"Body: {body[:200]}...", file=sys.stderr)
        return json.loads(body)
    return None

# Start java-lsp
env = os.environ.copy()
env['DYLD_LIBRARY_PATH'] = '/tmp/tree-sitter'
proc = subprocess.Popen(
    ['./dist/java-lsp'],
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    cwd='/Users/a/Space/Projects/HumanHorizon/lsp-suite/java-lsp',
    env=env
)

# Initialize
resp = send_request(proc, "initialize", {}, id=1)
print("Initialize response:", resp)

# Send documentSymbol request
params = {
    "textDocument": {
        "uri": "file:///tmp/test.java"
    }
}
resp = send_request(proc, "textDocument/documentSymbol", params, id=2)
print("DocumentSymbol response:", json.dumps(resp, indent=2))

# Shutdown
send_request(proc, "shutdown", {}, id=3)

proc.stdin.close()
proc.wait()