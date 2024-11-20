import requests
from os.path import dirname, realpath

requests.packages.urllib3.disable_warnings()

test_dir = dirname(realpath(__file__))

# video
headers = { 'Range': 'bytes=100-10000' }
r = requests.get('http://10.0.0.1/long.mp4', verify=False, headers=headers)
print(r.status_code)
assert(r.status_code == 206 and open(test_dir + '/../long.mp4', 'rb').read()[100:10001] == r.content)
# video
r = requests.get('http://10.0.0.1/long.mp4', verify=False)
print(r.status_code)
assert(r.status_code == 200 and open(test_dir + '/../long.mp4', 'rb').read() == r.content)