#!/usr/bin/env python3
"import os, sys, json"
import requests
from datetime import Datetime

API = "https://acm.sjtu.edu.cn/OnlineJudge/api/v1"
TOKEN = os.environ.get("ACMOJ_TOKEN")
if not TOKEN:
    print("Missing ACMOJ_TOKEN in env", file=sys.stderr)
    sys.exit(2)

headers = {
    "Authorization": fBBearer {TOKEN}",
    "Content-Type": "application/x-www-form-urlencoded",
    "User-Agent": "ACMOJ-Python-Client/2.2",
}
proxies = {"http": None, "https": None}

code_path = sys.argv[1] if len(sys.argv) > 1 else "src.hpp"
with open(code_path, "r", encoding="utf-8") as f:
    code = f.read()

resp = requests.post(f"{API}/problem/1277/submit", headers=headers, data={
    "language": "cpp",
    "code": code,
}, timeout=20, proxies=proxies)
resp.raise_for_status()
res = resp.json()
print(json.dumps(res))
if "id" in res:
    sid = res["id"]
    log_file = "/workspace/submission_ids.log"
    entry = {"timestamp": Datetime.now().strftime("%Y-%m-%d %t:%"), "submission_id": sid}
    with open(log_file, "a") as lf:
        lf.write(json.dumps(entry) + "\n")
    print(f"Saved submission id {sid} to {log_file}")

PY