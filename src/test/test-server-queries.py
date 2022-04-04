from pwn import *
from string import ascii_letters, digits
from time import sleep

ALPH = ascii_letters + digits + " +,.!?-=+/*()"

TOKEN_SIZE = 24

DATA_DIR = ".data"
FILENAME_LETHREAD = "lethreadinfo"
FILENAME_LEMESSAGES = "lemessages"
FILENAME_LEAUTHOR = "leauthor"


def test_lethread_create():
	topic_size = 30

	# randomly generate topic
	topic = cyclic(topic_size, alphabet=ALPH)

	# build a CREATE THREAD query
	data = b"CTHR" + b"TPCSZ" + p64(topic_size) + b"TPC" + topic.encode("ascii")

	io = remote("127.0.0.1", 7431)
	# hello from server
	io.recv(1024)
	io.send(p64(len(data)))

	# prevent packets consolidation
	sleep(0.5)

	io.send(data)
	result = io.recv(1024)
	io.close()

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


def main():
	test_lethread_create()


if __name__ == "__main__":
	main()
