from pwn import *
from string import ascii_letters, digits
from time import sleep

ALPH = ascii_letters + digits + " +,.!?-=+/*()"

TOKEN_SIZE = 24

DATA_DIR = ".data"
FILENAME_LETHREAD = "lethreadinfo"
FILENAME_LEMESSAGES = "lemessages"
FILENAME_LEAUTHOR = "leauthor"

lethread_topic = []


def query(payload):
	io = remote("127.0.0.1", 7431)

	# hello from server
	io.recv(1024)
	io.send(p64(len(payload)))

	# prevent packets consolidation
	sleep(0.5)

	io.send(payload)

	result = b""
	result_size = u64(io.recv(8))

	# well, that is the solution to all socket chunk shitty problems.
	while len(result) < result_size:
		result += io.recv(result_size, 3)

	io.close()

	return result, result_size


def test_lethread_basic(topic):
	global lethread_topic

	topic_size = len(topic)

	lethread_topic.append(topic)

	# build a CREATE THREAD query
	data = b"CTHR" + b"TPCSZ" + p64(topic_size) + b"TPC" + topic.encode("ascii")

	result, result_size = query(data)

	assert result[0:2] == b"OK"

	assert result[2:7] == b"THRID"
	lethread_id = u64(result[7:15])

	assert result[15:18] == b"TKN"
	token = result[18:18+TOKEN_SIZE]
	assert len(token) == TOKEN_SIZE

	# giving server some time to save files
	sleep(0.5)

	with open(f"{DATA_DIR}/{str(lethread_id)}/{FILENAME_LETHREAD}", "rb") as f:
		data = f.read()

		assert lethread_id == u64(data[0:8])
		assert topic_size == u64(data[32:40])

		assert topic == data[40:].decode("ascii")

	# build a GET THREAD query
	data = b"GTHR" + b"THRID" + p64(lethread_id)

	result, result_size = query(data)

	assert result[0:5] == b"THRID"
	assert u64(result[5:13]) == lethread_id

	assert result[13:18] == b"TPCSZ"
	assert u64(result[18:26]) == len(topic)

	assert result[26:29] == b"TPC"
	assert result[29:29+len(topic)].decode("ascii") == topic


def test_lethread_find(topic_part):
	topic_part_size = len(topic_part)

	data = b"FTHRTPCPSZ" + p64(topic_part_size) + b"TPCP" + topic_part.encode("ascii")

	result, result_size = query(data)

	x = 0

	print(f"Find ('{topic_part}'):")

	if result[x:x+4] == b"NFND":
		print("    Not found")
		return None

	if result[x:x+4] == b"IDAT":
		print("    Error: invalid request")
		return False

	while x + 1 < result_size:
		assert result[x:x+5] == b"THRID"
		x += 5
		lethread_id = u64(result[x:x+8])
		x += 8

		assert result[x:x+5] == b"TPCSZ"
		x += 5
		topic_size = u64(result[x:x+8])
		x += 8

		assert result[x:x+3] == b"TPC"
		x += 3
		topic = result[x:x+topic_size].decode("ascii")
		x += topic_size
		print(f"    <thread id={lethread_id} {topic=}>")


def test_meta():
	data = b"META"

	result, result_size = query(data)
	
	x = 0

	assert result[x:x+8] == b"MINTPCSZ"
	x += 8

	min_topic_size = u64(result[x:x+8])
	x += 8

	assert result[x:x+8] == b"MAXTPCSZ"
	x += 8

	max_topic_size = u64(result[x:x+8])
	x += 8

	assert result[x:x+8] == b"MINMSGSZ"
	x += 8

	min_message_size = u64(result[x:x+8])
	x += 8

	assert result[x:x+8] == b"MAXMSGSZ"
	x += 8

	max_message_size = u64(result[x:x+8])
	x += 8

	assert result[x:x+4] == b"THRN"
	x += 4

	threads_number = u64(result[x:x+8])
	x += 8

	assert result[x:x+5] == b"VERSZ"
	x += 5

	version_size = u64(result[x:x+8])
	x += 8

	assert result[x:x+3] == b"VER"
	x += 3

	version = result[x:x+version_size].decode("ascii")

	print(f"{version} {threads_number=} ({min_topic_size}<=topic_size<={max_topic_size}) ({min_message_size}<=message_size<={max_message_size})")


def main():
	with context.quiet:
		if args.NOBASIC == "":
			print("[.] Starting LeThread basic tests (CTHR, GTHR)...")
			for i in range(5):
				test_lethread_basic(cyclic(24, alphabet=ALPH))
			test_lethread_basic("veri cool topic!!!")
			print("[*] LeThread basic tests (CTHR, GTHR) passed successfully!")

		if args.NOFIND == "":
			print("[.] Starting LeThread FTHR test...")
			test_lethread_find("cool")
			assert test_lethread_find("a") == False
			test_lethread_find("aaa")
			print("[*] LeThread FTHR test passed successfully!")

		if args.FREEFIND != "":
			print("[.] LeThread find free mode. Ctrl+C to exit")
			while True:
				test_lethread_find(input()[:-1])
		
		if args.NOMETA == "":
			print("[.] Starting Meta Query test...")
			test_meta()
			print("[*] Meta Query passed successfully!")



if __name__ == "__main__":
	main()
