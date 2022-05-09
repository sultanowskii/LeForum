from pwn import *
from string import ascii_letters, digits
from time import sleep
from rich.progress import track
from random import randint

ALPH = ascii_letters + digits + " +,.!?-=+/*()"

TOKEN_SIZE = 24

DIR_SERVER = ".leforum_server"
FILENAME_LETHREAD = "lethreadinfo"
FILENAME_LEMESSAGES = "lemessages"
FILENAME_LEAUTHOR = "leauthor"


def query(payload):
	io = remote("127.0.0.1", 12345)

	io.send(p64(len(payload)))
	io.send(payload)

	result = b""
	result_size = u64(io.recv(8))

	# well, that is the solution to all socket chunk shitty problems.
	while len(result) < result_size:
		result += io.recv(result_size, 3)

	io.close()

	return result, result_size


def test_lethread_create(topic):
	topic_size = len(topic)

	# build a CREATE THREAD query
	data = b"CTHR" + b"TPCSZ" + p64(topic_size) + b"TPC" + topic.encode("ascii")

	result, result_size = query(data)

	x = 0

	assert result[x:x+5] == b"THRID"
	x += 5
	lethread_id = u64(result[x:x+8])
	x += 8

	assert result[x:x+3] == b"TKN"
	x += 3
	token = result[x:x+TOKEN_SIZE]
	x += TOKEN_SIZE
	assert len(token) == TOKEN_SIZE

	return lethread_id


def test_lethread_get(lethread_id):
	# build a GET THREAD query
	data = b"GTHR" + b"THRID" + p64(lethread_id)

	result, result_size = query(data)

	x = 0

	if result[x:x+4] == b"NFND":
		print("Not found")
		return None, None, []

	assert result[x:x+5] == b"THRID"
	x += 5
	assert u64(result[x:x+8]) == lethread_id
	x += 8

	assert result[x:x+5] == b"TPCSZ"
	x += 5
	topic_size = u64(result[x:x+8])
	x += 8

	assert result[x:x+3] == b"TPC"
	x += 3

	topic = result[29:29+topic_size].decode("ascii")
	x += topic_size

	print(f"Lethread id={lethread_id} topic='{topic}'")

	if x + 1 < result_size:
		print(result)
		assert result[x:x+6] == b"MSGCNT"
		x += 6

		msgcnt = u64(result[x:x+8])
		x += 8
	
		messages = []

		print(f"Message history:")
		for i in range(msgcnt):
			assert result[x:x+3] == b"MSG"
			x += 3
	
			by_author = bool(u8(result[x:x+1]))
			x += 1

			msgid = u64(result[x:x+8])
			x += 8

			msgtxtsz = u64(result[x:x+8])
			x += 8

			msgtxt = result[x:x+msgtxtsz].decode("ascii")
			x += msgtxtsz

			assert result[x:x+6] == b"MSGEND"
			x += 6

			print(f"    id={msgid} by_author={by_author} text: {msgtxt}")
		return topic, topic_size, messages
	

	return topic, topic_size, []


def test_lethread_basic(topic):
	topic_size = len(topic)
	lethread_id = test_lethread_create(topic)

	# giving server some time to save files
	sleep(0.5)

	with open(f"{DIR_SERVER}/{str(lethread_id)}/{FILENAME_LETHREAD}", "rb") as f:
		data = f.read()

		assert lethread_id == u64(data[0:8])
		assert topic_size == u64(data[32:40])

		assert topic == data[40:].decode("ascii")

	got_topic, got_topic_size, messages = test_lethread_get(lethread_id)

	assert got_topic_size == topic_size
	assert got_topic == topic


def test_lethread_find(topic_part):
	topic_part_size = len(topic_part)

	data = b"FTHRTPCPSZ" + p64(topic_part_size) + b"TPCP" + topic_part.encode("ascii")

	result, result_size = query(data)

	x = 0

	if result[x:x+4] == b"NFND":
		print("Not found")
		return None

	if result[x:x+4] == b"IDAT":
		print("Error: invalid request")
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

		print(f"LeThread id={lethread_id} {topic=}>")


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


def test_lethread_message(lethread_id, text):
	data = b"CMSGTHRID" + p64(lethread_id) + b"TXTSZ" + p64(len(text)) + b"TXT" + text.encode("ascii")

	result, result_size = query(data)

	if result[0:4] == b"NFND":
		print("Not found")
		return


	if result[0:4] == b"IDAT":
		print("Invalid message size")
		return

	assert result[0:2] == b"OK"


def test_mega_msg():
	lethread_id = test_lethread_create("LIMITS PUSHING")
	for i in track(range(10000), description="Posting messages..."):
		test_lethread_message(lethread_id, cyclic(randint(10, 500), alphabet=ALPH))

def main():
	with context.quiet:
		if args.AUTO != "":
			print("[.] Starting LeThread basic tests (CTHR, GTHR)...")
			for i in range(5):
				test_lethread_basic(cyclic(24, alphabet=ALPH))
			test_lethread_basic("veri cool topic!!!")
			print("[*] LeThread basic tests (CTHR, GTHR) passed successfully!")

			print("[.] Starting LeThread FTHR test...")
			test_lethread_find("cool")
			assert test_lethread_find("a") == False
			test_lethread_find("aaa")
			print("[*] LeThread FTHR test passed successfully!")

			print("[.] Starting Meta Query test...")
			test_meta()
			print("[*] Meta Query passed successfully!")

		if args.MMSG != "":
			print("[.] Starting Mega Message test...")
			test_mega_msg()
			print("[*] Finished!")

		if args.INTERACTIVE != "":
			print("[.] Interactive mode. Ctrl+C to exit")
			while True:
				cmd, *arg = input()[:-1].split()
				if cmd == "CTHR":
					test_lethread_create("".join(arg))
				if cmd == "GTHR":
					topic, topic_size, messages = test_lethread_get(int(arg[0]))
				if cmd == "FTHR":
					test_lethread_find("".join(arg))
				if cmd == "CMSG":
					test_lethread_message(int(arg[0]), arg[1])
				if cmd == "META":
					test_meta()


if __name__ == "__main__":
	main()
