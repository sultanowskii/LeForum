from pwn import *
from string import ascii_letters, digits
from time import sleep

ALPH = ascii_letters + digits + " +,.!?-=+/*()"

TOKEN_SIZE = 24

DATA_DIR = ".data"
FILENAME_LETHREAD = "lethreadinfo"
FILENAME_LEMESSAGES = "lemessages"
FILENAME_LEAUTHOR = "leauthor"


def query(payload):
	io = remote("127.0.0.1", 7431)

	# hello from server
	io.recv(1024)
	io.send(p64(len(payload)))

	# prevent packets consolidation
	sleep(0.5)

	io.send(payload)
	result_length = io.recv(8)
	result = io.recv(1024)
	io.close()

	return result, result_length


def test_lethread_basic():
	topic_size = 30

	# randomly generate topic
	topic = cyclic(topic_size, alphabet=ALPH)

	# build a CREATE THREAD query
	data = b"CTHR" + b"TPCSZ" + p64(topic_size) + b"TPC" + topic.encode("ascii")

	result, result_length = query(data)

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

	result, result_length = query(data)

	assert result[0:5] == b"THRID"
	assert u64(result[5:13]) == lethread_id

	assert result[13:18] == b"TPCSZ"
	assert u64(result[18:26]) == len(topic)

	assert result[26:29] == b"TPC"
	assert result[29:29+len(topic)].decode("ascii") == topic


def main():
	with context.quiet:
		print("[.] Starting LeThread basic tests (CTHR, GTHR)...")
		for i in range(5):
			test_lethread_basic()
		print("[*] LeThread basic tests (CTHR, GTHR) passed successfully!")

if __name__ == "__main__":
	main()
