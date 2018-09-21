from subprocess import call

call(["mkdir", "../tmp"])
call(["ln", "-s", "../src", "../tmp/AstroMech"])
result = call(["pio", "run", "-t", "upload", "-e", "feather32"])

if result is 0:
    call(["pio", "device", "monitor", "-b", "115200"])
