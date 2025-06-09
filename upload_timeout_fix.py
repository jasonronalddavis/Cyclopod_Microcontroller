Import("env")
import time

def before_upload(source, target, env):
    print(">> Waiting 3 seconds before upload to help USB stabilize...")
    time.sleep(3)

env.AddPreAction("upload", before_upload)
env.Append(UPLOAD_FLAGS=["--before", "default_reset", "--after", "hard_reset"])
